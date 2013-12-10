/* tz_objdetect.c
 *
 * Initial write: Ting Zhao
 */

#include "tz_error.h"
#include "tz_interface.h"
#include "tz_stack_draw.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_utils.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_objdetect.h"

INIT_EXCEPTION

#define STACK_LABEL_OBJECT_RECOVER_CHORD
#include "private/tz_stack_objlabel.c"

Object_3d* extract_object(IMatrix *chord, int seed, Object_3d *obj)
{
  if ((chord == NULL) || (obj == NULL) || (seed < 0)) {
    return NULL;
  }

  int index = seed;
  int offset = 0;
  dim_type sub[3];
  int i;

  while (index >= 0) {
    ind2sub(chord->dim, chord->ndim, index, sub);
    for (i = 0; i < 3; i++) {
      obj->voxels[offset][i] = (int) sub[i];
    }
    index = chord->array[index];
    offset++;
  }

  return obj;
}


/* Stack_Grow_Object(): Find an object in a stack from a seed.
 */
Object_3d* Stack_Grow_Object(Stack *stack, int flag, const Voxel_t seed)
{
  int area = stack->height * stack->width;
  int index = area * seed[2] + stack->width * seed[1] + seed[0];

  if (stack->array[index] != flag) {
    TRACE("No object found");
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  IMatrix *chord = Make_IMatrix(dim, 3);
  
  int label = flag + 1;
  int obj_size = Stack_Label_Object(stack, chord, index, flag, label);
  Object_3d *obj = Make_Object_3d(obj_size, 0);

  extract_object(chord, index, obj);

  Kill_IMatrix(chord);

  return obj;
}

Object_3d* Stack_Grow_Object_N(Stack *stack, IMatrix *chord, int flag, 
			       const Voxel_t seed, int n_nbr)
{
  /*
  int area = stack->height * stack->width;
  int index = area * seed[2] + stack->width * seed[1] + seed[0];
  */
  size_t index = Stack_Util_Offset(seed[0], seed[1], seed[2],
				   stack->width, stack->height, stack->depth);

  if (stack->array[index] != flag) {
    TRACE("No object found");
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  BOOL is_owner = FALSE;

  if (chord == NULL) {
    chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
    is_owner = TRUE;
  }
  
  int label = flag + 1;
  int obj_size = Stack_Label_Object_N(stack, chord, index, flag, label, 
				      n_nbr);
  Object_3d *obj = Make_Object_3d(obj_size, n_nbr);

  extract_object(chord, index, obj);

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return obj;
}

Object_3d* Stack_Grow_Object_Constraint(Stack *stack, int flag, 
					const Voxel_t seed, IMatrix *chord,
					Stack *code, int max)
{
  int area = stack->height * stack->width;
  int index = area * seed[2] + stack->width * seed[1] + seed[0];

  if (stack->array[index] != flag) {
    TRACE("No object found");
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  BOOL owner = FALSE;

  if (chord == NULL) {
    dim_type dim[3];
    dim[0] = stack->width;
    dim[1] = stack->height;
    dim[2] = stack->depth;

    chord = Make_IMatrix(dim, 3);
    owner = TRUE;
  }
  

  int label = flag + 1;
  int obj_size = Stack_Label_Object_Constraint(stack, chord, index, flag, 
					       label, code, max);
  Object_3d *obj = Make_Object_3d(obj_size, 0);

  extract_object(chord, index, obj);

  if (owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return obj;
}

/* Stack_Find_Object(): Find objects in an stack.
 * 
 * Args:  stack - input stack;
 *        flag - object flag, which means any pixel with the value flag 
 *               belongs to an object.
 *        min_size - minimal size of an object to detect.
 *
 * Return: object list.
 *
 * Note: The stack will be changed after calling the function. <flag> cannot
 *       be upper bound of the stack.
 */
Object_3d_List* Stack_Find_Object(Stack *stack, int flag, int min_size)
{
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  IMatrix *chord = Make_IMatrix(dim, 3);

  int obj_size;
  int nvoxel = Get_Stack_Size(stack);
  int i;

  Object_3d *obj = NULL;
  Object_3d_List *obj_list = NULL;

  int label = flag + 1;

  PROGRESS_BEGIN("Finding object");

  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == flag) {
      PROGRESS_STATUS(i / (nvoxel / 100 + 1))
      obj_size = Stack_Label_Object(stack, chord, i, flag, label);
#if 0
      IMatrix_Print(chord);
#endif

      if (obj_size >= min_size) {
	obj = Make_Object_3d(obj_size, 0);
	extract_object(chord, i, obj);
	if (obj_list == NULL) {
	  obj_list = Object_3d_List_New();
	  obj_list->data = obj;
	} else {
	  Object_3d_List_Add(&obj_list, obj);
	}
      }
      PROGRESS_REFRESH
    }
  }

  PROGRESS_END("done");

  Kill_IMatrix(chord);

  return obj_list;
}

/* Stack_Find_Object_N(): Find objects in a stack.
 * 
 * Args: stack - input stack, which will be modified after function call;
 *       chord - a matrix to store intermediate result. It could be NULL and
 *               the function will handle it by itself;
 *       flag - voxel intensity of the objects;
 *       min_size - minimal size of the objects. Only objects with size not
 *                  less than min_size will be returned;
 *       n_nbr - connectivity. It should have been named <conn>. Here I just
 *               leave the old variable name.
 *       
 * Return: an object list. It returns NULL if no object is found.
 */
Object_3d_List* Stack_Find_Object_N(Stack *stack, IMatrix *chord, int flag, 
				    int min_size, int n_nbr)
{
  if (stack->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  Object_3d *obj = NULL;
  Object_3d_List *obj_list = NULL;
  int obj_size = 0;

  if (n_nbr == 0) {
    int i, j, k;
    int offset = 0;
    for (k = 0; k < stack->depth; k++) {
      for (j = 0; j < stack->height; j++) {
	for (i = 0; i < stack->width; i++) {
	  if (stack->array[offset++] == flag) {
	    obj_size++;
	  }
	}
      }
    }

    if ((obj_size < min_size) || (obj_size == 0)) {
      return NULL;
    } else {
      obj = Make_Object_3d(obj_size, 0);
      offset = 0;
      int offset2 = 0;
      for (k = 0; k < stack->depth; k++) {
	for (j = 0; j < stack->height; j++) {
	  for (i = 0; i < stack->width; i++) {
	    if (stack->array[offset++] == flag) {
	      obj->voxels[offset2][0] = i;
	      obj->voxels[offset2][1] = j;
	      obj->voxels[offset2][2] = k;
	      offset2++;
	    }
	  }
	}
      }

      Object_3d_List_Add(&obj_list, obj);
    }
  } else {
    int nvoxel = Stack_Voxel_Number(stack);
    int i;
    int label = flag + 1;
    
    BOOL is_owner = FALSE;
    
    if (chord == NULL) {
      chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
      is_owner = TRUE;
    }
    
    Objlabel_Workspace ow;
    ow.conn = n_nbr;
    ow.chord = chord;
    ow.init_chord = FALSE;
    ow.recover_chord = FALSE;

    for (i = 0; i < nvoxel; i++) {
      ow.chord->array[i] = -1;
    }

    PROGRESS_BEGIN("Finding object");
    
    for (i = 0; i < nvoxel; i++) {
      if (stack->array[i] == flag) {
	PROGRESS_STATUS(i / (nvoxel / 100 + 1));
	
	obj_size = Stack_Label_Object_W(stack, i, flag, label, &ow);
	
	if (obj_size >= min_size) {
	  obj = Make_Object_3d(obj_size, n_nbr);
	  extract_object(chord, i, obj);
	  if (obj_list == NULL) {
	    obj_list = Object_3d_List_New();
	    obj_list->data = obj;
	  } else {
	    Object_3d_List_Add(&obj_list, obj);
	  }
	}
	PROGRESS_REFRESH;
      }
    }
    
    PROGRESS_END("done");

    if (is_owner == TRUE) {
      Kill_IMatrix(chord);
    }
  } 
  return obj_list;
}

Object_3d_List* Stack_Find_Object_Level(Stack *stack, IMatrix *chord, int flag, 
					int min_size, int n_nbr, 
					int max_level, Stack *code)
{
  if (stack->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  Object_3d *obj = NULL;
  Object_3d_List *obj_list = NULL;
  int obj_size = 0;

  if (n_nbr == 0) {
    int i, j, k;
    int offset = 0;
    for (k = 0; k < stack->depth; k++) {
      for (j = 0; j < stack->height; j++) {
	for (i = 0; i < stack->width; i++) {
	  if (stack->array[offset++] == flag) {
	    obj_size++;
	  }
	}
      }
    }

    if ((obj_size < min_size) || (obj_size == 0)) {
      return NULL;
    } else {
      obj = Make_Object_3d(obj_size, 0);
      offset = 0;
      int offset2 = 0;
      for (k = 0; k < stack->depth; k++) {
	for (j = 0; j < stack->height; j++) {
	  for (i = 0; i < stack->width; i++) {
	    if (stack->array[offset++] == flag) {
	      obj->voxels[offset2][0] = i;
	      obj->voxels[offset2][1] = j;
	      obj->voxels[offset2][2] = k;
	      offset2++;
	    }
	  }
	}
      }

      Object_3d_List_Add(&obj_list, obj);
    }
  } else {
    int nvoxel = Stack_Voxel_Number(stack);
    int i;
    int label = flag + 1;
    
    BOOL is_owner = FALSE;
    
    if (chord == NULL) {
      chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
      is_owner = TRUE;
    }
    
    Objlabel_Workspace ow;
    ow.conn = n_nbr;
    ow.chord = chord;
    ow.init_chord = FALSE;
    
    for (i = 0; i < nvoxel; i++) {
      ow.chord->array[i] = -1;
    }

    PROGRESS_BEGIN("Finding object");
    
    for (i = 0; i < nvoxel; i++) {
      if (stack->array[i] == flag) {
	PROGRESS_STATUS(i / (nvoxel / 100 + 1));
	
	obj_size = Stack_Label_Object_Level_Nw(stack, i, flag, label, code,
					       max_level, &ow);
	
	if (obj_size >= min_size) {
	  obj = Make_Object_3d(obj_size, n_nbr);
	  extract_object(chord, i, obj);
	  if (obj_list == NULL) {
	    obj_list = Object_3d_List_New();
	    obj_list->data = obj;
	  } else {
	    Object_3d_List_Add(&obj_list, obj);
	  }
	}
	PROGRESS_REFRESH;
      }
    }
    
    PROGRESS_END("done");

    if (is_owner == TRUE) {
      Kill_IMatrix(chord);
    }
  } 
  return obj_list;
}

Object_3d* Stack_Find_Largest_Object_N(Stack *stack, IMatrix *chord, int flag,
				       int conn)
{
  Objlabel_Workspace ow;
  ow.chord = chord;
  ow.conn = conn;
  ow.init_chord = TRUE;
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, (&ow));

  int obj_size = Stack_Label_Largest_Object_W(stack, flag, flag + 1, &ow);

  Object_3d *obj = Make_Object_3d(obj_size, conn);
  extract_object(ow.chord, ow.seed, obj);

  STACK_OBJLABEL_CLOSE_WORKSPACE((&ow));

  return obj;
}

/* Stack_Label_Background(): Label background.
 * 
 * Args: stack - stack to label;
 *       label - labeling value.
 *
 * Return: void.
 */
void Stack_Label_Background(Stack *stack, int label)
{
  int flag = 0;

  if (label == flag) {
    return;
  }

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  IMatrix *chord = Make_IMatrix(dim, 3);

  int i, j, k;
  int offset;

  int area = stack->width * stack->height;

  int chk_width = stack->width - 1;
  int chk_height = stack->height - 1;
  int chk_depth = stack->depth - 1;

  /* left and right planes */
  for (k = 0; k < stack->depth; k++) {
    PROGRESS_STATUS(k * 100 / stack->depth)
    for (j = 0; j < stack->height; j++) {
      offset = j * stack->width + k * area;
      if (stack->array[offset] == flag) {
	Stack_Label_Object(stack, chord, offset, flag, label);
      }

      offset += chk_width;
      if (stack->array[offset] == flag) {
	Stack_Label_Object(stack, chord, offset, flag, label);
      }     
    }
    PROGRESS_REFRESH
  }

  /* top and bottom planes */
  for (k = 0; k < stack->depth; k++) {
    PROGRESS_STATUS(k * 100 / stack->depth)
    for (i = 1; i < chk_width; i++) {
      offset = i + k * area;
      if (stack->array[offset] == flag) {
	Stack_Label_Object(stack, chord, offset, flag, label);
      }

      offset += chk_height * stack->width;
      if (stack->array[offset] == flag) {
	Stack_Label_Object(stack, chord, offset, flag, label);
      }
    }
    PROGRESS_REFRESH
  }

  if (Get_Con2d() == 0) {
    /* front and back planes */
    for (j = 1; j < chk_height; j++) {
      PROGRESS_STATUS(k * 100 / stack->depth)
      for  (i = 1; i < chk_width; i++) {
	offset = i + j * stack->width;
	if (stack->array[offset] == flag) {
	  Stack_Label_Object(stack, chord, offset, flag, label);
	}

	offset += chk_depth * area;
	if (stack->array[offset] == flag) {
	  Stack_Label_Object(stack, chord, offset, flag, label);
	}
      }
      PROGRESS_REFRESH
    }
  }

  Kill_IMatrix(chord);  
}

void Stack_Label_Background_N(Stack *stack, int label, int conn, 
			      IMatrix *chord)
{
  if (stack->kind != GREY) {
    PRINT_EXCEPTION("Unsupported stack kind", "The stack kind must be GREY.");
    return;
  }

  int flag = 0;

  if (label == flag) {
    return;
  }

  BOOL is_owner = FALSE;

  if (chord == NULL) {
    dim_type dim[3];
    dim[0] = stack->width;
    dim[1] = stack->height;
    dim[2] = stack->depth;
    chord = Make_IMatrix(dim, 3);
    is_owner = TRUE;
  }

  int i, j, k;
  int offset;

  int area = stack->width * stack->height;

  int chk_width = stack->width - 1;
  int chk_height = stack->height - 1;
  int chk_depth = stack->depth - 1;

  Objlabel_Workspace ow;
  ow.conn = conn;
  ow.chord = chord;
  ow.init_chord = FALSE;

  int nvoxel = Stack_Voxel_Number(stack);

  for (i = 0; i < nvoxel; i++) {
    ow.chord->array[i] = -1;
  }

  /* left and right planes */
  for (k = 0; k < stack->depth; k++) {
    PROGRESS_STATUS(k * 100 / stack->depth)
    for (j = 0; j < stack->height; j++) {
      offset = j * stack->width + k * area;
      if (stack->array[offset] == flag) {
	Stack_Label_Object_W(stack, offset, flag, label, &ow);
      }

      offset += chk_width;
      if (stack->array[offset] == flag) {
	Stack_Label_Object_W(stack, offset, flag, label, &ow);
      }     
    }
    PROGRESS_REFRESH
  }

  /* top and bottom planes */
  for (k = 0; k < stack->depth; k++) {
    PROGRESS_STATUS(k * 100 / stack->depth)
    for (i = 1; i < chk_width; i++) {
      offset = i + k * area;
      if (stack->array[offset] == flag) {
	Stack_Label_Object_W(stack, offset, flag, label, &ow);
      }

      offset += chk_height * stack->width;
      if (stack->array[offset] == flag) {
	Stack_Label_Object_W(stack, offset, flag, label, &ow);
      }
    }
    PROGRESS_REFRESH
  }

  if ((conn == 6) || (conn == 18) || (conn == 26)) { /* 3D connectivity */
    /* front and back planes */
    for (j = 1; j < chk_height; j++) {
      PROGRESS_STATUS(k * 100 / stack->depth)
      for  (i = 1; i < chk_width; i++) {
	offset = i + j * stack->width;
	if (stack->array[offset] == flag) {
	  Stack_Label_Object_W(stack, offset, flag, label, &ow);
	}

	offset += chk_depth * area;
	if (stack->array[offset] == flag) {
	  Stack_Label_Object_W(stack, offset, flag, label, &ow);
	}
      }
      PROGRESS_REFRESH
    }
  }

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);  
  }
}

/* Stack_Find_Hole(): Find holes of the objects in a stack.
 *
 * Note: a pixel in a hole has intensity 0.
 */
Object_3d_List* Stack_Find_Hole(Stack *stack)
{
  Stack_Label_Background(stack, 1);
  return Stack_Find_Object(stack, 0, 0);
}

Object_3d_List* Stack_Find_2dhole(Stack *stack, int iscon1)
{
  int old_iscon1 = Get_Chkcon();
  int old_con2d = Get_Con2d();

  Set_Connection(iscon1, 1);
  Stack_Label_Background(stack, 1);

  Set_Connection(old_iscon1, old_con2d);
  return Stack_Find_Object(stack, 0, 100);
}

Object_3d_List* Stack_Find_Hole_N(Stack *stack, int conn, IMatrix *chord)
{
  BOOL is_owner = FALSE;

  if (chord == NULL) {
    dim_type dim[3];
    dim[0] = stack->width;
    dim[1] = stack->height;
    dim[2] = stack->depth;
    chord = Make_IMatrix(dim, 3);
    is_owner = TRUE;
  }

  Stack_Label_Background_N(stack, 1, conn, chord);
  Object_3d_List *objs = Stack_Find_Object_N(stack, chord, 0, 0, conn);

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return objs;
}

/* Object_3d_Find_Hole(): Find holes in an object
 */
Object_3d_List* Object_3d_Find_Hole(Object_3d *obj)
{
  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj, 1, offset);
  Object_3d_List *objs = Stack_Find_Hole(stack);
  Kill_Stack(stack);

  Object_3d_List *tmpobjs = objs;
  while (tmpobjs != NULL) {
    Object_3d_Translate(tmpobjs->data, offset[0], offset[1], offset[2]);
    tmpobjs = tmpobjs->next;
  }

  return objs;
}

Object_3d_List* Object_3d_Find_2dhole(Object_3d *obj, int iscon1)
{
  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj, 1, offset);
  Object_3d_List *objs = Stack_Find_2dhole(stack, iscon1);
  Kill_Stack(stack);

  Object_3d_List *tmpobjs = objs;
  while (tmpobjs != NULL) {
    Object_3d_Translate(tmpobjs->data, offset[0], offset[1], offset[2]);
    tmpobjs = tmpobjs->next;
  }

  return objs;
}

Object_3d_List* Object_3d_Find_Hole_N(Object_3d *obj, int conn)
{
  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj, 1, offset);
  Object_3d_List *objs = Stack_Find_Hole_N(stack, conn, NULL);
  Kill_Stack(stack);

  Object_3d_List *tmpobjs = objs;
  while (tmpobjs != NULL) {
    Object_3d_Translate(tmpobjs->data, offset[0], offset[1], offset[2]);
    tmpobjs = tmpobjs->next;
  }

  return objs;
}
