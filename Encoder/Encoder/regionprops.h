#ifndef REGIONPROPS_H
#define REGIONPROPS_H

class RegionProps
{
public:
	long r_sums;
	long g_sums;
	long b_sums;
	long r_dist;
	long g_dist;
	long b_dist;
	long n;

	RegionProps();
};

inline RegionProps::RegionProps() {
	r_sums = 0;
	g_sums = 0;
	b_sums = 0;
	r_dist = 0;
	g_dist = 0;
	b_dist = 0;
	n = 0;
}

#endif
