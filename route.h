#pragma once

#include <opencv2/opencv.hpp>

extern CvPoint pointList[9];
extern int pointNum;


CvPoint *GetRoute(IplImage *Map, int *Pnum);
