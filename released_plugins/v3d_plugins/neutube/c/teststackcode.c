/* testvoxelgraph.c
 *
 * 02-01-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "tz_stack_draw.h"
#include "tz_stack_math.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_darray.h"
#include "tz_stack_bwdist.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_code.h"
#include "tz_stack_attribute.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
#if 0
  Stack *stack = Read_Stack("/tmp/binimg.tif");
  Stack_Not(stack, stack);

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;

  //dim_type sub[3] = {187, 175, 66};
  dim_type sub[3] = {144, 123, 106};
  int seed = (int) sub2ind(dim, 3, sub);
  
  Stack *out = Stack_Level_Code(stack, NULL, NULL, &seed, 1);
  IMatrix *chord = Make_IMatrix(dim, 3);

  Voxel_t v = {144, 123, 106};

  Object_3d *obj = Stack_Grow_Object_Constraint(stack, 1, v, chord, out, 15);

  Print_Object_3d(obj);

  double vec[3];
  Object_3d_Orientation(obj, vec, MAJOR_AXIS);
  darray_print2(vec, 1, 3);
  //stack = Object_To_Stack_Bw(obj, 255, NULL);
  //Stack_Label_Object_Constraint(stack, chord, seed, 1, 2, NULL, 15);
  
  Write_Stack("/tmp/codeimg.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/sphere_data.tif");
  Stack *stack2 = Copy_Stack(stack);
  Object_3d_List *objs = Stack_Find_Object_N(stack2, NULL, 1, 0, 26);
  Kill_Stack(stack2);
  Print_Object_3d_List_Compact(objs);
  Voxel_List *list = Object_3d_Central_Points(objs->data);
  Print_Voxel_List(list);

  Translate_Stack(stack, COLOR, 1);
  Voxel_List *tmp = list;
  while (tmp != NULL) {
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 0, 1.0);
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 1, 0.0);
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 2, 0.0);
    tmp = tmp->next;
  }

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  ow->conn = 26;
  Stack *stack = Read_Stack("../data/fly_neuron2/mask.tif");
  Stack *code = Stack_Boundary_Code(stack, NULL, ow);
  Write_Stack("../data/test.tif",  code);
#endif

#if 1
  Stack *stack = Read_Stack("../data/fly_neuron_a2_org.tif");
  Level_Code *tree = Stack_Region_Grow(stack, NULL, 6208180, 40, 15, NULL);

  Translate_Stack(stack, GREY16, 1);
  Zero_Stack(stack);
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  uint16 *array16 = (uint16 *) stack->array;

  for (i = 0; i < nvoxel; i++) {
    array16[i] = tree[i].level;
  }

  Stack_Binarize(stack);
  Translate_Stack(stack, GREY, 1);

  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
