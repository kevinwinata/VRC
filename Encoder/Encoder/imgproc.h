#ifndef IMGPROC_H
#define IMGPROC_H

#include <opencv2/core/core.hpp>
#include "regionprops.h"
#include "potrace\potracelib.h"
#include <map>

using cv::Mat;
using std::vector;
using std::map;

void colorMapSegmentation(Mat& img, Mat& img_seg, vector<vector<int> >& labels, vector<RegionProps>& props, map<long, potrace_path_t>& segments, int maxDistance);

void drawVector(Mat& img, potrace_path_t* p);

#endif
