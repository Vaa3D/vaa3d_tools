#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_darray.h"
#include "tz_stack_io.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  double res[] = {0.62, 0.62, 1.0};
  darray_write("../data/fly_neuron_a1_org.res", res, 3);

  Stack  *stack = Read_Stack("/Users/zhaot/Data/neurolineage/Volker_9D11/GMR_9D11_AE_01_33-fA01b_C071025_20071106221221390.tif");
  Stack *grey_stack = stack;
  //Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, 0.255);
  //Write_Stack("../data/fly_neuron4_org.tif", grey_stack);
  Write_Stack("../data/fly_neuron_a1_org.tif", grey_stack);

  int stat = 
    symlink("../data/fly_neuron_a1_org.tif", "../data/fly_neuron_a1_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
  }
  return 1;
}
