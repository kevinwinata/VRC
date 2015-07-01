#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <stack>
#include <array>
#include "imgproc.h"
#include "potrace\bitmap.h"

using cv::Point3_;
using cv::Point;
using cv::Vec3b;
using cv::Ptr;
using std::pair;
using std::stack;
using std::array;
using std::ofstream;

void colorMapSegmentation(Mat& img, vector<vector<int>>& labels, vector<RegionProps>& props, map<long, potrace_path_t>& segments, int maxDistance)
{
	stack<pair<int, int>> stack;
	long curlab = 0;
	props.clear();

	potrace_param_t *param = potrace_param_default();
	potrace_state_t *st;
	potrace_bitmap_t *obm;
	obm = bm_new(img.cols, img.rows);
	bm_clear(obm, 0);

	labels.resize(img.rows, vector<int>(img.cols, 0));

	array<pair<int, int>, 4> dir;
	dir[0] = std::make_pair(-1, 0);
	dir[1] = std::make_pair(1, 0);
	dir[2] = std::make_pair(0, -1);
	dir[3] = std::make_pair(0, 1);

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
					Point3_<uchar>* pixel = img.ptr<Point3_<uchar>>(pos.first, pos.second);
					labels[pos.first][pos.second] = curlab;
					BM_PUT(bm, pos.second, pos.first, 1);

					prop.addPixel(*pixel);

					Point3_<uchar>* temp;

					for (pair<int,int> p : dir) {
						int ypos = pos.first + p.first;
						int xpos = pos.second + p.second;
						if (ypos >= 0 && ypos < img.rows &&
							xpos >= 0 && xpos < img.cols &&
							labels[ypos][xpos] == 0)
						{
							temp = img.ptr<Point3_<uchar>>(ypos, xpos);
							int dist = prop.countDistToAvg(*temp);
							if (dist <= maxDistance) {
								stack.push(std::make_pair(ypos, xpos));
								prop.addDistance(*pixel, *temp);
							}
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

	std::cout << curlab << std::endl;
	std::cout << segments.size() << std::endl;
}

void drawSegments(Mat& img_seg, vector<vector<int>>& labels)
{
	for (int i = 0; i < img_seg.rows; i++) {
		for (int j = 0; j < img_seg.cols; j++) {
			Point3_<uchar>* p = img_seg.ptr<Point3_<uchar>>(i, j);
			p->x = labels[i][j] * 25 % 255;
			p->y = labels[i][j] * 100 % 255;
			p->z = labels[i][j] * 180 % 255;
		}
	}
}

void fillVector(Mat& img, int i, int j, int label, vector<vector<int>>& labels)
{
	array<pair<int, int>, 4> dir;
	dir[0] = std::make_pair(-1, 0);
	dir[1] = std::make_pair(1, 0);
	dir[2] = std::make_pair(0, -1);
	dir[3] = std::make_pair(0, 1);

	stack<pair<int, int>> stack;
	stack.push(std::make_pair(i, j));

	while (!stack.empty()) {
		pair<int, int> pos = stack.top(); stack.pop();
		labels[pos.first][pos.second] = label;
		uchar* p = img.ptr<uchar>(pos.first, pos.second);
		*p = 122;

		for (pair<int, int> p : dir) {
			int ypos = pos.first + p.first;
			int xpos = pos.second + p.second;
			if (ypos >= 0 && ypos < img.rows && xpos >= 0 && xpos < img.cols) {
				uchar* px = img.ptr<uchar>(ypos, xpos);
				if (labels[ypos][xpos] != label && *px == 0) {
					stack.push(std::make_pair(ypos, xpos));
				}
				else if (*px == 255) {
					labels[ypos][xpos] = label;
				}
			}
		}
	}

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			if (labels[i][j] == label) {
				uchar* p = img.ptr<uchar>(i, j);
				*p = 100;
			}
		}
	}
}

void drawVector(Mat& img, potrace_path_t* p)
{
	potrace_dpoint_t(*c)[3];
	Point prevPoint;
	while (p != NULL) {
		int n = p->curve.n;
		int* tag = p->curve.tag;
		c = p->curve.c;
		int cx = static_cast<int>(std::round(c[n - 1][2].x));
		int cy = static_cast<int>(std::round(c[n - 1][2].y));
		prevPoint = Point(cx, cy);

		for (int i = 0; i < n; i++) {
			int cx0 = static_cast<int>(std::round(c[i][0].x));
			int cy0 = static_cast<int>(std::round(c[i][0].y));
			int cx1 = static_cast<int>(std::round(c[i][1].x));
			int cy1 = static_cast<int>(std::round(c[i][1].y));
			int cx2 = static_cast<int>(std::round(c[i][2].x));
			int cy2 = static_cast<int>(std::round(c[i][2].y));

			switch (tag[i]) {
			case POTRACE_CORNER:
				cv::line(img,
					prevPoint,
					Point(cx1, cy1),
					255, 1, 8, 0);
				cv::line(img,
					Point(cx1, cy1),
					Point(cx2, cy2),
					255, 1, 8, 0);
				break;
			case POTRACE_CURVETO:
				Point points[1][5];
				points[0][0] = prevPoint;
				points[0][1] = Point(cx0, cy0);
				points[0][2] = Point(cx1, cy1);
				points[0][3] = Point(cx2, cy2);
				const Point* ppt[1] = { points[0] };
				int npt[] = { 4 };

				cv::polylines(img, ppt, npt, 4, true, 255, 1, 8, 0);
				break;
			}
			prevPoint = Point(cx2, cy2);
		}
		p = p->next;
	}
}

void writeVector(string filename, map<long, potrace_path_t>& segments, vector<RegionProps>& props, int width, int height)
{
	ofstream file;
	file.open(filename);

	file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	file << "<!DOCTYPE svg PUBLIC \" -//W3C//DTD SVG 1.1//EN\" ";
	file << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";

	file << "<svg version=\"1.1\"  xmlns=\"http://www.w3.org/2000/svg\" ";
	file << "width=\"" << width << "px\" ";
	file << "height = \"" << height << "\" ";
	file << "viewBox=\"0 0 " << width << " " << height << "\" xml:space=\"preserve\">\n";

	potrace_dpoint_t(*c)[3];

	for (auto& seg : segments) {
		string color = props[seg.first].getAvgColor();
		potrace_path_t *p = &seg.second;

		file << "<g>\n";
		file << "<path d=\"";
		while (p != NULL) {
			int n = p->curve.n;
			int* tag = p->curve.tag;
			c = p->curve.c;
			file << "M " << c[n - 1][2].x << " " << c[n - 1][2].y << " ";
			//printf("%f %f moveto\n", c[n - 1][2].x, c[n - 1][2].y);
			for (int i = 0; i<n; i++) {
				switch (tag[i]) {
				case POTRACE_CORNER:
					file << "L " << c[i][1].x << " " << c[i][1].y << " ";
					//printf("%f %f lineto\n", c[i][1].x, c[i][1].y);
					file << "L " << c[i][2].x << " " << c[i][2].y << " ";
					//printf("%f %f lineto\n", c[i][2].x, c[i][2].y);
					break;
				case POTRACE_CURVETO:
					file << "C " << 
						c[i][0].x << " " << c[i][0].y << " " <<
						c[i][1].x << " " << c[i][1].y << " " <<
						c[i][2].x << " " << c[i][2].y << " ";
					/*printf("%f %f %f %f %f %f curveto\n",
						c[i][0].x, c[i][0].y,
						c[i][1].x, c[i][1].y,
						c[i][2].x, c[i][2].y);*/
					break;
				}
			}
			if (p->next == NULL || p->next->sign == '+') {
				file << "\" fill=\"" << color << "\" stroke=\"none\"/>\n";
			}
			p = p->next;
		}
		//file << "\" fill=\"" << color << "\" stroke=\"none\"/>\n";
		file << "</g>\n";
	}
	file << "</svg>";
	file.close();
}
