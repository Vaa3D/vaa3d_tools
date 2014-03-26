#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_pixel_array.h"
#include "tz_stack_bwmorph.h"
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
  Stack_Threshold(stack, 1);
  Stack_Binarize(stack);
  
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *stack2 = Read_Stack(file_path);
  Stack_Xor(stack, stack2, stack2);
  sprintf(file_path, "../data/%s/bundle.tif", neuron_name);
  Write_Stack(file_path, stack2);
  printf("%s created.\n", file_path);

  /*
  Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
  Stack *stack3 = Stack_Dilate(stack, NULL, se);
  Stack_And(stack3, stack2, stack2);

  sprintf(file_path, "../data/%s/objseed.tif", neuron_name);
  Write_Stack(file_path, stack2);  
  */
  return 0;
}
