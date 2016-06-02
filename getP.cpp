#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "getP.h"


void cvThin(IplImage* src, IplImage* dst, int iterations = 1)
{
	cvCopy(src, dst);
	BwImage dstdat(dst);
	IplImage* t_image = cvCloneImage(src);
	BwImage t_dat(t_image);
	for (int n = 0; n < iterations; n++)
		for (int s = 0; s <= 1; s++) {
			cvCopy(dst, t_image);
			for (int i = 0; i < src->height; i++)
				for (int j = 0; j < src->width; j++)
					if (t_dat[i][j]) {
						int a = 0, b = 0;
						int d[8][2] = { { -1, 0 },{ -1, 1 },{ 0, 1 },{ 1, 1 },
						{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 } };
						int p[8];
						p[0] = (i == 0) ? 0 : t_dat[i - 1][j];
						for (int k = 1; k <= 8; k++) {
							if (i + d[k % 8][0] < 0 || i + d[k % 8][0] >= src->height ||
								j + d[k % 8][1] < 0 || j + d[k % 8][1] >= src->width)
								p[k % 8] = 0;
							else p[k % 8] = t_dat[i + d[k % 8][0]][j + d[k % 8][1]];
							if (p[k % 8]) {
								b++;
								if (!p[k - 1]) a++;
							}
						}
						if (b >= 2 && b <= 6 && a == 1)
							if (!s && !(p[2] && p[4] && (p[0] || p[6])))
								dstdat[i][j] = 0;
							else if (s && !(p[0] && p[6] && (p[2] || p[4])))
								dstdat[i][j] = 0;
					}
		}
	cvReleaseImage(&t_image);
}

IplImage *cvChangeBinary(IplImage *src, double thred) {
	IplImage *bina, *gray;
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	bina = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);

	cvCvtColor(src, gray, CV_BGR2GRAY);
	cvThreshold(gray, bina, thred, 255, CV_THRESH_BINARY_INV);

	cvReleaseImage(&gray);

	return bina;
}

IplImage *cvThinBinary(IplImage *bin, int turn) {
	IplImage *thin;
	thin = cvCreateImage(cvGetSize(bin), IPL_DEPTH_8U, 1);
	cvThin(bin, thin, turn);

	cvReleaseImage(&bin);

	return thin;
}
