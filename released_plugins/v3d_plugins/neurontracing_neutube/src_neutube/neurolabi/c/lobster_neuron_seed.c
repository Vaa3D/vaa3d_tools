#include <stdio.h>

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

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  Stack *stack = Read_Stack("../data/lobster_neuron_single.tif");
  Stack_Not(stack, stack);

  Stack *dist = Stack_Bwdist(stack);
  Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_ALTER1);
  
  Write_Stack("../data/lobster_neuron_seed.tif", seeds);


  Voxel_List *list = Stack_To_Voxel_List(seeds);
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);
  Pixel_Array_Write("../data/lobster_neuron_seeds.pa", pa);

  return 0;
}
