#include "regionprops.h"
#include <iostream>

using std::stringstream;

RegionProps::RegionProps() {
	r_sums = 0;
	g_sums = 0;
	b_sums = 0;
	r_dist = 0;
	g_dist = 0;
	b_dist = 0;
	n = 0;
}

void RegionProps::addPixel(Point3_<uchar>& pixel) {
	r_sums += pixel.z;
	g_sums += pixel.y;
	b_sums += pixel.x;
	n++;
}

void RegionProps::addDistance(Point3_<uchar>& pixel1, Point3_<uchar>& pixel2) {
	r_dist += std::abs(pixel1.z - pixel2.z);
	g_dist += std::abs(pixel1.y - pixel2.y);
	b_dist += std::abs(pixel1.x - pixel2.x);
}

string RegionProps::getAvgColor() {
	int r = static_cast<int>(std::round(r_sums / n));
	int g = static_cast<int>(std::round(g_sums / n));
	int b = static_cast<int>(std::round(b_sums / n));

	stringstream s;
	s << "#" << std::hex << r << g << b;
	return s.str();
}

int RegionProps::countDistToAvg(Point3_<uchar>& pixel) {
	return	std::abs(r_sums / n - pixel.z) +
			std::abs(g_sums / n - pixel.y) +
			std::abs(b_sums / n - pixel.x);
}

void RegionProps::printProps() {
	std::cout << "r_sums : " << r_sums << std::endl;
	std::cout << "g_sums : " << g_sums << std::endl;
	std::cout << "b_sums : " << b_sums << std::endl;
	std::cout << "r_dist : " << r_dist << std::endl;
	std::cout << "g_dist : " << g_dist << std::endl;
	std::cout << "b_dist : " << b_dist << std::endl;
	std::cout << "n : " << n << std::endl;
	std::cout << "color : " << getAvgColor() << std::endl;
}