/**file tz_math_lib.h
 * @author Ting Zhao
 */

#ifndef _TZ_MATH_LIB_H_
#define _TZ_MATH_LIB_H_

/**@brief Obsolete routines*/

#ifdef _MSC_VER
#define __typeof__ decltype
#endif

#define ARRAY_MAX(array,length,max_value)			\
  {							\
    __typeof__(length) iloop;				\
    max_value = array[0];				\
    for(iloop=0;iloop<length;iloop++) {			\
      if(max_value<array[iloop])			\
	max_value = array[iloop];			\
    }							\
  }

#define ARRAY_MIN(array,length,min_value)			\
  {							\
    __typeof__(length) iloop;				\
    min_value = array[0];				\
    for(iloop=0;iloop<length;iloop++) {			\
      if(min_value>array[iloop])			\
	min_value = array[iloop];			\
    }							\
  }

/**@addtogroup max_min_ find max or min
 * @brief Find min or max from an array.
 * @{
 */
int minint(int *array,long length);
int maxint(int *array,long length);
double mindouble(double *array,long length);
double maxdouble(double *array,long length);
float minfloat(float *array,long length);
float maxfloat(float *array,long length);
long minlong(long *array,long length);
long maxlong(long *array,long length);

/**@}*/

/*
 * signint() returns 1 if x is positive, -1 if x is negative, or 0 otherwise.
 */
int signint(int x);

/*
 * switchint() switches x and y.
 */
void switchint(int *x, int *y);

#endif
