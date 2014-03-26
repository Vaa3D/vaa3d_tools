/**@file imds.c
 * @author Ting Zhao
 * @date 10-Aug-2012
 */

#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"

enum {
  DS_NEAREST, DS_MAX, DS_MEAN
};

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string> --intv <int> <int> <int>",
    "[--option <string>] [--fgc]",
    NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Stack *stack = Read_Stack_U(Get_String_Arg("input"));
  Stack *out = NULL;
  
  int option = DS_NEAREST;
  if (Is_Arg_Matched("--option")) {
    const char *arg = Get_String_Arg("--option");
    if (eqstr(arg, "max")) {
      option = DS_MAX;
    } else if (eqstr(arg, "mean")) {
      option = DS_MEAN;
    } else if (eqstr(arg, "nearest")) {
      option = DS_NEAREST;
    } else {
      printf("Invalid option: %s\n. The default option (nearest) will be used.", arg);
    }
  }

  switch (option) {
    case DS_NEAREST:
      out = Downsample_Stack(stack, Get_Int_Arg("--intv", 1), 
          Get_Int_Arg("--intv", 2), Get_Int_Arg("--intv", 3));
      break;
    case DS_MAX:
      out = Downsample_Stack_Max(stack, Get_Int_Arg("--intv", 1), 
          Get_Int_Arg("--intv", 2), Get_Int_Arg("--intv", 3), NULL);
      break;
    case DS_MEAN:
      if (Is_Arg_Matched("--fgc")) {
        out = Downsample_Stack_Mean_F(stack, Get_Int_Arg("--intv", 1), 
            Get_Int_Arg("--intv", 2), Get_Int_Arg("--intv", 3), NULL);
      } else {
        out = Downsample_Stack_Mean(stack, Get_Int_Arg("--intv", 1), 
            Get_Int_Arg("--intv", 2), Get_Int_Arg("--intv", 3), NULL);
      }
      break;
    default:
      break;
  }
      
  Write_Stack(Get_String_Arg("-o"), out);

  return 0;
}
