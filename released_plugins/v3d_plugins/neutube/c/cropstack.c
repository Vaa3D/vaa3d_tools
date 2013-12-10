/* cropstack.c
 *
 * 17-Oct-2008
 */

/* cropstack: crop a stack
 *
 * cropstack infile -o outfile 
 * [-x<int>] [-y<int>] [-z<int>] [-w<int>] [-h<int>] [-d<int>]
 */

#include <utilities.h>
#include "tz_stack_lib.h"
#include "tz_image_io.h"

static int get_int_arg(const char *option, int default_value)
{
  if (Is_Arg_Matched((char*) option)) {
    return Get_Int_Arg((char*) option);
  }
  
  return default_value;
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -o <string>",
			 "[-x<int>] [-y<int>] [-z<int>]",
			 "[-w<int>] [-h<int>] [-d<int>]",
			 NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  Stack *stack = Read_Stack_U(Get_String_Arg("image"));
  
  int x = get_int_arg("-x", 0);
  int y = get_int_arg("-y", 0);
  int z = get_int_arg("-z", 0);
  int w = get_int_arg("-w", stack->width);
  int h = get_int_arg("-h", stack->height);
  int d = get_int_arg("-d", stack->depth);

  Stack *substack = Crop_Stack(stack, x, y, z, w, h, d, NULL);

  Write_Stack_U(Get_String_Arg("-o"), substack, Get_String_Arg("image"));

  return 0;
}
