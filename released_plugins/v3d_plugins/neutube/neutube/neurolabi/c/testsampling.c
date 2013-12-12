/* testsampling.c
 *
 * 29-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <image_lib.h>
#include "tz_stack_lib.h"
#include "tz_stack_sampling.h"
#include "tz_stack_attribute.h"
#include "tz_dmatrix.h"
#include "tz_stack_io.h"
#include "tz_error.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  int i;
  for (i = 0; i < Stack_Voxel_Number(stack); i++) {
    stack->array[i] = i;
  }
  Print_Stack(stack);

  
  printf("%g\n", Stack_Point_Sampling(stack, 1.1, 1.5, 1.0));
  printf("%g\n", Stack_Point_Sampling(stack, 1.1, 1.0, 1.5));
  printf("%g\n", Stack_Point_Sampling(stack, 1.1, 1.0, 1.0));
  printf("%g\n", Stack_Point_Sampling(stack, 1.0, 1.5, 1.3));
  printf("%g\n", Stack_Point_Sampling(stack, 1.0, 1.5, 1.0));
  printf("%g\n", Stack_Point_Sampling(stack, 1.0, 1.0, 1.3));
  printf("%g\n", Stack_Point_Sampling(stack, 1.0, 1.5, 1.0));

  DMatrix *dm;
  dim_type dim[3];
  dim[0] = 10;
  dim[1] = 4;
  dim[2] = 5;
  dm = Make_DMatrix(dim, 3);

  int j, k;
  int offset = 0;
  double x, y, z;

  dim[0] = matrix_size(dm->dim, dm->ndim);
  dim[1] = 3;
  DMatrix *points = Make_DMatrix(dim, 2);

  for (k = 0; k < dm->dim[2]; k++) {
    z = (double) (k ) * (stack->depth - 1) / (dm->dim[2] - 1);
    for (j = 0; j < dm->dim[1]; j++) {
      y = (double) (j ) * (stack->height - 1) / (dm->dim[1] - 1);
      for (i = 0; i < dm->dim[0]; i++) {
	x = (double) (i ) * (stack->width - 1) / (dm->dim[0] - 1);
	points->array[offset++] = x;
	points->array[offset++] = y;
	points->array[offset++] = z;
      }
    }
  }

  tic();
  for (j = 0; j < 100; j++) {
    Stack_Points_Sampling(stack, points->array, points->dim[0], dm->array);
  }
  printf("%llu\n", toc());

  DMatrix_Print(dm);
  Kill_Stack(stack);
  
  stack = Scale_Double_Stack(dm->array, dm->dim[0], dm->dim[1], dm->dim[2],
			     GREY);
  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
  Kill_DMatrix(dm);
  Kill_DMatrix(points);

  return 0;
}
