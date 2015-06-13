#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <cmath>
#include "imgproc.h"
#include "regionprops.h"

using cv::Mat;
using cv::Point3_;
using std::pair;
using std::stack;
using std::vector;

Mat img;
int maxDistance = 40;
vector< vector<int> > labels;

vector<RegionProps> props;

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		RegionProps& prop = props[labels[y][x]];
		std::cout << "label : " << labels[y][x] << std::endl;
		std::cout << "r_sums : " << prop.r_sums << std::endl;
		std::cout << "g_sums : " << prop.g_sums << std::endl;
		std::cout << "b_sums : " << prop.b_sums << std::endl;
		std::cout << "r_dist : " << prop.r_dist << std::endl;
		std::cout << "g_dist : " << prop.g_dist << std::endl;
		std::cout << "b_dist : " << prop.b_dist << std::endl;
		std::cout << "n : " << prop.n << std::endl << std::endl;
	}
}

void doSegmentation(int, void*)
{
	Mat img_seg(img.rows, img.cols, img.type());
	colorMapSegmentation(img, img_seg, labels, props, maxDistance);

	cv::namedWindow("Segmentated", CV_WINDOW_AUTOSIZE);
	cv::imshow("Segmentated", img_seg);
	cv::setMouseCallback("Segmentated", mouseCallback, NULL);
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

	cv::createTrackbar(" Max Distance:", "Source", &maxDistance, 100, doSegmentation);
	doSegmentation(0, 0);

	cv::waitKey(0); // Wait for a keystroke in the window
	return 0;
}
