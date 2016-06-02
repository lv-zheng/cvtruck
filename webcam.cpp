#include <chrono>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

#include "zhangsuen.hpp"

using namespace cv;

#if 0
void process_image(Mat& image)
{
	//Mat result;
	int border_type = BORDER_REPLICATE;
	int top = image.rows / 20, left = image.cols / 20;
	copyMakeBorder(image, image, top, top, left, left, border_type);
	//image = result;
}
#endif

#if 0
Mat process_image(const Mat& src)
{
	int scale = 1, delta = 0, ddepth = CV_16S;
	Mat src_gray, grad;
	GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
	cvtColor( src, src_gray, COLOR_RGB2GRAY );

	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs( grad_x, abs_grad_x );
	Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	return grad;
}
#endif

#if 0
void process_image(Mat&)
{
}
#endif

#if 0
int process_threshold = 0;
Mat process_image(const Mat& src)
{
	Mat src_gray, detected_edges, dst;

	cvtColor(src, src_gray, COLOR_BGR2GRAY);
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, canny_threshold, canny_threshold * 3, 3);
	dst = Scalar::all(0);
	src_gray.copyTo(dst, detected_edges);

	return detected_edges;
}
#endif


#if 0 // mid
int get_threshold(const Mat& image, double percentage)
{
	auto rows = image.rows;
	auto cols = image.cols * image.channels();

	std::vector<unsigned long long> bucket(256, 0);
	for (int i = 0; i < rows; ++i) {
		auto *p = image.ptr<unsigned char>(i);
		for (int j = 0; j < cols; ++j) {
			++bucket[p[j]];
		}
	}
	unsigned long long sum = 0;
	unsigned long long endv = rows * cols * percentage;
	for (int i = 0; i < 256; ++i) {
		sum += bucket[i];
		if (sum >= endv)
			return i;
	}
	return 255;
	//std::cout << sum << std::endl;
}
#endif

int get_threshold(const Mat& image, double percentage)
{
	auto rows = image.rows;
	auto cols = image.cols * image.channels();

	unsigned long long sum = 0;
	for (int i = 0; i < rows; ++i) {
		auto *p = image.ptr<unsigned char>(i);
		for (int j = 0; j < cols; ++j) {
			sum += p[j] * p[j];
		}
	}
	sum = sum * 2 * percentage / rows / cols;
	sum = std::sqrt(sum);
	return sum < 256 ? sum : 255;
}

int thres = 50;
std::vector<Point2f> points;
Mat process_image(const Mat& src)
{
	Mat result;
	
	cvtColor(src, result, COLOR_BGR2GRAY);
	GaussianBlur(result, result, Size(3,3), 0, 0, BORDER_DEFAULT);

	if (points.size() < 4)
		return result;

	Point2f frame[4] = {{0, 0}, {0, 512}, {512, 512}, {512, 0}};
	auto m = getPerspectiveTransform(points.data(), frame);
	warpPerspective(result, result, m, {512, 512});
	threshold(result, result, get_threshold(result, static_cast<double>(thres) / 100), 255, 1);
	return result;
}

void mouse_clicked(int event, int x, int y, int, void *)
{
	if (event == EVENT_LBUTTONDOWN && points.size() < 4) {
		std::cout << "Clicked " << x << ", " << y << std::endl;
		points.emplace_back(x, y);
	}
}

bool refresh_hough;
std::vector<std::pair<int, int>> ok_points;

void remove_points(int event, int x, int y, int, void *)
{
	if (event != EVENT_LBUTTONDOWN)
		return;
	ok_points.emplace_back(x, y);
	std::cout << "OK " << x << ", " << y << std::endl;
}

void save_image(const Mat& image, std::string suffix)
{
	std::string filename("img_");
	auto now = std::chrono::system_clock::now();
	filename += std::to_string(std::chrono::system_clock::to_time_t(now));
	filename += suffix;
	filename += ".png";
	bool success = imwrite(filename, image);
	if (success)
		std::cerr << "Saved ";
	else
		std::cerr << "Fail to save ";
	std::cerr << filename << std::endl;
}

std::vector<Vec4i> hough_lines(const Mat& image)
{
	Mat dst;
	Mat src;// = image;
	GaussianBlur(image, src, Size(1,1), 0, 0, BORDER_DEFAULT);
	cvtColor(image, dst, COLOR_GRAY2BGR);
	std::vector<Vec4i> lines;
	HoughLinesP(src, lines, 1, CV_PI / 180 / 8, 128, 96, 10);
	return lines;
}

Mat draw_lines(const Mat& image, const std::vector<Vec4i>& lines)
{
	Mat dst = image.clone();
	for (auto& l : lines) {
		std::cout << l[0] << ' ' << l[1] << ' ' << l[2] << ' ' << l[3] << std::endl;
		line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
	}
	return dst;
}

double pdistance(const std::pair<int, int>& a, const std::pair<int, int>& b)
{
	int delta_x = b.first - a.first;
	int delta_y = b.second - a.second;
	return std::sqrt(delta_x * delta_x + delta_y * delta_y);
}

void clear_bad_lines(std::vector<Vec4i>& lines)
{
	constexpr double max_dist = 10;
	for (unsigned i = 0; i < lines.size();) {
		bool ok1 = false;
		bool ok2 = false;
		auto& l = lines[i];
		for (auto& p : ok_points) {
			if (pdistance(p, {l[0], l[1]}) < max_dist)
				ok1 = true;
			if (pdistance(p, {l[2], l[3]}) < max_dist)
				ok2 = true;
			if (ok1 && ok2)
				break;
		}
		if (!ok1 || !ok2) {
			std::cout << "BAD " << l[0] << ' ' << l[1] << ' ' << l[2] << ' ' << l[3] << std::endl;
			lines.erase(lines.begin() + i);
		} else {
			++i;
		}
	}
}

constexpr char original_window_name[] = "original";
constexpr char modified_window_name[] = "modified";
constexpr char thinned_window_name[] = "thinned";

int main(int argc, char** argv)
{
	VideoCapture cap;

	if(!cap.open(0))
		return 1;

	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_FPS, 8);
	
	namedWindow(original_window_name, WINDOW_AUTOSIZE);
	namedWindow(modified_window_name, WINDOW_AUTOSIZE);
	namedWindow(thinned_window_name, WINDOW_AUTOSIZE);

#if 0
	createTrackbar("Canny Threshold:", modified_window_name,
		&process_threshold, 255, nullptr);
#endif

	setMouseCallback(original_window_name, mouse_clicked, nullptr);
	setMouseCallback(thinned_window_name, remove_points, nullptr);

	long fc = 0;
	std::vector<Vec4i> lines;
	Mat thinned;
	Mat processed_bak;
	while (1) {
		Mat frame;
		cap >> frame;

		if (frame.empty())
			break;
		++fc;

		Mat processed = process_image(frame);

		imshow(original_window_name, frame);
		imshow(modified_window_name, processed);

		int key = waitKey(1);
		bool if_break = false;
		if (key > 1048576)
			key -= 1048576;
		if (key != -1)
			std::cout << key << std::endl;

		switch (key) {
		case 27:
			if_break = true;
			break;
		case 'c':
			save_image(frame, "_original");
			save_image(processed, "_processed");
			break;
		case ',':
			++thres;
			if (thres > 100)
				thres = 100;
			std::cout << thres << std::endl;
			break;
		case '.':
			--thres;
			if (thres < 0)
				thres = 0;
			std::cout << thres << std::endl;
			break;
		case 'f':
			ok_points.clear();
			std::cout << "OK Points cleared" << std::endl;
			break;
		case 't': {
			int passes = 3;
			thinning(processed, thinned, passes);
			refresh_hough = true;
			lines = hough_lines(thinned);
			processed_bak = processed.clone();
			break;
			}
		case 'o':
			refresh_hough = true;
			break;
		default:;
		}
		if (if_break)
			break;
		if (refresh_hough) {
			refresh_hough = false;
			if (!ok_points.empty())
				clear_bad_lines(lines);
			Mat hough_src;
			cvtColor(processed_bak, hough_src, COLOR_GRAY2BGR);
			auto hough = draw_lines(hough_src, lines);
			imshow(thinned_window_name, hough);
		}
	}

	//cap.close();
	return 0;
}
