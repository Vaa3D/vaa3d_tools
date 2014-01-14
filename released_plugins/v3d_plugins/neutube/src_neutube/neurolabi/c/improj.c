/**@file improj.c
 * @brief >> stack projection
 * @author Ting Zhao
 * @date 31-Dec-2009
 */

#include <string.h>
#include "tz_utilities.h"
#include "tz_mc_stack.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string> [--dim <string>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("--dim")) {
    if (strcmp(Get_String_Arg("--dim"), "x") == 0 || 
        strcmp(Get_String_Arg("--dim"), "X") == 0) {
      Stack *stack = Read_Stack(Get_String_Arg("input"));
      Image *image = Proj_Stack_Xmax(stack);
      Write_Image(Get_String_Arg("-o"), image);
    }
  } else {
    Mc_Stack *stack = Read_Mc_Stack(Get_String_Arg("input"), -1);
    Mc_Stack *proj = Mc_Stack_Mip(stack);

    Write_Mc_Stack(Get_String_Arg("-o"), proj, NULL);
  }
  return 0;
}
