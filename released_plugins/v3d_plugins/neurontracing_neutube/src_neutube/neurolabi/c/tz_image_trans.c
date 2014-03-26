#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_stack_attribute.h"
#include "tz_image_trans.h"

Mc_Stack Mc_Stack_View_Stack(const Stack *stack)
{
  Mc_Stack mc_stack;
  mc_stack.kind = stack->kind;
  mc_stack.width = stack->width;
  mc_stack.height = stack->height;
  mc_stack.depth = stack->depth;
  mc_stack.nchannel = 1;
  mc_stack.array = stack->array;

  return mc_stack;
}

Stack Stack_View_Mc_Stack(const Mc_Stack *mc_stack, int channel)
{
  Stack stack;
  stack.kind = mc_stack->kind;
  stack.width = mc_stack->width;
  stack.height = mc_stack->height;
  stack.depth = mc_stack->depth;
  stack.array = mc_stack->array + Mc_Stack_Voxel_Number(mc_stack) * channel;
  stack.text = NULL;

  return stack;
}

Mc_Stack* Stack_To_Mc_Stack(const Stack *stack, Mc_Stack *mc_stack)
{
  TZ_PRIOR_ASSERT(mc_stack != NULL, 
		  stack->array != mc_stack->array,
		  "In place transformation not allowed");

  int mc_stack_kind = stack->kind;
  int mc_stack_nchannel = 1;
  if (stack->kind == COLOR) {
    mc_stack_kind = GREY;
    mc_stack_nchannel = 3;
  }

  if (mc_stack == NULL) {
    mc_stack = Make_Mc_Stack(mc_stack_kind, stack->width, stack->height, 
			     stack->depth, mc_stack_nchannel);
  }

  if (stack->kind != COLOR) {
    if (stack->array != mc_stack->array) {
      memcpy(mc_stack->array, stack->array, Stack_Array_Bsize(stack));
    }
  } else { /* transpose */
    int i, j;
    int length = Stack_Voxel_Number(stack);
    color_t *stack_array = (color_t *) stack->array;
    uint8* mc_stack_array[3];
    mc_stack_array[0] = mc_stack->array;
    mc_stack_array[1] = mc_stack->array + length;
    mc_stack_array[2] = mc_stack->array + length + length;

    for (i = 0; i < length; i++) {
      for (j = 0; j < 3; j++) {
	mc_stack_array[j][i] = stack_array[i][j];
      }
    }
  }

  return mc_stack;
}
