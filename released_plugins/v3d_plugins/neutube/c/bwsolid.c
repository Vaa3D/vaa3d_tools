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
 * bwsolid - make objects in a binary stack more solid
 *
 * bwsolid [-mf value] infile -o outfile
 */

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-mf <int>]",
			 " <image:string>",
			 " -o <string>",
			 NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  char *image_file = Get_String_Arg("image");
  Stack *stack = Read_Stack(image_file);

  Stack *clear_stack = NULL;

  if (Is_Arg_Matched("-mf")) {  
    printf("Majority filtering ...\n");
    int mnbr = Get_Int_Arg("-mf");
    clear_stack = Stack_Majority_Filter_R(stack, NULL, 26, mnbr);
    Kill_Stack(stack);
  } else {
    clear_stack = stack;
  }

  printf("Dilating ...\n");
  Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
  Stack *dilate_stack = Stack_Dilate(clear_stack, NULL, se);
  Kill_Stack(clear_stack);

  /*
  printf("Hole filling ...\n");
  Stack *fill_stack = Stack_Fill_Hole_N(dilate_stack, NULL, 1, 4, NULL);
  Kill_Stack(dilate_stack);
  */
  Stack *fill_stack = dilate_stack;

  printf("Eroding ...\n");
  Stack *mask = Stack_Erode_Fast(fill_stack, NULL, se);  
  Kill_Stack(fill_stack);

  char *out_file = Get_String_Arg("-o");
  Write_Stack(out_file, mask);
  printf("%s created.\n", out_file);

  Kill_Stack(mask);

  return 0;
}
