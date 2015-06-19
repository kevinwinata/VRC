#include <opencv2/imgproc.hpp>
#include "imgproc.h"
#include "potrace\bitmap.h"
#include <stack>

using cv::Point3_;
using cv::Point;
using std::pair;
using std::stack;
using std::round;

void colorMapSegmentation(Mat& img, Mat& img_seg, vector<vector<int> >& labels, vector<RegionProps>& props, map<long, potrace_path_t>& segments, int maxDistance)
{
	stack<pair<int, int> > stack;
	long curlab = 0;
	props.clear();

	potrace_param_t *param = potrace_param_default();
	potrace_state_t *st;
	potrace_bitmap_t *obm;
	obm = bm_new(img.cols, img.rows);
	bm_clear(obm, 0);

	labels.resize(img.rows, vector<int>(img.cols, 0));

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			if (labels[i][j] == 0) {

				stack.push(std::make_pair(i, j));

				RegionProps curprop;
				props.push_back(curprop);
				RegionProps& prop = props.back();

				potrace_bitmap_t *bm = bm_dup(obm);

				curlab++;

				while (!stack.empty()) {
					pair<int, int> pos = stack.top(); stack.pop();
					Point3_<uchar>* pixel = img.ptr<Point3_<uchar> >(pos.first, pos.second);
					labels[pos.first][pos.second] = curlab;
					BM_PUT(bm, pos.second, pos.first, 1);

					prop.r_sums += pixel->z;
					prop.g_sums += pixel->y;
					prop.b_sums += pixel->x;
					prop.n++;

					Point3_<uchar>* temp;

					if (pos.first - 1 >= 0 &&
						labels[pos.first - 1][pos.second] == 0)
					{
						temp = img.ptr<Point3_<uchar> >(pos.first - 1, pos.second);
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
						temp = img.ptr<Point3_<uchar> >(pos.first + 1, pos.second);
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
						temp = img.ptr<Point3_<uchar> >(pos.first, pos.second - 1);
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
						temp = img.ptr<Point3_<uchar> >(pos.first, pos.second + 1);
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

				st = potrace_trace(param, bm);
				if (st && st->plist && st->status == POTRACE_STATUS_OK) {
					segments[curlab] = *(st->plist);
				}
				
				bm_free(bm);
			}
		}
	}

	for (int i = 0; i < img_seg.rows; i++) {
		for (int j = 0; j < img_seg.cols; j++) {
			Point3_<uchar>* p = img_seg.ptr<Point3_<uchar> >(i, j);
			p->x = labels[i][j] * 25 % 255;
			p->y = labels[i][j] * 100 % 255;
			p->z = labels[i][j] * 180 % 255;
		}
	}

}

void drawVector(Mat& img, potrace_path_t* p)
{
	potrace_dpoint_t(*c)[3];
	while (p != NULL) {
		int n = p->curve.n;
		int* tag = p->curve.tag;
		c = p->curve.c;
		Point prevPoint = Point(c[n - 1][2].x, c[n - 1][2].y);
		for (int i = 0; i < n; i++) {
			switch (tag[i]) {
			case POTRACE_CORNER:
				cv::line(img,
					prevPoint,
					Point(c[i][1].x, c[i][1].y),
					cv::Scalar(0, 0, 0), 1, 8, 0);
				cv::line(img,
					Point(c[i][1].x, c[i][1].y),
					Point(c[i][2].x, c[i][2].y),
					cv::Scalar(0, 0, 0), 1, 8, 0);
				break;
			case POTRACE_CURVETO:
				Point points[1][5];
				points[0][0] = prevPoint;
				points[0][1] = Point(c[i][0].x, c[i][0].y);
				points[0][2] = Point(c[i][1].x, c[i][1].y);
				points[0][3] = Point(c[i][2].x, c[i][2].y);
				const Point* ppt[1] = { points[0] };
				int npt[] = { 4 };

				cv::polylines(img, ppt, npt, 4, true, cv::Scalar(0, 0, 0), 1, 8, 0);
				break;
			}
			prevPoint = Point(c[i][2].x, c[i][2].y);
		}
		p = p->next;
	}
}
