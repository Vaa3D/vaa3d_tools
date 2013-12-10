/**@file tz_stack_io.h
 * @brief I/O for stack
 * @author Ting Zhao
 * @date 29-Feb-2008
 */

#ifndef _TZ_STACK_IO_H_
#define _TZ_STACK_IO_H_

#include <image_lib.h>
#include "tz_cdefs.h"

__BEGIN_DECLS

/* 
 * Print_Stack() prints a stack to the screen.
 *
 * Print_Stack_Info() prints the brief information of a stack to the screen. 
 */
/**@brief Print a stack to the screen
 *
 * Print_Stack() prints detailed information of <stack> into the screen. <stack>
 * can be NULL.
 */
void Print_Stack(const Stack *stack);

/**@brief Print brief information of a stack.
 *
 * Print_Stack_Info() prints brief information of <stack>, which can be NULL.
 */
void Print_Stack_Info(const Stack *stack);

__END_DECLS

#endif
