/**@file tz_stack_relation.h
 * @brief stack comparison
 * @author Ting Zhao
 * @date 12-Dec-2007
 */

/* compare two stacks */

#ifndef _TZ_STACK_RELATION_H_
#define _TZ_STACK_RELATION_H_

#include "image_lib.h"
#include "tz_cdefs.h"

__BEGIN_DECLS

/**@brief Test if two stacks have the same size.
 *
 * Stack_Same_Size() returns TRUE if the dimensions of <stack1> and <stack2> 
 * are the same, otherwise it returns FASLE.
 */
BOOL Stack_Same_Size(const Stack *stack1, const Stack *stack2);

/**@brief Test if two stacks have the same kind.
 *
 * Stack_Same_Kind() returns TRUE if the kinds of <stack1> and <stack2> are the
 * same, otherwise it returns FASLE.
 */
BOOL Stack_Same_Kind(const Stack *stack1, const Stack *stack2);

/**@brief Test if two stacks have the same attributes.
 *
 * Stack_Same_Attribute() returns TRUE if all the attributes (NOT including the
 * voxel values) of <stack1> and <stack2> are the same, otherwise it returns 
 * FASLE.
 */
BOOL Stack_Same_Attribute(const Stack *stack1, const Stack *stack2);

/**@brief Test if two stacks are identical.
 *
 * Stack_Identical() returns TRUE if <stack1> and <stack2> have exactly the same
 * attributes and voxel values, otherwise it returns FALSE.
 */
BOOL Stack_Identical(const Stack *stack1, const Stack *stack2);

__END_DECLS

#endif

