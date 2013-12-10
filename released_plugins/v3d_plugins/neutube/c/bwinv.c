/* bwinv.c
 *
 * 23-Jun-2008 Initial write: Ting Zhao
 */

#include <utilities.h>
#include "tz_stack_math.h"

/*
 * bwinv imgfile -o out
 */
int main(int argc, char *argv[])
{
  static char *Spec[] = {
    "<image:string> -o <string>",
    NULL};

  Process_Arguments(argc, argv, Spec, 1);

  Stack *stack = Read_Stack(Get_String_Arg("image"));
  Stack_Not(stack, stack);
  Write_Stack(Get_String_Arg("-o"), stack);

  Kill_Stack(stack);
  
  return 1;
}
