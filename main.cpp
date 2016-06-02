#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <cctype>
#include <iostream>
#include "getP.h"
#include "trans.h"
#include "route.h"
#include "Control.h"


int select_object = 0;
CvRect selection;
IplImage* image = 0;
int track_object = 0;


int tmp_tracking();
void selectionCall(int mouseEvent, int x, int y, int flags, void* param);
CvScalar hsv2rgb(float hue);



int main() {
	IplImage *Map = getMap();
	int nPoint = 0;
	CvPoint *route = GetRoute(Map, &nPoint);

	SetControlMap(Map, route, nPoint);

	tmp_tracking();

}


int tmp_tracking() {

	CvCapture* capture;
	capture = cvCreateCameraCapture(-1);

	if (!capture) { 
		std::cout << "Open failed." << std::endl; 
		return -1;
	}



	std::cout << "Hot keys: \n"
		"\tESC - quit the program\n"
		"\tc - stop the tracking\n"
		"\tb - switch to/from backprojection view\n"
		"\th - show/hide object histogram\n"
		"To initialize tracking, select the object with mouse\n" << std::endl;


	cvNamedWindow("FirstHistogram", 1);
	cvNamedWindow("SecondHistogram", 1);
	cvNamedWindow("CamShiftDemo", 1);


	int vmin = 10;				//Value in HSV Channel 
	int vmax = 256;			//Value in HSV Channel
	int smin = 30;				//Saturation in HSV Channel

	cvSetMouseCallback("CamShiftDemo", selectionCall, 0);

	cvCreateTrackbar("Vmin", "CamShiftDemo", &vmin, 256, 0);
	cvCreateTrackbar("Vmax", "CamShiftDemo", &vmax, 256, 0);
	cvCreateTrackbar("Smin", "CamShiftDemo", &smin, 256, 0);

	//Initialize some values
	IplImage  *first_hsv = 0, *second_hsv = 0;			//Transform the origin image into HSV image
	IplImage *first_hue = 0, *second_hue = 0;				//split HUE element from HSV area
	IplImage *first_mask = 0, *second_mask = 0;			//Mask for ?????????????
	IplImage *first_backproject = 0, *second_backproject = 0;//For back Project??  ????????
	IplImage *first_histimg = 0, *second_histimg = 0;			//create a hist img   
	CvHistogram *first_hist = 0, *second_hist = 0;			//create a hist struct
	int first_backproject_mode = 0;//what's this?????????      ????????
	int second_backproject_mode = 0;
	int show_hist = 1;//set show_hist to 1, which measns true

	CvRect track_window1;
	CvRect track_window2;
	//Meanshift跟踪算法返回的Box类  
	//typedef struct CvBox2D{  
	//CvPoint2D32f center; /* 盒子的中心 */  
	//CvSize2D32f size; /* 盒子的长和宽 */  
	//float angle; /* 水平轴与第一个边的夹角，用弧度表示*/  
	//}CvBox2D;  

	CvBox2D track_box1;
	CvBox2D track_box2;
	//typedef struct CvConnectedComp{  
	//double area; /* 连通域的面积 */  
	//float value; /* 分割域的灰度缩放值 */  
	//CvRect rect; /* 分割域的 ROI */  
	//} CvConnectedComp; 
	CvConnectedComp track_comp1;  // information of tracking area
	CvConnectedComp track_comp2;

	//划分直方图bins的个数，越多越精确
	int hdims = 16;

	//像素值的范围  
	float hranges_arr[] = { 0,180 };
	float* hranges = hranges_arr;


	while (true)
	{
		IplImage* frame = cvQueryFrame(capture);
		frame = perspectiveTransform3(frame);
		if (!frame) return -1; 
		if (!image)
			//image为0,表明刚开始还未对image操作过,先建立一些缓冲区  
		{
			image = cvCreateImage(cvGetSize(frame), 8, 3);
			image->origin = frame->origin;
			first_hsv = cvCreateImage(cvGetSize(frame), 8, 3);
			second_hsv = cvCreateImage(cvGetSize(frame), 8, 3);
			first_hue = cvCreateImage(cvGetSize(frame), 8, 1);
			second_hue = cvCreateImage(cvGetSize(frame), 8, 1);
			first_mask = cvCreateImage(cvGetSize(frame), 8, 1);
			second_mask = cvCreateImage(cvGetSize(frame), 8, 1);
			//分配掩膜图像空间  
			first_backproject = cvCreateImage(cvGetSize(frame), 8, 1);
			second_backproject = cvCreateImage(cvGetSize(frame), 8, 1);
			//分配反向投影图空间,大小一样,单通道  
			first_hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
			second_hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
			//分配直方图空间  
			first_histimg = cvCreateImage(cvSize(320, 200), 8, 3);
			second_histimg = cvCreateImage(cvSize(320, 200), 8, 3);
			//分配用于直方图显示的空间  
			cvZero(first_histimg);
			cvZero(second_histimg);
			//置背景为黑色  
		}

		cvCopy(frame, image, 0);
		cvCvtColor(image, first_hsv, CV_BGR2HSV);
		cvCvtColor(image, second_hsv, CV_BGR2HSV);
		//把图像从RGB表色系转为HSV表色系  



		if (track_object == -1 || track_object == 1)
			//track_object非零,表示有需要跟踪的物体  
		{
			int _vmin = vmin, _vmax = vmax;
			cvInRangeS(first_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
				cvScalar(180, 256, MAX(_vmin, _vmax), 0), first_mask);
			//制作掩膜板，只处理像素值为H：0~180，S：smin~256，V：vmin~vmax之间的部分  

			cvSplit(first_hsv, first_hue, 0, 0, 0);

			//分离H分量  

			if (track_object == -1)
				//如果需要跟踪的物体还没有进行属性提取，则进行选取框类的图像属性提取  
			{
				float max_val = 0.f;
				cvSetImageROI(first_hue, selection);
				//设置原选择框为ROI  
				cvSetImageROI(first_mask, selection);
				//设置掩膜板选择框为ROI  
				cvCalcHist(&first_hue, first_hist, 0, first_mask);
				//得到选择框内且满足掩膜板内的直方图  
				cvGetMinMaxHistValue(first_hist, 0, &max_val, 0, 0);
				cvConvertScale(first_hist->bins, first_hist->bins, max_val ? 255. / max_val : 0., 0);
				// 对直方图的数值转为0~255  
				cvResetImageROI(first_hue);
				//去除ROI  
				cvResetImageROI(first_mask);
				//去除ROI  
				track_window1 = selection;
				track_object = 1;
				//置track_object为1,表明属性提取完成  
				cvZero(first_histimg);

				int bin_w = first_histimg->width / hdims;
				for (int i = 0; i < hdims; i++)
					//画直方图到图像空间  
				{
					int val = cvRound(cvGetReal1D(first_hist->bins, i)*first_histimg->height / 255);
					CvScalar color = hsv2rgb(i*180.f / hdims);
					cvRectangle(first_histimg, cvPoint(i*bin_w, first_histimg->height),
						cvPoint((i + 1)*bin_w, first_histimg->height - val),
						color, -1, 8, 0);

				}
			}
			cvCalcBackProject(&first_hue, first_backproject, first_hist);
			//计算hue的反向投影图  
			cvAnd(first_backproject, first_mask, first_backproject, 0);
			//得到掩膜内的反向投影  
			cvCamShift(first_backproject, track_window1,
				cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
				&track_comp1, &track_box1);

			//使用MeanShift算法对backproject中的内容进行搜索,返回跟踪结果  
			track_window1 = track_comp1.rect;
			//得到跟踪结果的矩形框  

			if (first_backproject_mode)
				cvCvtColor(first_backproject, image, CV_GRAY2BGR);

         		if (image->origin) {
				//in case of image reverse....................
				track_box1.angle = -track_box1.angle;
			}
			cvEllipseBox(image, track_box1, CV_RGB(255, 0, 0), 3, CV_AA, 0);
			//画出跟踪结果的位置  

		}
			else if (track_object == -2 || track_object == 2){
				//track_object非零,表示有需要跟踪的物体  
				int _vmin = vmin, _vmax = vmax;
				
				cvInRangeS(first_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
					cvScalar(180, 256, MAX(_vmin, _vmax), 0), first_mask);
		//制作掩膜板，只处理像素值为H：0~180，S：smin~256，V：vmin~vmax之间的部分  
				

				
				cvSplit(first_hsv, first_hue, 0, 0, 0); 

				cvInRangeS(second_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
					cvScalar(180, 256, MAX(_vmin, _vmax), 0), second_mask);
				//制作掩膜板，只处理像素值为H：0~180，S：smin~256，V：vmin~vmax之间的部分  

				cvSplit(second_hsv, second_hue, 0, 0, 0);

				//分离H分量  

				if (track_object == -2)
					//如果需要跟踪的物体还没有进行属性提取，则进行选取框类的图像属性提取  
				{
					float max_val = 0.f;
					cvSetImageROI(second_hue, selection);
					//设置原选择框为ROI  
					cvSetImageROI(second_mask, selection);
					//设置掩膜板选择框为ROI  
					cvCalcHist(&second_hue, second_hist, 0, second_mask);
					//得到选择框内且满足掩膜板内的直方图  
					cvGetMinMaxHistValue(second_hist, 0, &max_val, 0, 0);
					cvConvertScale(second_hist->bins, second_hist->bins, max_val ? 255. / max_val : 0., 0);
					// 对直方图的数值转为0~255  
					cvResetImageROI(second_hue);
					//去除ROI  
					cvResetImageROI(second_mask);
					//去除ROI  
					track_window2 = selection;
					track_object = 2;
					//置track_object为1,表明属性提取完成  
					cvZero(second_histimg);

					int bin_w = second_histimg->width / hdims;
					for (int i = 0; i < hdims; i++)
						//画直方图到图像空间  
					{
						int val = cvRound(cvGetReal1D(second_hist->bins, i)*second_histimg->height / 255);
						CvScalar color = hsv2rgb(i*180.f / hdims);
						cvRectangle(second_histimg, cvPoint(i*bin_w, second_histimg->height),
							cvPoint((i + 1)*bin_w, second_histimg->height - val),
							color, -1, 8, 0);

					}
				}
				cvCalcBackProject(&second_hue, second_backproject, second_hist);
				//计算hue的反向投影图  
				cvAnd(second_backproject, second_mask, second_backproject, 0);
				//得到掩膜内的反向投影  
				cvCamShift(second_backproject, track_window2,
					cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
					&track_comp2, &track_box2);

				//使用MeanShift算法对backproject中的内容进行搜索,返回跟踪结果  
				track_window2 = track_comp2.rect;
				//得到跟踪结果的矩形框  

				if (second_backproject_mode)
					cvCvtColor(second_backproject, image, CV_GRAY2BGR);
				cvCalcBackProject(&first_hue, first_backproject, first_hist);
				//计算hue的反向投影图  
				cvAnd(first_backproject, first_mask, first_backproject, 0);
				//得到掩膜内的反向投影  
				cvCamShift(first_backproject, track_window1,
					cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
					&track_comp1, &track_box1);

				//使用MeanShift算法对backproject中的内容进行搜索,返回跟踪结果  
				track_window1 = track_comp1.rect;
				//得到跟踪结果的矩形框  

				if (first_backproject_mode)
					cvCvtColor(first_backproject, image, CV_GRAY2BGR);

				
				static CvPoint controlP1;
				if (track_comp1.rect.x) {
					controlP1.x = track_comp1.rect.x + track_comp1.rect.width / 2;
					controlP1.y = track_comp1.rect.y + track_comp1.rect.height / 2;
					std::cout << "Point:" << "x-" << controlP1.x << "  :y-" << controlP1.y << std::endl;
				}
				static CvPoint controlP2;
				//if (track_comp2.rect.x) {
					//controlP2.x = track_comp2.rect.x + track_comp2.rect.width / 2;
					//controlP2.y = track_comp2.rect.y + track_comp2.rect.height / 2;
					controlP2.x = 0;
					controlP2.y = 0;
					//std::cout << "Point:" << "x-" << controlP2.x << "  :y-" << controlP2.y << std::endl;
				//}
				Control(&controlP1, &controlP2);

				if (image->origin) {
					//in case of image reverse....................
					track_box1.angle = -track_box1.angle;
					track_box2.angle = -track_box2.angle;
				}
				cvEllipseBox(image, track_box1, CV_RGB(255, 0, 0), 3, CV_AA, 0);
				cvEllipseBox(image, track_box2, CV_RGB(255, 0, 0), 3, CV_AA, 0);
				//画出跟踪结果的位置  


			}

			if (select_object && selection.width > 0 && selection.height > 0)
				//如果正处于物体选择，画出选择框  
			{
				cvSetImageROI(image, selection);
				cvXorS(image, cvScalarAll(255), image, 0);
				cvResetImageROI(image);
			}

			cvShowImage("CamShiftDemo", image);
			cvShowImage("FirstHistogram", first_histimg);
			cvShowImage("SecondHistogram", second_histimg);



			char c = cvWaitKey(10);
			if ((char)c == 27)
				break;
			switch ((char)c)
				//按键切换功能  
			{
			case 'b':case 'B':
				first_backproject_mode ^= 1;
				second_backproject_mode ^= 1;
				break;
			case 'c':case 'C':
				track_object = 0;
				cvZero(first_histimg);
				cvZero(second_histimg);
				break;
			case 'h':case 'H':
				show_hist ^= 1;
				if (!show_hist) {
					cvDestroyWindow("FirstHistogram");
					cvDestroyWindow("SecondHistogram");
				}
				else {
					cvNamedWindow("FirstHistogram", 1);
					cvNamedWindow("SecondHistogram", 1);
				}break;
			default:
				;
			}

		}
		return 0;

}





//set mouse call back function
void selectionCall(int mouseEvent, int x, int y, int flags, void* param) {
	static CvPoint origin = cvPoint(x, y);
	if (!image)return;
	if (image->origin)y = image->height - y;
	if (select_object)
		{
			selection.x = MIN(x, origin.x);
			selection.y = MIN(y, origin.y);
			selection.width = selection.x + CV_IABS(x - origin.x);
			selection.height = selection.y + CV_IABS(y - origin.y);

			selection.x = MAX(selection.x, 0);
			selection.y = MAX(selection.y, 0);
			selection.width = MIN(selection.width, image->width);
			selection.height = MIN(selection.height, image->height);
			selection.width -= selection.x;
			selection.height -= selection.y;
		}

	switch (mouseEvent)
	{
	case CV_EVENT_LBUTTONDOWN:
		//鼠标按下,开始点击选择跟踪物体  
		origin = CvPoint(x, y);
		selection = cvRect(x, y, 0, 0);
		select_object = 1;
		break;
	case CV_EVENT_LBUTTONUP:
		//鼠标松开,完成选择跟踪物体  
		if (selection.width > 0 && selection.height > 0)
			select_object = 0;
			//How to set the judgement to two selection????????????
			//如果选择物体有效，则打开跟踪功能  
			if(track_object==0)track_object = -1;
			else if (track_object == 1 || track_object == 2)track_object = -2;
		break;
	}

}



//Transform Hue in HSV to RGB
CvScalar hsv2rgb(float hue)
{
	int rgb[3], p, sector;
	static const int sector_data[][3] =
	{ { 0,2,1 },{ 1,2,0 },{ 1,0,2 },{ 2,0,1 },{ 2,1,0 },{ 0,1,2 } };
	hue *= 0.033333333333333333333333333333333f;
	sector = cvFloor(hue);
	p = cvRound(255 * (hue - sector));
	p ^= sector & 1 ? 255 : 0;

	rgb[sector_data[sector][0]] = 255;
	rgb[sector_data[sector][1]] = 0;
	rgb[sector_data[sector][2]] = p;

	return cvScalar(rgb[2], rgb[1], rgb[0], 0);
}
