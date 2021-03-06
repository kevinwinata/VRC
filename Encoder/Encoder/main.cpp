#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "imgproc.h"
#include "regionprops.h"
#include "potrace\potracelib.h"

using cv::Mat;
using cv::Point3_;
using std::vector;

Mat img;
int maxDistance = 40;
vector<vector<int>> labels;
vector<RegionProps> props;
map<long, potrace_path_t> segments;

void printRegionProps(int x, int y)
{
	RegionProps& prop = props[labels[y][x] - 1];
	std::cout << "label : " << labels[y][x] << std::endl;
	prop.printProps();
	std::cout << std::endl;
}

void printVectors(int x, int y)
{
	potrace_path_t *p = &segments[labels[y][x]];
	printf("%%!PS-Adobe-3.0 EPSF-3.0\n");
	printf("%%%%BoundingBox: 0 0 %d %d\n", img.cols, img.rows);
	printf("gsave\n");

	/* draw each curve */
	potrace_dpoint_t(*c)[3];
	while (p != NULL) {
		int n = p->curve.n;
		int* tag = p->curve.tag;
		c = p->curve.c;
		printf("%f %f moveto\n", c[n - 1][2].x, c[n - 1][2].y);
		for (int i = 0; i<n; i++) {
			switch (tag[i]) {
			case POTRACE_CORNER:
				printf("%f %f lineto\n", c[i][1].x, c[i][1].y);
				printf("%f %f lineto\n", c[i][2].x, c[i][2].y);
				break;
			case POTRACE_CURVETO:
				printf("%f %f %f %f %f %f curveto\n",
					c[i][0].x, c[i][0].y,
					c[i][1].x, c[i][1].y,
					c[i][2].x, c[i][2].y);
				break;
			}
		}
		/* at the end of a group of a positive path and its negative
		children, fill. */
		if (p->next == NULL || p->next->sign == '+') {
			//printf("0 setgray fill\n");
		}
		p = p->next;
	}

	std::cout << std::endl;
}

void showVectors(int x, int y)
{
	Mat img_vec = Mat::zeros(img.rows, img.cols, CV_8UC1);
	drawVector(img_vec, &segments[labels[y][x]]);

	/*vector<vector<int>> vlabels;
	vlabels.resize(img_vec.rows, vector<int>(img_vec.cols, 0));
	fillVector(img_vec, y, x, 1, vlabels);*/

	cv::imshow("Vectors", img_vec);
}

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		//printRegionProps(x, y);
		//printVectors(x, y);
		//showVectors(x, y);
		writeVector("example.svg", segments, props, img.cols, img.rows);
	}
}

void doSegmentation(int, void*)
{
	colorMapSegmentation(img, labels, props, segments, maxDistance);

	Mat img_seg(img.rows, img.cols, img.type());
	drawSegments(img_seg, labels);

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

	cv::namedWindow("Vectors", cv::WINDOW_AUTOSIZE);

	cv::waitKey(0); // Wait for a keystroke in the window
	return 0;
}
