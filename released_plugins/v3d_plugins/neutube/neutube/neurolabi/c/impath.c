/**@file impath.c
 * @brief >> shortest path for stack
 * @author Ting Zhao
 * @date 25-Nov-2008
 */

#include <utilities.h>
#include <stdlib.h>
#include <string.h>
#include "tz_utilities.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_graph.h"
#include "tz_arrayqueue.h"
#include "tz_graph_utils.h"
#include "tz_unipointer_linked_list.h"
#include "tz_stack_graph.h"
#include "tz_stack_utils.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_lib.h"
#include "tz_utilities.h"

/*
 * impath from <int> <int> <int> to <int> <int> <int> imagefile -o out
 */

int main(int argc, char *argv[])
{
  static char *Spec[] = {"-from <int> <int> <int> -to <int> <int> <int>", 
			 "<image:string> -o <out:string>",
			 NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int start[3];
  int end[3];
  start[0] = Get_Int_Arg("-from", 1);
  start[1] = Get_Int_Arg("-from", 2);
  start[2] = Get_Int_Arg("-from", 3);

  end[0] = Get_Int_Arg("-to", 1);
  end[1] = Get_Int_Arg("-to", 2);
  end[2] = Get_Int_Arg("-to", 3);

  Stack *stack = Read_Stack(Get_String_Arg("image"));
  Stack *stack2 = Copy_Stack(stack);

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;

  Stack_Graph_Workspace_Set_Range_M(sgw, start[0], end[0], start[1], end[1], 
				    start[2], end[2], 10, 10, 10, 10, 10, 10);

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start, end, sgw);

  Stack *canvas = Translate_Stack(stack2, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;

  int i;
  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    ima.arrayc[index][0] = 255;
    ima.arrayc[index][1] = 0;
    ima.arrayc[index][2] = 0;
  }
    
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  Write_Stack("../data/test.tif", canvas);  

  return 0;
}
