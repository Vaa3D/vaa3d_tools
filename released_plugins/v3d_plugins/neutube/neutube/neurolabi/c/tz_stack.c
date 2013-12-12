#include <stdio.h>
#include <stdlib.h>
#include "tz_stack_lib.h"
#include "tz_stack_utils.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_attribute.h"
#include "tz_arrayqueue.h"
#include "tz_error.h"
#include "tz_stack.h"
#include "tz_fimage_lib.h"
#include "tz_interface.h"

void Stack_Bc_Autoadjust(Stack *stack)
{
  Stretch_Stack_Value_Q(stack, 0.9995);
}

Stack *Stack_Locmax_Region(const Stack *stack, int conn)
{
  Stack *result = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(result);
  
  int neighbor[26];
  int is_in_bound[26];
  int nbound = 0;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);

  int nvoxel = Stack_Voxel_Number(stack);

  Arrayqueue *aq = Make_Arrayqueue(nvoxel);
  Initialize_Arrayqueue(aq);

#define STACK_LOCMAX_REGION_INIT_QUEUE(stack_array, test);		\
  for (i = 0; i < conn; i++) {						\
    if (test) {								\
      if (stack_array[offset] < stack_array[offset + neighbor[i]]) {	\
	Arrayqueue_Add_Last(aq, offset);				\
	result->array[offset] = 0;					\
	break;								\
      }									\
    }									\
  }
 
#define STACK_LOCMAX_REGION_INIT_ALL(stack_array)			\
  for (z = 0; z < stack->depth; z++) {					\
    for (y = 0; y < stack->height; y++) {				\
      for (x = 0; x < stack->width; x++) {				\
	if (stack_array[offset] > 0) {					\
	  nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth, \
					     x, y, z, is_in_bound);	\
	  if (nbound == conn) {						\
	    STACK_LOCMAX_REGION_INIT_QUEUE(stack_array, 1);		\
	  } else {							\
	    STACK_LOCMAX_REGION_INIT_QUEUE(stack_array, is_in_bound[i]); \
	  }								\
	} else {							\
	  result->array[offset] = 0;					\
	}								\
	offset++;							\
      }									\
    }									\
  }

  int i;
  int x, y, z;
  int offset = 0;
  Image_Array ia;
  ia.array = stack->array;

  switch (stack->kind) {
  case GREY:
    STACK_LOCMAX_REGION_INIT_ALL(ia.array);
    break;
  case GREY16:
    STACK_LOCMAX_REGION_INIT_ALL(ia.array16);
    break;
  case FLOAT32:
    STACK_LOCMAX_REGION_INIT_ALL(ia.array32);
    break;
  case FLOAT64:
    STACK_LOCMAX_REGION_INIT_ALL(ia.array64);
    break;
  default:
    PRINT_EXCEPTION("Fatal error", "Unsupported stack kind.");
  }

  int c = aq->head;
  int counter = 1;

#define STACK_LOCMAX_REGION_UPDATE_QUEUE(stack_array, test)	\
  for (i = 0; i < conn; i++) {					\
    if (test) {							\
      int nb = c + neighbor[i];					\
      if (result->array[nb] == 1) {				\
	if (stack_array[nb] <= stack_array[c]) {		\
	  ARRAYQUEUE_ADD_LAST(aq, nb);				\
	  result->array[nb] = 0;				\
	}							\
      }								\
    }								\
  }

#define STACK_LOCMAX_REGION_UPDATE_ALL(stack_array)			\
  {									\
    HOURGLASS_BEGIN("Finding local maxima", hg);			\
    do {								\
      if (counter == 0) {						\
	HOURGLASS_UPDATE(hg);						\
      }									\
      nbound = Stack_Neighbor_Bound_Test_I(conn, stack->width, stack->height, \
					   stack->depth, c, is_in_bound); \
      if (nbound == conn) {						\
	STACK_LOCMAX_REGION_UPDATE_QUEUE(stack_array, 1);		\
      } else {								\
	STACK_LOCMAX_REGION_UPDATE_QUEUE(stack_array, is_in_bound[i]);	\
      }									\
      ASSERT(c != aq->array[c], "self loop");				\
      c = aq->array[c];							\
      									\
      counter++;							\
      if (counter > 1000000) {						\
	counter = 0;							\
      }									\
    } while (c >= 0);							\
    									\
    HOURGLASS_END("done");						\
  }

  switch (stack->kind) {
  case GREY:
    STACK_LOCMAX_REGION_UPDATE_ALL(ia.array);
    break;
  case GREY16:
    STACK_LOCMAX_REGION_UPDATE_ALL(ia.array16);
    break;
  case FLOAT32:
    STACK_LOCMAX_REGION_UPDATE_ALL(ia.array32);
    break;
  case FLOAT64:
    STACK_LOCMAX_REGION_UPDATE_ALL(ia.array64);
    break;
  default:
    PRINT_EXCEPTION("Fatal error", "Unsupported stack kind.");
  }

  Kill_Arrayqueue(aq);

  return result;
}

Filter_3d* Gaussian_Filter_3d(double sigma_x, double sigma_y, double sigma_z)
{
  double sigma[3];
  sigma[0] = sigma_x;
  sigma[1] = sigma_y;
  sigma[2] = sigma_z;
  
  return Gaussian_3D_Filter_F(sigma, NULL);
}

Filter_3d* Mexihat_Filter_3d(double sigma)
{
  return Mexihat_3D_F(sigma, NULL);
}

Stack* Filter_Stack(const Stack *stack, const Filter_3d *filter)
{
  FMatrix *result = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(result->array, stack->width, stack->height,
				 stack->depth, stack->kind);
  Kill_FMatrix(result);
  return out;
}
