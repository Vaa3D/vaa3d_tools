#include <stdio.h>

#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_matlabio.h"


INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  Stack *stack = Read_Stack("../data/lobster_neuron_binary.tif");

  Matlab_Array mr;
  mr_read("../data/lobster_neuron_binary_seed.mat", &mr);

  int seed = ((int *) (mr.array))[0];
  printf("%d\n", seed);

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  
  IMatrix *chord = Make_IMatrix(dim, 3);
  Stack_Label_Object(stack, chord, seed - 1, 1, 2);
  
  Kill_IMatrix(chord);

  Stack_Threshold(stack, 1);
  Stack_Binarize(stack);

  Write_Stack("../data/lobster_neuron_single.tif", stack);

  return 0;
}
