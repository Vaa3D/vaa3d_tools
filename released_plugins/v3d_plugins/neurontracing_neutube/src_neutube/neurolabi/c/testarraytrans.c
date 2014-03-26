/* testvoxelgraph.c
 *
 * 02-01-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "tz_arraytrans.h"
#include "tz_stack_draw.h"
#include "tz_voxel_graphics.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Matlab_Array mr;
  mr_read("../data/mattest5.mat", &mr);
  Array_Link *a1 = Mar_To_Object_3d(&mr);

  Matlab_Array mr2;
  mr_read("../data/tree.mat", &mr2);
  Array_Link *a2 = Mar_To_Object_3d(&mr2);

  int i;
  Stack *stack = Read_Stack("../data/testneurotrace.tif");
  stack = Translate_Stack(stack, COLOR, 1);

  Object_3d *line = NULL;
  Rgb_Color color;
  Set_Color(&color, 255 , 0, 0);
  for (i = 1; i < Get_Object_3d_At(a2)->size; i++) {

    //Set_Color(&color, 255 * (i % 2), (i / 11) % 255, i % 255);

    int start = Get_Object_3d_At(a2)->voxels[i][0];
    int end = Get_Object_3d_At(a2)->voxels[i][1];


    line = Line_To_Object_3d(Get_Object_3d_At(a1)->voxels[start],
			     Get_Object_3d_At(a1)->voxels[end]);


    printf("%d, %d, %d\n", Get_Object_3d_At(a1)->voxels[start][0], 
	   Get_Object_3d_At(a1)->voxels[start][1],
	   Get_Object_3d_At(a1)->voxels[start][2]);
    printf("%d, %d, %d\n", Get_Object_3d_At(a1)->voxels[end][0], 
	   Get_Object_3d_At(a1)->voxels[end][1],
	   Get_Object_3d_At(a1)->voxels[end][2]);

    Stack_Draw_Object_Bwc(stack, line, color);


    //Print_Object_3d(line);
    Kill_Object_3d(line);

  }

  Write_Stack("../data/test.tif", stack);

  Free_Array_Link(a1);
  Free_Array_Link(a2);

  return 0;
}
