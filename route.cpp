#include "stdafx.h"
#include "route.h"

CvPoint mapPoints[9];
int pointNum = 0;


void mapCall(int mouseEvent, int x, int y, int flags, void* param) {
	if (mouseEvent == CV_EVENT_LBUTTONDOWN) {
		std::cout << "Clicked on " << "( " << x << " , " << y << " )" << std::endl;
		if (pointNum < 9)
			mapPoints[pointNum++] = CvPoint(x, y);
		else if (pointNum >= 9)std::cout << "No more changes " << std::endl;
	}
}

CvPoint *GetRoute(IplImage *Map, int *Pnum) {
	cvNamedWindow("Do Map", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Do Map", mapCall);
	cvShowImage("Do Map", Map);
	char wait;
	while (true) {
		wait = cvWaitKey(35);
		if (wait == 'f' || wait == 'F') break;
	}
	*Pnum = pointNum;
	return mapPoints;
}
