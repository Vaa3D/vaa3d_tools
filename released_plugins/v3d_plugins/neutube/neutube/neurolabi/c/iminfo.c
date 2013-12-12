/**@file iminfo.c
 * @author Ting Zhao
 * @date 28-Jul-2010
 */

#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_int_histogram.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> [-size] [-hist]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (argc == 2) {
    int size[3] = {0, 0, 0};
    if (Is_Lsm(Get_String_Arg("image")) == 1) {
      Lsm_Size(Get_String_Arg("image"), size);
    } else if (Is_Tiff(Get_String_Arg("image")) == 1) { 
      Tiff_Size(Get_String_Arg("image"), size);
    } else {
      printf("No information for this format.\n");
    }
    printf("%d x %d x %d\n", size[0], size[1], size[2]);
  } else {

    Stack *stack = Read_Stack(Get_String_Arg("image"));

    if (Is_Arg_Matched("-hist")) {
      int *hist = Stack_Hist(stack);
      Print_Int_Histogram(hist);
    }

    Kill_Stack(stack);
  }

  return 0;
}
