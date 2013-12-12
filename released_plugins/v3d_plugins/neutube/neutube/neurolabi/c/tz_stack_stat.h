/**@file tz_stack_stat.h
 * @brief stack statistics
 * Initial write: Ting Zhao
 */

#ifndef _TZ_STACK_STAT_H_
#define _TZ_STACK_STAT_H_

#include "image_lib.h"
#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup stack_stat_ Stack statistics
 * @{
 */

/* Stack_Max() and Stack_Min() returns the maximum and minimum of the values of
 * a stack. The returned value is a double. The place where the stat is from 
 * is stored in <index> when <index> is not NULL.
 * The functions do not support multiple channel stack.
 */
double Stack_Max(const Stack *stack, size_t *index);
double Stack_Min(const Stack *stack, size_t *index);

int Stack_Common_Intensity(Stack *stack, int low, int high);

/*
 * Stack_Sum() returns the sum of all voxel intensities of a stack. 
 * Stack_Mean() returns the mean of all voxel intensities of a stack.
 */
double Stack_Sum(const Stack *stack);
double Stack_Mean(const Stack *stack);

/*
 * Stack_Fgarea() returns the foreground area of a stack. Only voxels with
 * intensity 0 are considered as background.
 */
size_t Stack_Fgarea(const Stack *stack);
size_t Stack_Fgarea_T(const Stack *stack, int thre);

/**@}
 */

__END_DECLS

#endif
