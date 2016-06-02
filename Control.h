#pragma once

#include <opencv2/opencv.hpp>
#include "truck.hpp"
#include "getP.h"
#include "trans.h"
#include "route.h"
#include <math.h>

#define goalOffset 8
#define offset 13
#define COM "COM5"  //perhaps not, we should change it
#define ControlLag 10


extern IplImage *ControlMap;
extern CvPoint *ControlRoute;
extern int CPointn;


IplImage *getMap();
void SetControlMap(IplImage *map, CvPoint *route, int nPoint);
int Control(CvPoint *head, CvPoint *tail);
