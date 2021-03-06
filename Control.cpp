#include "stdafx.h"
#include "Control.h"
#include "truck.hpp"
#include "getP.h"
#include "trans.h"
#include "route.h"
#include "camera.hpp"
#include <math.h>

IplImage *ControlMap;
CvPoint *ControlRoute;
int CPointn;
std::unique_ptr<lvzheng::truck> truck;


IplImage *getMap() {
	CvCapture *cap = open_camera();
	IplImage *cPic;
	cvNamedWindow("Cam", CV_WINDOW_AUTOSIZE);
	char wait;
	double threshold = 100;
	int thinT = 9;
	do
	{
		cPic = cvQueryFrame(cap);
		if (!cPic) break;
		cPic = cvChangeBinary(cPic, threshold);

		cvShowImage("Cam", cPic);
		wait = cvWaitKey(33);
		if (wait == 'q') break;		//	e代表escape，退出
		else if (wait == 'u' || wait == 'U')			//	u代表up，上升阈值
			threshold += 2.5;
		else if (wait == 'd' || wait == 'D')			//	d代表down，下降阈值
			threshold -= 2.5;
		else if (wait == 'c' || wait == 'C')
			break;
		else
			cvReleaseImage(&cPic);
	} while (true);
	cvReleaseCapture(&cap);
	cvSetMouseCallback("Cam", mouseCall);
	cvShowImage("Cam", cPic);
	IplImage *Pic;
	while (true) {
		wait = cvWaitKey(50);
		if (wait == 'p') break;
	}
	Pic = perspectiveTransform(cPic);
	Pic = cvThinBinary(Pic, thinT);

	cvShowImage("Cam", Pic);

	cvWaitKey(0);

	cvReleaseImage(&cPic);
	cvDestroyWindow("Cam");

	return Pic;
}

void SetControlMap(IplImage *map, CvPoint *route, int nPoint) {
	ControlRoute = route;
	CPointn = nPoint;
	ControlMap = cvCreateImage(cvGetSize(map), IPL_DEPTH_8U, 1);
	for (int i = 0; i < nPoint - 1; i++)
	{
		cvLine(ControlMap, ControlRoute[i], ControlRoute[i+1], CV_RGB(255, 255, 255), 2);
	}
	for (int i = 0; i < nPoint; i++)
	{
		cvDrawCircle(ControlMap, ControlRoute[i], 5, CV_RGB(255, 255, 255));
	}
}

int Control(CvPoint *head, CvPoint *tail) {

	static int goalPoint = 0;
	int x1 = tail->x, y1 = tail->y;
	int x2 = head->x, y2 = head->y;
	int x3 = (ControlRoute + goalPoint)->x, y3 = (ControlRoute + goalPoint)->y;
	double dst23 = sqrt((double)((x3 - x2)*(x3 - x2) + (y3 - y2)*(y3 - y2)));
	double dst12 = sqrt((double)((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
	double dst13 = sqrt((double)((x3 - x1)*(x3 - x1) + (y3 - y1)*(y3 - y1)));
	//std::cout << "x1:" << x1 << "," << y1 << std::endl;
	//std::cout << "x2:" << x2 << "," << y2 << std::endl;
	//std::cout << "goal:" << x3 << "," << y3 << std::endl;
	if (dst23 < offset) {
		goalPoint++;
		std::cout << "i have get the point " << goalPoint << std::endl;
		if (goalPoint == CPointn) {
			truck->stop();
			return 100;
		}
		return 0;
	}

	double costheta = (double)((x3 - x1)*(x2 - x1) + (y3 - y1)*(y2 - y1)) / (dst12 * dst13);
	double sintheta = 1.0 - costheta*costheta;
	//std::cout << sintheta << std::endl;
	int turn = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

	if (dst13 > 200) {
		if (dst13*sintheta > goalOffset) {
			truck->go(turn > 0 ? lvzheng::truck::direction::RIGHT : lvzheng::truck::direction::LEFT, lvzheng::truck::strength::MEDIUM);
			//cvWaitKey(ControlLag);
			//truck->stop();
			return 0;
		}
	}
	else {
		if (dst13*sintheta > goalOffset) {
			truck->go(turn > 0 ? lvzheng::truck::direction::RIGHT : lvzheng::truck::direction::LEFT, lvzheng::truck::strength::MEDIUM);
			//cvWaitKey(ControlLag);
			//truck->stop();
			return 0;
		}
	}

	if (costheta < 0) {
		truck->go(turn > 0 ? lvzheng::truck::direction::RIGHT : lvzheng::truck::direction::LEFT, lvzheng::truck::strength::MEDIUM);
		//cvWaitKey(20);
	}
	truck->go(lvzheng::truck::direction::FORWARD, lvzheng::truck::strength::MEDIUM);
	//cvWaitKey(ControlLag);


	//cvNamedWindow("Control Window", CV_WINDOW_AUTOSIZE);
	//cvShowImage("Control Window", ControlMap);
	//cvWaitKey(0);
	return 0;
}
