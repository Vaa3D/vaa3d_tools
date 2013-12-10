/**@file tz_stack_math.h
 * @brief math routines for stack
 * @author Ting Zhao
 * @date 04-Dec-2007
 */
#ifndef _TZ_STACK_MATH_H_
#define _TZ_STACK_MATH_H_

#include "tz_image_lib_defs.h"
#include "tz_cdefs.h"

__BEGIN_DECLS

/* Logical Operations
 *
 * Stack_And(), Stack_Or(), Stack_Xor(), Stack_Not() are binary logical 
 * operations of two stacks <stack1> and <stack2>. The results will be 
 * stored in <out>, which could refer to the same object as <stack1>, <stack2>
 * or <stack> does for in place calculation. If out is NULL, a new stack will 
 * be returned.
 *
 * Stack_Not() does NOT operation on a stack. It turns non-zero voxels to 0 and
 * other voxels to 1.
 */
Stack* Stack_And(Stack *stack1, Stack *stack2, Stack *out);
Stack* Stack_Or(Stack *stack1, Stack *stack2, Stack *out);
Stack* Stack_Xor(Stack *stack1, Stack *stack2, Stack *out);
Stack* Stack_Not(Stack *stack, Stack *out);

/**@brief Binary subtraction.
 *
 * Stack_Bsub() subtracts \a stack2 from \a stack1 and store the result in \a 
 * out if \a out is not NULL. ( 1 - 0 = 1; 1 - 1 = 0; 0 - 0 = 0; 0 - 1 = 0)
 */
Stack* Stack_Bsub(Stack *stack1, Stack *stack2, Stack *out);

/* Extended logical operations (in place calculation is allowed)
 *  
 * (f | g)(x) = max(f(x), g(x))
 * (f & g)(x) = min(f(x), g(x))
 * fc(x) = tmax - f(x)
 */
Stack* Stack_And_E(Stack *stack1, Stack *stack2, Stack *out);
Stack* Stack_Or_E(Stack *stack1, Stack *stack2, Stack *out);
Stack* Stack_Complement(Stack *stack, double tmax, Stack *out);

/* 
 * Stack_Mask() applies the mask <stack2> on <stack1> and stores the masked
 * stack in <out> if it is not NULL. If <out> is NULL, a new stack will be 
 * created and returned. <stack2> must be GREY kind and <out> must have the
 * same attributes as <stack1> if it is not NULL. 
 * It does in-place operation when <stack1> is the same as <out>.
 */
Stack *Stack_Mask(Stack *stack1, const Stack *stack2, Stack *out);

/* Mathematical Operations
 *
 * Stack_Add() and Stack_Sub() are addition and subtraction operations two 
 * stacks. If <out> is not NULL, it is used to store the result. <out> could 
 * refer to the object <stack1> or <stack2> for in place calculation. If out 
 * is NULL, a new stack will be returned.
 */
Stack *Stack_Add(Stack *stack1, Stack *stack2, Stack *out);
Stack *Stack_Sub(Stack *stack1, Stack *stack2, Stack *out);

Stack *Stack_Mul(Stack *stack1, Stack *stack2, Stack *out);

void Stack_Addc_M(Stack *stack,int value);
void Stack_Subc(Stack *stack,int subtr);
void Stack_Csub(Stack *stack, int subtr);

Stack *Stack_Add_R(Stack *stack1, Stack *stack2, const int *range, Stack *out);
Stack *Stack_Sub_R(Stack *stack1, Stack *stack2, const int *range, Stack *out);

__END_DECLS

#endif
