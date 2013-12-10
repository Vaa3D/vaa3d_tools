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
#if 0
  Matlab_Array mr;
  mr_read("../data/mattest5.mat", &mr);
  Array_Link *a1 = Mar_To_Object_3d(&mr);

  Matlab_Array mr2;
  mr_read("../data/tree.mat", &mr2);
  Array_Link *a2 = Mar_To_Object_3d(&mr2);

  int start = Get_Object_3d_At(a2)->voxels[90][0];
  int end = Get_Object_3d_At(a2)->voxels[90][1];

  printf("%d, %d\n", start, end);

  printf("%d, %d, %d\n", Get_Object_3d_At(a1)->voxels[start][0], 
	 Get_Object_3d_At(a1)->voxels[start][1],
	 Get_Object_3d_At(a1)->voxels[start][2]);
  printf("%d, %d, %d\n", Get_Object_3d_At(a1)->voxels[end][0], 
	 Get_Object_3d_At(a1)->voxels[end][1],
	 Get_Object_3d_At(a1)->voxels[end][2]);

  Object_3d *obj = Line_To_Object_3d(Get_Object_3d_At(a1)->voxels[start],
				     Get_Object_3d_At(a1)->voxels[end]);
  
  Print_Object_3d(obj);

  Stack *stack = Object_To_Stack_Bw(obj, 255, NULL);
  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
  Kill_Object_3d(obj);
#endif

#if 1
  int start[] = {3, 3, 3};
  int end[] = {1, 1, 1};

  Object_3d *obj = Line_To_Object_3d(start, end);
  Print_Object_3d(obj);

#endif

  return 1;
}
