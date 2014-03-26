#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Stack  *stack = Read_Stack("/Users/zhaot/Data/neurolineage/2_to_5_lineages/easy-syn-d120-hs48-B4R.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 1, NULL);
  Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, 0.255);

  Write_Stack("../data/fly_neuron2_org.tif", grey_stack);
  Write_Stack("../data/fly_neuron2.tif", resample_stack);

  int stat = 
    symlink("../data/fly_neuron_org.tif", "../data/fly_neuron_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
  }

  Kill_Stack(grey_stack);
  Kill_Stack(resample_stack);

  return 1;
}
