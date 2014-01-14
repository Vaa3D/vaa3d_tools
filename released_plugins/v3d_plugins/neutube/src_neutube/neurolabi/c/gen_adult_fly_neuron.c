#include <stdio.h>

#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Stack  *stack = Read_Stack("/Users/zhaot/Data/neurolineage/MARCM_clones/Brain 1 Left - WT-72hr-40x-2um-1,0z--5-23-05--#B8leftlobe.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 1, NULL);
  Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, 0.255);
  Write_Stack("../data/adult_fly_neuron_org.tif", grey_stack);
  Write_Stack("../data/adult_fly_neuron.tif", resample_stack);

  return 1;
}
