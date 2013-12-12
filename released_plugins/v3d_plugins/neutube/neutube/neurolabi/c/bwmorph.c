/* bwsolid.c
 *
 * 16-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include <string.h>
#include "tz_error.h"
#include "tz_stack_lib.h"
#include "tz_stack_bwmorph.h"

/*
 * bwmorph - morphological operation
 *
 * bwmorph -M<string> infile -o outfile
 */

int main(int argc, char *argv[])
{
  static char *Spec[] = {"-M<string> [-v <int>] [-conn <int>]",
			 " <image:string> -o <string>",
			 NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  char *image_file = Get_String_Arg("image");
  Stack *stack = Read_Stack(image_file);

  const char *method = Get_String_Arg("-M");

  if (strcmp(method, "mainobj") == 0) {
    int minsize = Get_Int_Arg("-v");
    int conn = 26;
    if (Is_Arg_Matched("-conn")) {
      conn = Get_Int_Arg("-conn");
    }
    Stack_Remove_Small_Object(stack, stack, minsize, conn);
  }

  char *out_file = Get_String_Arg("-o");
  Write_Stack(out_file, stack);

  Kill_Stack(stack);

  return 0;
}
