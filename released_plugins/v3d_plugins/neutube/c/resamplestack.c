#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_darray.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  if (argc < 5) {
    printf("resamplestack <src> <dst> <res_xy> <res_z>\n");
    return 1;
  }
  
  if (strcmp(argv[1], argv[2]) == 0) {
    printf("Source and target are the same. Nothing to be done.\n");
    return 1;
  }

  Stack  *stack = Read_Stack(argv[1]);

  double res_xy = atof(argv[3]);
  double res_z = atof(argv[4]);
  printf("%g\n", res_xy / res_z);
  Stack *resample_stack = Resample_Stack_Depth(stack, NULL, res_xy / res_z);

  Write_Stack(argv[2], resample_stack);

  Kill_Stack(stack);
  Kill_Stack(resample_stack);

  return 1;
}
