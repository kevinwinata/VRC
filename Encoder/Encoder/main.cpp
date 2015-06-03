#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <cmath>

using cv::Mat;
using cv::Point3_;
using std::pair;
using std::stack;
using std::vector;

Mat img;
int maxDistance = 40;
int** labels;

class RegionProps
{
	public:
		long r_sums;
		long g_sums;
		long b_sums;
		long r_dist;
		long g_dist;
		long b_dist;
		long n;

		RegionProps();
};

RegionProps::RegionProps() {
	r_sums = 0;
	g_sums = 0;
	b_sums = 0;
	r_dist = 0;
	g_dist = 0;
	b_dist = 0;
	n = 0;
}

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

void colorMap(int, void*)
{
	stack<pair<int, int>> stack;
	long curlab = 0;
	props.clear();
	RegionProps first;
	props.push_back(first);

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
				RegionProps curprop;
				props.push_back(curprop);
				RegionProps& prop = props.back();
				curlab++;

				while (!stack.empty()) {
					pair<int, int> pos = stack.top(); stack.pop();
					Point3_<uchar>* pixel = img.ptr<Point3_<uchar>>(pos.first, pos.second);
					labels[pos.first][pos.second] = curlab;

					prop.r_sums += pixel->z;
					prop.g_sums += pixel->y;
					prop.b_sums += pixel->x;
					prop.n++;

					Point3_<uchar>* temp;

					if (pos.first - 1 >= 0 &&
						labels[pos.first - 1][pos.second] == 0) 
					{
						temp = img.ptr<Point3_<uchar>>(pos.first - 1, pos.second);
						int dist = std::abs(prop.r_sums / prop.n - temp->z) +
							std::abs(prop.g_sums / prop.n - temp->y) +
							std::abs(prop.b_sums / prop.n - temp->x);
						if (dist <= maxDistance) {
							stack.push(std::make_pair(pos.first - 1, pos.second)); 
							prop.r_dist += std::abs(pixel->z - temp->z);
							prop.g_dist += std::abs(pixel->y - temp->y);
							prop.b_dist += std::abs(pixel->x - temp->x);
						}
					}

					if (pos.first + 1 < img.rows &&
						labels[pos.first + 1][pos.second] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first + 1, pos.second);
						int dist = std::abs(prop.r_sums / prop.n - temp->z) +
							std::abs(prop.g_sums / prop.n - temp->y) +
							std::abs(prop.b_sums / prop.n - temp->x);
						if (dist <= maxDistance) {
							stack.push(std::make_pair(pos.first + 1, pos.second));
							prop.r_dist += std::abs(pixel->z - temp->z);
							prop.g_dist += std::abs(pixel->y - temp->y);
							prop.b_dist += std::abs(pixel->x - temp->x);
						}
					}

					if (pos.second - 1 >= 0 &&
						labels[pos.first][pos.second - 1] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first, pos.second - 1);
						int dist = std::abs(prop.r_sums / prop.n - temp->z) +
							std::abs(prop.g_sums / prop.n - temp->y) +
							std::abs(prop.b_sums / prop.n - temp->x);
						if (dist <= maxDistance) {
							stack.push(std::make_pair(pos.first, pos.second - 1));
							prop.r_dist += std::abs(pixel->z - temp->z);
							prop.g_dist += std::abs(pixel->y - temp->y);
							prop.b_dist += std::abs(pixel->x - temp->x);
						}
					}

					if (pos.second + 1 < img.cols &&
						labels[pos.first][pos.second + 1] == 0)
					{
						temp = img.ptr<Point3_<uchar>>(pos.first, pos.second + 1);
						int dist = std::abs(prop.r_sums / prop.n - temp->z) +
							std::abs(prop.g_sums / prop.n - temp->y) +
							std::abs(prop.b_sums / prop.n - temp->x);
						if (dist <= maxDistance) {
							stack.push(std::make_pair(pos.first, pos.second + 1));
							prop.r_dist += std::abs(pixel->z - temp->z);
							prop.g_dist += std::abs(pixel->y - temp->y);
							prop.b_dist += std::abs(pixel->x - temp->x);
						}
					}
				}
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

	cv::createTrackbar(" Max Distance:", "Source", &maxDistance, 100, colorMap);
	colorMap(0, 0);

	cv::waitKey(0); // Wait for a keystroke in the window
	return 0;
}