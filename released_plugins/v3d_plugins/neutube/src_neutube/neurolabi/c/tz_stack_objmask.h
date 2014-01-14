/**@file tz_stack_objmask.h
 * @brief 3d object as mask
 * @author Ting Zhao
 * @date 10-Mar-2008
 */

#ifndef _TZ_STACK_OBJMASK_H_
#define _TZ_STACK_OBJMASK_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_object_3d.h"

__BEGIN_DECLS

/* 
 * Stack_Foreground_Area_O() returns the area of foreground of the overlapped
 * region of <stack> and <obj>. It returns 0 if either of the input is NULL.
 */
int Stack_Foreground_Area_O(const Stack *stack, const Object_3d *obj);

__END_DECLS

#endif 
