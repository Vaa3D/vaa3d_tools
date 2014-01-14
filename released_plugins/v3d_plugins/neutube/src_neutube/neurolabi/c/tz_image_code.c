/* tz_image_code.c
 *
 * 23-Oct-2007 Initial write:  Ting Zhao
 */

#include <stdlib.h>
#include <assert.h>
#include <utilities.h>
#include "tz_image_code.h"
#include "tz_stack_code.h"
#include "tz_stack_lib.h"
#include "tz_imatrix.h"
#include "tz_stack_attribute.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_utils.h"
#include "tz_error.h"

INIT_EXCEPTION

#define NEIGHBOR_LABEL 65535
#define BRANCH_LABEL 65534
#define TRUE 1
#define FALSE 0

#define STACK_LABEL_OBJECT_RECOVER_CHORD
#include "private/tz_stack_objlabel.c"
#undef STACK_LABEL_OBJECT_RECOVER_CHORD
#include "private/tz_image_code.c"

/* Set_Chkcon(): set neiborhood flag.
 *
 * Args: iscon1 - 4-connection(1) or 8-connection(0) for 2D.
 *                6-connection(1) or 26-connection(0) for 3D.
 *
 * Return: void.
 */
void Set_Chkcon(int iscon1)
{
  chk_iscon1 = iscon1;
}

int Get_Chkcon()
{
  return chk_iscon1;
}

/* Set_Chkcon(): set neiborhood flag.
 *
 * Args: iscon1 - 4-connection(1) or 8-connection(0) for 2D.
 *                6-connection(1) or 26-connection(0) for 3D.
 *
 * Return: void.
 */
void Set_Connection(int iscon1, int con_2d)
{
  chk_iscon1 = iscon1;
  ccon_2d = con_2d;
}

int Get_Con2d()
{
  return ccon_2d;
}

/* Image_Level_Code(): Build level code for an image.
 *
 * Note: The caller is responsible for clearing the returned pointer. No aliasing
 *       is allowed between the input pointers.
 *
 * Args:  image - input image.
 *        code - output coded image. A new image will be created if it is NULL.
 *        link - An array for storing intermediate results. It must be NULL or 
 *               have the length of the number of pixels.
 *        seed - starting pixel.
 *
 * Return: code image. The intensity of a pixel is its level from the seed.
 */
Image* Image_Level_Code(Image *image, Image *code, int *link, 
			const int *seed, int nseed)
{
  assert(image != NULL);

  if (image->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  if (image->array[seed[0]] == 0) {
    TZ_WARN(ERROR_OTHER);
    TRACE("The seed is in the background.");
    return NULL;
  }

  int link_owner = FALSE;

  if (link == NULL) {
    link = (int*)Guarded_Malloc(sizeof(int) * cnpixel, "Image_Level_Code");
    link_owner = TRUE;
  }

  if (code == NULL) {
    code = Make_Image(GREY16, image->width, image->height);
  }
  
  uint16 *code_array = (uint16 *) code->array;
  int neighbor[8];

  level_code(image->array, code_array, link, image->width, image->height, 1,
	     seed, nseed, neighbor, 0);
	     
  if (link_owner == TRUE) {
    free(link);
  }

  return code;
}

/* Image_Branch_Code(): Build branch code for an image.
 *
 * Note: The caller is responsible for clearing the returned pointer. No aliasing
 *       is allowed between the input pointers.
 *
 * Args:  image - input image.
 *        code - output coded image. A new image will be created if it is NULL.
 *        link - An array for storing intermediate results. It must be NULL or 
 *               have the length of the number of pixels.
 *        seed - starting pixel.
 *
 * Return: code image. The intensity of a pixel is its branch label.
 */
Image* Image_Branch_Code(Image *image, Image *code, int *link, uint16 *edge, int seed)
{
  assert(image != NULL);

  if (image->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  if (image->array[seed] == 0) {
    TZ_WARN(ERROR_OTHER);
    TRACE("The seed is in the background.");
    return NULL;
  }
  
  init_imginfo(image->width, image->height, 1);
 
  int link_owner = FALSE;

  if (link == NULL) {
    link = (int*)Guarded_Malloc(sizeof(int) * cnpixel, "Image_Branch_Code");
    link_owner = TRUE;
  }

  if (code == NULL) {
    code = Make_Image(GREY16, image->width, image->height);
  }
  
  uint16 *code_array = (uint16 *) code->array;
  int neighbor[8];

  branch_code(image->array, code_array, link, edge, image->width, image->height, 1,
	     seed, neighbor);

  if (link_owner == TRUE) {
    free(link);
  }

  return code;
}

/* Stack_Level_Code(): Build level code for a stack.
 *
 * Note: The caller is responsible for clearing the returned pointer. No aliasing
 *       is allowed between the input pointers.
 *
 * Args:  stack - input stack.
 *        code - output coded sack. A new stack will be created if it is NULL.
 *        link - An array for storing intermediate results. It must be NULL or 
 *               have the length of the number of pixels.
 *        seed - starting pixel.
 *
 * Return: code stack. The intensity of a pixel is its level from the seed.
 */
Stack* Stack_Level_Code(Stack *stack, Stack *code, int *link, 
			const int *seed, int nseed)
{
  assert(stack != NULL);

  if (stack->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  /*
  if (stack->array[seed[0]] == 0) {
    TZ_WARN(ERROR_OTHER);
    TRACE("The seed is in the background.");
    //return NULL;
  }
  */

  init_imginfo(stack->width, stack->height, stack->depth);

  int link_owner = FALSE;

  if (link == NULL) {
    link = (int*)Guarded_Malloc(sizeof(int) * cnpixel, "Stack_Level_Code");
    link_owner = TRUE;
  }

  if (code == NULL) {
    code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);
  }

  uint16 *code_array = (uint16 *) code->array;
  int neighbor[26];

  level_code(stack->array, code_array, link, stack->width, stack->height, 
	     stack->depth, seed, nseed, neighbor, 0);
	     
  if (link_owner == TRUE) {
    free(link);
  }

  return code;
}

Stack* Stack_Level_Code_Constraint(Stack *stack, Stack *code, int *link, 
				   const int *seed, int nseed, int max_level)
{
  assert(stack != NULL);

  if (stack->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  if (stack->array[seed[0]] == 0) {
    TZ_WARN(ERROR_OTHER);
    TRACE("The seed is in the background.");
    return NULL;
  }

  init_imginfo(stack->width, stack->height, stack->depth);

  int link_owner = FALSE;

  if (link == NULL) {
    link = (int*)Guarded_Malloc(sizeof(int) * cnpixel, "Stack_Level_Code");
    link_owner = TRUE;
  }

  if (code == NULL) {
    code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);
  }

  uint16 *code_array = (uint16 *) code->array;
  int neighbor[26];

  level_code(stack->array, code_array, link, stack->width, stack->height, 
	     stack->depth, seed, nseed, neighbor, max_level);
	     
  if (link_owner == TRUE) {
    free(link);
  }

  return code;  
}


/* Stack_Branch_Code(): Build branch code for an image.
 *
 * Note: The caller is responsible for clearing the returned pointer. No aliasing
 *       is allowed between the input pointers.
 *
 * Args:  stack - input stack.
 *        code - output coded stack. A new stack will be created if it is NULL.
 *        link - An array for storing intermediate results. It must be NULL or 
 *               have the length of the number of pixels.
 *        seed - starting pixel.
 *
 * Return: code image. The intensity of a pixel is its branch label.
 */
Stack* Stack_Branch_Code(Stack *stack, Stack *code, int *link, uint16 *edge, int seed)
{
  assert(stack != NULL);

  if (stack->kind != GREY) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  if (stack->array[seed] == 0) {
    TZ_WARN(ERROR_OTHER);
    TRACE("The seed is in the background.");
    return NULL;
  }
  
  init_imginfo(stack->width, stack->height, stack->depth);
 
  int link_owner = FALSE;

  if (link == NULL) {
    link = (int*)Guarded_Malloc(sizeof(int) * cnpixel, "Stack_Branch_Code");
    link_owner = TRUE;
  }

  if (code == NULL) {
    code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);
  }
  
  uint16 *code_array = (uint16 *) code->array;
  int neighbor[26];

  branch_code(stack->array, code_array, link, edge, stack->width, stack->height, stack->depth,
	     seed, neighbor);

  if (link_owner == TRUE) {
    free(link);
  }

  return code;
}

/* Stack_Region_Grow(): Region grow in a stack.
 *
 * Note: The caller is responsible for clearing the returned tree code.
 *
 * Args: stack - input stack;
 *       tree - output tree code. A new tree will be created if it is NULL;
 *       seed - growing seed;
 *       threshold - threshold of pixel intensity (>=threshold);
 *       difference - allowed maximum difference between neighbor pixels (>=difference).
 *       mask - a 3D mask of the stack. Set it to NULL if there is no mask.
 *
 * Return: a tree code.
 */
Level_Code* Stack_Region_Grow(const Stack *stack, Level_Code *tree, int seed, 
			      int threshold, int difference, const Stack *mask)
{
  if (stack->array[seed] < threshold) {
    TZ_WARN(ERROR_OTHER);
    fprintf(stderr, "The seed is in the background.\n");
    return NULL;
  }

  if (mask != NULL) {
    if (mask->array[seed] == 0) {
      TZ_WARN(ERROR_OTHER);
      fprintf(stderr, "The seed is in the background.\n");
      return NULL;
    }
  }

  int npixel = stack->width * stack->height * stack->depth;
  
  if (tree == NULL) {
    tree = (Level_Code *) Guarded_Malloc(sizeof(Level_Code) * npixel, "Stack_Region_Grow");
  }
  
  int x, y, z;
  int n_nbrs = 0;
  int entrance = seed;
  int neighbor[26];
  int *b = NULL;
  int i;
  int c = seed; /* center pixel */
  int nb; /* neighobr pixel */
  int prev;
  
  for (i = 0; i < npixel; i++) {
    tree[i].level = 0;
    //tree[i].branch = 0;
    tree[i].link = -1;
  }

  cwidth = stack->width;
  cheight = stack->height;
  cdepth = stack->depth;
  chk_width  = cwidth - 1;
  chk_height = cheight - 1;
  chk_depth = cdepth - 1;
  carea = cwidth*cheight;
  
  if (chk_iscon1) {
    neighbor[0] = -carea;
    neighbor[1] = -cwidth;
    neighbor[2] = -1;
    neighbor[3] =  1;
    neighbor[4] =  cwidth;
    neighbor[5] =  carea;
    n_nbrs = 6;
  } else {
    for (z = -carea; z <= carea; z += carea)
      for (y = -cwidth; y <= cwidth; y += cwidth)
	for (x = -1; x <= 1; x += 1)
	  if (! (x==0 && y == 0 && z == 0))
	    neighbor[n_nbrs++] = z+y+x;
  }

  tree[seed].level = 1;
  //tree[seed].branch = 1;
  tree[seed].link = -1;

  if (mask == NULL) {
    while (entrance > 0) {
      c = entrance;
      prev = -1;
      
      do {
	b = boundary_pixels_3d(c);
	for (i = 0; i < n_nbrs; i++) {
	  if (b[i]) {
	    nb = c + neighbor[i];
	    if (stack->array[nb] >= threshold && 
		abs((int) (stack->array[c]) - stack->array[nb]) <= difference && 
		!tree[nb].level) {
	      tree[nb].level = tree[c].level + 1;
	      tree[nb].link = prev;
	      prev = nb;
	    }
	  }
	}
	c = tree[c].link;
      } while (c >= 0);
      entrance = prev;
    }
  } else {
    while (entrance > 0) {
      c = entrance;
      prev = -1;
      
      do {
	b = boundary_pixels_3d(c);
	for (i = 0; i < n_nbrs; i++) {
	  if (b[i]) {
	    nb = c + neighbor[i];
	    if (stack->array[nb] >= threshold && 
		abs((int) (stack->array[c]) - stack->array[nb]) <= difference && 
		!tree[nb].level &&
		mask->array[nb]) {
	      tree[nb].level = tree[c].level + 1;
	      tree[nb].link = prev;
	      prev = nb;
	    }
	  }
	}
	c = tree[c].link;
      } while (c >= 0);
      entrance = prev;
    }
  }

  return tree;  
}

/* Stack_Label_Object(): Label an object in a binary stack.
 *
 * Args: stack - binary stack to label;
 *       chord - a 3D matrix to store the queue of the voxels in the object;
 *       seed - index of the voxel to start;
 *       flag - value of a pixel that can be labeled;
 *       label - value of the label.
 *
 * Return: the size of the object.
 *
 * Note: The function does not check if <stack> is binary. It does not check if
 *       seed or label is valid.
 */
int Stack_Label_Object(Stack *stack, IMatrix *chord, int seed, int flag,
		       int label)
{
  if (chord->ndim != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if ((stack->width != chord->dim[0]) || (stack->height != chord->dim[1]) 
      || (stack->depth != chord->dim[2])) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (stack->array[seed] != flag) {
    TZ_WARN(ERROR_OTHER);
    fprintf(stderr, "The seed does not have the right flag.\n");
    return 0;
  }

  int npixel = Get_Stack_Size(stack);
  
  int n_nbrs = 0;
  int neighbor[26];
  int *b = NULL;
  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  for (i = 0; i < npixel; i++) {
    chord->array[i] = -1;
  }

  init_imginfo(stack->width, stack->height, stack->depth);
  n_nbrs = init_neighbor(neighbor);
 
  int obj_size = 0;
  int next = c;

  stack->array[seed] = label;

  do {
    b = boundary_pixels(c);

    /* add all unlabeled neighbors to the queue*/    
    for (i = 0; i < n_nbrs; i++) {
      if (b[i]) {
	nb = c + neighbor[i];
	/*process unlabeled white neighbors*/
	if ((stack->array[nb] == flag) && (chord->array[nb] == -1)) {
	  chord->array[next] = nb;
	  next = nb;
	  stack->array[nb] = label;
	}
      }
    }

    c = chord->array[c]; /* move to next voxel */
    obj_size++;
  } while (c >= 0);

  return obj_size;
}

/* Stack_Label_Object_Constraint(): Label an object in a binary stack with
 *                                  constraint.
 *
 * Args: stack - binary stack to label;
 *       chord - a 3D matrix to store the queue of the voxels in the object;
 *       seed - index of the voxel to start;
 *       flag - value of a pixel that can be labeled;
 *       label - value of the label.
 *       code - constraint label
 *       max - max value of the constraint (it is size if <code> is NULL)
 *
 * Return: the size of the object.
 *
 * Note: The function does not check if <stack> is binary. It does not check if
 *       seed or label is valid.
 */
int Stack_Label_Object_Constraint(Stack *stack, IMatrix *chord, int seed, 
				  int flag, int label, Stack *code, int max)
{
  if (chord->ndim != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if ((stack->width != chord->dim[0]) || (stack->height != chord->dim[1]) 
      || (stack->depth != chord->dim[2])) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (stack->array[seed] != flag) {
    TZ_WARN(ERROR_OTHER);
    fprintf(stderr, "The seed does not have the right flag.\n");
    return 0;
  }

  int npixel = Get_Stack_Size(stack);
  
  int n_nbrs = 0;
  int neighbor[26];
  int *b = NULL;
  int i;
  int c = seed; /* center pixel */
  int nb;       /* neighobr pixel */
  
  for (i = 0; i < npixel; i++) {
    chord->array[i] = -1;
  }

  init_imginfo(stack->width, stack->height, stack->depth);
  n_nbrs = init_neighbor(neighbor);
 
  int obj_size = 0;
  int next = c;
  uint16 *code_array = NULL;
  BOOL do_label = TRUE;

  if (code != NULL) {
    code_array = (uint16 *)code->array;
  }

  stack->array[seed] = label;

  do {
    b = boundary_pixels(c);

    /* add all unlabeled neighbors to the queue*/    
    for (i = 0; i < n_nbrs; i++) {
      if (b[i]) {
	nb = c + neighbor[i];
	/*process unlabeled white neighbors*/
	if (code != NULL) {
	  if (code_array[nb] > max) {
	    do_label = FALSE;
	  } else {
	    do_label = TRUE;
	  }
	} else {
	  do_label = TRUE;
	}

	if ((stack->array[nb] == flag) && (chord->array[nb] == -1)
	    && (do_label == TRUE)) {
	  chord->array[next] = nb;
	  next = nb;
	  stack->array[nb] = label;
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

  return obj_size;
}

Stack* Stack_Boundary_Code(Stack *stack, Stack *code, Objlabel_Workspace *ow)
{
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);

  int nvoxel = Stack_Voxel_Number(stack);
  int i;

  if (ow->init_chord == TRUE) {
    for (i = 0; i < nvoxel; i++) {
      ow->chord->array[i] = -1;
    }
  }

  if (code == NULL) {
    code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);
  }

  Zero_Stack(code);

  int neighbor[26];
  int is_in_bound[26];
  int nbound;
  BOOL is_boundary = FALSE;

  Stack_Neighbor_Offset(ow->conn, stack->width, stack->height, neighbor);

  int prev_seed = -1;
  int offset = 0;
  int x, y, z;

  cwidth = stack->width - 1;
  cheight = stack->height - 1;
  cdepth = stack->depth - 1;

  uint16 *code_array = (uint16 *) code->array;

  for (z = 0; z < stack->depth; z++) {
    for (y = 0; y < stack->height; y++) {
      for (x = 0; x < stack->width; x++) {
	is_boundary = FALSE;
	if (stack->array[offset] == 1) {
	  nbound = Stack_Neighbor_Bound_Test(ow->conn, cwidth, cheight,
					     cdepth, x, y, z, is_in_bound);
	  if (nbound < ow->conn) {
	    is_boundary = TRUE;
	  } else {
	    for (i = 0; i < ow->conn; i++) {
	      if (stack->array[offset + neighbor[i]] == 0) {
		is_boundary = TRUE;
		break;
	      }
	    }
	  }
	}
	if (is_boundary == TRUE) {
	  code_array[offset] = 1;
	  ow->chord->array[offset] = prev_seed;
	  prev_seed = offset;
	}
	offset++;
      }
    }
  }

  int entrance = prev_seed;

  int area = stack->width * stack->height;
  int c;
  int prev;
  int nb;
  int *link = ow->chord->array;

  /* generate level field  */
  while (entrance > 0) {
    c = entrance;
    prev = -1;

    do {					
      if (code_array[c] == 1) {
	Stack_Util_Coord_A(c, stack->width, area, &x, &y, &z);
	nbound = Stack_Neighbor_Bound_Test(ow->conn, cwidth, cheight,
					     cdepth, x, y, z, is_in_bound);
	if (nbound < ow->conn) {
	  for (i = 0; i < ow->conn; i++) {
	    if (is_in_bound[i]) {
	      nb = c + neighbor[i];
	      if (stack->array[nb] && !code_array[nb]) {
		code_array[nb] = 2;
		link[nb] = prev;
		prev = nb;
	      }
	    }
	  }
	} else {
	  for (i = 0; i < nbound; i++) {
	    nb = c + neighbor[i];
	    if (stack->array[nb] && !code_array[nb]) {
	      code_array[nb] = 2;
	      link[nb] = prev;
	      prev = nb;
	    }
	  }
	}
      } else {
	for (i = 0; i < ow->conn; i++) {
	  nb = c + neighbor[i];
	  if (stack->array[nb] && !code_array[nb]) {
	    code_array[nb] = code_array[c] + 1;
	    link[nb] = prev;
	    prev = nb;
	  }
	}
      }
      c = link[c];
    } while (c >= 0);
    entrance = prev;
  }

  
  STACK_OBJLABEL_CLOSE_WORKSPACE(ow);

  return code;
}
