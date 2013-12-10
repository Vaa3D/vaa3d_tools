/* tz_voxel_graphics.c
 *
 * Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "tz_math.h"
#include "tz_image_lib.h"
#include "tz_voxel_graphics.h"

#include "private/tz_voxel_graphics.c"

INIT_EXCEPTION

/* Stack_To_Voxel_List(): Builds a voxel list from a stack.
 *
 * Args: stack - input stack.
 *
 * Return: a voxel list or NULL.
 *
 * Note: The order of the voxels in the list is the reverse of their order
 *       in the stack array.
 */
#define STACK_TO_VOXEL_LIST(stack, array, list)		\
  for (k = 0; k < stack->depth; k++) {			\
    for (j = 0;  j < stack->height; j++) {		\
      for (i = 0; i < stack->width; i++) {		\
	if (array[offset] > 0) {			\
	  Voxel_List_Add(&list, Make_Voxel(i, j, k));	\
	}						\
	offset++;					\
      }							\
    }							\
  }

Voxel_List* Stack_To_Voxel_List(const Stack *stack)
{
  if (stack == NULL) {
    THROW(ERROR_POINTER_NULL);
  }
  
  Voxel_List *list = NULL;
  int i, j, k;
  int offset = 0;

  DEFINE_SCALAR_ARRAY_ALL(array, stack);
  switch (stack->kind) {
  case GREY:
    STACK_TO_VOXEL_LIST(stack, array_grey, list);
    break;
  case GREY16:
    STACK_TO_VOXEL_LIST(stack, array_grey16, list);
    break;
  case FLOAT32:
    STACK_TO_VOXEL_LIST(stack, array_float32, list);
    break;
  case FLOAT64:
    STACK_TO_VOXEL_LIST(stack, array_float64, list);
    break;
  default:
    THROW(ERROR_DATA_TYPE);
    break;
  }

  return list;
}

#define STORE_VOXEL(obj, n, x, y, z)			\
  obj->voxels[n][0] = x;				\
  obj->voxels[n][1] = y;				\
  obj->voxels[n][2] = z;				\
  n++;

/* Line_To_Object_3d(): Convert a line to a 3d object.
 *
 * Args: v1 - starting point of the line;
 *       v2 - ending point of the line.
 *
 * Return: a 3D object.
 *
 * Note: The algorithm is described in "3D Scan-Conversion Algorithms for 
 *       Voxel-Based Graphics", Arie Kaufman and Eyal Shimony, Interactive
 *       Graphics, October 23-24, 1986. pp. 45-75.
 */
Object_3d* Line_To_Object_3d(const Voxel_t v1, const Voxel_t v2)
{
  Object_3d *obj = NULL;
  int n = 0;

  if ((v1[0] == v2[0]) && (v1[1] == v2[1]) && (v1[2] == v2[2])) {
    obj = Make_Object_3d(1, 26);
    STORE_VOXEL(obj, n, v1[0], v1[1], v1[2]);

    return obj;
  }

  Voxel_Graph_Conversion vgc;

  Voxel_t start, end;
  Copy_Tvoxel(start, v1);
  Copy_Tvoxel(end, v2);

  voxel_graph_line_standarize(start, end, &vgc);

  int x = start[0];
  int y = start[1];
  int z = start[2];

  int delta_x = end[0] - start[0];

  int delta_y = end[1] - start[1];
  int ysign = signint(delta_y);
  delta_y = abs(delta_y);

  int delta_z = end[2] - start[2];
  int zsign = signint(delta_z);
  delta_z = abs(delta_z);

  int dy = 2 * delta_y - delta_x;
  int dz = 2 * delta_z - delta_x;

  int yinc1 = 2 * delta_y;
  int yinc2 = 2 * (delta_y - delta_x);
  
  int zinc1 = 2 * delta_z;
  int zinc2 = 2 * (delta_z - delta_x);

  obj = Make_Object_3d(delta_x + delta_y + delta_z + 3, 26);

  STORE_VOXEL(obj, n, x, y, z);
  
  while (x < end[0]) {
    x++;
    if (dy < 0) {
      dy += yinc1;
    } else {
      dy += yinc2;
      y += ysign;
    }

    if (dz < 0) {
      dz += zinc1;
    } else {
      dz += zinc2;
      z += zsign;
    }

    STORE_VOXEL(obj, n, x, y, z);
  }
  
  obj->size = n;

  voxel_graph_line_inverse_conversion_object(obj, &vgc);  

  return obj;
}

#undef STORE_VOXEL
