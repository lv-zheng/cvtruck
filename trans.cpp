#include "stdafx.h"
#include "trans.h"
#include "camera.hpp"
#include <iostream>

int pointNumber = 0;
CvPoint2D32f originPoints[4];

CvCapture* initCamera() {
	CvCapture * cam = open_camera();
	if (!cam)
	{
		std::cout << "open failed" << std::endl;
		return NULL;
	}
	return cam;
}


void mouseCall(int mouseEvent, int x, int y, int flags, void* param) {
	if (mouseEvent == CV_EVENT_LBUTTONDOWN) {
		std::cout << "Clicked on " << "( " << x << " , " << y << " )" << std::endl;
		if (pointNumber < 4)
			originPoints[pointNumber++] = CvPoint2D32f(x, y);

		else if (pointNumber >= 4)std::cout << "No more changes " << std::endl;
	}
}

IplImage* perspectiveTransform(IplImage* originImg) {

	//initilize the transform points

	CvPoint2D32f transPoints[4];
	transPoints[0] = CvPoint2D32f(20.20);
	transPoints[1] = CvPoint2D32f(originImg->width - 20, 20);
	transPoints[2] = CvPoint2D32f(20, originImg->height - 20);
	transPoints[3] = CvPoint2D32f(originImg->width - 20, originImg->height - 20);

	//initilize the transform matrix

	CvMat* transMat = cvCreateMat(3, 3, CV_32FC1);

	//initilize the transImg

	IplImage* transImg = cvCreateImage(CvSize(originImg->width, originImg->height), IPL_DEPTH_8U, 1);


	cvGetPerspectiveTransform(originPoints, transPoints, transMat);
	cvWarpPerspective(originImg, transImg, transMat);


	return transImg;
}

IplImage* perspectiveTransform3(IplImage* originImg) {

	//initilize the transform points

	CvPoint2D32f transPoints[4];
	transPoints[0] = CvPoint2D32f(20.20);
	transPoints[1] = CvPoint2D32f(originImg->width - 20, 20);
	transPoints[2] = CvPoint2D32f(20, originImg->height - 20);
	transPoints[3] = CvPoint2D32f(originImg->width - 20, originImg->height - 20);

	//initilize the transform matrix

	CvMat* transMat = cvCreateMat(3, 3, CV_32FC1);

	//initilize the transImg

	IplImage* transImg = cvCreateImage(CvSize(originImg->width, originImg->height), IPL_DEPTH_8U, 3);


	cvGetPerspectiveTransform(originPoints, transPoints, transMat);
	cvWarpPerspective(originImg, transImg, transMat);


	return transImg;
}
