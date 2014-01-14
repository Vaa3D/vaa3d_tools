#include <stdio.h>

#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Stack  *stack = Read_Stack("/Users/zhaot/Data/neurolineage/2_to_5_lineages/hard-syn-d120-hs48-B2R.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 1, NULL);
  Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, 0.255);
  Write_Stack("../data/fly_neuron3_org.tif", grey_stack);
  Write_Stack("../data/fly_neuron3.tif", resample_stack);

  return 1;
}
