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
  /* save resolution information */
  double res[] = {0.7, 0.7, 1.6};
  darray_write("../data/mouse_neuron2_org.res", res, 3);

  int length;
  darray_read("../data/mouse_neuron2_org.res", res, &length);
  darray_print2(res, 3, 1);  
  
  double z_scale = res[0] / res[2];

  File_Bundle fb;
  fb.prefix = "/Users/zhaot/Data/neuromorph/mouse/ctrl/ctrl";
  fb.suffix = NULL;
  fb.num_width = 1;
  fb.first_num = 0;

  Stack *grey_stack = Read_Stack_Planes(&fb);
  Print_Stack_Info(grey_stack);

  Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, z_scale);

  Write_Stack("../data/mouse_neuron2_org.tif", grey_stack);
  Write_Stack("../data/mouse_neuron2.tif", resample_stack);

  int stat = 
    symlink("../data/mouse_neuron2_org.tif", "../data/mouse_neuron2_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
  }

  Kill_Stack(grey_stack);
  Kill_Stack(resample_stack);

  return 1;
}
