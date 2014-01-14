/**@file diadem_d1_split.c
 * @author Ting Zhao
 * @date 28-Jul-2010
 */

#include "tz_utilities.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tz_string.h"
#include "tz_error.h"
#include "tz_stack_watershed.h"
#include "tz_stack_attribute.h"
#include "tz_arrayview.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_string.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_relation.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_stat.h"
#include "tz_object_3d_linked_list.h"
#include "tz_objdetect.h"
#include "tz_stack_draw.h"
#include "tz_stack_math.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>",
    "[-count <int>] [-dist <int>] [-minobj <int>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);


  Stack *input = Read_Stack(Get_String_Arg("input"));
  
  int nregion = Stack_Max(input, NULL);
  int nvoxel = Stack_Voxel_Number(input);
  int i;
  Stack *stack = Make_Stack(GREY, Stack_Width(input), Stack_Height(input),
      	Stack_Depth(input));
  Stack *out = Make_Stack(GREY, Stack_Width(input), Stack_Height(input),
      	Stack_Depth(input));
  Zero_Stack(out);

  int nobj = 0;
  for (i = 1; i <= nregion; i++) {
    int j;
    int count = 0;
    for (j = 0; j < nvoxel; j++) {
      stack->array[j] = (input->array[j] == i);
    }

    Stack *out3 = NULL;
    int maxcount = 100000;
    if (Is_Arg_Matched("-count")) {
      maxcount = Get_Int_Arg("-count");
    }
    if (count > maxcount) {
      out3 = Copy_Stack(stack);
      Stack_Addc_M(out3, nobj);
      nobj++;
    } else {
      Stack *distmap = Stack_Bwdist_L_U16P(stack, NULL, 0);

      Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
      ws->mask = Copy_Stack(distmap);
      int mindist = 10;
      if (Is_Arg_Matched("-dist")) {
	mindist = Get_Int_Arg("-dist");
      }
      Stack_Threshold_Binarize(ws->mask, mindist);
      Translate_Stack(ws->mask, GREY, 1);
      int minobj = 100;
      if (Is_Arg_Matched("-minobj")) {
	minobj = Get_Int_Arg("-minobj");
      }
      Object_3d_List *objs = Stack_Find_Object(ws->mask, 1, minobj);
      Zero_Stack(ws->mask);
      Stack_Draw_Objects_Bw(ws->mask, objs, -255);

      ws->min_level = 1;
      ws->start_level = 65535;
      out3 = Stack_Watershed(distmap, ws);
      Stack_Addc_M(out3, nobj);
      nobj += Object_3d_List_Length(objs);
      Kill_Stack(distmap);
      Kill_Stack_Watershed_Workspace(ws);
      Kill_Object_3d_List(objs);
    }
    Stack_Add(out, out3, out);
    Kill_Stack(out3);
  }

  printf("number of regions: %d\n", nobj);
  Write_Stack(Get_String_Arg("-o"), out);
  char cmd[500];
  sprintf(cmd, "touch %s_done", Get_String_Arg("-o"));
  system(cmd);

  return 0;
}
