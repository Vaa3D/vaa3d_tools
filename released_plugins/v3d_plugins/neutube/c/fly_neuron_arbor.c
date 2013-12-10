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
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];  
  
  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Stack *stack = Read_Stack(file_path);
  int i;
  int n = Stack_Voxel_Number(stack);
  for (i = 0; i < n; i++) {
    stack->array[i] = (stack->array[i] == 2);
  }
  
  sprintf(file_path, "../data/%s/arbor.tif", neuron_name);
  Write_Stack(file_path, stack);

  Kill_Stack(stack);

  return 0;
}
