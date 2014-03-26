/* tz_object_3d_operation.c
 *
 * 16-Dec-2006 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "image_lib.h"
#include "tz_objdetect.h"
#include "tz_stack_draw.h"
#include "tz_stack_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_object_3d_linked_list_ext.h"
#include "tz_object_3d_operation.h"

Object_3d_List *Object_Sub(const Object_3d *obj1, const Object_3d *obj2)
{
  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj1, 1, offset);
  Object_3d *tmp_obj2 = Copy_Object_3d(obj2);
  Object_3d_Translate(tmp_obj2, offset[0], offset[1], offset[2]);
  Stack_Draw_Object_Bw(stack, tmp_obj2, 0);
  Kill_Object_3d(tmp_obj2);
  Object_3d_List *objs = Stack_Find_Object(stack, 1, 0);
  Kill_Stack(stack);

  return objs;
}

Object_3d* Object_Intersect(const Object_3d *obj1, const Object_3d *obj2)
{
  Object_3d_List *objs = NULL;

  if ((obj1 == NULL) || (obj2 == NULL)) {
    return NULL;
  }


  if (obj1 == obj2) {
    objs = Object_3d_List_New();
    return Copy_Object_3d(obj1);
  }

  int corner1[6], corner2[6];
  Object_3d_Range(obj1, corner1);
  Object_3d_Range(obj2, corner2);

  /* check the bounding boxes first */
  if ((corner1[3] < corner2[0]) || (corner1[4] < corner2[1]) || 
      (corner1[5] < corner2[2]) ||
      (corner2[3] < corner1[0]) || (corner2[4] < corner1[1]) || 
      (corner2[5] < corner1[2])) {
    return NULL;
  }

  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj1, 1, offset);
  Object_3d *tmp_obj = Copy_Object_3d(obj2);

  Object_3d_Translate(tmp_obj, -offset[0], -offset[1], -offset[2]);
  
  //int indices[obj2->size];
  size_t* indices = (size_t*)malloc(sizeof(size_t) * obj2->size);
  Object_3d_Indices(tmp_obj, 
		    stack->width, stack->height, stack->depth, indices);
  Kill_Object_3d(tmp_obj);

  int i;
  for (i = 0; i < obj2->size; i++) {
    if (Is_Valid_Array_Index(indices[i])) {
      if (stack->array[indices[i]] == 1) {
	stack->array[indices[i]] = 2;
      }
    }
  }
  free(indices);
  
  objs = Stack_Find_Object(stack, 2, 0);
  Kill_Stack(stack);

  tmp_obj = Object_3d_List_To_Object(objs, NULL);

  Object_3d_List_Removeall(&objs);

  return tmp_obj;  
}

/* Object_Union(): Union of two objects.
 *
 * Args: obj1 - the first object;
 *       obj2 - the second object.
 *
 * Return: the new object.
 *
 * Note: The caller is responsible for freeing the returned object.
 */
Object_3d* Object_Union(const Object_3d *obj1, const Object_3d *obj2)
{
  Object_3d_List *objs = Object_3d_List_New();
  objs->data = (Object_3d *) obj1;
  
  Object_3d *union_obj = Object_3d_List_To_Object(objs, NULL);

  Object_3d_List_Unlinkall(&objs);

  return union_obj;
}

/* Object_3d_Boundary(): Extract boundary of an object.
 *
 * Args: obj1 - input object;
 *       nnbr - neighborhood option.
 *
 * Return: a new object.
 */
Object_3d* Object_3d_Boundary(const Object_3d *obj1, int nnbr)
{
  int offset[3];
  int margin[3] = {1, 1, 1};
  Stack *stack = Object_To_Stack_Bwm(obj1, 1, margin, offset);
  //Stack *stack2 = Stack_Boundary_N(stack, NULL, nnbr);
  Stack *stack2 = Stack_Perimeter(stack, NULL, nnbr);
  Kill_Stack(stack);
  Object_3d_List *objs = Stack_Find_Object_N(stack2, NULL, 1, 0, nnbr);
  Kill_Stack(stack2);  
  Object_3d *obj = Object_3d_List_To_Object(objs, NULL);
  Object_3d_List_Removeall(&objs);
  
  Object_3d_Translate(obj, offset[0], offset[1], offset[2]);

  return obj;
}

/* Object_3d_Border(): Find the border between two objects.
 */
Object_3d* Object_3d_Border(const Object_3d *obj1, const Object_3d *obj2)
{
  int margin[3] = {1, 1, 1};
  int offset[3];
 
  Stack *stack = Object_To_Stack_Bwm(obj1, 1, margin, offset);
  Stack_Not(stack, stack);
  Stack *boundary = Stack_Boundary(stack, NULL);
  Kill_Stack(stack);
  Object_3d_List *objs = Stack_Find_Object(boundary, 1, 0);
  Kill_Stack(boundary);
  Object_3d *bound_obj = Object_3d_List_To_Object(objs, NULL);
  Object_3d_List_Removeall(&objs);
  Object_3d_Translate(bound_obj, offset[0], offset[1], offset[2]);

  return bound_obj;
}
