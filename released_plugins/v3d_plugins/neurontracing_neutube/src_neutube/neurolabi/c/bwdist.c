/* bwdist.c
 *
 * 04-Jun-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include <string.h>
#include "tz_stack_bwmorph.h"
#include "tz_stack_lib.h"
#include "tz_stack_math.h"

/*
 * bwdist - build a distance map for a binary stack
 *
 * bwdist [-b<int>] infile -o outfile
 *
 * -i: inverse the image
 * -b: byte number for output file (1 (default) or 2)
 * -sq: build square distance map
 */
int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -o <string>",
			 "[-b<int> | -sq] [-i] [--plane]",
			 NULL};


  Process_Arguments(argc, argv, Spec, 1);

  char *image_file = Get_String_Arg("image");
  
  Stack *stack = Read_Stack(image_file);

  if (Is_Arg_Matched("-i")) {
    Stack_Not(stack, stack);
  }

  Stack *distmap = NULL;

  if (!Is_Arg_Matched("-sq")) {
    distmap = Stack_Bwdist_L(stack, NULL, NULL);
    Kill_Stack(stack);
    
    int kind = GREY;
    if (Is_Arg_Matched("-b")) {
      kind = Get_Int_Arg("-b");
    }
    
    stack = Scale_Float_Stack((float *) distmap->array, distmap->width,
			     distmap->height, distmap->depth, kind);
    Kill_Stack(distmap);
    distmap = stack;
  } else {
    if (Is_Arg_Matched("--plane")) {
      distmap = Stack_Bwdist_L_U16P(stack, NULL, 0);
      Translate_Stack(distmap, GREY, 1);
    } else {
      distmap = Stack_Bwdist_L_U16(stack, NULL, 0);
    }
    Kill_Stack(stack);
  }

  char *out_file = Get_String_Arg("-o");
  Write_Stack(out_file, distmap);

  Kill_Stack(distmap);
  
  return 1;
}
