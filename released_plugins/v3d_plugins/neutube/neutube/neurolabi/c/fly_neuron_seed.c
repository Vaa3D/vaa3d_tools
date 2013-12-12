#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_iarray.h"
#include "tz_stack_attribute.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_matlabio.h"
#include "tz_darray.h"
#include "tz_stack_draw.h"
#include "tz_stack_code.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack.h"
#include "tz_voxel_graphics.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  //char matlab_path[100];

  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  /*
  if (argc < 3) {
    strcpy(matlab_path, "/Applications/MATLAB74/bin/matlab");
  } else {
    strcpy(matlab_path, argv[2]);
  }
  */

  char file_path[100];
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);
  //Stack_Not(stack, stack);

  //Set_Matlab_Path(matlab_path);
  //Stack *dist = Stack_Bwdist(stack);
  /*
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  ow->conn = 18;
  Stack_Not(stack, stack);
  Stack *dist = Stack_Boundary_Code(stack, NULL, ow);
  Stack_Not(stack, stack);
  */

  Stack *dist = Stack_Bwdist_L_U16(stack, NULL, 1);

  sprintf(file_path, "../data/%s/dist.tif", neuron_name);
  Write_Stack(file_path, dist);
  printf("Distance map %s created.\n", file_path);

  /*
  Write_Stack(file_path, 
	      Scale_Double_Stack((double *)dist->array, stack->width, 
				 stack->height, stack->depth, GREY));
  */

  Kill_Stack(stack);

  //Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_FLAT);
  //Stack_Clean_Locmax(dist, seeds);

  Stack *seeds = Stack_Locmax_Region(dist, 26);
  //Write_Stack("../data/test.tif", seeds);

  Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);
  Zero_Stack(seeds);
  int objnum = 0;
  while (objs != NULL) {
    Object_3d *obj = objs->data;
    Voxel_t center;
    Object_3d_Central_Voxel(obj, center);
    Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
    objs = objs->next;
    objnum++;
  }
  printf("objnum: %d\n", objnum);
  sprintf(file_path, "../data/%s/seeds.tif", neuron_name);
  Write_Stack(file_path, seeds);
  printf("%s (seed mask) created.\n", file_path);

  /*
  int max_idx = 0;
  double max_dist = darray_max((double *) (dist->array), 
			       Stack_Voxel_Number(dist),
			       &max_idx);
  */
  
  Voxel_List *list = Stack_To_Voxel_List(seeds);
  printf("%d\n", Voxel_List_Length(list));
  int *seed_offset = (int *) malloc(sizeof(int) * Voxel_List_Length(list));
  int i;
  int idx = Voxel_List_Length(list) - 1;
  int stack_length = Stack_Voxel_Number(seeds);
  for (i = 0; i < stack_length; i++) {
    if (seeds->array[i] > 0) {
      seed_offset[idx--] = i;
    }
  }

  if (idx != -1) {
    TZ_WARN(ERROR_DATA_VALUE);
  }

  sprintf(file_path, "../data/%s/seed_offset.ar", neuron_name);
  iarray_write(file_path, seed_offset, Voxel_List_Length(list));
  printf("%s (seed indices) created.\n", file_path);
  
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);

  sprintf(file_path, "../data/%s/seeds.pa", neuron_name);
  Pixel_Array_Write(file_path, pa);
  printf("%s (seed values d^2) created.\n", file_path);

  //double *pa_array = (double *) pa->array;
  uint16 *pa_array16 = (uint16 *) pa->array;
  double *pa_array = darray_malloc(pa->size);
  for (i = 0; i < pa->size; i++) {
    pa_array[i] = sqrt(pa_array16[i]);
  }

  sprintf(file_path, "../data/%s/seeds.ar", neuron_name);
  darray_write(file_path, pa_array, pa->size);
  printf("%s (seed values d) created.\n", file_path);

  /*
  printf("mean: %g, std: %g\n", gsl_stats_mean(pa_array, 1, pa->size), 
	 sqrt(gsl_stats_variance(pa_array, 1, pa->size)));
  printf("%g\n", gsl_stats_max(pa_array, 1, pa->size));
  */
  //int max_idx2;
  //double max_dist2 = darray_max(pa_array, pa->size, &max_idx2);
  
  int n = Stack_Voxel_Number(seeds);
  //double *dist_array = (double *) dist->array;  
  uint16 *dist_array = (uint16 *) dist->array;  
  for (i = 0; i < n; i++) {
    if (dist_array[i] == 1) {
      seeds->array[i] = 0;
    }
  }
  sprintf(file_path, "../data/%s/large_seeds.tif", neuron_name);
  Write_Stack(file_path, seeds);
  printf("%s (seeds for d > 1) created.\n", file_path);
  
  Kill_Stack(dist);
  Kill_Stack(seeds);
  free(seed_offset);
  Kill_Pixel_Array(pa);

  
  return 0;
}
