#ifndef LVZHENG_CAMERA_HPP
#define LVZHENG_CAMERA_HPP

#include <opencv2/opencv.hpp>

static CvCapture *open_camera()
{
	CvCapture* capture;
	capture = cvCreateCameraCapture(-1);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
	return capture;
}

#endif // LVZHENG_CAMERA_HPP
