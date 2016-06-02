#pragma once

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>

template<class T> class Image
{
private:
	IplImage* imgp;
public:
	Image(IplImage* img = 0) { imgp = img; }
	~Image() { imgp = 0; }
	void operator=(IplImage* img) { imgp = img; }
	inline T* operator[](const int rowIndx) {
		return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));
	}
};
typedef Image<unsigned char>  BwImage;


IplImage *cvChangeBinary(IplImage *src, double thred);
IplImage *cvThinBinary(IplImage *bin, int turn = 2);
CvSeq *cvHoughBinary(IplImage *bin, int MaxNum, int MinLen, int MinGap);
IplImage* perspectiveTransform(IplImage* originImg);
