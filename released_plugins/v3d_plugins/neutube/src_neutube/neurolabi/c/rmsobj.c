/* rmsobj.c
 *
 * 06-Oct-2008 Initial write: Ting Zhao
 */

#include <utilities.h>
#include "tz_stack_objlabel.h"
#include "tz_stack_threshold.h"

/* rmsobj image -s threshold -o output [-n #neighbor]
 */

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -s <int> -o <string>",
			 "[-n <int>]",
			 NULL};

  Process_Arguments(argc, argv, Spec, 1);
  
  Stack *stack = Read_Stack(Get_String_Arg("image"));
  
  int n_nbr = 26;
  if (Is_Arg_Matched("-n")) {
    n_nbr = Get_Int_Arg("-n");
  }

  Stack_Label_Large_Objects_N(stack, NULL, 1, 2, Get_Int_Arg("-s") + 1, n_nbr);
  
  Stack_Threshold_Binarize(stack, 2);
  
  Write_Stack(Get_String_Arg("-o"), stack);

  return 0;
}
