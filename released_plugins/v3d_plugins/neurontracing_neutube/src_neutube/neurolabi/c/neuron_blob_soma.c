#include <stdio.h>
#include <string.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_draw.h"
#include "tz_pixel_array.h"
#include "tz_stack_bwmorph.h"
#include "tz_dimage_lib.h"
#include "tz_stack_objmask.h"
#include "tz_dmatrix.h"

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
  
  sprintf(file_path, "../data/%s/blobmask.tif", neuron_name);
  Stack *stack1 = Read_Stack(file_path);

  Stack *stack2 = Copy_Stack(stack1);
  int i;
  for (i = 0; i < nvoxel; i++) {
    stack2->array[i] = 3;
  }

  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Write_Stack(file_path, stack2);

  Kill_Stack(stack1);
  Kill_Stack(stack2);

  return 0;
}
