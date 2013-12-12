/* tz_stack_stat.c
 * Initial write: Ting Zhao
 */

#include <string.h>
#include "tz_image_lib.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_attribute.h"
#include "tz_error.h"
#include "tz_int_histogram.h"
#include "tz_stack_stat.h"

INIT_EXCEPTION

#define STACK_STAT_ASSIGN_VALUE(des, src)	\
  if (des != NULL) {				\
    memcpy(des, &src, sizeof(src));		\
  }

#define STACK_MAX(array, length, max, index, i)	\
  max = array[0];				\
  for (i = 1; i < length; i++) {		\
    if (array[i] > max) {			\
      index = i;				\
      max = array[i];				\
    }						\
  }

double Stack_Max(const Stack *stack, size_t *index)
{
  DEFINE_PIXEL_ALL(value);
  DEFINE_ARRAY_ALL(array, stack);

  size_t length = Stack_Voxel_Number(stack);
  size_t i;
  size_t max_index = 0;
  double max_value = 0;

  switch (Stack_Kind(stack)) {
  case GREY:
    STACK_MAX(array_grey, length, value_grey, max_index, i);
    max_value = (double) value_grey;
    break;
  case GREY16:
    STACK_MAX(array_grey16, length, value_grey16, max_index, i);
    max_value = (double) value_grey16;
    break;
  case FLOAT32:
    STACK_MAX(array_float32, length, value_float32, max_index, i);
    max_value = (double) value_float32;
    break;
  case FLOAT64:
    STACK_MAX(array_float64, length, value_float64, max_index, i);
    max_value = (double) value_float64;
    break;
  case COLOR:
    /* no meaning. just to disable compile warning */
    UNUSED_PARAMETER(value_color[0]);
    UNUSED_PARAMETER(array_color[0]);
  default:
    THROW(ERROR_DATA_TYPE);
  }

  if (index != NULL) {
    *index = max_index;
  }

  return max_value;
}

#undef STACK_MAX

#define STACK_MIN(array, length, min, index, i)	\
  min = array[0];				\
  for (i = 1; i < length; i++) {		\
    if (array[i] < min) {			\
      index = i;				\
      min = array[i];				\
    }						\
  }

double Stack_Min(const Stack *stack, size_t *index)
{
  DEFINE_PIXEL_ALL(value);
  DEFINE_ARRAY_ALL(array, stack);

  size_t length = Stack_Voxel_Number(stack);
  size_t i;
  size_t min_index = 0;

  switch (Stack_Kind(stack)) {
  case GREY:
    STACK_MIN(array_grey, length, value_grey, min_index, i);
    return (double) value_grey;
  case GREY16:
    STACK_MIN(array_grey16, length, value_grey16, min_index, i);
    return (double) value_grey16;
  case FLOAT32:
    STACK_MIN(array_float32, length, value_float32, min_index, i);
    return (double) value_float32;
  case FLOAT64:
    STACK_MIN(array_float64, length, value_float64, min_index, i);
    return (double) value_float64;
  case COLOR:
    /* no meaning. just to disable compile warning */
    UNUSED_PARAMETER(value_color[0]);
    UNUSED_PARAMETER(array_color[0]);
  default:
    THROW(ERROR_DATA_TYPE);
  }

  if (index != NULL) {
    *index = min_index;
  }

  return 0;
}

#undef STACK_MIN

/* Stack_Common_Intensity(): Most common intensity of a stack.
 */
int Stack_Common_Intensity(Stack *stack, int low, int high)
{
  int *hist = Stack_Hist(stack);
  int comm = Int_Histogram_Mode(hist, low, high);
  free(hist);
  return comm;
}

#define STACK_SUM(stack_array)			\
  for (i = 0; i < nvoxel; i++) {		\
    sum += (double) stack_array[i];		\
  }

double Stack_Sum(const Stack *stack)
{
  double sum = 0.0;
  size_t i;
  size_t nvoxel = Stack_Voxel_Number(stack);

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  switch (Stack_Kind(stack)) {
  case GREY:
    STACK_SUM(array_grey);
    break;
  case GREY16:
    STACK_SUM(array_grey16);
    break;
  case FLOAT32:
    STACK_SUM(array_float32);
    break;
  case FLOAT64:
    STACK_SUM(array_float64);
    break;
  default:
    PRINT_EXCEPTION("Unsupported image kind",
		    "Make sure the stack kind is one of the following:\n GREY, GREY16, FLOAT32, FLOAT64");
    break;
  }
  
  return sum;
}
#undef STACK_SUM

/*
 * Calculate the mean of the stack.
 */
double Stack_Mean(const Stack *stack)
{
  double s = Stack_Sum(stack);
  return s / (double) Stack_Voxel_Number(stack);
}

#define STACK_FGAREA(array)			\
  for (i = 0; i < length; i++) {		\
    if (array[i] != 0) {			\
      fg_area++;				\
    }						\
  }

size_t Stack_Fgarea(const Stack *stack)
{
  size_t fg_area = 0;
  
  size_t length = Stack_Voxel_Number(stack);
  size_t i;

  SCALAR_STACK_OPERATION(stack, STACK_FGAREA);

  return fg_area;
}

#define STACK_FGAREA_T(array)			\
  for (i = 0; i < length; i++) {		\
    if (array[i] > thre) {			\
      fg_area++;				\
    }						\
  }

size_t Stack_Fgarea_T(const Stack *stack, int thre)
{
  size_t fg_area = 0;
  
  size_t length = Stack_Voxel_Number(stack);
  size_t i;

  SCALAR_STACK_OPERATION(stack, STACK_FGAREA_T);

  return fg_area;  
}
