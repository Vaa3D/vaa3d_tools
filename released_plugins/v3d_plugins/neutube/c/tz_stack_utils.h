/**@file tz_stack_utils.h
 * @brief some utilities for stack
 * @author Ting Zhao
 * @date 09-Jan-2008
 */

#ifndef _TZ_STACK_UTILS_H_
#define _TZ_STACK_UTILS_H_

#include "tz_cdefs.h"
#include "tz_stdint.h"

__BEGIN_DECLS

/**@def STACK_UTIL_OFFSET*/
#define STACK_UTIL_OFFSET(x, y, z, width, area)	\
  (((size_t) x) + (width) * (y) + ((size_t) area) * (z))

/**@def STACK_UTIL_COORD*/
#define STACK_UTIL_COORD(index, width, area, x, y, z)	\
  {							\
    z = (index) / (area);				\
    y = (index) % (area);				\
    x = y % (width);					\
    y = y / (width);					\
  }

/*
 * Stack_Util_Offset() returns the offset of the coordinate (<x>, <y>, <z>)
 * in the array of a stack with <width>, <height> and <depth>. It returns -1
 * if (<x>, <y>, <z>) is out of bound.
 */
ssize_t Stack_Util_Offset(int x, int y, int z, int width, int height, int depth);

/*
 * Stack_Util_Coord() turns the index of a voxel in a stack with slice size 
 * <width> x <height> to its coordinate (<x>, <y>, <z>).
 */
void Stack_Util_Coord(size_t index, int width, int height, 
		      int *x, int *y, int *z);
void Stack_Util_Coord_A(size_t index, int width, int area, 
			int *x, int *y, int *z);

double Stack_Util_Voxel_Distance(size_t index1, size_t index2, 
				 int width, int height);

size_t Stack_Subindex(size_t sindex, int x0, int y0, int z0, 
		      int swidth, int sarea, int width, int area);

/* FlyEM utilities */
uint32_t Color_To_Value(const uint8_t* color);
uint8_t Value_To_Color(uint32_t value, uint8_t* color);

__END_DECLS

#endif
