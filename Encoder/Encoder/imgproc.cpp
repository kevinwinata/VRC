#include "imgproc.h"
#include "potrace\bitmap.h"
#include <stack>

using cv::Point3_;
using std::pair;
using std::stack;

#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

void colorMapSegmentation(Mat& img, Mat& img_seg, vector<vector<int> >& labels, vector<RegionProps>& props, vector<potrace_bitmap_t>& segments, int maxDistance)
{
	stack<pair<int, int> > stack;
	long curlab = 0;
	props.clear();
	RegionProps first;
	props.push_back(first);

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
				segments.push_back(*bm);

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
