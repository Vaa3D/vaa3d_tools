#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_matlabio.h"
#include "tz_voxel_graphics.h"
#include "tz_darray.h"
#include "tz_geo3d_scalar_field.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{
  char neuron_name[100];
  char matlab_path[100];

  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  if (argc < 3) {
    strcpy(matlab_path, "/Applications/MATLAB74/bin/matlab");
  } else {
    strcpy(matlab_path, argv[2]);
  }

  char file_path[100];

  /*
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *stack = Read_Stack(file_path);
  Stack_Not(stack, stack);

  Set_Matlab_Path(matlab_path);
  Stack *dist = Stack_Bwdist(stack);
  */

  
  sprintf(file_path, "../data/%s/dist.tif", neuron_name);
  Stack *dist = Read_Stack(file_path);
  
  //Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_ALTER1);
  sprintf(file_path, "../data/%s/seeds.tif", neuron_name);
  Stack *seeds = Read_Stack(file_path);
  

  //sprintf(file_path, "../data/%s/grow_bundle.tif", neuron_name);
  sprintf(file_path, "../data/%s/bundle.tif", neuron_name);
  Stack *bundle = Read_Stack(file_path);
  Stack_And(seeds, bundle, seeds);

  sprintf(file_path, "../data/%s/bundle_seed.tif", neuron_name);
  Write_Stack(file_path, seeds);
  printf("%s created.\n", file_path);

  Voxel_List *list = Stack_To_Voxel_List(seeds);
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);
  sprintf(file_path, "../data/%s/bundle_seeds.pa", neuron_name);
  Pixel_Array_Write(file_path, pa);
  printf("%s created.\n", file_path);

  int i;
  //double *pa_array = (double *) pa->array;
  uint16 *pa_array16 = (uint16 *) pa->array;
  double *pa_array = darray_malloc(pa->size);
  for (i = 0; i < pa->size; i++) {
    pa_array[i] = sqrt(pa_array16[i]);
  }

  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);
  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(pa->size);

  for (i = 0; i < pa->size; i++) {
    field->points[i][0] = voxel_array[i]->x;
    field->points[i][1] = voxel_array[i]->y;
    field->points[i][2] = voxel_array[i]->z;
    field->values[i] = pa_array[i];
  }

  sprintf(file_path, "../data/%s/seeds", neuron_name);
  Write_Geo3d_Scalar_Field(file_path, field);
  printf("%s created.\n", file_path);

  printf("mean: %g, std: %g\n", gsl_stats_mean(pa_array, 1, pa->size), 
	 sqrt(gsl_stats_variance(pa_array, 1, pa->size)));
  printf("%g\n", gsl_stats_max(pa_array, 1, pa->size));

  return 0;
}
