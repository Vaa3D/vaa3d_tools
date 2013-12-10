/**@file tz_image_trans.h
 * @brief image data structure transformation
 * @author Ting Zhao
 * @date 01-Dec-2008
 */

#ifndef _TZ_IMAGE_TRANS_H_
#define _TZ_IMAGE_TRANS_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

/*
 * Tranform a stack to an agent multi-channel stack.
 */
Mc_Stack Mc_Stack_View_Stack(const Stack *stack);

Stack Stack_View_Mc_Stack(const Mc_Stack *mstack, int channel);

/*
 * Transform a stack to a multi-channel stack. In-place transformation is not
 * allowed. A COLOR stack will be transformed into a 3-channel GREY stack.
 */
Mc_Stack* Stack_To_Mc_Stack(const Stack *stack, Mc_Stack *mc_stack);

__END_DECLS

#endif
