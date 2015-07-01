#ifndef IMGPROC_H
#define IMGPROC_H

#include <opencv2/core/core.hpp>
#include <map>
#include "regionprops.h"
#include "potrace\potracelib.h"

using cv::Mat;
using std::vector;
using std::map;
using std::string;

void colorMapSegmentation(Mat& img, vector<vector<int>>& labels, vector<RegionProps>& props, map<long, potrace_path_t>& segments, int maxDistance);

void drawSegments(Mat& img_seg, vector<vector<int>>& labels);

void drawVector(Mat& img, potrace_path_t* p);

void fillVector(Mat& img, int i, int j, int label, vector<vector<int>>& labels);

void writeVector(string filename, map<long, potrace_path_t>& segments, vector<RegionProps>& props, int width, int height);

#endif
