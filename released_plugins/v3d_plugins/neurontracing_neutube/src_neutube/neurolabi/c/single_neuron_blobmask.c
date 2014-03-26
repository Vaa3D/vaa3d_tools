#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_iarray.h"
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
#include "tz_stack_sampling.h"
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
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);

  Stack *mask = Read_Stack(file_path);

  sprintf(file_path, "../data/%s/seeds.pa", neuron_name);
  Pixel_Array *pa = Pixel_Array_Read(file_path);
  double *pa_array = (double *) pa->array;
  
  sprintf(file_path, "../data/%s/seed_offset.ar", neuron_name);
  int seed_number = pa->size;
  int *seed_offset = (int *) malloc(sizeof(int) * seed_number);
  iarray_read(file_path, seed_offset, &seed_number);

  int max_idx;
  double max_dist = darray_max(pa_array, pa->size, &max_idx);
  max_idx = seed_offset[max_idx];

  Stack_Label_Object_Dist_N(mask, NULL, max_idx, 1, 2, max_dist * 2.0, 26);

  Stack_Threshold_Binarize(mask, 1);

  sprintf(file_path, "../data/%s/blobmask.tif", neuron_name);
  Write_Stack(file_path, mask);
  
  free(seed_offset);
  Kill_Stack(mask);

  return 0;
}
