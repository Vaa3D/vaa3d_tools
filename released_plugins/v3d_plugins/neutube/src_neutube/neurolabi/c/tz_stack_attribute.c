/* tz_tack_attribute.h
 *
 * 11-Dec-2003 Initial write: Ting Zhao
 */

#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_attribute.h"

INIT_EXCEPTION

int Stack_Width(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return stack->width;
}

int Stack_Height(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return stack->height;
}

int Stack_Depth(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return stack->depth;
}

int Stack_Kind(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return stack->kind;
}

int Stack_Plane_Area(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return Stack_Width(stack) * Stack_Height(stack);
}

size_t Stack_Volume(const Stack *stack)
{
  if (stack == NULL) {
    return 0;
  }

  return (size_t) Stack_Width(stack) * Stack_Height(stack) * Stack_Depth(stack);
}

void Stack_Set_Width(Stack *stack, int width)
{
  stack->width = width;
}

void Stack_Set_Height(Stack *stack, int height)
{
  stack->height = height;
}

void Stack_Set_Depth(Stack *stack, int depth)
{
  stack->depth = depth;
}

void Stack_Set_Kind(Stack *stack, int kind)
{
  stack->kind = kind;
}

void Stack_Set_Attribute(Stack *stack, int width, int height, int depth, 
    int kind)
{
  stack->width = width;
  stack->height = height;
  stack->depth = depth;
  stack->kind = kind;
}

int Stack_Channel_Number(const Stack *stack)
{
  if (stack->kind == COLOR) {
    return 3;
  } else {
    return 1;
  }
}

size_t Stack_Voxel_Number(const Stack *stack)
{
  return Get_Stack_Size(stack);
}

size_t Stack_Voxel_Bsize(const Stack *stack)
{
  switch (stack->kind) {
  case GREY:
    return 1;
  case GREY16:
    return 2;
  case FLOAT32:
    return 4;
  case FLOAT64:
    return 8;
  case COLOR:
    return 3;
  default:
    break;
  }

  return 0;
}

size_t Stack_Array_Bsize(const Stack *stack)
{
  return Stack_Voxel_Number(stack) * Stack_Voxel_Bsize(stack);
}

BOOL Stack_Contain_Point(const Stack *stack, int x, int y, int z)
{
  if ((x < 0) || (y < 0) || (z < 0) || (x >= stack->width) ||
      (y >= stack->height) || (z >= stack->depth)) {
    return FALSE;
  }

  return TRUE;
}

BOOL Stack_Is_Empty(const Stack *stack)
{
  if (stack != NULL) {
    if ((stack->array == NULL) || (stack->kind == 0) || (stack->width == 0) ||
	(stack->height == 0)) {
      return TRUE;
    }
  }

  return FALSE;
}

#define STACK_IS_DARK(stack_array)		\
  for (i = 0; i < nvoxel; i++) {		\
    if (stack_array[i] > 0) {			\
      return FALSE;				\
    }						\
  }

BOOL Stack_Is_Dark(const Stack *stack)
{
  int nvoxel = Stack_Voxel_Number(stack);
  size_t i;

  Image_Array ima;
  ima.array = stack->array;

  STACK_ARRAY_OPERATION(ima, STACK_IS_DARK);

  return TRUE;
}
