/**@file tz_stack_attribute.h
 * @brief stack attributes
 * @author Ting Zhao
 */

/* Rountines for getting attributes of a $stack. 
 * See ../lib/myers.pack/image_lib.h for $stack definition. 
 */

#ifndef _TZ_STACK_ATTRIBUTE_H_
#define _TZ_STACK_ATTRIBUTE_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#include "image_lib.h"

/**@addtogroup stack_attr_ Stack attributes (tz_stack_attribute.h)
 * 
 * @{ 
 */

/*
 * Stack_Width(), Stack_Height(), Stack_Depth() and Stack_Kind() returns the 
 * widht, height, depth and kind of a stack.
 */
int Stack_Width(const Stack *stack);
int Stack_Height(const Stack *stack);
int Stack_Depth(const Stack *stack);
int Stack_Kind(const Stack *stack);
int Stack_Plane_Area(const Stack *stack);
size_t Stack_Volume(const Stack *stack);

void Stack_Set_Width(Stack *stack, int width);
void Stack_Set_Height(Stack *stack, int height);
void Stack_Set_Depth(Stack *stack, int depth);
void Stack_Set_Kind(Stack *stack, int kind);
void Stack_Set_Attribute(Stack *stack, int width, int height, int depth, 
    int kind);


/*
 * Stack_Channel_Number() returns the number of the channels of a stack.
 */
int Stack_Channel_Number(const Stack *stack);

/*
 * Stack_Voxel_Number() returns the number of voxels of a stack. 
 * Stack_Voxel_Bsize() returns the number of bytes of each voxel. 
 * Stack_Array_Bsize() returns the total number of bytes of all voxels.
 */
size_t Stack_Voxel_Number(const Stack *stack);
size_t Stack_Voxel_Bsize(const Stack *stack);
size_t Stack_Array_Bsize(const Stack *stack);

/**@brief Test if a point is within the stack.
 *
 * Stack_Contain_Point() returns TRUE if (<x>, <y>, <z>) is within the bounding
 * box of <stack>.
 */
BOOL Stack_Contain_Point(const Stack *stack, int x, int y, int z);

/**@brief Test if a stack is empty.
 *
 * Stack_Is_Empty() returns TURE if \a stack is not empty, otherwise it returns
 * FALSE. NULL is considered as non-empty.
 */
BOOL Stack_Is_Empty(const Stack *stack);

/**@} */

BOOL Stack_Is_Dark(const Stack *stack);

__END_DECLS

#endif
