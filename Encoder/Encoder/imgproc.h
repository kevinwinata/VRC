#ifndef IMGPROC_H
#define IMGPROC_H

#include <opencv2/core/core.hpp>
#include "regionprops.h"
#include "potrace\potracelib.h"
#include <map>

using cv::Mat;
using std::vector;
using std::map;

void colorMapSegmentation(Mat& img, vector<vector<int>>& labels, vector<RegionProps>& props, map<long, potrace_path_t>& segments, int maxDistance);

void drawSegments(Mat& img_seg, vector<vector<int>>& labels);

void drawVector(Mat& img, potrace_path_t* p);

void fillVector(Mat& img, int i, int j, int label, vector<vector<int>>& labels);

#endif
