/**@file tz_stack.h
 * @brief High-level interface for stack processing
 * @author Ting Zhao
 */

#ifndef _TZ_STACK_H_
#define _TZ_STACK_H_

#include "tz_cdefs.h"
#include <image_lib.h>
#include "tz_object_3d.h"
#include "tz_fimage_lib.h"

typedef FMatrix Filter_3d;

/**@addtogroup high_level_ High-level stack routines (tz_stack.h)
 *
 * @brief High-level routines for stack processing
 *
 * @note For those functions that returns a pointer, the caller is resposible
 * for freeing its space. For example, if it returns a Stack pointer, the 
 * caller must call Kill_Stack() after using it.
 *
 * Other things to know:
 * A 3d object is defined as a continuous region of foreground voxels in a 
 * stack. The continuity is defined under a certain neighborhood system, which
 * has three options: 6 (distance 1), 18 (distance sqrt(2)) and 26 (distance 
 * sqrt(3)). So the caller can pass one of the three values if a routine
 * requires neighborhood specification.
 */

__BEGIN_DECLS

/**@brief scale stack intensities 
 *
 * Stack_Bc_Autoadjust() changes the brighteness and contrast of a stack so that
 * most of its information is visible while displaying.
 */
void Stack_Bc_Autoadjust(Stack *stack);

/**@brief find local maxima
 *
 * Stack_Locmax_Region() labels the local maximum regions of <stack> with 1
 * and other regions with 0. The result is returned as a new object. <conn>
 * specifies the neighborhood system.
 */
Stack* Stack_Locmax_Region(const Stack *stack, int conn);

/**@brief Filtering routines.
 *
 * A filter can be created by a 3d filter creation function such as
 * Gaussian_Filter_3d(). To do filtering, just simply pass the stack and a
 * filter to Filter_Stack() and the result will be returned. The returned
 * stack has streched intensities.
 */
Filter_3d* Gaussian_Filter_3d(double sigma_x, double sigma_y, double sigma_z);
Filter_3d* Mexihat_Filter_3d(double sigma);
Stack* Filter_Stack(const Stack *stack, const Filter_3d *filter);

/**@brief find objects
 *
 * Find_Stack_Objects() returns an array of 3d objects. The number of objects
 * is stored in n, which does not need to be initialized. <conn>
 * specifies the neighborhood system. <stack> must be a binary stack.
 * 
 * Example:
 * Object_3d *objs = Find_Stack_Objects(stack, 26, &n);
 * for (i = 0; i < n; i++) {
 *   for (j = 0; j < objs[i]->size; j++) {
 *     printf ("The coordinate of the jth voxel of ith object is (%d %d %d)",
 *             objs[i]->voxels[j][0], objs[i]->voxels[j][1], 
 *             objs[i]->voxels[j][2]);
 *   }
 * }
 */
Object_3d* Find_Stack_Objects(const Stack *stack, int conn, int *n);

__END_DECLS

#endif
