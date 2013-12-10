/* tz_stack_objlabel.c
 *
 * Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_interface.h"
#include "tz_stack_draw.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_utils.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_objdetect.h"
#include "tz_stack_stat.h"

INIT_EXCEPTION

#define STACK_LABEL_OBJECT_RECOVER_CHORD
#define STACK_LABEL_OBJECT_BY_CHORD
//#include "private/tz_stack_objlabel.c"

#ifdef STACK_LABEL_OBJECT_RECOVER_CHORD
static void stack_label_object_recover_chord(const IMatrix *chord, int seed)
{
  int index = seed;
  int next = index;

  while (index >= 0) {
    next = chord->array[index];
    chord->array[index] = -1;
    index = next;
  }
}
#endif

#ifdef STACK_LABEL_OBJECT_BY_CHORD
static void stack_label_object_by_chord(Stack *stack, const IMatrix *chord,
					int label, int seed)
{
  int index = seed;

  if (stack->kind == GREY) {
    while (index >= 0) {
      stack->array[index] = label;
      index = chord->array[index];
    }
  } else {
    uint16_t* array16 = (uint16_t*) stack->array;
    while (index >= 0) {
      array16[index] = label;
      index = chord->array[index];
    }
  }
}
#endif

#define STACK_OBJLABEL_CHECK_CHORD(stack, chord, is_owner)		\
  if (chord == NULL) {							\
    chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);	\
    is_owner = TRUE;							\
  } else {								\
    if (chord->ndim != 3) {						\
      THROW(ERROR_DATA_TYPE);						\
    }									\
    									\
    if ((stack->width != chord->dim[0]) || (stack->height != chord->dim[1]) \
	|| (stack->depth != chord->dim[2])) {				\
      THROW(ERROR_DATA_COMPTB);						\
    }									\
  }
  
Objlabel_Workspace *New_Objlabel_Workspace()
{
  Objlabel_Workspace *ow = (Objlabel_Workspace *) 
    Guarded_Malloc(sizeof(Objlabel_Workspace), "New_Objlabel_Workspace");
  Default_Objlabel_Workspace(ow);

  return ow;
}

void Default_Objlabel_Workspace(Objlabel_Workspace *ow)
{
  ow->conn = 26;
  ow->seed = -1;
  ow->init_chord = TRUE;
  ow->recover_chord = FALSE;
  ow->chord = NULL;
  ow->u = NULL;
}

void Delete_Objlabel_Workspace(Objlabel_Workspace *ow)
{
  free(ow);
}

void Kill_Objlabel_Workspace(Objlabel_Workspace *ow)
{
  if (ow->chord != NULL) {
    Kill_IMatrix(ow->chord);
  }
  if (ow->u != NULL) {
    free(ow->u);
  }
  free(ow);
}

/* Stack_Label_Object_N(): Label an object in a stack.
 *
 * Args: chord - a matrix for storing neighbor queque;
 *       seed - index of the seed voxel;
 *       flag - value of the voxel to label;
 *       label - labeling value;
 *       n_nbr - neighborhood opion.
 *
 * Return: number of voxels labeled.
 */
int Stack_Label_Object_N(Stack *stack, IMatrix *chord, int seed, int flag,
			 int label, int n_nbr)
{
  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = n_nbr;
  ow.chord = chord;
  ow.init_chord = TRUE;

  return Stack_Label_Object_W(stack, seed, flag, label, &ow);
}

int Stack_Label_Object_W(Stack *stack, int seed, int flag, int label, 
			 Objlabel_Workspace *ow)
{
  TZ_ASSERT(stack->kind == GREY || stack->kind == GREY16, "Unsupported kind.");

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);
  ow->seed = seed;

  uint16_t* array16 = (uint16_t*) stack->array;
  uint8_t* array8 = (uint8_t*) stack->array;

  if (stack->kind == GREY) {
    if (stack->array[seed] != flag) {
      TZ_WARN(ERROR_OTHER);
      fprintf(stderr, "The seed does not have the right flag.\n");
      return 0;
    }
  } else {
    if (array16[seed] != flag) {
      TZ_WARN(ERROR_OTHER);
      fprintf(stderr, "The seed does not have the right flag.\n");
      return 0;
    }
  }

  int npixel = Get_Stack_Size(stack);
  
  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  if (ow->init_chord == TRUE) {
    for (i = 0; i < npixel; i++) {
      ow->chord->array[i] = -1;
    }
  }

  int obj_size = 0;
  int next = c;

  if (stack->kind == GREY) {
    stack->array[seed] = label;
  } else {
    array16[seed] = label;
  }
  
  int x, y, z;
  int is_in_bound[26];
  int n_in_bound = 0;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  int neighbor[26];
  Stack_Neighbor_Offset(ow->conn, stack->width, stack->height, neighbor);

#define STACK_LABEL_OBJECT_N_UPDATE_QUEUE(stack_array)			\
    {									\
      nb = c + neighbor[i];						\
      /*process unlabeled white neighbors*/				\
      if ((stack_array[nb] == flag) && (ow->chord->array[nb] == -1)) {	\
	ow->chord->array[next] = nb;					\
        TZ_ASSERT(ow->chord->array[next] != next, "loop"); \
	next = nb;							\
	stack_array[nb] = label;					\
      }									\
    }	

  int area = stack->width * stack->height;

  do {
    z = c / area;
    if (z > stack->depth) {
      n_in_bound = 0;
    } else {
      y = c % area;
      x = y % stack->width;
      y = y / stack->width;

      if ((x > stack->width) || (y > stack->height)) {
	n_in_bound = 0;
      } else {
	//Stack_Util_Coord(c, stack->width, stack->height, &x, &y, &z);
	if ((x > 0) && (x < cwidth) && (y > 0) && (y < cheight) && 
	    (z > 0) && (z < cdepth)) { 
	  n_in_bound = ow->conn;
	} else {
	  n_in_bound = Stack_Neighbor_Bound_Test_S(ow->conn, 
						 cwidth, cheight, cdepth, 
						 x, y, z, is_in_bound);
	}
      }
    }
					       
    /* add all unlabeled neighbors to the queue*/    
    if (n_in_bound == ow->conn) { /* no boundary check required */
      if (stack->kind == GREY) {
        for (i = 0; i < ow->conn; i++) {
          STACK_LABEL_OBJECT_N_UPDATE_QUEUE(array8);
        }
      } else {
        for (i = 0; i < ow->conn; i++) {
          STACK_LABEL_OBJECT_N_UPDATE_QUEUE(array16);
        }
      }
    } else {
      for (i = 0; i < ow->conn; i++) {
	if (is_in_bound[i]) {
          if (stack->kind == GREY) {
            STACK_LABEL_OBJECT_N_UPDATE_QUEUE(array8);
          } else {
            STACK_LABEL_OBJECT_N_UPDATE_QUEUE(array16);
          }
	}
      }
    }

    c = ow->chord->array[c]; /* move to next voxel */
    obj_size++;
  } while (c >= 0);

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  return obj_size;
}

#define STACK_LABEL_OBJECT_CONSTRAINT_N_UPDATE_QUEUE(stack_array)	\
    nb = c + neighbor[i];						\
    do_label = TRUE;							\
    if ((stack_array[nb] == flag) && (chord->array[nb] == -1)) {	\
      /*process unlabeled white neighbors*/				\
      if (code != NULL) {						\
	if (code_array[c] + 1 > max) {					\
	  do_label = FALSE;						\
	} else {							\
	  do_label = TRUE;						\
	}								\
      }									\
    } else {								\
      do_label = FALSE;							\
    }									\
									\
    if (do_label == TRUE) {						\
      code_array[nb] = code_array[c] + 1;				\
      chord->array[next] = nb;						\
      next = nb;							\
      stack->array[nb] = label;						\
    }	

int Stack_Label_Object_Level_N(Stack *stack, IMatrix *chord, int seed, 
			       int flag, int label, Stack *code, int max,
			       int n_nbr)
{
  BOOL is_owner = FALSE;

  if (chord == NULL) {
    chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
    is_owner = TRUE;
  } else {
    if (chord->ndim != 3) {
      THROW(ERROR_DATA_TYPE);
    }

    if ((stack->width != chord->dim[0]) || (stack->height != chord->dim[1]) 
	|| (stack->depth != chord->dim[2])) {
      THROW(ERROR_DATA_COMPTB);
    }
  }

  if (stack->array[seed] != flag) {
    TZ_WARN(ERROR_OTHER);
    fprintf(stderr, "The seed does not have the right flag.\n");
    return 0;
  }

  int npixel = Get_Stack_Size(stack);

  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  for (i = 0; i < npixel; i++) {
    chord->array[i] = -1;
  }
 
  int obj_size = 0;
  int next = c;
  uint16 *code_array = NULL;
  BOOL do_label = TRUE;

  if (code != NULL) {
    code_array = (uint16 *)code->array;
  }

  stack->array[seed] = label;

  int neighbor[26];
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
  int x, y, z;
  int is_in_bound[26];
  int n_in_bound = 0;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  do {
    Stack_Util_Coord(c, stack->width, stack->height, &x, &y, &z);
    n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, cwidth, cheight, cdepth, 
					   x, y, z, is_in_bound);
					       
    /* add all unlabeled neighbors to the queue*/    
    if (n_in_bound == n_nbr) { /* no boundary check required */
      for (i = 0; i < n_nbr; i++) {
	STACK_LABEL_OBJECT_CONSTRAINT_N_UPDATE_QUEUE(stack->array);
      }
    } else {
      for (i = 0; i < n_nbr; i++) {
	if (is_in_bound[i]) {
	  STACK_LABEL_OBJECT_CONSTRAINT_N_UPDATE_QUEUE(stack->array);
	}
      }
    }

    c = chord->array[c]; /* move to next voxel */
    obj_size++;

    if (code == NULL) {
      if (obj_size >= max) {
	break;
      }
    }
  } while (c >= 0);

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return obj_size;
}

int Stack_Label_Object_Level_Nw(Stack *stack, int seed, 
				int flag, int label, Stack *code, int max,
				Objlabel_Workspace *ow)
{
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);

  if (stack->array[seed] != flag) {
    TZ_WARN(ERROR_OTHER);
    fprintf(stderr, "The seed does not have the right flag.\n");
    return 0;
  }

  IMatrix *chord = ow->chord;

  int npixel = Get_Stack_Size(stack);

  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  for (i = 0; i < npixel; i++) {
    chord->array[i] = -1;
  }
 
  int obj_size = 0;
  int next = c;
  uint16 *code_array = NULL;
  BOOL do_label = TRUE;

  if (code != NULL) {
    code_array = (uint16 *)code->array;
  }

  stack->array[seed] = label;

  int n_nbr = ow->conn;
  int neighbor[26];
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
  int x, y, z;
  int is_in_bound[26];
  int n_in_bound = 0;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  do {
    Stack_Util_Coord(c, stack->width, stack->height, &x, &y, &z);
    n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, cwidth, cheight, cdepth, 
					   x, y, z, is_in_bound);
					       
    /* add all unlabeled neighbors to the queue*/    
    if (n_in_bound == n_nbr) { /* no boundary check required */
      for (i = 0; i < n_nbr; i++) {
	STACK_LABEL_OBJECT_CONSTRAINT_N_UPDATE_QUEUE(stack->array);
      }
    } else {
      for (i = 0; i < n_nbr; i++) {
	if (is_in_bound[i]) {
	  STACK_LABEL_OBJECT_CONSTRAINT_N_UPDATE_QUEUE(stack->array);
	}
      }
    }

    c = chord->array[c]; /* move to next voxel */
    obj_size++;

    if (code == NULL) {
      if (obj_size >= max) {
	break;
      }
    }
  } while (c >= 0);

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  return obj_size;  
}


int Stack_Label_Object_Dist_N(Stack *stack, IMatrix *chord, int seed, 
			      int flag, int label, double max_dist,
			      int n_nbr)
{
  BOOL is_owner = FALSE;

  if (chord == NULL) {
    chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
    is_owner = TRUE;
  } else {
    if (chord->ndim != 3) {
      THROW(ERROR_DATA_TYPE);
    }
    
    if ((stack->width != chord->dim[0]) || (stack->height != chord->dim[1]) 
	|| (stack->depth != chord->dim[2])) {
      THROW(ERROR_DATA_COMPTB);
    }
  }

  if (flag >= 0) {
    if (stack->array[seed] != flag) {
      TZ_WARN(ERROR_OTHER);
      fprintf(stderr, "The seed does not have the right flag.\n");
      return 0;
    }
  }

  int npixel = Get_Stack_Size(stack);

  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  for (i = 0; i < npixel; i++) {
    chord->array[i] = -1;
  }
 
  int obj_size = 0;
  int next = c;
  BOOL do_label = TRUE;
  double max_dist_square = max_dist * max_dist;

  stack->array[seed] = label;

  int neighbor[26];
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);

#define STACK_LABEL_OBJECT_DIST_N_UPDATE_QUEUE				\
  {									\
    nb = c + neighbor[i];						\
    Stack_Util_Coord(nb, stack->width, stack->height, &nx, &ny, &nz);	\
    /*process unlabeled white neighbors*/				\
    dx = nx - cx;							\
    dy = ny - cy;							\
    dz = nz - cz;							\
    if (dx * dx + dy * dy + dz * dz > max_dist_square) {		\
      do_label = FALSE;							\
    } else {								\
      do_label = TRUE;							\
    }									\
    									\
    if ((((flag < 0) && (stack->array[nb] != label)) ||			\
	(stack->array[nb] == flag)) &&					\
	(chord->array[nb] == -1) && (do_label == TRUE)) {		\
      chord->array[next] = nb;						\
      next = nb;							\
      stack->array[nb] = label;						\
    }									\
  }
  
  int x, y, z;
  int cx, cy, cz;
  int nx, ny, nz;
  int dx, dy, dz;
  int is_in_bound[26];
  int n_in_bound = 0;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  Stack_Util_Coord(seed, stack->width, stack->height, &cx, &cy, &cz);

  do {
    Stack_Util_Coord(c, stack->width, stack->height, &x, &y, &z);
    n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, cwidth, cheight, cdepth, 
					   x, y, z, is_in_bound);
    
    /* add all unlabeled neighbors to the queue*/    
    if (n_in_bound == n_nbr) { /* no boundary check required */
      for (i = 0; i < n_nbr; i++) {
	STACK_LABEL_OBJECT_DIST_N_UPDATE_QUEUE;
      }
    } else {
      for (i = 0; i < n_nbr; i++) {
	if (is_in_bound[i]) {
	  STACK_LABEL_OBJECT_DIST_N_UPDATE_QUEUE;
	}
      }
    }

    c = chord->array[c]; /* move to next voxel */
    obj_size++;
  } while (c >= 0);

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return obj_size;  
}

int Stack_Label_Objects_N(Stack *stack, IMatrix *chord, 
			  int flag, int label, int n_nbr)
{
  TZ_ASSERT(label > flag, "Invalid label");

  int start_label = label;

  BOOL is_owner = FALSE;

  STACK_OBJLABEL_CHECK_CHORD(stack, chord, is_owner);

  int nvoxel = Stack_Voxel_Number(stack);
  int nobj = 0;
  int i;

  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = n_nbr;
  ow.chord = chord;
  ow.init_chord = FALSE;
  
  for (i = 0; i < nvoxel; i++) {
    ow.chord->array[i] = -1;
  }

  PROGRESS_BEGIN("Labeling object");
  uint16_t *array16 = (uint16_t*) stack->array;
  uint8_t *array8 = (uint8_t*) stack->array;
  for (i = 0; i < nvoxel; i++) {
    BOOL is_flag = FALSE;
    if (stack->kind == GREY) {
      is_flag = (array8[i] == flag);
    } else {
      is_flag = (array16[i] == flag);
    }

    if (is_flag == TRUE) {
      if (label > 255) {
        if (stack->kind == GREY) {
          Translate_Stack(stack, GREY16, 1);
          array16 = (uint16_t*) stack->array;
        }
      }
      PROGRESS_STATUS(i / (nvoxel / 100 + 1));
      Stack_Label_Object_W(stack, i, flag, label, &ow);
      label++;
      if (label > 65535) {
        TZ_WARN(ERROR_DATA_VALUE);
        label = start_label;
      }
      nobj++;
      PROGRESS_REFRESH;
    }
  }
  PROGRESS_END("done");

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return nobj;
}

int Stack_Label_Objects_Ns(Stack *stack, IMatrix *chord, 
			   int flag, int label, int slabel, int n_nbr)
{
  TZ_ASSERT(stack->kind == GREY, "Unsupported kind.");
  TZ_ASSERT(slabel <= 255, "Invalid lable");

  BOOL is_owner = FALSE;

  STACK_OBJLABEL_CHECK_CHORD(stack, chord, is_owner);

  int nvoxel = Stack_Voxel_Number(stack);
  int nobj = 0;
  int i;

  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = n_nbr;
  ow.chord = chord;
  ow.init_chord = FALSE;
  
  for (i = 0; i < nvoxel; i++) {
    ow.chord->array[i] = -1;
  }

  PROGRESS_BEGIN("Labeling object");
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == flag) {
      PROGRESS_STATUS(i / (nvoxel / 100 + 1));
      Stack_Label_Object_W(stack, i, flag, label, &ow);
      nobj++;
      stack->array[i] = slabel;
      PROGRESS_REFRESH;
    }
  }
  PROGRESS_END("done");

  if (is_owner == TRUE) {
    Kill_IMatrix(chord);
  }

  return nobj;
}

int Stack_Label_Large_Objects_N(Stack *stack, IMatrix *chord, 
				int flag, int label, int minsize,
				int n_nbr)
{
  TZ_ASSERT(stack->kind == GREY, "GREY stack required.");

  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = n_nbr;
  ow.chord = chord;
  ow.init_chord = TRUE;

  return Stack_Label_Large_Objects_W(stack, flag, label, minsize, &ow);  
}

int Stack_Label_Largest_Object_N(Stack *stack, IMatrix *chord, 
				 int flag, int label, int n_nbr)
{ 
  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = n_nbr;
  ow.chord = chord;
  ow.init_chord = TRUE;

  return Stack_Label_Largest_Object_W(stack, flag, label, &ow);  
}

int Stack_Label_Large_Objects_W(Stack *stack, int flag, int label, int minsize,
				Objlabel_Workspace *ow)
{  
  ASSERT(label > flag, "label too small");

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);

  int small_label = label;
  int large_label = small_label + 1;

  int nvoxel = Stack_Voxel_Number(stack);
  int i;

  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
    ow->init_chord = FALSE;
  }

  int obj_size = 0;
  int large_object_number = 0;

  uint16_t *array16 = (uint16_t*) stack->array;
  uint8_t *array8 = (uint8_t*) stack->array;

  PROGRESS_BEGIN("Labeling object");
  for (i = 0; i < nvoxel; i++) {
    BOOL is_flag = FALSE;
    if (stack->kind == GREY) {
      is_flag = (array8[i] == flag);
    } else {
      is_flag = (array16[i] == flag);
    }

    if (is_flag == TRUE) {
      PROGRESS_STATUS(i / (nvoxel / 100 + 1));
      if (large_label > 255) {
        if (stack->kind == GREY) {
          Translate_Stack(stack, GREY16, 1);
          array16 = (uint16_t*) stack->array;
        }
      }
      obj_size = Stack_Label_Object_W(stack, i, flag, large_label, ow);
      if (obj_size < minsize) {
	stack_label_object_by_chord(stack, ow->chord, small_label, i);
      } else {
	large_object_number++;
        ++large_label;
        if (large_label > 65535) {
          TZ_WARN(ERROR_DATA_VALUE);
          large_label = small_label + 1;
        }
      }
      PROGRESS_REFRESH;
    }
  }
  PROGRESS_END("done");

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);
  
  return large_object_number;  
}

int Stack_Label_Largest_Object_W(Stack *stack, int flag, int label,
				 Objlabel_Workspace *ow)
{
  TZ_ASSERT(ow->chord != NULL, "NULL chord is not allowed. Wait for fix.");
  TZ_ASSERT(ow->conn <= 26, "Invalid neighborhood system.");

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);

  int small_label = label;
  int large_label = small_label + 1;

  int nvoxel = Stack_Voxel_Number(stack);
  int i;

  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
    ow->init_chord = FALSE;
  }

  int obj_size = 0;
  int max_size = 0;
  ow->seed = -1;
  int large_seed = -1;

  PROGRESS_BEGIN("Labeling object");
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == flag) {
      PROGRESS_STATUS(i / (nvoxel / 100 + 1));
      obj_size = Stack_Label_Object_W(stack, i, flag, small_label, ow);
      if (obj_size > max_size) { /* update the largest object */
	if (large_seed >= 0) { /* relabel the previous largest object */
	  stack_label_object_by_chord(stack, ow->chord, small_label, 
              large_seed);
	}
#ifdef _DEBUG_
        printf("%d\n", obj_size);
        if (obj_size > 10000) {
          printf("debug here\n");
        }
#endif
	stack_label_object_by_chord(stack, ow->chord, large_label, i);
	large_seed = i;
	max_size = obj_size;
      }
      PROGRESS_REFRESH;
    }
  }
  PROGRESS_END("done");

  ow->seed = large_seed;

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  return max_size;
}

#define STACK_GROW_OBJECT_ENQUEUE(queue, e)				\
  if (qtop < 0) {							\
    qtop = e;								\
    qbot = e;								\
    queue[qbot] = -1;							\
  } else {								\
    queue[qbot] = e;							\
    qbot = e;								\
    mask->array[e] = 0;							\
    seed->array[e] = 1;							\
    queue[e] = -1;							\
  }

#define STACK_GROW_OBJECT_DEQUEUE(queue, e)	\
  e = qtop;					\
  qtop = queue[qtop];

#define STACK_GROW_OBJECT_ENQUEUE_NEIGHBORS(queue, e, test)	\
  for (i = 0; i < conn; i++) {					\
    nidx = e + neighbor[i];					\
    if (test) {							\
      STACK_GROW_OBJECT_ENQUEUE(queue, nidx);			\
    }								\
  }

void Stack_Grow_Object_S(Stack *seed, Stack *mask, Objlabel_Workspace *ow)
{
  ASSERT(seed->kind = GREY, "GREY stack only");
  ASSERT(mask->kind = GREY, "GREY stack only");

  STACK_OBJLABEL_OPEN_WORKSPACE(seed, ow);
  
  int i;
  int nvoxel = Stack_Voxel_Number(seed);
  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
  }

  int neighbor[26];
  int nbound;
  int is_in_bound[26];
  
  int x, y, z;
  int offset = 0;
  int qbot = -1;
  int qtop = -1;

  int cwidth = seed->width - 1;
  int cheight = seed->height - 1;
  int cdepth = seed->depth - 1;

  int *queue = ow->chord->array;
  int nidx;
  int conn = ow->conn;
 
  PROGRESS_BEGIN("Oject growing");

  Stack_Neighbor_Offset(conn, seed->width, seed->height, neighbor);
  for (z = 0; z < seed->depth; z++) {
    PROGRESS_STATUS(z * 90 / seed->depth);
    for (y = 0; y < seed->height; y++) {
      for (x = 0; x < seed->width; x++) {
	if (seed->array[offset] == 1) {
	  mask->array[offset] = 0;
	  nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth, 
					     x, y, z, is_in_bound);
	  if (nbound < conn) {
	    for (i = 0; i < conn; i++) {
	      if (is_in_bound[i]) {
		if (seed->array[offset + neighbor[i]] == 0) {
		  STACK_GROW_OBJECT_ENQUEUE(queue, offset);
		  break;
		}
	      }
	    }
	  } else {
	    for (i = 0; i < conn; i++) {
	      if (seed->array[offset + neighbor[i]] == 0) {
		STACK_GROW_OBJECT_ENQUEUE(queue, offset);
		break;
	      }
	    }
	  }
	}
	offset++;
      }
    }
    PROGRESS_REFRESH;
  }

  int area = seed->width * seed->height;
  while (qtop >= 0) {
    STACK_GROW_OBJECT_DEQUEUE(queue, offset);
    STACK_UTIL_COORD(offset, seed->width, area, x, y, z);
    nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth, 
				       x, y, z, is_in_bound);
    if (nbound < conn) {
      STACK_GROW_OBJECT_ENQUEUE_NEIGHBORS(queue, offset, 
					  (is_in_bound[i] && 
					   mask->array[nidx] == 1));
    } else {
      STACK_GROW_OBJECT_ENQUEUE_NEIGHBORS(queue, offset, 
					  (mask->array[nidx] == 1));
    }
  }

  PROGRESS_END("done");

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);
  
}

void Stack_Build_Seed_Graph(Stack *stack, int *seed, int nseed,
			    uint8_t **connmat, Objlabel_Workspace *ow)
{
  if (stack->kind != GREY16) {
    PRINT_EXCEPTION("Unsupported stack kind", "The stack must be GREY16");
    return;
  }

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);
  
  int i, j, k;
  int nvoxel = Stack_Voxel_Number(stack);

  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
  }

  uint16 *mask = (uint16 *) stack->array;

  const int conn = 26;

  /* each seed has a queue */
  int *queue_head = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_tail = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_length = iarray_malloc(nseed);  /* queue_length malloced */

  /* At the beginning, each queue has one element, the corresponding seed */
  iarraycpy(queue_head, seed, 0, nseed);
  iarraycpy(queue_tail, seed, 0, nseed);

  for (i = 0; i < nseed; i++) {
    queue_length[i] = 1;
  }

  int neighbor[26];
  int bound[26];
  int nbound;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  BOOL stop = FALSE;
  int x, y, z;

  Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);
  
#define STACK_SEED_GRAPH_UPDATE_QUEUE(test)				\
  for (k = 0; k < conn; k++) {						\
    if (test) {								\
      int checking_voxel = queue_head[i] + neighbor[k];			\
      if ((mask[checking_voxel] > 0) && (mask[checking_voxel] != label)) { \
	if (mask[checking_voxel] == 1) {				\
	  ow->chord->array[queue_tail[i]] = checking_voxel;		\
	  queue_tail[i] = checking_voxel;				\
	  mask[checking_voxel] = label;					\
	  queue_length[i]++;						\
	} else {							\
	  if (mask[checking_voxel] > label) {				\
	    connmat[i][mask[checking_voxel] - 2] = 1;			\
	  } else {							\
	    connmat[mask[checking_voxel] - 2][i] = 1;			\
	  }								\
	}								\
      }									\
    }									\
  }

  while (stop == FALSE) {
    stop = TRUE;
    for (i = 0; i < nseed; i++) {
      if (queue_length[i] > 0) {
	int label = i + 2;
	for (j = 0; j < queue_length[i]; j++) {
	  Stack_Util_Coord(queue_head[i], stack->width, stack->height, 
			   &x, &y, &z);
	  nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth,
					     x, y, z, bound);
	  if (nbound == conn) {
	    STACK_SEED_GRAPH_UPDATE_QUEUE(1);
	    /*
	    for (k = 0; k < conn; k++) {
	      if (1) {
		int checking_voxel = queue_head[i] + neighbor[k];
		if ((mask[checking_voxel] > 0) && (mask[checking_voxel] != label)) { 
		  if (mask[checking_voxel] == 1) {				
		    ow->chord->array[queue_tail[i]] = checking_voxel;		
		    queue_tail[i] = checking_voxel;				
		    mask[checking_voxel] = label;  
		    queue_length[i]++;		
		  } else {				
		    if (mask[checking_voxel] > label) {		
		      connmat[mask[checking_voxel] - 2][i] = 1;		
		    } else {				
		      connmat[i][mask[checking_voxel] - 2] = 1;	
		    }						
		  }						
		}								
	      }					
	    }
	    */
	  } else {
	    STACK_SEED_GRAPH_UPDATE_QUEUE(bound[k]);
	  }
	}
	queue_head[i] = ow->chord->array[queue_head[i]];
	queue_length[i]--;
	stop = FALSE;
      }
    }
  }

  free(queue_head); /* queue_head freed */
  free(queue_tail); /* queue_head freed */
  free(queue_length);  /* queue_length freed */

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);
}

Graph* Stack_Build_Seed_Graph_G(Stack *stack, int *seed, int nseed,
				BOOL weighted, Objlabel_Workspace *ow)
{
  if (stack->kind != GREY16) {
    PRINT_EXCEPTION("Unsupported stack kind", "The stack must be GREY16");
    return NULL;
  }

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);
  
  int i, j, k;
  int nvoxel = Stack_Voxel_Number(stack);

  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
  }

  uint16 *mask = (uint16 *) stack->array;

  const int conn = 26;

  /* each seed has a queue */
  int *queue_head = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_tail = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_length = iarray_malloc(nseed);  /* queue_length malloced */

  /* At the beginning, each queue has one element, the corresponding seed */
  iarraycpy(queue_head, seed, 0, nseed);
  iarraycpy(queue_tail, seed, 0, nseed);

  for (i = 0; i < nseed; i++) {
    queue_length[i] = 1;
  }

  int neighbor[26];
  int bound[26];
  int nbound;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  BOOL stop = FALSE;
  int x, y, z;

  Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);
  
#define STACK_SEED_GRAPH_UPDATE_QUEUE_G(test)				\
  for (k = 0; k < conn; k++) {						\
    if (test) {								\
      int checking_voxel = queue_head[i] + neighbor[k];			\
      if ((mask[checking_voxel] > 0) && (mask[checking_voxel] != label)) { \
	if (mask[checking_voxel] == 1) {				\
	  ow->chord->array[queue_tail[i]] = checking_voxel;		\
	  queue_tail[i] = checking_voxel;				\
	  mask[checking_voxel] = label;					\
	  queue_length[i]++;						\
	} else {							\
	  int v1, v2, tmp;						\
	  v1 = i;							\
	  v2 = mask[checking_voxel] - 2;				\
	  ASSERT(v1 != v2, "Bug in Stack_Build_Seed_Graph_G()");	\
	  if (v1 > v2) {						\
	    SWAP2(v1, v2, tmp);						\
	  }								\
	  if (Graph_Edge_Index(v1, v2, gw) < 0) {			\
	    if (weighted == TRUE) {					\
	      double dist = Stack_Util_Voxel_Distance(seed[v1], seed[v2], stack->width, stack->height); \
	      Graph_Add_Weighted_Edge(graph, v1, v2, dist);		\
	    } else {							\
	      Graph_Add_Edge(graph, v1, v2);				\
	    }								\
	    Graph_Expand_Edge_Table(v1, v2, graph->nedge -1, gw);	\
	  }								\
	}								\
      }									\
    }									\
  }

  Graph *graph = Make_Graph(nseed, nseed, weighted);
  Graph_Workspace *gw = New_Graph_Workspace();

  while (stop == FALSE) {
    stop = TRUE;
    for (i = 0; i < nseed; i++) {
      if (queue_length[i] > 0) {
	int label = i + 2;
	for (j = 0; j < queue_length[i]; j++) {
	  Stack_Util_Coord(queue_head[i], stack->width, stack->height, 
			   &x, &y, &z);
	  nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth,
					     x, y, z, bound);
	  if (nbound == conn) {
	    STACK_SEED_GRAPH_UPDATE_QUEUE_G(1);
	  } else {
	    STACK_SEED_GRAPH_UPDATE_QUEUE_G(bound[k]);
	  }
	}
	queue_head[i] = ow->chord->array[queue_head[i]];
	queue_length[i]--;
	stop = FALSE;
      }
    }
  }

  free(queue_head); /* queue_head freed */
  free(queue_tail); /* queue_head freed */
  free(queue_length);  /* queue_length freed */

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  return graph;
}


Objlabel_Workspace* Make_Objlabel_Workspace_Gg(int width, int height, 
					       int depth)
{
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  dim_type dim[3];
  dim[0] = width;
  dim[1] = height;
  dim[2] = depth;
  ow->chord = Make_IMatrix(dim, 3);
  ow->u = Guarded_Malloc(width * height * depth, "Make_Objlabel_Workspace_Gg");
  
  return ow;
}

void Init_Objlabel_Workspace_Gg(Objlabel_Workspace *ow)
{
  int nvoxel = matrix_size(ow->chord->dim, ow->chord->ndim);
  int i;
  uint16 *level = (uint16 *) ow->u;
  for (i = 0; i < nvoxel; i++) {
    ow->chord->array[i] = -1;
    level[i] = 1;
  }
}

/**
 * Stack_Build_Seed_Graph_Gg() allows users to build a seed graph using 
 * approximate geodesdic distances. The workspace should be created by
 * Make_Objlabel_Workspace_Gg() and initialized by Init_Objlabel_Workspace_Gg()
 * if necessary.
 */
Graph* Stack_Build_Seed_Graph_Gg(Stack *stack, int *seed, int nseed,
				BOOL weighted, Objlabel_Workspace *ow)
{
  if (stack->kind != GREY16) {
    PRINT_EXCEPTION("Unsupported stack kind", "The stack must be GREY16");
    return NULL;
  }
  
  int i, j, k;

  uint16 *level = (uint16 *) ow->u;
  uint16 *mask = (uint16 *) stack->array;

  const int conn = 26;

  /* each seed has a queue */
  int *queue_head = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_tail = iarray_malloc(nseed); /* queue_head malloced */
  int *queue_length = iarray_malloc(nseed);  /* queue_length malloced */

  /* At the beginning, each queue has one element, the corresponding seed */
  iarraycpy(queue_head, seed, 0, nseed);
  iarraycpy(queue_tail, seed, 0, nseed);

  for (i = 0; i < nseed; i++) {
    queue_length[i] = 1;
  }

  int neighbor[26];
  int bound[26];
  int nbound;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  BOOL stop = FALSE;
  int x, y, z;

  Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);
  
#define STACK_SEED_GRAPH_UPDATE_QUEUE_GG(test)				\
  for (k = 0; k < conn; k++) {						\
    if (test) {								\
      int checking_voxel = queue_head[i] + neighbor[k];			\
      if ((mask[checking_voxel] > 0) && (mask[checking_voxel] != label)) { \
	if (mask[checking_voxel] == 1) {				\
	  ow->chord->array[queue_tail[i]] = checking_voxel;		\
	  queue_tail[i] = checking_voxel;				\
	  mask[checking_voxel] = label;					\
	  level[checking_voxel] = level[queue_head[i]] + 1;		\
	  queue_length[i]++;						\
	} else {							\
	  int v1, v2, tmp;						\
	  v1 = i;							\
	  v2 = mask[checking_voxel] - 2;				\
	  ASSERT(v1 != v2, "Bug in Stack_Build_Seed_Graph_G()");	\
	  if (v1 > v2) {						\
	    SWAP2(v1, v2, tmp);						\
	  }								\
	  if (Graph_Edge_Index(v1, v2, gw) < 0) {			\
	    if (weighted == TRUE) {					\
	      /*double dist = Stack_Util_Voxel_Distance(seed[v1], seed[v2], stack->width, stack->height);*/ \
	      double dist = level[checking_voxel] + level[queue_head[i]]; \
	      Graph_Add_Weighted_Edge(graph, v1, v2, dist);		\
	    } else {							\
	      Graph_Add_Edge(graph, v1, v2);				\
	    }								\
	    Graph_Expand_Edge_Table(v1, v2, graph->nedge -1, gw);	\
	  }								\
	}								\
      }									\
    }									\
  }

  Graph *graph = Make_Graph(nseed, nseed, weighted);
  Graph_Workspace *gw = New_Graph_Workspace();

  while (stop == FALSE) {
    stop = TRUE;
    for (i = 0; i < nseed; i++) {
      if (queue_length[i] > 0) {
	int label = i + 2;
	for (j = 0; j < queue_length[i]; j++) {
	  Stack_Util_Coord(queue_head[i], stack->width, stack->height, 
			   &x, &y, &z);
	  nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth,
					     x, y, z, bound);
	  if (nbound == conn) {
	    STACK_SEED_GRAPH_UPDATE_QUEUE_GG(1);
	  } else {
	    STACK_SEED_GRAPH_UPDATE_QUEUE_GG(bound[k]);
	  }
	}
	queue_head[i] = ow->chord->array[queue_head[i]];
	queue_length[i]--;
	stop = FALSE;
      }
    }
  }

  free(queue_head); /* queue_head freed */
  free(queue_tail); /* queue_head freed */
  free(queue_length);  /* queue_length freed */

  return graph;
}

static BOOL is_border_voxel(uint16_t *signal_array, size_t index, int conn, 
    int *neighbor_offset, int *is_in_bound, int nbound)
{
  if (signal_array[index] == 0) {
    return FALSE;
  }

  int j;
  for (j = 0; j < conn; ++j) {
    if (nbound == conn || is_in_bound[j]) {
      size_t neighbor_index = index + neighbor_offset[j];
      if (signal_array[neighbor_index] != signal_array[index]) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

Graph* Stack_Label_Field_Neighbor_Graph(Stack *stack, int threshold,
    Objlabel_Workspace *ow)
{
  TZ_ASSERT(stack->kind == GREY16, "Invalid stack kind");
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);


  Graph *graph = Make_Graph(0, 1, TRUE);
  graph->nvertex = Stack_Max(stack, NULL) + 1;

  Graph_Workspace *gw = New_Graph_Workspace();

  uint16_t *signal_array = (uint16_t*) stack->array;

  int neighbor_offset[26];
  int is_in_bound[26];

  Stack_Neighbor_Offset(ow->conn, stack->width, stack->height, neighbor_offset);

  /* Identify number of objects */
  int object_number = 0;
  int width = stack->width;
  int height = stack->height;
  int depth = stack->depth;
  size_t voxelNumber = Stack_Voxel_Number(stack);
  size_t index;

  //Stack *mask = Make_Stack(GREY, width, height, depth);

  for (index = 0; index < voxelNumber; ++index) {
    if (object_number < signal_array[index]) {
      object_number = signal_array[index];
    }

    if (ow->init_chord == TRUE) {
      ow->chord->array[index] = -1;
    }

    //mask->array[index] = 0;
  }

  //uint8_t *visited = mask->array;

  Int_Arraylist *seed_head = Make_Int_Arraylist(object_number + 1, 0);
  Int_Arraylist *seed_tail = Make_Int_Arraylist(object_number + 1, 0);
  Int_Arraylist *seed_point = Make_Int_Arraylist(object_number + 1, 0);
  int i;
  for (i = 0; i < seed_head->length; ++i) {
    seed_head->array[i] = -1;
    seed_point->array[i] = -1;
    seed_tail->array[i] = -1;
  }

  int *seed_queue = ow->chord->array;

  /* Initialize the seeds for each object */
  for (index = 0; index < voxelNumber; ++index) {
    uint16_t object_label = signal_array[index];
    int *current_seed_head = seed_head->array + object_label;
    int *current_seed_tail = seed_tail->array + object_label;
    int *current_seed_point = seed_point->array + object_label;
    int nbound = Stack_Neighbor_Bound_Test_I(ow->conn, width, height, depth,
        index, is_in_bound);
    if (is_border_voxel(signal_array, index, ow->conn,
          neighbor_offset, is_in_bound, nbound) == TRUE) { 
      //visited[index] = 1;
      if (*current_seed_head < 0) {
        *current_seed_head = index;
        *current_seed_tail = index;
        *current_seed_point = *current_seed_head;
      } else {
        seed_queue[*current_seed_point] = index;
        *current_seed_point = index;
      }
    }
  }

  int current_level = 0;
  /* While the current growing level is below the threshold */
  while (current_level < threshold) {
    /* Grow each object */
    uint16_t object_label;
    for (object_label = 1; object_label <= object_number; ++object_label) {
      int current_tail = seed_tail->array[object_label];
      int new_tail = current_tail;
      int seed = seed_head->array[object_label];
      while (seed >= 0) {
        int nbound = Stack_Neighbor_Bound_Test_I(ow->conn, width, height, depth,
            seed, is_in_bound);
        int j;
        for (j = 0; j < ow->conn; ++j) {
          if (nbound == ow->conn || is_in_bound[j]) {
            int neighbor_index = seed + neighbor_offset[j];
            uint16_t neighbor_label = signal_array[neighbor_index];
            if (neighbor_label == 0) {
              seed_queue[new_tail] = neighbor_index;
              new_tail = neighbor_index;
              signal_array[neighbor_index] = object_label;
              //visited[neighbor_index] = 1;
            } else if (neighbor_label != object_label) {
              /* If x-y does not exist */
              if (Graph_Edge_Index_U(object_label, neighbor_label, gw)
                  < 0) {
                double weight = current_level * 2;
                if (object_label > neighbor_label) {
                  weight += 1.0;
                }
                Graph_Add_Weighted_Edge(graph, 
                    object_label, neighbor_label, weight);
                Graph_Expand_Edge_Table(object_label, neighbor_label,
                    graph->nedge - 1, gw);
              }
            }
          }
        }
        if (seed == current_tail) {
          break;
        }
        seed = seed_queue[seed];
      }

      if (current_tail >= 0) {
        seed_head->array[object_label] = seed_queue[current_tail];
        seed_tail->array[object_label] = new_tail;
      }
    }

    ++current_level;
  }

  Kill_Int_Arraylist(seed_head);
  Kill_Int_Arraylist(seed_tail);
  Kill_Int_Arraylist(seed_point);

  Kill_Graph_Workspace(gw);

  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  //Kill_Stack(mask);

  return graph;
}
