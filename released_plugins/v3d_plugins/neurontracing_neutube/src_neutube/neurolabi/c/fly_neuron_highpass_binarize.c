#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_dimage_lib.h"
#include "tz_stack_threshold.h"
#include "tz_dmatrix.h"
#include "tz_stack_stat.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s/highpass.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);

  printf("%d\n", Stack_Common_Intensity(stack, 0, 255));
  Stack_Sub_Common(stack, 0, 255);
  Stack_Binarize(stack);

  sprintf(file_path, "../data/%s/highpass_binary.tif", neuron_name);
  Write_Stack(file_path, stack);
  printf("%s created.\n", file_path);
  
  /*
  Stack *stack2 = Read_Stack(file_path);
  Stack_Threshold(stack2, 50);
  Stack_Binarize(stack2);
  sprintf(file_path, "../data/%s/highpass_binary_ext.tif", neuron_name);
  Write_Stack(file_path, stack);
  Kill_Stack(stack2);
  */

  Kill_Stack(stack);

  return 0;
}
