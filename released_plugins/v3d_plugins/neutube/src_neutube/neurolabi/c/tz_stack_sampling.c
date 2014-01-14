/* tz_stack_sampling.c
 *
 * 09-Jan-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_utils.h"
#include "tz_utilities.h"
#include "tz_stack_sampling.h"
#include "tz_u8array.h"
#include "tz_u16array.h"
#include "tz_farray.h"
#include "tz_darray.h"

INIT_EXCEPTION

#define OBJECT_3D_SAMPLING(array, parray, suffix, obj, stack)		\
  for (i = 0; i < obj->size; i++) {					\
    offset = Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],	\
			       obj->voxels[i][2], stack->width,		\
			       stack->height, stack->depth);		\
    TZ_CONCATU(parray, suffix)[i] = TZ_CONCATU(array, suffix)[offset];	\
  }

Pixel_Array *Object_3d_Sampling(const Stack *stack, const Object_3d *obj)
{
  Pixel_Array *pa = Make_Pixel_Array(stack->kind, obj->size);

  DEFINE_SCALAR_ARRAY_ALL(array, stack);
  DEFINE_SCALAR_ARRAY_ALL(parray, pa);

  int i;
  int offset;

  switch (stack->kind) {
  case GREY:
    OBJECT_3D_SAMPLING(array, parray, grey, obj, stack);
    break;
  case GREY16:
    OBJECT_3D_SAMPLING(array, parray, grey16, obj, stack);
    break;
  case FLOAT32:
    OBJECT_3D_SAMPLING(array, parray, float32, obj, stack);
    break;
  case FLOAT64:
    OBJECT_3D_SAMPLING(array, parray, float64, obj, stack);
    break;
  default:
    THROW(ERROR_DATA_TYPE);
    break;
  }

  return pa;
}

#undef OBJECT_3D_SAMPLING

#define VOXEL_LIST_SAMPLING(array, parray, suffix, v, stack)		\
  for (i = 0; i < length; i++) {					\
    offset = Stack_Util_Offset(v[i]->x, v[i]->y, v[i]->z,		\
			       stack->width, stack->height, stack->depth); \
    TZ_CONCATU(parray, suffix)[i] = TZ_CONCATU(array, suffix)[offset];	\
  }

Pixel_Array *Voxel_List_Sampling(const Stack *stack, const Voxel_List *list)
{
  int length = Voxel_List_Length(list);

  Pixel_Array *pa = Make_Pixel_Array(stack->kind, length);

  DEFINE_SCALAR_ARRAY_ALL(array, stack);
  DEFINE_SCALAR_ARRAY_ALL(parray, pa);

  int i;
  int offset;

  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);
  
  switch (stack->kind) {
  case GREY:
    VOXEL_LIST_SAMPLING(array, parray, grey, voxel_array, stack);
    break;
  case GREY16:
    VOXEL_LIST_SAMPLING(array, parray, grey16, voxel_array, stack);
    break;
  case FLOAT32:
    VOXEL_LIST_SAMPLING(array, parray, float32, voxel_array, stack);
    break;
  case FLOAT64:
    VOXEL_LIST_SAMPLING(array, parray, float64, voxel_array, stack);
    break;
  default:
    safe_free((void **) &voxel_array, free);
    THROW(ERROR_DATA_TYPE);
    break;
  }

  if (voxel_array != NULL) {
    free(voxel_array);
  }

  return pa;
}

#undef VOXEL_LIST_SAMPLING

#define INDEX_ARRAY_SAMPLING(stack_array, pa_array)			\
  for (i = 0; i < n; i++) {						\
    pa_array[i] = stack_array[indices[i]];				\
  }

Pixel_Array* Stack_Index_Sampling(const Stack *stack, const int *indices, 
				  int n)
{
  Pixel_Array *pa = Make_Pixel_Array(stack->kind, n);
  Image_Array stack_ima;
  Image_Array pa_ima;
  
  stack_ima.array = stack->array;
  pa_ima.array = (uint8*)pa->array;
  int i;

  switch (stack->kind) {
  case GREY:
    INDEX_ARRAY_SAMPLING(stack_ima.array, pa_ima.array);
    break;
  case GREY16:
    INDEX_ARRAY_SAMPLING(stack_ima.array16, pa_ima.array16);
    break;
  case FLOAT32:
    INDEX_ARRAY_SAMPLING(stack_ima.array32, pa_ima.array32);
    break;
  case FLOAT64:
    INDEX_ARRAY_SAMPLING(stack_ima.array64, pa_ima.array64);
    break;
  case COLOR:
    for (i = 0; i < n; i++) {
      pa_ima.arrayc[i][0] = stack_ima.arrayc[indices[i]][0];
      pa_ima.arrayc[i][1] = stack_ima.arrayc[indices[i]][1];
      pa_ima.arrayc[i][2] = stack_ima.arrayc[indices[i]][2];
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return pa;
}


#define STACK_POINT_SAMPLING3(stack_array)				\
  /* Calculate the weighted sum */					\
  if (wx_high > 0.0) {							\
    if (wy_high > 0.0) {						\
      if (wz_high > 0.0) {						\
	sum = wx_low  * (double) stack_array[offset];			\
	offset++;							\
	sum += wx_high  * (double) stack_array[offset];			\
	sum *= wy_low * wz_low;						\
	offset += stack->width;						\
	double tmp_sum = wx_high * (double) stack_array[offset];	\
	offset--;							\
	tmp_sum += wx_low  * (double) stack_array[offset];		\
	sum += tmp_sum * wy_high * wz_low;				\
	offset += area;							\
	tmp_sum = wx_low * (double) stack_array[offset];		\
	offset++;							\
	tmp_sum += wx_high * (double) stack_array[offset];		\
	sum += tmp_sum * wy_high * wz_high;				\
	offset -= stack->width;						\
	tmp_sum = wx_high * (double) stack_array[offset];		\
	offset--;							\
	tmp_sum += wx_low * (double) stack_array[offset];		\
	sum += tmp_sum * wy_low * wz_high;				\
      } else { /* wz_high == 0.0 */					\
	sum += wx_low * wy_low * (double) stack_array[offset];		\
	offset++;							\
	sum += wx_high * wy_low * (double) stack_array[offset];		\
	offset += stack->width;						\
	sum += wx_high * wy_high * (double) stack_array[offset];	\
	offset--;							\
	sum += wx_low * wy_high * (double) stack_array[offset];		\
      }									\
    } else { /* wy_high == 0.0 */					\
      if (wz_high > 0.0) {						\
	sum += wx_low * wz_low * (double) stack_array[offset];		\
	offset++;							\
	sum += wx_high * wz_low * (double) stack_array[offset];		\
	offset += area;							\
	sum += wx_high * wz_high * (double) stack_array[offset];	\
	offset --;							\
	sum += wx_low * wz_high * (double) stack_array[offset];		\
      } else { /* wz_high == 0.0 */					\
	sum += wx_low * (double) stack_array[offset];			\
	offset++;							\
	sum += wx_high * (double) stack_array[offset];			\
      }									\
    }									\
  } else { /* wx_high == 0.0 */						\
    if (wy_high > 0.0) {						\
      if (wz_high > 0.0) {						\
	sum += wy_low * wz_low * (double) stack_array[offset];		\
	offset += stack->width;						\
	sum += wy_high * wz_low * (double) stack_array[offset];		\
	offset += area;							\
	sum += wy_high * wz_high * (double) stack_array[offset];	\
	offset -= stack->width;						\
	sum += wy_low * wz_high * (double) stack_array[offset];		\
      } else { /* wz_high == 0.0 */					\
	sum += wy_low * (double) stack_array[offset];			\
	offset += stack->width;						\
	sum += wy_high * (double) stack_array[offset];			\
      }									\
    } else { /* wy_high == 0.0 */					\
      if (wz_high > 0.0) {						\
	sum += wz_low * (double) stack_array[offset];			\
	offset += area;							\
	sum += wz_high * (double) stack_array[offset];			\
      } else { /* wz_high == 0.0 */					\
	sum = (double) stack_array[offset];				\
      }									\
    }									\
  }

#define STACK_POINT_SAMPLING2(stack_array)				\
  {									\
    /* Calculate the weighted sum */					\
    pixels[0] = stack_array[offset];					\
    pixels[1] = stack_array[offset + 1];				\
    pixels[2] = stack_array[offset + stack->width];			\
    pixels[3] = stack_array[offset + area];				\
    sum = pixels[0] * (wx_low - wy_high - wz_high) + wx_high * pixels[1] + wy_high * pixels[2] + wz_high * pixels[3]; \
  }

#define STACK_POINT_SAMPLING4(stack_array)				\
  {									\
    stack_array += offset;						\
    sum = wx_low  * ((double) (*(stack_array++)));			\
    sum += wx_high  * ((double) (*stack_array));			\
    sum *= wy_low * wz_low;						\
    stack_array += stack->width;					\
    double tmp_sum = wx_high * ((double) (*(stack_array--)));		\
    tmp_sum += wx_low  * ((double) (*stack_array));			\
    sum += tmp_sum * wy_high * wz_low;					\
    stack_array += area;						\
    tmp_sum = wx_low * ((double) (*(stack_array++)));			\
    tmp_sum += wx_high * ((double) (*stack_array));			\
    sum += tmp_sum * wy_high * wz_high;					\
    stack_array -= stack->width;					\
    tmp_sum = wx_high * ((double) (*(stack_array--)));			\
    tmp_sum += wx_low * ((double) (*stack_array));			\
    sum += tmp_sum * wy_low * wz_high;					\
  }

#define STACK_POINT_SAMPLING(stack_array)				\
  /* Calculate the weighted sum */					\
  stack_array += offset;						\
  if (wx_high > 0.0) {							\
    if (wy_high > 0.0) {						\
      if (wz_high > 0.0) {						\
	sum = wx_low  * ((double) (*(stack_array++)));			\
	sum += wx_high  * ((double) (*stack_array));			\
	sum *= wy_low * wz_low;						\
	stack_array += stack->width;					\
	double tmp_sum = wx_high * ((double) (*(stack_array--)));	\
	tmp_sum += wx_low  * ((double) (*stack_array));			\
	sum += tmp_sum * wy_high * wz_low;				\
	stack_array += area;						\
	tmp_sum = wx_low * ((double) (*(stack_array++)));		\
	tmp_sum += wx_high * ((double) (*stack_array));			\
	sum += tmp_sum * wy_high * wz_high;				\
	stack_array -= stack->width;					\
	tmp_sum = wx_high * ((double) (*(stack_array--)));		\
	tmp_sum += wx_low * ((double) (*stack_array));			\
	sum += tmp_sum * wy_low * wz_high;				\
      } else { /* wz_high == 0.0 */					\
	sum += wx_low * wy_low * ((double) (*(stack_array++)));		\
	sum += wx_high * wy_low * ((double) (*stack_array));		\
	stack_array += stack->width;					\
	sum += wx_high * wy_high * ((double) (*(stack_array--)));	\
	sum += wx_low * wy_high * ((double) (*stack_array));		\
      }									\
    } else { /* wy_high == 0.0 */					\
      if (wz_high > 0.0) {						\
	sum += wx_low * wz_low * ((double) (*(stack_array++)));		\
	sum += wx_high * wz_low * ((double) (*stack_array));		\
	stack_array += area;						\
	sum += wx_high * wz_high * ((double) (*(stack_array--)));	\
	sum += wx_low * wz_high * ((double) (*stack_array));		\
      } else { /* wz_high == 0.0 */					\
	sum += wx_low * ((double) (*(stack_array++)));			\
	sum += wx_high * ((double) (*stack_array));			\
      }									\
    }									\
  } else { /* wx_high == 0.0 */						\
    if (wy_high > 0.0) {						\
      if (wz_high > 0.0) {						\
	sum += wy_low * wz_low * ((double) (*stack_array));		\
	stack_array += stack->width;					\
	sum += wy_high * wz_low * ((double) (*stack_array));		\
	stack_array += area;						\
	sum += wy_high * wz_high * ((double) (*stack_array));		\
	stack_array -= stack->width;					\
	sum += wy_low * wz_high * ((double) (*stack_array));		\
      } else { /* wz_high == 0.0 */					\
	sum += wy_low * ((double) (*stack_array));			\
	stack_array += stack->width;					\
	sum += wy_high * ((double) (*stack_array));			\
      }									\
    } else { /* wy_high == 0.0 */					\
      if (wz_high > 0.0) {						\
	sum += wz_low * ((double) (*stack_array));			\
	stack_array += area;						\
	sum += wz_high * ((double) (*stack_array));			\
      } else { /* wz_high == 0.0 */					\
	sum = ((double) (*stack_array));				\
      }									\
    }									\
  }

double Stack_Point_Sampling(const Stack *stack, double x, double y, double z)
{
  if (stack == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  /* If the point is out of range */
  if ((x > stack->width - 1) || (x < 0) || (y > stack->height - 1) || (y < 0)
      || (z > stack->depth - 1) || (z < 0)) {
    return NaN;
  } else {
    double sum = 0.0;

    /* Get weights of surrounded voxels. */
    int x_low = (int)(x);
    double wx_high = x - x_low;
    double wx_low = 1.0 - wx_high;
    int y_low = (int)(y);
    double wy_high = y - y_low;
    double wy_low = 1.0 - wy_high;
    int z_low = (int)(z);
    double wz_high = z - z_low;
    double wz_low = 1.0 - wz_high;
    //double pixels[4];

    size_t area = stack->width * stack->height;

    size_t offset =  area *  z_low + stack->width * y_low + x_low;
    
    DEFINE_SCALAR_ARRAY_ALL(array, stack);
    switch (stack->kind) {
    case GREY:
      STACK_POINT_SAMPLING(array_grey);
      break;
    case GREY16:
      STACK_POINT_SAMPLING(array_grey16);
      break;
    case FLOAT32:
      STACK_POINT_SAMPLING(array_float32);
      break;
    case FLOAT64:
      STACK_POINT_SAMPLING(array_float64);
      break;
    default:
      perror("Unsuppoted image kind");
      TZ_ERROR(ERROR_DATA_TYPE);
    }

    /* Return the weighted sum */
    return sum;
  }
}

double* Stack_Points_Sampling(const Stack *stack, const double *points,
			      int length, double *array)
{
  if (array == NULL) {
    array = (double *) Guarded_Malloc(sizeof(double) * length, 
				      "Stack_Points_Sampling");
  }

  int i;
  for (i = 0; i < length; i++) {
    array[i] = Stack_Point_Sampling(stack, points[0], points[1], points[2]);
    points += 3;
  }

  return array;
}

double* Stack_Points_Sampling_M(const Stack *stack, const Stack *mask,
				const double *points,
				int length, double *array)
{
  if (array == NULL) {
    array = (double *) Guarded_Malloc(sizeof(double) * length, 
				      "Stack_Points_Sampling");
  }

  int i;
  for (i = 0; i < length; i++) {
    if (Stack_Point_Sampling(mask, points[0], points[1], points[2]) > 0.0) {
      array[i] = NaN;
    } else {
      array[i] = Stack_Point_Sampling(stack, points[0], points[1], points[2]);
    }
    points += 3;
  }

  return array;
  
}

double* Stack_Points_Sampling_Z(const Stack *stack, double z_scale,
				const double *points, int length, double *array)
{
  if (array == NULL) {
    array = (double *) Guarded_Malloc(sizeof(double) * length, 
				      "Stack_Points_Sampling_Z");
  }

  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, points, length, array);
  } else {
    int i;
    for (i = 0; i < length; i++) {
      array[i] = Stack_Point_Sampling(stack, points[0], points[1], 
				      points[2] * z_scale);
      points += 3;
    }
  }

  return array;  
}

double* Stack_Points_Sampling_Zm(const Stack *stack, double z_scale,
				 const Stack *mask,
				 const double *points, int length, 
				 double *array)
{
  if (array == NULL) {
    array = (double *) Guarded_Malloc(sizeof(double) * length, 
				      "Stack_Points_Sampling_Z");
  }

  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, points, length, array);
  } else {
    int i;
    for (i = 0; i < length; i++) {
      if (Stack_Point_Sampling(mask, points[0], points[1], 
			       points[2] * z_scale) > 0.0) {
	array[i] = NaN;
      } else {
	array[i] = Stack_Point_Sampling(stack, points[0], points[1], 
					points[2] * z_scale);
      }
      points += 3;
    }
  }

  return array;  
}
