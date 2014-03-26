#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_objdetect.h"
#include "tz_stack_draw.h"
#include "tz_stack_bwmorph.h"
#include "tz_object_3d_linked_list_ext.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s/holemask.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);

  /*
  Object_3d_List *holes = Stack_Find_Hole_N(stack, 8, NULL);
  Object_3d *obj = Object_3d_List_To_Object(holes, NULL);
  
  sprintf(file_path, "../data/%s/holes.obj", neuron_name);
  Save_Object_3d(obj, file_path);

  Zero_Stack(stack);
  Stack_Draw_Objects_Bw(stack, holes, 1);
  */

  Stack_Label_Background_N(stack, 1, 4, NULL);
  Stack_Not(stack, stack);
  sprintf(file_path, "../data/%s/holeimg.tif", neuron_name);
  Write_Stack(file_path, stack);
  printf("%s created ...\n", file_path);

  Kill_Stack(stack);

  return 0;
}
