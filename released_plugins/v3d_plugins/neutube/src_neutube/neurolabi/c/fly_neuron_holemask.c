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
#include "tz_stack_bwmorph.h"
#include "tz_dimage_lib.h"
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
  Stack *stack1 = Read_Stack(file_path);
  sprintf(file_path, "../data/%s/highpass_binary.tif", neuron_name);
  Stack *stack2 = Read_Stack(file_path);

  Stack_And(stack1, stack2, stack1);
  
  sprintf(file_path, "../data/%s/holemask.tif", neuron_name);
  Write_Stack(file_path, stack1);
  printf("%s created.\n", file_path);

  Kill_Stack(stack1);
  Kill_Stack(stack2);

  return 0;
}
