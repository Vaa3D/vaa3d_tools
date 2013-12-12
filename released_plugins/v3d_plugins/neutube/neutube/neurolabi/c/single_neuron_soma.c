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
#include "tz_stack_attribute.h"
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
  
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *mask = Read_Stack(file_path);

  sprintf(file_path, "../data/%s/blobmask.tif", neuron_name);
  Stack *blobmask = Read_Stack(file_path);

  int voxel_number = Stack_Voxel_Number(mask);
  int i;
  for (i = 0; i < voxel_number; i++) {
    if (blobmask->array[i] > 0) {
      mask->array[i] = 3;
    }
  }

  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Write_Stack(file_path, mask);


  Kill_Stack(mask);
  Kill_Stack(blobmask);

  return 0;
}
