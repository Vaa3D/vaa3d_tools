/**@file tz_image_array.h
 * @brief utilities for image array
 * @author Ting Zhao
 * @date 19-Feb-2008
 */

#ifndef _TZ_IMAGE_ARRAY_H_
#define _TZ_IMAGE_ARRAY_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#include <image_lib.h>

/*
 * Image_Array_Max_Index() returns the index of the maximum value in the first
 * <length> elements of <array>, whose element has type <kind>. 
 *
 * Image_Array_Max_Index_M() does the same thing but the elements are checked
 * only when the corresponding element in the binary array <mask> has value 1.
 * If <mask> is NULL, it is the same as Image_Array_Max_Index(). It returns 
 * INVALID_ARRAY_INDEX if no element is available.
 */
size_t Image_Array_Max_Index(const void *array, int kind, size_t length);
size_t Image_Array_Max_Index_M(const void *array, int kind, size_t length, 
			    const uint8 *mask);

/*
 * Image_Array_Min_Index() and Image_Array_Min_Index_M() are functions for
 * finding the index of the minimal number.
 */
size_t Image_Array_Min_Index(const void *array, int kind, size_t length);
size_t Image_Array_Min_Index_M(const void *array, int kind, size_t length, 
			    const uint8 *mask);

/*
 * Image_Array_Minmax_Index() finds the indices for the minimum and maximum
 * in the first <length> elements of <array> and stores them in <index>.
 * 
 * Image_Array_Minmax_Index_M is the masked version of 
 * Image_Array_Minmax_Index().
 */
void Image_Array_Minmax_Index(const void *array, int kind, size_t length, 
			      size_t *index);
void Image_Array_Minmax_Index_M(const void *array, int kind, size_t length,
				const uint8 *mask, size_t *index);

/*
 * Image_Array_Hist() creates a histogram for <array>. The result is stored
 * in <hist> and the returned pointer is the same as <hist> if <hist> is not
 * NULL. Otherwise it returns a new pointer.
 *
 * Image_Array_Hist_M() is the masked version of Image_Array_Hist().
 *
 * Note: <kind> must be GREY or GREY16 for these two functions.
 */
int* Image_Array_Hist(const void *array, int kind, size_t length, int hist[]);
int* Image_Array_Hist_M(const void *array, int kind, size_t length, 
			const uint8* mask, int hist[]);

double Image_Array_Sum(const void *array, int kind, size_t length);
double Image_Array_Mean(const void *array, int kind, size_t length);

__END_DECLS

#endif 
