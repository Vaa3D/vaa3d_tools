 /* tz_image_array.h
 *
 * 19-Feb-2008 Initial write: Ting Zhao
 */

#include "tz_image_array.h"
#include <stdlib.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#else
#include <memory.h>
#endif
#include "tz_error.h"
#include "tz_image_lib_defs.h"
#include "tz_u8array.h"
#include "tz_u16array.h"
#include "tz_farray.h"
#include "tz_darray.h"
#include "tz_int_histogram.h"

#define DEFINE_IMAGE_ARRAY(prefix, suffix, data_type, array)	\
  data_type *TZ_CONCATU(prefix, suffix) = (data_type *) array;

#define DEFINE_IMAGE_ARRAY_ALL(prefix, array)		\
  DEFINE_IMAGE_ARRAY(prefix, grey, uint8, array)	\
  DEFINE_IMAGE_ARRAY(prefix, grey16, uint16, array)	\
  DEFINE_IMAGE_ARRAY(prefix, float32, float32, array)

#define IMAGE_ARRAY_MAX_INDEX(array, length, index, i)	\
  index = 0;						\
  for (i = 1; i < length; i++) {			\
    if (array[index] < array[i]) {			\
      index = i;					\
    }							\
  }

#define IMAGE_ARRAY_MAX_INDEX_M(array, length, mask, index, i)		\
  if (mask == NULL) {							\
    IMAGE_ARRAY_MAX_INDEX(array, length, index, i);			\
  } else {								\
    for (index = 0; index < length; index++) {				\
      if (mask[index] == 1) {						\
	break;								\
      }									\
    }									\
    if (index < length) {						\
      for (i = index; i < length; i++) {				\
	if (mask[i] == 1) {						\
	  if (array[index] < array[i]) {				\
	    index = i;							\
	  }								\
	}								\
      }									\
    } else {								\
      index = INVALID_ARRAY_INDEX;					\
    }									\
  }

size_t Image_Array_Max_Index(const void *array, int kind, size_t length)
{
  return Image_Array_Max_Index_M(array, kind, length, NULL);
}

size_t Image_Array_Max_Index_M(const void *array, int kind, size_t length, 
			    const uint8 *mask)
{
  size_t index = 0;

  DEFINE_IMAGE_ARRAY_ALL(array, array);
  size_t i;
  switch (kind) {
  case GREY:
    IMAGE_ARRAY_MAX_INDEX_M(array_grey, length, mask, index, i);
    break;
  case GREY16:
    IMAGE_ARRAY_MAX_INDEX_M(array_grey16, length, mask, index, i);
    break;
  case FLOAT32:
    IMAGE_ARRAY_MAX_INDEX_M(array_float32, length, mask, index, i);
    break;
  default:
    fprintf(stderr, "Unsupported image kind");
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return index;
}

#define IMAGE_ARRAY_MIN_INDEX(array, length, index, i)	\
  index = 0;						\
  for (i = 1; i < length; i++) {			\
    if (array[index] > array[i]) {			\
      index = i;					\
    }							\
  }

#define IMAGE_ARRAY_MIN_INDEX_M(array, length, mask, index, i)		\
  if (mask == NULL) {							\
    IMAGE_ARRAY_MIN_INDEX(array, length, index, i);			\
  } else {								\
    for (index = 0; index < length; index++) {				\
      if (mask[index] == 1) {						\
	break;								\
      }									\
    }									\
    if (index < length) {						\
      for (i = index; i < length; i++) {				\
	if (mask[i] == 1) {						\
	  if (array[index] > array[i]) {				\
	    index = i;							\
	  }								\
	}								\
      }									\
    } else {								\
      index = INVALID_ARRAY_INDEX;					\
    }									\
  }

size_t Image_Array_Min_Index(const void *array, int kind, size_t length)
{
  return Image_Array_Min_Index_M(array, kind, length, NULL);
}

size_t Image_Array_Min_Index_M(const void *array, int kind, size_t length, 
			    const uint8 *mask)
{
  size_t index = 0;

  DEFINE_IMAGE_ARRAY_ALL(array, array);
  size_t i;
  switch (kind) {
  case GREY:
    IMAGE_ARRAY_MIN_INDEX_M(array_grey, length, mask, index, i);
    break;
  case GREY16:
    IMAGE_ARRAY_MIN_INDEX_M(array_grey16, length, mask, index, i);
    break;
  case FLOAT32:
    IMAGE_ARRAY_MIN_INDEX_M(array_float32, length, mask, index, i);
    break;
  default:
    fprintf(stderr, "Unsupported image kind");
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return index;
}

#define IMAGE_ARRAY_MINMAX_INDEX(array, length, index, i)		\
  index[0] = index[1] =  0;						\
  for (i = 1; i < length; i++) {					\
    if (array[index[0]] > array[i]) {					\
      index[0] = i;							\
    } else {								\
      if (array[index[1]] < array[i]) {					\
	index[1] = i;							\
      }									\
    }									\
  }

#define IMAGE_ARRAY_MINMAX_INDEX_M(array, length, mask, index, i)	\
  if (mask == NULL) {							\
    IMAGE_ARRAY_MINMAX_INDEX(array, length, index, i);			\
  } else {								\
    for (index[0] = 0; index[0] < length; index[0]++) {			\
      if (mask[index[0]] == 1) {					\
	index[1] = index[0];						\
	break;								\
      }									\
    }									\
    if (index[0] < length) {						\
      for (i = index[0]; i < length; i++) {				\
	if (mask[i] == 1) {						\
	  if (array[index[0]] > array[i]) {				\
	    index[0] = i;						\
	  } else {							\
	    if (array[index[1]] < array[i]) {				\
	      index[1] = i;						\
	    }								\
	  }								\
	}								\
      }									\
    } else {								\
      index[0] = index[1] = INVALID_ARRAY_INDEX;			\
    }									\
  }

void Image_Array_Minmax_Index(const void *array, int kind, size_t length, 
			      size_t *index)
{
  Image_Array_Minmax_Index_M(array, kind, length, NULL, index);
}

void Image_Array_Minmax_Index_M(const void *array, int kind, size_t length,
				const uint8 *mask, size_t *index)
{
  DEFINE_IMAGE_ARRAY_ALL(array, array);
  int i;
  switch (kind) {
  case GREY:
    IMAGE_ARRAY_MINMAX_INDEX_M(array_grey, length, mask, index, i);
    break;
  case GREY16:
    IMAGE_ARRAY_MINMAX_INDEX_M(array_grey16, length, mask, index, i);
    break;
  case FLOAT32:
    IMAGE_ARRAY_MINMAX_INDEX_M(array_float32, length, mask, index, i);
    break;
  default:
    fprintf(stderr, "Unsupported image kind");
    TZ_ERROR(ERROR_DATA_VALUE);
  }
}

#define IMAGE_ARRAY_HIST_M(array, length, mask, hist, range, minmax_index, i) \
  IMAGE_ARRAY_MINMAX_INDEX_M(array, length, mask, minmax_index, i);	\
  if (Is_Valid_Array_Index(minmax_index[0]) == FALSE ||                 \
      Is_Valid_Array_Index(minmax_index[1]) == FALSE) {			\
    return NULL;							\
  }									\
  range = array[minmax_index[1]] - array[minmax_index[0]];		\
  if (hist == NULL) {							\
    hist = (int *) malloc(sizeof(int) * (range + 3));			\
  }									\
  hist[0] = range + 1;							\
  hist[1] = array[minmax_index[0]];					\
  bzero(hist + 2, sizeof(int) * hist[0]);				\
  hist -= array[minmax_index[0]];					\
  hist += 2;								\
  if (mask == NULL) {							\
    for (i = 0; i < length; i++) {					\
      if (hist[array[i]] < INT_HISTOGRAM_MAX_COUNT) {                  \
        hist[array[i]]++;						\
      }                                                                 \
    }									\
  } else {								\
    for (i = 0; i < length; i++) {					\
      if (mask[i] == 1) {						\
        if (hist[array[i]] < INT_HISTOGRAM_MAX_COUNT) {                  \
          hist[array[i]]++;						\
        }                                   \
      }									\
    }									\
  }									\
  hist += array[minmax_index[0]];					\
  hist -= 2;

int* Image_Array_Hist(const void *array, int kind, size_t length, int hist[])
{
  return Image_Array_Hist_M(array, kind, length, NULL, hist);
}


int* Image_Array_Hist_M(const void *array, int kind, size_t length, 
			const uint8* mask, int hist[])
{
  DEFINE_IMAGE_ARRAY_ALL(array, array);

  size_t i;
  int range;
  size_t minmax_index[2];

  switch (kind) {
  case GREY:
    IMAGE_ARRAY_HIST_M(array_grey, length, mask, hist, range, minmax_index, i);
    break;
  case GREY16:
    IMAGE_ARRAY_HIST_M(array_grey16, length, mask, hist, range, minmax_index, 
		       i);
    break;
  case FLOAT32:
    UNUSED_PARAMETER(array_float32);
  default:
    fprintf(stderr, "Unsupported image kind");
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return hist;
}

double Image_Array_Sum(const void *array, int kind, size_t length)
{
  switch (kind) {
  case GREY:
    return u8array_sum_d((const tz_uint8*)array, length);
  case GREY16:
    return u16array_sum_d((const tz_uint16*)array, length);
  case FLOAT32:
    return farray_sum_d((const float*)array, length);
  case FLOAT64:
    return darray_sum_d((const double*)array, length);
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return 0.0;
}

double Image_Array_Mean(const void *array, int kind, size_t length)
{
  switch (kind) {
  case GREY:
    return u8array_mean_d((const tz_uint8*)array, length);
  case GREY16:
    return u16array_mean_d((const tz_uint16*)array, length);
  case FLOAT32:
    return farray_mean_d((const float*)array, length);
  case FLOAT64:
    return darray_mean_d((const double*)array, length);
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return 0.0;
}
