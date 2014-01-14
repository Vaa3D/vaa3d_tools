#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_dimage_lib.h"
#include "tz_dmatrix.h"
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s/soma.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);
  Stack_Threshold(stack, 2);
  Stack_Binarize(stack);

  double sigma[3] = {1.0, 1.0, .5};
  DMatrix *filter = Gaussian_2D_Filter_D(sigma, NULL);
  filter->ndim = 3;
  filter->dim[2] = 1;
  DMatrix *out = Filter_Stack_Fast_D(stack, filter, NULL, 0);
  Kill_Stack(stack);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
			     out->dim[2], GREY);

  sprintf(file_path, "../data/%s/smoothsoma.tif", neuron_name);
  Write_Stack(file_path, stack);

  return 0;
}
