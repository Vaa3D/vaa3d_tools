#include <stdio.h>
#include <string.h>
#include "tz_vrml_io.h"
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_pixel_array.h"
#include "tz_stack_bwmorph.h"
#include "tz_dimage_lib.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_stack_objlabel.h"
#include "tz_voxel_linked_list.h"
#include "tz_voxel_graphics.h"
#include "tz_dmatrix.h"
#include "tz_string.h"
#include "tz_stack_sampling.h"

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
  Stack *soma_mask = Read_Stack(file_path);

  sprintf(file_path, "../data/%s/seeds.pa", neuron_name);
  Pixel_Array *pa = Pixel_Array_Read(file_path);
  double *pa_array = (double *) pa->array;

  sprintf(file_path, "../data/%s/seeds.tif", neuron_name);
  Stack *seed_stack = Read_Stack(file_path);
  Voxel_List *list = Stack_To_Voxel_List(seed_stack);
  Kill_Stack(seed_stack);

  Pixel_Array *region_pa = Voxel_List_Sampling(soma_mask, list);
  uint8 *region_array = (uint8 *) region_pa->array;

  sprintf(file_path, "../data/%s/soma.wrl", neuron_name);
  FILE *fp = fopen(file_path, "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  int i;
  for (i = 0; i < region_pa->size; i++) {
    if (region_array[i] == 3) {
      Vrml_Node_Begin_Fprint(fp, "Transform", 0);
      fprint_space(fp, 2);
      fprintf(fp, "translation %d %d %d\n", list->data->x, list->data->y,
	      list->data->z);
      fprint_space(fp, 2);
      fprintf(fp, "children\n");
      Vrml_Node_Begin_Fprint(fp, "Shape", 2);
      /* print a sphere */
      Vrml_Sphere_Fprint(fp, pa_array[i], 4);
      Vrml_Node_End_Fprint(fp, "Shape", 2);
      Vrml_Node_End_Fprint(fp, "Transform", 0);
    }
    list = list->next;
  }

  fclose(fp);

  return 0;
}
