/**@file tz_int_histogram.h
 * @brief Histogram of integers
 * @author Ting Zhao
 * @date 20-Feb-2008
 */

#ifndef _TZ_INT_HISTOGRAM_H_
#define _TZ_INT_HISTOGRAM_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/*
 * Int histogram data struture description:
 *
 * It's an integer array, which could be defined as
 * int *hist;
 * hist[0] is the length of the histogram (not the length of the hist array)
 * and hist[1] is the starting value of the histogram. So hist[2] is the count
 * of the value hist[1] and hist[i] (i >= 2) is the count of hist[1] + i - 2.
 * The total length of the array is hist[0] + 2. The range of the histogram
 * is [ hist[1], hist[0] + hist[1] - 1 ].
 */

#define INT_HISTOGRAM_MAX_COUNT 2147483647

/*
 * Int_Histogram_Length() returns the length of the histogram <hist>, not the
 * length of the whole array.
 */
int Int_Histogram_Length(const int *hist);

int Int_Histogram_Min(const int *hist);
int Int_Histogram_Max(const int *hist);

int Int_Histogram_Quantile(const int *hist, double q);

int* Int_Histogram_Array(int *hist);
const int* Int_Histogram_Const_Array(const int *hist);

/*
 * Print_Int_Histogram() prints a histogram to the screen.
 */
void Print_Int_Histogram(const int *hist);


/*
 * Int_Histogram_Range() calculates the range of <hist> and stores the result
 * in <low> and <high>. If <low> or <high> is NULL, it will be ignored.
 */
void Int_Histogram_Range(const int *hist, int *low, int *high);

/*
 * Int_Histogram_Slope() returns the slope of <hist> in the range [low, high].
 * Here the slope is defined as the slope of the line passing through the first
 * maximum and the first minimum of <hist> in the defined range.
 */
double Int_Histogram_Slope(const int *hist, int low, int high);

/*
 * Int_Histogram_Mode() returns the most common value within the range <low>
 * and <high> in a histogram. Set <low> to hist[1] and <high> to
 * hist[0] + hist[1] - 1 for full range search.
 */
int Int_Histogram_Mode(const int *hist, int low, int high);

/*
 * Int_Histogram_Diff() calculates the differentian of <hist>, which is 
 * y[i] = x[i+1] - x[i]. The result is stored in <diff> and the returned 
 * pointer is also <diff> if <diff> is not NULL. Otherwise a new pointer is 
 * created and returned.
 */
int* Int_Histogram_Diff(const int *hist, int *diff);

/*
 * Int_Histogram_Triangle_Threshold() returns the threshold from the Triangle
 * algorithm int the range [<low>, <high>]. Set <low> to hist[1] and <high> to
 * hist[0] + hist[1] - 1 for full range search.
 */
int Int_Histogram_Triangle_Threshold(const int *hist, int low, int high);
int Int_Histogram_Triangle_Threshold2(const int *hist, int low, int high,
				      double alpha);

/*
 * Int_Histogram_Rc_Threshold() returns the RC threshold in the range 
 * [<low>, <high>]. 
 */
int Int_Histogram_Rc_Threshold(const int *hist, int low, int high);

/*
 * Int_Histogram_Stable_Threshold() returns the first stable point of <hist>.
 * Here a stale point is defined as a point where the left differention is
 * no less than -1 and the right differention is no greater than -1.
 */
int Int_Histogram_Stable_Point(const int *hist, int low, int high);

int* Int_Histogram_Equalize(const int *hist, int min, int max);

int* Int_Histogram_Equal_Map(const int *hist, int nbin, int *map);
int* Int_Histogram_Equal_Info_Map(const int *hist, int nbin, int *map);

__END_DECLS

#endif
