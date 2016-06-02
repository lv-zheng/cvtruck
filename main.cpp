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
	//Meanshift�����㷨���ص�Box��  
	//typedef struct CvBox2D{  
	//CvPoint2D32f center; /* ���ӵ����� */  
	//CvSize2D32f size; /* ���ӵĳ��Ϳ� */  
	//float angle; /* ˮƽ�����һ���ߵļнǣ��û��ȱ�ʾ*/  
	//}CvBox2D;  

	CvBox2D track_box1;
	CvBox2D track_box2;
	//typedef struct CvConnectedComp{  
	//double area; /* ��ͨ������ */  
	//float value; /* �ָ���ĻҶ�����ֵ */  
	//CvRect rect; /* �ָ���� ROI */  
	//} CvConnectedComp; 
	CvConnectedComp track_comp1;  // information of tracking area
	CvConnectedComp track_comp2;

	//����ֱ��ͼbins�ĸ�����Խ��Խ��ȷ
	int hdims = 16;

	//����ֵ�ķ�Χ  
	float hranges_arr[] = { 0,180 };
	float* hranges = hranges_arr;


	while (true)
	{
		IplImage* frame = cvQueryFrame(capture);
		frame = perspectiveTransform3(frame);
		if (!frame) return -1; 
		if (!image)
			//imageΪ0,�����տ�ʼ��δ��image������,�Ƚ���һЩ������  
		{
			image = cvCreateImage(cvGetSize(frame), 8, 3);
			image->origin = frame->origin;
			first_hsv = cvCreateImage(cvGetSize(frame), 8, 3);
			second_hsv = cvCreateImage(cvGetSize(frame), 8, 3);
			first_hue = cvCreateImage(cvGetSize(frame), 8, 1);
			second_hue = cvCreateImage(cvGetSize(frame), 8, 1);
			first_mask = cvCreateImage(cvGetSize(frame), 8, 1);
			second_mask = cvCreateImage(cvGetSize(frame), 8, 1);
			//������Ĥͼ��ռ�  
			first_backproject = cvCreateImage(cvGetSize(frame), 8, 1);
			second_backproject = cvCreateImage(cvGetSize(frame), 8, 1);
			//���䷴��ͶӰͼ�ռ�,��Сһ��,��ͨ��  
			first_hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
			second_hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
			//����ֱ��ͼ�ռ�  
			first_histimg = cvCreateImage(cvSize(320, 200), 8, 3);
			second_histimg = cvCreateImage(cvSize(320, 200), 8, 3);
			//��������ֱ��ͼ��ʾ�Ŀռ�  
			cvZero(first_histimg);
			cvZero(second_histimg);
			//�ñ���Ϊ��ɫ  
		}

		cvCopy(frame, image, 0);
		cvCvtColor(image, first_hsv, CV_BGR2HSV);
		cvCvtColor(image, second_hsv, CV_BGR2HSV);
		//��ͼ���RGB��ɫϵתΪHSV��ɫϵ  



		if (track_object == -1 || track_object == 1)
			//track_object����,��ʾ����Ҫ���ٵ�����  
		{
			int _vmin = vmin, _vmax = vmax;
			cvInRangeS(first_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
				cvScalar(180, 256, MAX(_vmin, _vmax), 0), first_mask);
			//������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���  

			cvSplit(first_hsv, first_hue, 0, 0, 0);

			//����H����  

			if (track_object == -1)
				//�����Ҫ���ٵ����廹û�н���������ȡ�������ѡȡ�����ͼ��������ȡ  
			{
				float max_val = 0.f;
				cvSetImageROI(first_hue, selection);
				//����ԭѡ���ΪROI  
				cvSetImageROI(first_mask, selection);
				//������Ĥ��ѡ���ΪROI  
				cvCalcHist(&first_hue, first_hist, 0, first_mask);
				//�õ�ѡ�������������Ĥ���ڵ�ֱ��ͼ  
				cvGetMinMaxHistValue(first_hist, 0, &max_val, 0, 0);
				cvConvertScale(first_hist->bins, first_hist->bins, max_val ? 255. / max_val : 0., 0);
				// ��ֱ��ͼ����ֵתΪ0~255  
				cvResetImageROI(first_hue);
				//ȥ��ROI  
				cvResetImageROI(first_mask);
				//ȥ��ROI  
				track_window1 = selection;
				track_object = 1;
				//��track_objectΪ1,����������ȡ���  
				cvZero(first_histimg);

				int bin_w = first_histimg->width / hdims;
				for (int i = 0; i < hdims; i++)
					//��ֱ��ͼ��ͼ��ռ�  
				{
					int val = cvRound(cvGetReal1D(first_hist->bins, i)*first_histimg->height / 255);
					CvScalar color = hsv2rgb(i*180.f / hdims);
					cvRectangle(first_histimg, cvPoint(i*bin_w, first_histimg->height),
						cvPoint((i + 1)*bin_w, first_histimg->height - val),
						color, -1, 8, 0);

				}
			}
			cvCalcBackProject(&first_hue, first_backproject, first_hist);
			//����hue�ķ���ͶӰͼ  
			cvAnd(first_backproject, first_mask, first_backproject, 0);
			//�õ���Ĥ�ڵķ���ͶӰ  
			cvCamShift(first_backproject, track_window1,
				cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
				&track_comp1, &track_box1);

			//ʹ��MeanShift�㷨��backproject�е����ݽ�������,���ظ��ٽ��  
			track_window1 = track_comp1.rect;
			//�õ����ٽ���ľ��ο�  

			if (first_backproject_mode)
				cvCvtColor(first_backproject, image, CV_GRAY2BGR);

         		if (image->origin) {
				//in case of image reverse....................
				track_box1.angle = -track_box1.angle;
			}
			cvEllipseBox(image, track_box1, CV_RGB(255, 0, 0), 3, CV_AA, 0);
			//�������ٽ����λ��  

		}
			else if (track_object == -2 || track_object == 2){
				//track_object����,��ʾ����Ҫ���ٵ�����  
				int _vmin = vmin, _vmax = vmax;
				
				cvInRangeS(first_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
					cvScalar(180, 256, MAX(_vmin, _vmax), 0), first_mask);
		//������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���  
				

				
				cvSplit(first_hsv, first_hue, 0, 0, 0); 

				cvInRangeS(second_hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),//call cvScalar function to transform to RGB
					cvScalar(180, 256, MAX(_vmin, _vmax), 0), second_mask);
				//������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���  

				cvSplit(second_hsv, second_hue, 0, 0, 0);

				//����H����  

				if (track_object == -2)
					//�����Ҫ���ٵ����廹û�н���������ȡ�������ѡȡ�����ͼ��������ȡ  
				{
					float max_val = 0.f;
					cvSetImageROI(second_hue, selection);
					//����ԭѡ���ΪROI  
					cvSetImageROI(second_mask, selection);
					//������Ĥ��ѡ���ΪROI  
					cvCalcHist(&second_hue, second_hist, 0, second_mask);
					//�õ�ѡ�������������Ĥ���ڵ�ֱ��ͼ  
					cvGetMinMaxHistValue(second_hist, 0, &max_val, 0, 0);
					cvConvertScale(second_hist->bins, second_hist->bins, max_val ? 255. / max_val : 0., 0);
					// ��ֱ��ͼ����ֵתΪ0~255  
					cvResetImageROI(second_hue);
					//ȥ��ROI  
					cvResetImageROI(second_mask);
					//ȥ��ROI  
					track_window2 = selection;
					track_object = 2;
					//��track_objectΪ1,����������ȡ���  
					cvZero(second_histimg);

					int bin_w = second_histimg->width / hdims;
					for (int i = 0; i < hdims; i++)
						//��ֱ��ͼ��ͼ��ռ�  
					{
						int val = cvRound(cvGetReal1D(second_hist->bins, i)*second_histimg->height / 255);
						CvScalar color = hsv2rgb(i*180.f / hdims);
						cvRectangle(second_histimg, cvPoint(i*bin_w, second_histimg->height),
							cvPoint((i + 1)*bin_w, second_histimg->height - val),
							color, -1, 8, 0);

					}
				}
				cvCalcBackProject(&second_hue, second_backproject, second_hist);
				//����hue�ķ���ͶӰͼ  
				cvAnd(second_backproject, second_mask, second_backproject, 0);
				//�õ���Ĥ�ڵķ���ͶӰ  
				cvCamShift(second_backproject, track_window2,
					cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
					&track_comp2, &track_box2);

				//ʹ��MeanShift�㷨��backproject�е����ݽ�������,���ظ��ٽ��  
				track_window2 = track_comp2.rect;
				//�õ����ٽ���ľ��ο�  

				if (second_backproject_mode)
					cvCvtColor(second_backproject, image, CV_GRAY2BGR);
				cvCalcBackProject(&first_hue, first_backproject, first_hist);
				//����hue�ķ���ͶӰͼ  
				cvAnd(first_backproject, first_mask, first_backproject, 0);
				//�õ���Ĥ�ڵķ���ͶӰ  
				cvCamShift(first_backproject, track_window1,
					cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
					&track_comp1, &track_box1);

				//ʹ��MeanShift�㷨��backproject�е����ݽ�������,���ظ��ٽ��  
				track_window1 = track_comp1.rect;
				//�õ����ٽ���ľ��ο�  

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
				//�������ٽ����λ��  


			}

			if (select_object && selection.width > 0 && selection.height > 0)
				//�������������ѡ�񣬻���ѡ���  
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
				//�����л�����  
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
		//��갴��,��ʼ���ѡ���������  
		origin = CvPoint(x, y);
		selection = cvRect(x, y, 0, 0);
		select_object = 1;
		break;
	case CV_EVENT_LBUTTONUP:
		//����ɿ�,���ѡ���������  
		if (selection.width > 0 && selection.height > 0)
			select_object = 0;
			//How to set the judgement to two selection????????????
			//���ѡ��������Ч����򿪸��ٹ���  
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
