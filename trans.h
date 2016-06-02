#pragma once
#include <opencv2/opencv.hpp>


extern CvPoint2D32f originPoints[4];
extern int pointNumber;
/*
*Method:Initialize the deafault camera
*Usage:
*/

CvCapture* initCamera();

/*
*Method: GetPoints from button-click from users
*Usage:
*/
void mouseCall(int mouseEvent, int x, int y, int flags, void* param);

/*
*Method: Transform Perspective of camera
*Usage:
*/

IplImage* perspectiveTransform(IplImage* originImg);
IplImage* perspectiveTransform3(IplImage* originImg);
