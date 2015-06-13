#ifndef IMGPROC_H
#define IMGPROC_H

#include <opencv2/core/core.hpp>
#include "regionprops.h"

using cv::Mat;
using std::vector;

void colorMapSegmentation(Mat& img, Mat& img_seg, vector< vector<int> >& labels, vector<RegionProps>& props, int maxDistance);

#endif
