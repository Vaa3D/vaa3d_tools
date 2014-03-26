#include <stdio.h>
#include <string.h>
#include <gsl/gsl_statistics.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_stack_bwmorph.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_stack_attribute.h"
#include "tz_dimage_lib.h"
#include "tz_arrayview.h"
#include "tz_dmatrix.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];

#if 0 /* local maxima */
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *stack1 = Read_Stack(file_path);

  sprintf(file_path, "../data/%s/highpass_locmin.tif", neuron_name);
  Stack *stack2 = Read_Stack(file_path);
  
  Stack_Add(stack1, stack2, stack1);
  sprintf(file_path, "../data/%s/cell_mark.tif", neuron_name);
  Write_Stack(file_path, stack1);
#endif

#if 0
  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *stack1 = Read_Stack(file_path);

  DMatrix *filter = Mexihat_2D_D(2.0, NULL);
  filter->ndim = 3;
  filter->dim[2] = 1;

  DMatrix *out = Filter_Stack_Fast_D(stack1, filter, NULL, 0);

# if 1
  int nvoxel = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < nvoxel; i++) {
    out->array[i] = fabs(out->array[i]);
  } 
# endif

  Stack *stack2 = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
				   out->dim[2], GREY);



  sprintf(file_path, "../data/%s/edge.tif", neuron_name);
  Write_Stack(file_path, stack2);

  Kill_Stack(stack1);
  Kill_Stack(stack2);
#endif

#if 1
  sprintf(file_path, "../data/%s/mask.tif", neuron_name);
  Stack *stack2 = Read_Stack(file_path);
  sprintf(file_path, "../data/%s/seeds.pa", neuron_name);
  Pixel_Array *pa = Pixel_Array_Read(file_path);
  double *pa_array = (double *) pa->array;
  double max_dist = gsl_stats_max(pa_array, 1, pa->size);
  double mean_dist = gsl_stats_mean(pa_array, 1, pa->size);
  double std_dist = sqrt(gsl_stats_variance(pa_array, 1, pa->size));
  
  int erode_size = (int) (mean_dist + std_dist * 2 + 0.5);
  Struct_Element *se = Make_Ball_Se(erode_size);
  Stack *stack3 = Stack_Erode_Fast(stack2, NULL, se);
  int dilate_size = (int) (max_dist + 0.5);
  se = Make_Ball_Se(dilate_size);
  Stack *stack4 = Stack_Dilate(stack3, NULL, se);

  Stack_Sub(stack2, stack4, stack2);
  Write_Stack("../data/test.tif", stack2);

  Kill_Stack(stack2);
  Kill_Stack(stack3);
  Kill_Stack(stack4);
  Kill_Pixel_Array(pa);
  
#endif

  return 0;
}
