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
vector< vector<int> > labels;
vector<RegionProps> props;
vector<potrace_bitmap_t> segments;

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
		std::cout << "n : " << prop.n << std::endl;

		potrace_bitmap_t *bm = &segments[labels[y][x]];
		potrace_param_t *param = potrace_param_default();
		potrace_state_t *st = potrace_trace(param, bm);
		potrace_path_t *p = st->plist;
		printf("%%!PS-Adobe-3.0 EPSF-3.0\n");
		printf("%%%%BoundingBox: 0 0 %d %d\n", img.cols, img.rows);
		printf("gsave\n");

		/* draw each curve */
		potrace_dpoint_t(*c)[3];
		p = st->plist;
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
				printf("0 setgray fill\n");
			}
			p = p->next;
		}
		potrace_state_free(st);
		potrace_param_free(param);

		std::cout << std::endl;
	}
}

void doSegmentation(int, void*)
{
	Mat img_seg(img.rows, img.cols, img.type());
	colorMapSegmentation(img, img_seg, labels, props, segments, maxDistance);

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
