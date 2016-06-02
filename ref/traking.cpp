//���˶�����ĸ��٣�  
//��������̶�,����֡� Ȼ���ڼ�������ͨ�� �����С��ȥ������  
//���������һ,����Ҫ���ٵ�������ɫ�ͱ���ɫ�нϴ����� ���û�����ɫ�ĸ��� ��CAMSHIFT ³���Զ��ǽϺõ�  
//�����������,�米�����к�ǰ��һ������ɫ ����Ҫ�õ�һЩ����Ԥ���Ե��㷨 �翨�����˲��� ���Ժ�CAMSHIFT
#pragma once
#include"stdafx.h"

#ifdef _CH_  
#pragma package <opencv>  
#endif  

#ifndef _EiC  
#include <opencv2/opencv.hpp>
#include <stdio.h>  
#include <ctype.h>  
#include <iostream>
#include "getP.h"
#include "trans.h"
#include "route.h"
#include "Control.h"
#endif  
#ifndef _TRACKING
#define _TRACKING
IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histimg = 0;
//��HSV�е�Hue�������и���  
CvHistogram *hist = 0;
//ֱ��ͼ��  
int backproject_mode = 0;
int select_object = 0;
int track_object = 0;
int show_hist = 1;
CvPoint origin;
CvRect selection;
CvRect track_window;
CvBox2D track_box;
//Meanshift�����㷨���ص�Box��  
//typedef struct CvBox2D{  
//CvPoint2D32f center; /* ���ӵ����� */  
//CvSize2D32f size; /* ���ӵĳ��Ϳ� */  
//float angle; /* ˮƽ�����һ���ߵļнǣ��û��ȱ�ʾ*/  
//}CvBox2D;  
CvConnectedComp track_comp;

//���Ӳ���   
//typedef struct CvConnectedComp{  
//double area; /* ��ͨ������ */  
//float value; /* �ָ���ĻҶ�����ֵ */  
//CvRect rect; /* �ָ���� ROI */  
//} CvConnectedComp;  
int hdims = 16;
//����ֱ��ͼbins�ĸ�����Խ��Խ��ȷ  
float hranges_arr[] = { 0,180 };
//����ֵ�ķ�Χ  
float* hranges = hranges_arr;
//���ڳ�ʼ��CvHistogram��  
int vmin = 10, vmax = 256, smin = 30;
//�������û�����  
void on_mouse(int event, int x, int y, int flags, void* param)
//���ص�����,�ú����������и���Ŀ���ѡ��  
{
	if (!image)
		return;

	if (image->origin)
		y = image->height - y;
	//���ͼ��ԭ������������,�����Ϊ����  

	if (select_object)
		//select_objectΪ1,��ʾ����������Ŀ��ѡ��  
		//��ʱ�Ծ�����selection�õ�ǰ�����λ�ý�������  
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

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		//��갴��,��ʼ���ѡ���������  
		origin = cvPoint(x, y);
		selection = cvRect(x, y, 0, 0);
		select_object = 1;
		break;
	case CV_EVENT_LBUTTONUP:
		//����ɿ�,���ѡ���������  
		select_object = 0;
		if (selection.width > 0 && selection.height > 0)
			//���ѡ��������Ч����򿪸��ٹ���  
			track_object = -1;
		break;
	}
}


CvScalar hsv2rgb(float hue)
//���ڽ�Hue��ת����RGB��  
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


int traking() {
	CvCapture* capture = 0;

	capture = cvCreateCameraCapture(-1);
	//cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
	//cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
	//cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 8);
	/*
	if (argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
	//������ͷ
	capture = cvCaptureFromCAM(argc == 2 ? argv[1][0] - '0' : 0);
	else if (argc == 2)
	//��avi
	capture = cvCaptureFromAVI(argv[1]);*/
	if (!capture)
		//����Ƶ��ʧ��  
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	printf("Hot keys: \n"
		"\tESC - quit the program\n"
		"\tc - stop the tracking\n"
		"\tb - switch to/from backprojection view\n"
		"\th - show/hide object histogram\n"
		"To initialize tracking, select the object with mouse\n");
	//��ӡ�������б�  

	cvNamedWindow("Histogram", 1);
	//������ʾֱ��ͼ  
	cvNamedWindow("CamShiftDemo", 1);


	//������ʾ��Ƶ  
	cvSetMouseCallback("CamShiftDemo", on_mouse, 0);
	//�������ص�����  
	cvCreateTrackbar("Vmin", "CamShiftDemo", &vmin, 256, 0);
	cvCreateTrackbar("Vmax", "CamShiftDemo", &vmax, 256, 0);
	cvCreateTrackbar("Smin", "CamShiftDemo", &smin, 256, 0);
	//���û�����  

	for (;;)
		//������Ƶ֡������ѭ��  
	{
		IplImage* frame = 0;
		int i, bin_w, c;

		frame = cvQueryFrame(capture);
		if (!frame)
			break;

		if (!image)
			//imageΪ0,�����տ�ʼ��δ��image������,�Ƚ���һЩ������  
		{
			image = cvCreateImage(cvGetSize(frame), 8, 3);
			image->origin = frame->origin;
			hsv = cvCreateImage(cvGetSize(frame), 8, 3);
			hue = cvCreateImage(cvGetSize(frame), 8, 1);
			mask = cvCreateImage(cvGetSize(frame), 8, 1);
			//������Ĥͼ��ռ�  
			backproject = cvCreateImage(cvGetSize(frame), 8, 1);
			//���䷴��ͶӰͼ�ռ�,��Сһ��,��ͨ��  
			hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
			//����ֱ��ͼ�ռ�  
			histimg = cvCreateImage(cvSize(320, 200), 8, 3);
			//��������ֱ��ͼ��ʾ�Ŀռ�  
			cvZero(histimg);
			//�ñ���Ϊ��ɫ  
		}

		cvCopy(frame, image, 0);
		cvCvtColor(image, hsv, CV_BGR2HSV);
		//��ͼ���RGB��ɫϵתΪHSV��ɫϵ  

		if (track_object)
			//track_object����,��ʾ����Ҫ���ٵ�����  
		{
			int _vmin = vmin, _vmax = vmax;

			cvInRangeS(hsv, cvScalar(0, smin, MIN(_vmin, _vmax), 0),
				cvScalar(180, 256, MAX(_vmin, _vmax), 0), mask);
			//������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���  
			cvSplit(hsv, hue, 0, 0, 0);
			//����H����  

			if (track_object < 0)
				//�����Ҫ���ٵ����廹û�н���������ȡ�������ѡȡ�����ͼ��������ȡ  
			{
				float max_val = 0.f;
				cvSetImageROI(hue, selection);
				//����ԭѡ���ΪROI  
				cvSetImageROI(mask, selection);
				//������Ĥ��ѡ���ΪROI  
				cvCalcHist(&hue, hist, 0, mask);
				//�õ�ѡ�������������Ĥ���ڵ�ֱ��ͼ  
				cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);
				cvConvertScale(hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0);
				// ��ֱ��ͼ����ֵתΪ0~255  
				cvResetImageROI(hue);
				//ȥ��ROI  
				cvResetImageROI(mask);
				//ȥ��ROI  
				track_window = selection;
				track_object = 1;
				//��track_objectΪ1,����������ȡ���  
				cvZero(histimg);
				bin_w = histimg->width / hdims;
				for (i = 0; i < hdims; i++)
					//��ֱ��ͼ��ͼ��ռ�  
				{
					int val = cvRound(cvGetReal1D(hist->bins, i)*histimg->height / 255);
					CvScalar color = hsv2rgb(i*180.f / hdims);
					cvRectangle(histimg, cvPoint(i*bin_w, histimg->height),
						cvPoint((i + 1)*bin_w, histimg->height - val),
						color, -1, 8, 0);
				}
			}

			cvCalcBackProject(&hue, backproject, hist);
			//����hue�ķ���ͶӰͼ  
			cvAnd(backproject, mask, backproject, 0);
			//�õ���Ĥ�ڵķ���ͶӰ  
			cvCamShift(backproject, track_window,
				cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
				&track_comp, &track_box);
			//ʹ��MeanShift�㷨��backproject�е����ݽ�������,���ظ��ٽ��  
			track_window = track_comp.rect;
			//�õ����ٽ���ľ��ο�  

			if (backproject_mode)
				cvCvtColor(backproject, image, CV_GRAY2BGR);

			static CvPoint controlP, controlPTMP;
			if (track_comp.rect.x) {
				controlP.x = track_comp.rect.x + track_comp.rect.width / 2;
				controlP.y = track_comp.rect.y + track_comp.rect.height / 2;
				controlPTMP.x = controlP.x - 20;
				controlPTMP.y = controlP.y - 20;
				std::cout << "Point:" << "x-" << controlP.x << "  :y-" << controlP.y << std::endl;
				Control(&controlP, &controlPTMP);
			}

			std::cout << "aaaa";

			if (image->origin)
				track_box.angle = -track_box.angle;
			cvEllipseBox(image, track_box, CV_RGB(255, 0, 0), 3, CV_AA, 0);
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
		cvShowImage("Histogram", histimg);




		c = cvWaitKey(10);
		if ((char)c == 27)
			break;
		switch ((char)c)
			//�����л�����  
		{
		case 'b':
			backproject_mode ^= 1;
			break;
		case 'c':
			track_object = 0;
			cvZero(histimg);
			break;
		case 'h':
			show_hist ^= 1;
			if (!show_hist)
				cvDestroyWindow("Histogram");
			else
				cvNamedWindow("Histogram", 1);
			break;
		default:
			;
		}
	}
	cvReleaseCapture(&capture);

	cvDestroyWindow("CamShiftDemo");

	return 0;
}

int main()
{
	IplImage *Map = getMap();
	int nPoint = 0;
	CvPoint *route = GetRoute(Map, &nPoint);

	SetControlMap(Map, route, nPoint);


	traking();

	return 0;
}
#endif
#ifdef _EiC  
main(1, "camshiftdemo.c");
#endif  