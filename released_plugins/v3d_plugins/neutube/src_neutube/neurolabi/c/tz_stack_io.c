/* tz_stack_io.c
 *
 * 29-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_image_lib_defs.h"
#include "tz_stack_io.h"

/* Print_Stack_Info: Print stack information.
 *
 * Args: stack - input stack.
 *
 * return: void.
 */
void Print_Stack_Info(const Stack *stack)
{
  if (stack == NULL) {
    printf("NULL stack\n");
  } else {
    printf("Width: %d; Height: %d; Depth: %d; Kind: %d\n",
	   stack->width, stack->height, stack->depth, stack->kind);
  }
}

#define PRINT_STACK(array, format)		\
  for(k=0;k<stack->depth;k++) {			\
    printf("Slice %d:\n",k);			\
    for(j=0;j<stack->height;j++) {		\
      for(i=0;i<stack->width;i++) {		\
	printf(format, array[offset]);		\
	printf(" ");				\
	offset++;				\
      }						\
      printf("\n");				\
    }						\
    printf("\n");				\
  }

void Print_Stack(const Stack* stack) 
{
  Print_Stack_Info(stack);

  if (stack != NULL) {
    int i,j,k;
    long offset=0;
    DEFINE_SCALAR_ARRAY_ALL(array, stack);

    switch(stack->kind) {
    case GREY:
      PRINT_STACK(array_grey, "%u");
      break;
    case GREY16:
      PRINT_STACK(array_grey16, "%u");
      break;
    case FLOAT32:
      PRINT_STACK(array_float32, "%.4f");
      break;
    case FLOAT64:
      PRINT_STACK(array_float64, "%.4f");
      break;
    default:
      fprintf(stderr,"Unsupported stack kind in Print_Stack()\n");
    }
  }
}
