#include <stdio.h>
#include <string.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_pixel_array.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_draw.h"
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
  
  sprintf(file_path, "../data/%s/objseed.tif", neuron_name);
  Stack *seedimg = Read_Stack(file_path);
  Object_3d_List *objs = Stack_Find_Object_N(seedimg, NULL, 1, 0, 26);

  Stack *stack2 = NULL;
  Object_3d *obj = NULL;
  stack2 = NULL;

  Object_3d_List *objs2 = NULL; 

  sprintf(file_path, "../data/%s/bundle.tif", neuron_name);
  stack2 = Read_Stack(file_path);

  while (objs != NULL) {
    obj = Stack_Grow_Object(stack2, 1, objs->data->voxels[0]);
    if (obj != NULL) {
      if (obj->size > 1000) {
	if (objs2 == NULL) {
	  objs2 = Object_3d_List_New();
	  objs2->data = obj;
	} else {
	  Object_3d_List_Add(&objs2, obj);
	  Print_Object_3d_Info(obj);
	}
      } else {
	Kill_Object_3d(obj);
      }
    }
    objs = objs->next;
  }

  Print_Object_3d_List_Compact(objs2);

  Stack *bundle = Make_Stack(GREY, stack2->width, stack2->height, stack2->depth);
  Zero_Stack(bundle);

  Stack_Draw_Objects_Bw(bundle, objs2, 1);

  sprintf(file_path, "../data/%s/grow_bundle.tif", neuron_name);
  Write_Stack(file_path, bundle);

  return 0;
}
