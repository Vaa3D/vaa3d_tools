/**@file diadem_c1_data.c
 * @author Ting Zhao
 * @date 05-Apr-2010
 */

#include <stdio.h>
#include <string.h>
#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_error.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int depth = dir_fnum(Get_String_Arg("input"), "tif");
  
  Stack *stack = Make_Stack(GREY, 512, 512, depth);
  
  char sname[500];
  int i;
  int area = stack->width * stack->height;
  int offset = 0;
  for (i = 1; i <= depth; i++) {
    sprintf(sname,"%s/%02d.tif", Get_String_Arg("input"), i);
    printf("Reading %s\n", sname);
    TZ_ASSERT(fexist(sname), "File does not exist");
    Stack *slice = Read_Stack(sname);
    memcpy(stack->array + offset, slice->array, area);
    offset += area;
    Free_Stack(slice);
  }

  Write_Stack(Get_String_Arg("-o"), stack);

  return 0;
}
