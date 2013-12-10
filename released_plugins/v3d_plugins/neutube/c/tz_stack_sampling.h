/**@file tz_stack_sampling.h
 * @brief sampling stack
 * @author Ting Zhao
 * @date 09-Jan-2008
 */

#ifndef _TZ_STACK_SAMPLING_H_
#define _TZ_STACK_SAMPLING_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_pixel_array.h"
#include "tz_object_3d.h"
#include "tz_voxel_linked_list.h"

__BEGIN_DECLS

/**@addtogroup stack_sampling_ stack sampling
 *
 * @{
 */

/**@addtogroup grid_sampling_ grid sampling
 *
 * @{
 */
/**@brief sample points of an object
 *
 * Object_3d_Sampling() returns the values of all points in \a obj from \a
 * stack.
 */
Pixel_Array *Object_3d_Sampling(const Stack *stack, const Object_3d *obj);

/**@brief sample points of a voxel list
 *
 * Voxel_List_Sampling() returns the values of all points in \a list from \a
 * stack.
 */
Pixel_Array* Voxel_List_Sampling(const Stack *stack, const Voxel_List *list);

/**@brief sample points from indices
 *
 * Stack_Index_Sampling returns the values of \n points in the index array
 * \a indices. Each element of \a indices is the index of a point in \a stack.
 */
Pixel_Array* Stack_Index_Sampling(const Stack *stack, const int *indices, 
				  int n);

/**@}
 */

/**@addtogroup point_sampling_ Point sampling
 * @{
 */

/*
 * Stack_Point_Sampling() returns the sampled value from a point at the
 * position (<x>, <y>, <z>). It returns 0 if the position is out of range.
 */
double Stack_Point_Sampling(const Stack *stack, double x, double y, double z);

/*
 * Stack_Points_Sampling() samples values from a stack at the positions 
 * specified by <points>, in which each contiguous three elements is the
 * coordinate of a point and the number of points is <length>. The result
 * is stored in <array>, which is returned as well, if it is not NULL.
 * Otherwise it returns a new pointer.
 */
double* Stack_Points_Sampling(const Stack *stack, const double *points,
			      int length, double *array);
double* Stack_Points_Sampling_M(const Stack *stack, const Stack *mask,
				const double *points,
				int length, double *array);

double* Stack_Points_Sampling_Z(const Stack *stack, double z_scale,
			      const double *points, int length, double *array);
double* Stack_Points_Sampling_Zm(const Stack *stack, double z_scale,
				 const Stack *mask,
				 const double *points, int length, 
				 double *array);


/**@*/

/**@}*/

__END_DECLS

#endif
