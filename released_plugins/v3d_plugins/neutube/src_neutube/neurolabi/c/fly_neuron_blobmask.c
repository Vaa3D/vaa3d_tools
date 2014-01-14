#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>
#include <utilities.h>
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
#include "tz_stack_sampling.h"
#include "tz_stack_draw.h"

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

  //sprintf(file_path, "../data/%s/seeds.pa", neuron_name);
  //Pixel_Array *pa = Pixel_Array_Read(file_path);
  //double *pa_array = (double *) pa->array;
  //double max_dist = gsl_stats_max(pa_array, 1, pa->size);

  sprintf(file_path, "../data/%s/seeds.ar", neuron_name);
  int length;
  double *pa_array = darray_read(file_path, &length);
  gsl_sort(pa_array, 1, length);

  //double mean_dist = gsl_stats_median_from_sorted_data(pa_array, 1, length);
  double mean_dist = darray_mean(pa_array, length);
  double std_dist = sqrt(gsl_stats_variance(pa_array, 1, length));
  
  /* for fly neuron data, a good eroding size is around 6*/
  int erode_size = (int) (mean_dist + std_dist + 0.5);
  //int erode_size = 6;

  //int erode_size = (int)
  //  round(gsl_stats_quantile_from_sorted_data(pa_array, 1, length, 0.85));
  //int erode_size = 6;

  //darray_write("../data/test.bn",pa_array, length);

  printf("%d\n", erode_size);

  int dilate_size = erode_size * 2;

  Struct_Element *se = Make_Ball_Se(erode_size);
  Stack *erode_mask = Stack_Erode_Fast(mask, NULL, se);
  
  Free_Se(se);

  se = Make_Ball_Se(dilate_size);
  Stack *dilate_mask = Stack_Dilate(erode_mask, NULL, se);


  /*
  Stack *erode_region = Copy_Stack(erode_mask);
  int region_start = 2;
  int nobj = Stack_Label_Objects_N(erode_region, NULL, 1, region_start, 26);
  double *dist = darray_malloc(nobj);
  int i;
  for (i = 0; i < nobj; i++) {
    dist[i] = 0.0;
  }

  sprintf(file_path, "../data/%s/erode_region.tif", neuron_name);
  Write_Stack(file_path, erode_region);

  sprintf(file_path, "../data/%s/seeds.tif", neuron_name);
  Stack *seed_stack = Read_Stack(file_path);
  Voxel_List *list = Stack_To_Voxel_List(seed_stack);
  Kill_Stack(seed_stack);
  Pixel_Array *region_pa = Voxel_List_Sampling(erode_region, list);
  uint8 *region_array = (uint8 *) region_pa->array;
  //  u8array_print2(region_array, region_pa->size, 1);

  dist -= region_start;
  for (i = 0; i < region_pa->size; i++) {
    if (region_array[i] >= region_start) {
      if (dist[region_array[i]] <  pa_array[i]) {
	dist[region_array[i]] = pa_array[i];
      }
    }
  }
  dist += region_start;

  darray_print2(dist, nobj, 1);

  Stack *dilate_mask = Copy_Stack(erode_mask);
  Stack *tmp_dilate = Copy_Stack(erode_mask);
  for (i = 0; i < nobj; i++) {
    se = Make_Ball_Se((int) (dist[i] + 0.5));
    Stack_Dilate_Rm(erode_mask, tmp_dilate, se, 
		    erode_region, region_start + i);
    Stack_Or(dilate_mask, tmp_dilate, dilate_mask);
    Free_Se(se);
  }
  
  Kill_Stack(tmp_dilate);
  */

  Stack_And(mask, dilate_mask, dilate_mask);

  sprintf(file_path, "../data/%s/blobmask.tif", neuron_name);
  Write_Stack(file_path, dilate_mask);
  printf("%s created.\n", file_path);

  sprintf(file_path, "../data/%s/large_seeds.tif", neuron_name);
  Stack *seeds = Read_Stack(file_path);
  Stack *blob = dilate_mask;
  Stack_Not(blob, blob);
  Stack_And(seeds, blob, seeds);

  Kill_Stack(mask);
  Kill_Stack(dilate_mask);
  Kill_Stack(erode_mask);

  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *signal = Read_Stack(file_path);
  Stack *canvas = Translate_Stack(signal, COLOR, 0);
  Stack_Draw_Voxel_Mc(canvas, seeds, 4, 255, 0, 0);
  sprintf(file_path, "../data/%s/seedimg.tif", neuron_name);
  Write_Stack(file_path, canvas);
  printf("%s (show where tracing seeds are located) created.\n", file_path);

  return 0;
}
