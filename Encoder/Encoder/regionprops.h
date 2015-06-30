#ifndef REGIONPROPS_H
#define REGIONPROPS_H

#include <opencv2/core/core.hpp>

using cv::Point3_;

class RegionProps
{
private:
	long r_sums;
	long g_sums;
	long b_sums;
	long r_dist;
	long g_dist;
	long b_dist;
	long n;

public:
	RegionProps();
	void addPixel(Point3_<uchar>& pixel);
	void addDistance(Point3_<uchar>& pixel1, Point3_<uchar>& pixel2);
	int countDistToAvg(Point3_<uchar>& pixel);
	void printProps();
};

#endif
