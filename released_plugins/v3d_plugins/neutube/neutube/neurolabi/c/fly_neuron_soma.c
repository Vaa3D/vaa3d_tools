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
  
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *stack2 = Read_Stack(file_path);
  Stack *stack3 = Copy_Stack(stack2);

  sprintf(file_path, "../data/%s/blobmask.tif", neuron_name);
  Stack *stack1 = Read_Stack(file_path);
  Object_3d_List *objs = Stack_Find_Object_N(stack1, NULL, 1, 7, 26);

  sprintf(file_path, "../data/%s/holeimg.tif", neuron_name);
  Stack *hole_stack = Read_Stack(file_path);
  
  int value;
  int label = 2;

  char id_file_path[100];
  sprintf(id_file_path, "../data/%s/region_id.txt", neuron_name);

  FILE *id_fp = fopen(id_file_path, "w");

  while (objs != NULL) {
    int hole_area = Stack_Foreground_Area_O(hole_stack, objs->data);
    printf("%d / %lu\n", hole_area, objs->data->size);
    //if (hole_area > (int) ((double) (objs->data->size) * 0.01)) {
    if (hole_area > 100) {
      value = 3;
    } else {
      value = 2;
    }
    
    if (objs->data->size + hole_area < TZ_PI * 10.0 * 10.0) {
      TRACE("small object labeled");
    }

    fprintf(id_fp, "%d %d\n", label, value);

    Stack_Draw_Object_Bw(stack3, objs->data, label++);
    Stack_Draw_Object_Bw(stack2, objs->data, value);

    objs = objs->next;
  }

  fprintf(id_fp, "-1 -1\n");

  fclose(id_fp);

  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Write_Stack(file_path, stack2);
  printf("%s (2: arbor, 3: soma) created.\n", file_path);

  sprintf(file_path, "../data/%s/region_label.tif", neuron_name);
  Write_Stack(file_path, stack3);
  printf("%s (one unique label for one region) created.\n", file_path);

  Kill_Stack(stack1);
  Kill_Stack(stack2);
  Kill_Stack(stack3);
  Kill_Stack(hole_stack);

  return 0;
}
