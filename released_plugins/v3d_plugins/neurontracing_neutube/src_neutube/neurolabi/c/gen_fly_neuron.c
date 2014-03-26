#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_darray.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Stack  *stack = Read_Stack("/Users/zhaot/Data/neurolineage/2_to_5_lineages/easy-syn-d120-hs24-B15L.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 1, NULL);


  /* save resolution information */
  double res[] = {0.51, 0.51, 2.0};
  darray_write("../data/fly_neuron_org.res", res, 3);

  Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, 
					       res[0] / res[2]);

  Write_Stack("../data/fly_neuron_org.tif", grey_stack);
  Write_Stack("../data/fly_neuron.tif", resample_stack);

  int stat = 
    symlink("../data/fly_neuron_org.tif", "../data/fly_neuron_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
  }

  int length;
  darray_read2("../data/fly_neuron_org.res", res, &length);
  darray_print2(res, 3, 1);

  Kill_Stack(grey_stack);
  Kill_Stack(resample_stack);

  return 1;
}
