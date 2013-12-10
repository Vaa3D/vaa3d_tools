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
  sprintf(file_path, "../data/%s_org.tif", neuron_name);
  Stack *org_stack = Read_Stack(file_path);

  int thre = Stack_Find_Threshold_Locmax2(org_stack, 0, 65535, 0.8);
  printf("threshold: %d\n", thre);
  Kill_Stack(org_stack);

  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *stack = Read_Stack(file_path);

  Stack_Threshold(stack, thre);
  Stack_Binarize(stack);

  Translate_Stack(stack, GREY, 1);

  sprintf(file_path, "../data/%s/threshold.tif", neuron_name);
  Write_Stack(file_path, stack);
  printf("%s created.\n", file_path);

  Stack *clear_stack = Stack_Majority_Filter_R(stack, NULL, 26, 8);
  Kill_Stack(stack);

  sprintf(file_path, "../data/%s/clear_mask.tif", neuron_name);
  Write_Stack(file_path, clear_stack);
  printf("%s created.\n", file_path);
 
  Struct_Element *se = Make_Disc_Se(2);
  Stack *dilate_stack = Stack_Dilate(clear_stack, NULL, se);
  Kill_Stack(clear_stack);

  Stack *fill_stack = Stack_Fill_Hole_N(dilate_stack, NULL, 1, 4, NULL);
  Kill_Stack(dilate_stack);

  Stack *mask = Stack_Erode_Fast(fill_stack, NULL, se);  
  Kill_Stack(fill_stack);

  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Write_Stack(file_path, mask);
  printf("%s (foreground mask) created.\n", file_path);

  Free_Se(se);
  Kill_Stack(mask);

  return 0;
}
