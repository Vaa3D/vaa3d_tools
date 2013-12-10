#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_darray.h"
#include "tz_stack_io.h"
#include "tz_image_io.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  /* save resolution information */
  double res[] = {0.5375, 0.5375, 1.0};

  FILE *fp = fopen("../data/mouse_single3_org.res", "w");
  fprintf(fp, "%g %g %g\n", res[0], res[1], res[2]);

  /*
  darray_write("../data/mouse_single_org.res", res, 3);

  int length;
  double *res2 = darray_read("../data/mouse_single_org.res", &length);
  darray_print2(res2, 3, 1);  
  
  double z_scale = res[0] / res[2];
  */

  File_Bundle_S fb;
  //fb.prefix = "/Users/zhaot/Data/neuromorph/Stacks/Z-11-May-2007-13-05-006 c5/Z-11-May-2007-13-05-006_Ch1_000";
  fb.prefix = "/Users/zhaot/Data/neuromorph/Stacks/Z-14-May-2007-13-39-005 c3/Z-14-May-2007-13-39-005_Ch1_000";
  fb.suffix = NULL;
  fb.num_width = 3;
  fb.first_num = 1;
  fb.last_num = -1;

  Stack *grey_stack = Read_Stack_Planes_S(&fb);
  Print_Stack_Info(grey_stack);

  //Stack *resample_stack = Resample_Stack_Depth(grey_stack, NULL, z_scale);

  Write_Stack("../data/mouse_single3_org.tif", grey_stack);

  Kill_Stack(grey_stack);
  //Kill_Stack(resample_stack);

  return 1;
}
