/**@file imsample.c
 * @brief >> resample image
 * @author Ting Zhao
 * @date 14-Jun-2009
 */

#include <utilities.h>
#include "tz_stack_lib.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -o <string>", 
			 "[-u | -d] <x:int> <y:int> <z:int>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Stack *out = NULL;

  Stack *stack = Read_Stack(Get_String_Arg("image"));

  int wintv = Get_Int_Arg("x");
  int hintv = Get_Int_Arg("y");
  int dintv = Get_Int_Arg("z");

  if (Is_Arg_Matched("-u")) {
    out = Upsample_Stack(stack, wintv, hintv, dintv, NULL);
  } else if (Is_Arg_Matched("-d")) {
    out = Downsample_Stack_Mean(stack, wintv, hintv, dintv, NULL);    
  }

  if (out != NULL) {
    Write_Stack(Get_String_Arg("-o"), out);
  }

  return 0;
}
