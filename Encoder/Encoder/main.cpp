#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stack>
#include <cmath>

using cv::Mat;
using cv::Point3_;
using std::pair;
using std::stack;

Mat img;
int maxDistance = 15;
int** labels;

double countDist(Point3_<uchar>* p1, Point3_<uchar>* p2, int mode)
{
	switch (mode) {
	case 0:
		return	std::sqrt(
			(p1->x - p2->x) * (p1->x - p2->x) +
			(p1->y - p2->y) * (p1->y - p2->y) +
			(p1->z - p2->z) * (p1->z - p2->z)
			);
	case 1:
		return	std::abs(p1->x - p2->x) +
			std::abs(p1->y - p2->y) +
			std::abs(p1->z - p2->z);
	}
}

void colorMap(int, void*)
{
	stack<pair<int, int>> stack;
	int curlab = 1;

	labels = new int*[img.rows];
	for (int i = 0; i < img.rows; ++i) {
		labels[i] = new int[img.cols];
		for (int j = 0; j < img.rows; ++j) {
			labels[i][j] = 0;
		}
	}

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			if (labels[i][j] == 0) {
				stack.push(std::make_pair(i, j));
				while (!stack.empty()) {
					pair<int, int> pos = stack.top(); stack.pop();
					// std::cout << pos.first << " " << pos.second << std::endl;
					Point3_<uchar>* pixel = img.ptr<Point3_<uchar>>(pos.first, pos.second);
					labels[pos.first][pos.second] = curlab;

					Point3_<uchar>* temp;

					if (pos.first - 1 >= 0 &&
						labels[pos.first - 1][pos.second] == 0) 
					{
						temp = img.ptr<Point3_<uchar>>(pos.first - 1, pos.second);
						if (countDist(pixel,temp,1) <= maxDistance)
							stack.push(std::make_pair(pos.first - 1,pos.second));
					}

					if (pos.first + 1 < img.rows &&
						labels[pos.first + 1][pos.second] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first + 1, pos.second);
						if (countDist(pixel, temp, 1) <= maxDistance)
							stack.push(std::make_pair(pos.first + 1, pos.second));
					}

					if (pos.second - 1 >= 0 &&
						labels[pos.first][pos.second - 1] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first, pos.second - 1);
						if (countDist(pixel, temp, 1) <= maxDistance)
							stack.push(std::make_pair(pos.first, pos.second - 1));
					}

					if (pos.second + 1 < img.cols &&
						labels[pos.first][pos.second + 1] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first, pos.second + 1);
						if (countDist(pixel, temp, 1) <= maxDistance)
							stack.push(std::make_pair(pos.first, pos.second + 1));
					}
				}
				curlab++;
			}
		}
	}

	Mat img_seg(img.rows, img.cols, img.type());

	for (int i = 0; i < img_seg.rows; i++) {
		for (int j = 0; j < img_seg.cols; j++) {
			Point3_<uchar>* p = img_seg.ptr<Point3_<uchar> >(i, j);
			p->x = labels[i][j] * 25 % 255;
			p->y = labels[i][j] * 100 % 255;
			p->z = labels[i][j] * 180 % 255;
		}
	}

	cv::namedWindow("Segmentated", CV_WINDOW_AUTOSIZE);
	cv::imshow("Segmentated", img_seg);
}


int main(int argc, char** argv)
{
	if (argc != 2)
	{
		return -1;
	}

	img = cv::imread(argv[1], cv::IMREAD_COLOR); // Read the file

	if (!img.data) // Check for invalid input
	{
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	cv::namedWindow("Source", cv::WINDOW_AUTOSIZE);
	cv::imshow("Source", img);

	cv::createTrackbar(" Max Distance:", "Source", &maxDistance, 50, colorMap);
	colorMap(0, 0);

	cv::waitKey(0); // Wait for a keystroke in the window
	return 0;
}