#pragma once

#include <opencv2/opencv.hpp>
#include "truck.hpp"
#include "getP.h"
#include "trans.h"
#include "route.h"
#include <math.h>
#include <memory>

#define goalOffset 17
#define offset 25


extern IplImage *ControlMap;
extern CvPoint *ControlRoute;
extern int CPointn;
extern std::unique_ptr<lvzheng::truck> truck;

IplImage *getMap();
void SetControlMap(IplImage *map, CvPoint *route, int nPoint);
int Control(CvPoint *head, CvPoint *tail);
