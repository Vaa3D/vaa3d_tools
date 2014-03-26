/* tz_stack_bwmorph.c
 *
 * 05-Nov-2007
 */

#include <stdlib.h>
#include <string.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_iimage_lib.h"
#include "tz_iarray.h"
#include "tz_farray.h"
#include "tz_u16array.h"
#include "tz_utilities.h"
#include "tz_stack_math.h"
#include "tz_interface.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_relation.h"
#include "tz_stack_threshold.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_stack_bwmorph.h"
#include "tz_fmatrix.h"
#include "tz_fimage_lib.h"
#include "tz_stack.h"
#include "tz_voxel_graphics.h"
#include "tz_pixel_array.h"
#include "tz_stack_sampling.h"
#include "tz_int_linked_list.h"
#include "tz_math.h"
#include "tz_arrayview.h"
#include "tz_constant.h"
#include "tz_stack_objlabel.h"

INIT_EXCEPTION

#include "private/tz_stack_bwmorph.c"

#define IS_FOREGOUND_VALUE(v) ((v) > 0)

static uint8_t *Conn26_Topology_Flag = NULL;
static uint8_t *Conn26_Topology_Cut_Flag = NULL;

/* 2^26 - 1*/
static const int Max_Z3_Code = 67108863;

uint8_t* Get_Conn26_Topology_Cut_Flag()
{
  const char *flag_file = "conn26_cut_id.bn";

  if (Conn26_Topology_Cut_Flag == NULL) {
    Conn26_Topology_Cut_Flag = (uint8_t*) 
      Guarded_Calloc(Max_Z3_Code + 2, 1, "Get_Conn26_Topology_Flag"); 
    if (!fexist(flag_file)) {
      Stack *stack = Make_Stack(GREY, 3, 3, 3);
      dim_type dim[3] = {3, 3, 3};

      FILE *fp = fopen(flag_file, "wb");
      IMatrix *chord = Make_IMatrix(dim, 3);
      int i = 0;
      fwrite(&i, sizeof(int), 1, fp); 
      for (i = 1; i <= Max_Z3_Code; ++i) {
        int id = i;
        Zero_Stack(stack);
        int index = 0;
        while (id > 0) {
          if (id % 2 > 0) {
            stack->array[index] = 1;
          }
          id /= 2;
          ++index;
          if (index == 13) {
            ++index;
          }
        }

        int nobj = Stack_Label_Objects_N(stack, chord, 1, 2, 26);
        if (nobj > 1) {
          fwrite(&i, sizeof(int), 1, fp); 
        }
      }

      /* file end */
      i = -1;
      fwrite(&i, sizeof(int), 1, fp); 

      Kill_Stack(stack);
      Kill_IMatrix(chord);
      fclose(fp);
    }

    FILE *fp = fopen(flag_file, "rb");

    int id = 0;
    while (id >= 0) {
      fread(&id, sizeof(int), 1, fp);
      Conn26_Topology_Cut_Flag[id] = 1;
    }

    //Conn26_Topology_Flag[0] = 0;

    fclose(fp);
  }

  return Conn26_Topology_Cut_Flag;
}

uint8_t* Get_Conn26_Topology_Flag()
{
  if (Conn26_Topology_Flag == NULL) {
    Conn26_Topology_Flag = (uint8_t*) 
      Guarded_Calloc(Max_Z3_Code + 2, 1, "Get_Conn26_Topology_Flag");
    uint8_t *Conn26_Topology_Cut_Flag = Get_Conn26_Topology_Cut_Flag();
    memcpy(Conn26_Topology_Flag, Conn26_Topology_Cut_Flag, Max_Z3_Code + 2);

    const char *flag_file = "conn26_dig_id.bn";
    if (!fexist(flag_file)) {
      Stack *stack = Make_Stack(GREY, 3, 3, 3);
      Stack *chord = Make_Stack(GREY, 3, 3, 3);

      FILE *fp = fopen(flag_file, "wb");

      int i;
      for (i = 1; i < Max_Z3_Code; ++i) {
        if (Conn26_Topology_Cut_Flag[i] == 0) {
          int id = i;
          Zero_Stack(stack);
          int index = 0;
          while (id > 0) {
            if (id % 2 > 0) {
              stack->array[index] = 1;
            }
            id /= 2;
            ++index;
            if (index == 13) {
              ++index;
            }
          }

#ifdef _DEBUG_
          printf("%d", i);
#endif
          if (Stack_Has_Hole_Z3(stack, chord) == TRUE) {
            fwrite(&i, sizeof(int), 1, fp); 
#ifdef _DEBUG_
            printf("***");
#endif
          }
#ifdef _DEBUG_
          printf("\n");
#endif
        }
      }

      /* file end */
      i = Max_Z3_Code; /* The last code has a hole */
      fwrite(&i, sizeof(int), 1, fp); 

      i = -1;
      fwrite(&i, sizeof(int), 1, fp); 

      Kill_Stack(stack);
      Kill_Stack(chord);
      fclose(fp);
    }

    FILE *fp = fopen(flag_file, "rb");

    int id = 0;
    fread(&id, sizeof(int), 1, fp);
    while (id >= 0) {
      Conn26_Topology_Flag[id] = 1;
      fread(&id, sizeof(int), 1, fp);
    }

    fclose(fp);
  }

  return Conn26_Topology_Flag;
}

/* Print_Se(): print structure element.
 * 
 * Args: se - structure element.
 */
void Print_Se(Struct_Element *se)
{
  int i;

  printf("size: %d\n", se->size);
  for (i = 0; i < se->size; i++) {
    printf("[%d, %d, %d]: %u\n", se->offset[i][0], se->offset[i][1],
	   se->offset[i][2], se->mask[i]);
  }
}

/* New_Se(): new a structure element.
 *
 * Note: It only allocate space to the structure element. Use Free_Se() to free
 *       the space.
 *
 * Args: size - size of the structure element (number of functioning pixels).
 *
 * Return: a pointer to the structure element. 
 */
Struct_Element *New_Se(int size)
{
  Struct_Element *se = (Struct_Element *) malloc(sizeof(Struct_Element));
  se->size = size;
  se->offset = (Coordinate *) malloc(sizeof(Coordinate) * size);
  se->mask = (uint8 *) malloc(sizeof(uint8) * size);
  Se_In_Use++;

  return se;
}

/* Free_Se(): free a structure element.
 *
 * Args: se - pointer of the structure element to free.
 *
 * Return: void.
 */
void Free_Se(Struct_Element *se)
{
  if (se != NULL) {
    free(se->offset);
    free(se->mask);
    free(se);
    Se_In_Use--;
  }
}

int Se_Usage()
{
  return Se_In_Use;
}

void Kill_Struct_Element(Struct_Element *se)
{
  if (se != NULL) {
    free(se->offset);
    free(se->mask);
    free(se);
    Se_In_Use--;
  }
}

/* Make_Corner_Se_2D(): Make a 2D corner detector.
 *
 * Args: orient - orientation.
 *        1:        2:       3:       4:
 *            1         1        0 0    0 0
 *          0 1 1     1 1 0    1 1 0    0 1 1
 *          0 0         0 0      1        1
 *
 * Return: a structure element. 
 */
Struct_Element *Make_Corner_Se_2D(int orient)
{
  Struct_Element *se = New_Se(6);
  switch(orient) {
  case 1:
    se->offset[0][0] = 0;
    se->offset[0][1] = -1;
    se->offset[0][2] = 0;
    se->offset[1][0] = -1;
    se->offset[1][1] = 0;
    se->offset[1][2] = 0;
    se->offset[2][0] = 0;
    se->offset[2][1] = 0;
    se->offset[2][2] = 0;
    se->offset[3][0] = 1;
    se->offset[3][1] = 0;
    se->offset[3][2] = 0;
    se->offset[4][0] = -1;
    se->offset[4][1] = 1;
    se->offset[4][2] = 0;
    se->offset[5][0] = 0;
    se->offset[5][1] = 1;
    se->offset[5][2] = 0;
    se->mask[0] = 1;
    se->mask[1] = 0;
    se->mask[2] = 1;
    se->mask[3] = 1;
    se->mask[4] = 0;
    se->mask[5] = 0;
    break;
  case 2:
    se->offset[0][0] = 0;
    se->offset[0][1] = -1;
    se->offset[0][2] = 0;
    se->offset[1][0] = -1;
    se->offset[1][1] = 0;
    se->offset[1][2] = 0;
    se->offset[2][0] = 0;
    se->offset[2][1] = 0;
    se->offset[2][2] = 0;
    se->offset[3][0] = 1;
    se->offset[3][1] = 0;
    se->offset[3][2] = 0;
    se->offset[4][0] = 0;
    se->offset[4][1] = 1;
    se->offset[4][2] = 0;
    se->offset[5][0] = 1;
    se->offset[5][1] = 1;
    se->offset[5][2] = 0;
    se->mask[0] = 1;
    se->mask[1] = 1;
    se->mask[2] = 1;
    se->mask[3] = 0;
    se->mask[4] = 0;
    se->mask[5] = 0;
    break;
  case 3:
    se->offset[0][0] = 0;
    se->offset[0][1] = -1;
    se->offset[0][2] = 0;
    se->offset[1][0] = 1;
    se->offset[1][1] = -1;
    se->offset[1][2] = 0;
    se->offset[2][0] = -1;
    se->offset[2][1] = 0;
    se->offset[2][2] = 0;
    se->offset[3][0] = 0;
    se->offset[3][1] = 0;
    se->offset[3][2] = 0;
    se->offset[4][0] = 1;
    se->offset[4][1] = 0;
    se->offset[4][2] = 0;
    se->offset[5][0] = 0;
    se->offset[5][1] = 1;
    se->offset[5][2] = 0;
    se->mask[0] = 0;
    se->mask[1] = 0;
    se->mask[2] = 1;
    se->mask[3] = 1;
    se->mask[4] = 0;
    se->mask[5] = 1;
    break;
  case 4:
    se->offset[0][0] = -1;
    se->offset[0][1] = -1;
    se->offset[0][2] = 0;
    se->offset[1][0] = 0;
    se->offset[1][1] = -1;
    se->offset[1][2] = 0;
    se->offset[2][0] = -1;
    se->offset[2][1] = 0;
    se->offset[2][2] = 0;
    se->offset[3][0] = 0;
    se->offset[3][1] = 0;
    se->offset[3][2] = 0;
    se->offset[4][0] = 1;
    se->offset[4][1] = 0;
    se->offset[4][2] = 0;
    se->offset[5][0] = 0;
    se->offset[5][1] = 1;
    se->offset[5][2] = 0;
    se->mask[0] = 0;
    se->mask[1] = 0;
    se->mask[2] = 0;
    se->mask[3] = 1;
    se->mask[4] = 1;
    se->mask[5] = 1;
    break;
  default:
    TZ_WARN(ERROR_DATA_VALUE);
    Free_Se(se);
    se = NULL;
  }

  return se;
}

/* Make_Ball_Se(): Make ball-shape structure element.
 */
Struct_Element *Make_Ball_Se(int r)
{
  int d = r * 2 + 1;
  Struct_Element *se = New_Se(d * d * d);
  int i, j, k;
  int n = 0;

  for (k = -r; k <= r; k++) {
    for (j = -r; j <= r; j++) {
      for (i = -r; i <= r; i++) {
	if (i * i + j * j + k * k <= r * r) {
	  se->offset[n][0] = i;
	  se->offset[n][1] = j;
	  se->offset[n][2] = k;
	  se->mask[n] = 1;
	  n++;
	}
      }
    }
  }

  se->size = n;

  return se;
}

/* Make_Disc_Se(): Make disc-shape structure element.
 */
Struct_Element *Make_Disc_Se(int r)
{
  int d = r * 2 + 1;
  Struct_Element *se = New_Se(d * d);
  int i, j;
  int n = 0;

  for (j = -r; j <= r; j++) {
    for (i = -r; i <= r; i++) {
      if (i * i + j * j <= r * r) {
	se->offset[n][0] = i;
	se->offset[n][1] = j;
	se->offset[n][2] = 0;
	se->mask[n] = 1;
	n++;
      }
    }
  }

  se->size = n;

  return se;
}

/* Make_Disc_Se(): Make disc-shape structure element.
 */
Struct_Element *Make_Ring_Se(int ri, int ro)
{
  int d = ro * 2 + 1;

  Struct_Element *se = New_Se(d * d);
  int i, j;
  int n = 0;

  for (j = -ro; j <= ro; j++) {
    for (i = -ro; i <= ro; i++) {
      if ((i * i + j * j <= ro * ro) &&
          (i * i + j * j >= ri * ri)) {
	se->offset[n][0] = i;
	se->offset[n][1] = j;
	se->offset[n][2] = 0;
	se->mask[n] = 1;
	n++;
      }
    }
  }

  se->size = n;

  return se;
}

/* Make_Cuboid_Se(): Make cuboid-shape structure element.
 */
Struct_Element* Make_Cuboid_Se(int width, int height, int depth)
{
  Struct_Element *se = New_Se(width * height * depth);
  int corner_x = - width / 2;
  int corner_y = - height / 2;
  int corner_z = - depth / 2;
  int i, j, k;
  int n = 0;

  for (k = 0; k < depth; k++) {
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
	se->offset[n][0] = i + corner_x;
	se->offset[n][1] = j + corner_y;
	se->offset[n][2] = k + corner_z;
	se->mask[n] = 1;
	n++;
      }
    }
  }

  return se;
}

/* Make_Rect_Se(): Make rectangle-shape structure element.
 */
Struct_Element* Make_Rect_Se(int width, int height)
{
  Struct_Element *se = New_Se(width * height);
  int corner_x = - width / 2;
  int corner_y = - height / 2;

  int i, j;
  int n = 0;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      se->offset[n][0] = i + corner_x;
      se->offset[n][1] = j + corner_y;
      se->offset[n][2] = 0;
      se->mask[n] = 1;
      n++;
    }
  }

  return se;
}

Struct_Element *Make_Cross_Se(BOOL is_2d)
{
  Struct_Element *se = NULL;
  if (is_2d == TRUE) {
    se = New_Se(5);
  } else {
    se = New_Se(7);
  }

  se->offset[0][0] = 0;
  se->offset[0][1] = 0;
  se->offset[0][2] = 0;
  se->offset[1][0] = -1;
  se->offset[1][1] = 0;
  se->offset[1][2] = 0;
  se->offset[2][0] = 1;
  se->offset[2][1] = 0;
  se->offset[2][2] = 0;
  se->offset[3][0] = 0;
  se->offset[3][1] = -1;
  se->offset[3][2] = 0;
  se->offset[4][0] = 0;
  se->offset[4][1] = 1;
  se->offset[4][2] = 0;

  if (is_2d == FALSE) {
    se->offset[5][0] = 0;
    se->offset[5][1] = 0;
    se->offset[5][2] = -1;
    se->offset[6][0] = 0;
    se->offset[6][1] = 0;
    se->offset[6][2] = 1;
  }

  int i;
  for (i = 0; i < se->size; i++) {
    se->mask[i] = 1;
  }
  
  return se;
}

Struct_Element *Make_Zline_Se(int length)
{
  Struct_Element *se = NULL;
  se = New_Se(length);
  int i;
  for (i = 0; i < length; i++) {
    se->offset[i][0] = 0;
    se->offset[i][1] = 0;
    se->offset[i][2] = i - length / 2;
    se->mask[i] = 1;
  }

  return se;
}

/* Se_To_Stack(): Turn a structure element to a stack.
 *
 * Note: The caller is responsible for freeing the returned stack. 
 *
 * Args: se - input structure element.
 *
 * Return: a stack.
 */
Stack *Se_To_Stack(Struct_Element *se)
{
  int corner[3];
  int size[3];
  se_boundbox(se, corner, size);

  Stack *stack = Make_Stack(GREY, size[0], size[1], size[2]);
  int i;
  int length = Get_Stack_Size(stack);
  for (i = 0; i < length; i++) {
    stack->array[i] = 2;
  }

  for (i = 0; i < se->size; i++) {
    Set_Stack_Pixel(stack, se->offset[i][0] - corner[0], 
		    se->offset[i][1] - corner[1],
		    se->offset[i][2] - corner[2],
		    0, se->mask[i]);
  }

  return stack;
}

/* Stack_To_Se(): Turn a stack to a structure element.
 *
 * Note: The caller is responsible for freeing the returned stack. 
 *       The meaning of the stack pixel intensity:
 *       0 - background
 *       1 - foreground
 *       other - ignorable
 *
 * Args: stack - input stack, which must be GREY kind;
 *       center - position of the structure element;
 *       set - 0 includes foreground and background, 
 *             1 includes foreground only
 *
 * Return: a structure element.
 */
Struct_Element *Stack_To_Se(const Stack *stack, Coordinate center, int set)
{
  if (stack == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    return NULL;
  }

  if (stack->kind != GREY) {
    TZ_WARN(ERROR_DATA_COMPTB);
    return NULL;
  } 

  int length = 0;
  int i, j, k;
  int stack_size = Get_Stack_Size(stack);
  
  for (i = 0; i < stack_size; i++) {
    if (stack->array[i] <= 1)
      length++;
  }

  if (length == 0) {
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  Struct_Element *se = New_Se(length); 
  int p = 0;
  int q = 0;

  switch (set) {
  case 0:
    for (k = 0; k < stack->depth; k++) {
      for (j = 0; j < stack->height; j++) {
	for (i = 0; i < stack->width; i++) {
	  if (stack->array[q] <= 1) {
	    se->offset[p][0] = i - center[0];
	    se->offset[p][1] = j - center[1];
	    se->offset[p][2] = k - center[2];
	    se->mask[p] = stack->array[q];
	    p++;
	  }
	  q++;
	}
      }
    }
    break;
  case 1:
    for (k = 0; k < stack->depth; k++) {
      for (j = 0; j < stack->height; j++) {
	for (i = 0; i < stack->width; i++) {
	  if (stack->array[q] == 1) {
	    se->offset[p][0] = i - center[0];
	    se->offset[p][1] = j - center[1];
	    se->offset[p][2] = k - center[2];
	    se->mask[p] = stack->array[q];
	    p++;
	  }
	  q++;
	}
      }
    }
    break;
  default:
    Free_Se(se);
    THROW(ERROR_DATA_VALUE);
  }

  return se;
}


/* Stack_Erode: Erode a stack.
 */
Stack* Stack_Erode(const Stack *in, Stack *out, const Struct_Element *se)
{
  int i;
  for (i = 0; i <  se->size; i++) {
    if (se->mask[i] == 0) {
      THROW(ERROR_DATA_VALUE);
    }
  }

  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  } else {
    Copy_Stack_Array(out, in);
  }

  Stack_Hitmiss(in, out, se);

  return out;
}

/* Stack_Dilate: Dilate a stack.
 */
Stack* Stack_Dilate(const Stack *in, Stack *out, const Struct_Element *se)
{
  int i;
  for (i = 0; i <  se->size; i++) {
    if (se->mask[i] == 0) {
      THROW(ERROR_DATA_VALUE);
    }
  }

  if (out == NULL) {
    out = Copy_Stack((Stack *)in);
  } else {
    Copy_Stack_Array(out, in);
  }

  int offset = 0;  
  int n;

  
  Struct_Element *se2 = se_dilate(se);
  int *neighbor = (int *) malloc(sizeof(int) * se2->size);
  se_indices(se2, in->width, in->height, neighbor);

  //Stack *tmp_stack = Stack_Boundary(in, NULL);
  Stack *tmp_stack = Stack_Perimeter(in, NULL, 26);
  Coordinate coord;
  int stack_size[3];
  stack_size[0] = in->width;
  stack_size[1] = in->height;
  stack_size[2] = in->depth;

  for (coord[2] = 0; coord[2] < stack_size[2]; coord[2]++) {
    for (coord[1] = 0; coord[1] < stack_size[1]; coord[1]++) {
      for (coord[0] = 0; coord[0] < stack_size[0]; coord[0]++) {
	if (tmp_stack->array[offset] == 1) {
	  for (n = 0; n < se2->size; n++) {
	    if (hit_out(coord, &(se2->offset[n]), 1, stack_size) == FALSE) {
	      out->array[offset + neighbor[n]] = 1;
	    }
	  }
	}
	offset++;
      }
    }
  }

  free(neighbor);
  Free_Se(se2);
  Kill_Stack(tmp_stack);

  return out;
}

Stack* Stack_Dilate_Rm(const Stack *in, Stack *out, const Struct_Element *se,
		       const Stack *region_mask, int region)
{
  int i;
  for (i = 0; i <  se->size; i++) {
    if (se->mask[i] == 0) {
      THROW(ERROR_DATA_VALUE);
    }
  }

  if (out == NULL) {
    out = Copy_Stack((Stack *)in);
  } else {
    Copy_Stack_Array(out, in);
  }

  int offset = 0;  
  int n;

  
  Struct_Element *se2 = se_dilate(se);
  int *neighbor = (int *) malloc(sizeof(int) * se2->size);
  se_indices(se2, in->width, in->height, neighbor);

  //Stack *tmp_stack = Stack_Boundary(in, NULL);
  Stack *tmp_stack = Stack_Perimeter(in, NULL, 26);
  Coordinate coord;
  int stack_size[3];
  stack_size[0] = in->width;
  stack_size[1] = in->height;
  stack_size[2] = in->depth;

  for (coord[2] = 0; coord[2] < stack_size[2]; coord[2]++) {
    for (coord[1] = 0; coord[1] < stack_size[1]; coord[1]++) {
      for (coord[0] = 0; coord[0] < stack_size[0]; coord[0]++) {
	if (region_mask->array[offset] == region) {
	    if (tmp_stack->array[offset] == 1) {
	      for (n = 0; n < se2->size; n++) {
		if (hit_out(coord, &(se2->offset[n]), 1, stack_size) == FALSE) {
		  out->array[offset + neighbor[n]] = 1;
		}
	      }
	    }
	  }
	offset++;
      }
    }
  }

  free(neighbor);
  Free_Se(se2);
  Kill_Stack(tmp_stack);

  return out;  
}

/* Stack_Erode_Fast(): Fast version of eroding a stack.
 *
 * Args: in - input stack;
 *       out - output stack. A new stack will be created if it is NULL.
 *
 * Return: output stack.
 */
Stack* Stack_Erode_Fast(const Stack *in, Stack *out, const Struct_Element *se)
{
  if (in == NULL) {
    return NULL;
  }

  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  } else {
    Copy_Stack_Array(out, in);
  }

  int i;
  for (i = 0; i <  se->size; i++) {
    if (se->mask[i] == 0) {
      THROW(ERROR_DATA_VALUE);
    }
  }

  int corner[3], size[3];
  se_boundbox(se, corner, size);
  int boundBoxVolume = size[0] * size[1] * size[2];

  if ((corner[0] > 0) || (corner[1] > 0) || (corner[2] > 0)) {
    THROW(ERROR_DATA_VALUE);
  }

  if ((corner[0] + size[0] -1 < 0) || (corner[1] + size[1] -1 < 0) ||
      (corner[2] + size[2] -1 < 0)) {
    THROW(ERROR_DATA_VALUE);
  }

  /* Calculate block sum of the input stack. This will be used for
   * screening out unmatched pixels. */
  IMatrix *im = Get_Int_Matrix3(in);

  dim_type dim[3];
  translate_intdim(size, dim, 3);

  IMatrix *im2 = IMatrix_Blocksum(im, dim, NULL);
  Kill_IMatrix(im);

  iarray_abs(corner, 3);
  translate_intdim(corner, dim, 3);

  dim_type crop_size[3];
  crop_size[0] = in->width;
  crop_size[1] = in->height;
  crop_size[2] = in->depth;

  im = Crop_IMatrix(im2, dim, crop_size, NULL);
  Kill_IMatrix(im2);
  /****************************************/

  int area = in->width * in->height;
  int *neighbor = (int *) malloc(sizeof(int) * se->size);
  for (i = 0; i <  se->size; i++) {
    neighbor[i] = se->offset[i][0] + se->offset[i][1] * in->width + 
      se->offset[i][2] * area;
  }

  int coord[3];
  int s;
  int offset = 0;
  int thre = se->size;
  int stack_size[3];
  
  stack_size[0] = in->width;
  stack_size[1] = in->height;
  stack_size[2] = in->depth;

  printf("bound: %d\n", boundBoxVolume);
  printf("thre: %d\n", thre);

  for (coord[2] = 0; coord[2] < in->depth; coord[2]++) {
    PROGRESS_STATUS((100 * coord[2]) / in->depth)
    for (coord[1] = 0; coord[1] < in->height; coord[1]++) {
      for (coord[0] = 0; coord[0] < in->width; coord[0]++) {
	if (in->array[offset] == 1) {
          if (im->array[offset] < boundBoxVolume) {
            if (hit_out(coord, se->offset, se->size, stack_size) == FALSE) {
              if ((im->array[offset] >= thre)) {
                    printf("pixel off\n");
                for (s = 0; s < se->size; s++) {
                  if (in->array[offset + neighbor[s]] != 1) {
                    out->array[offset] = 0;
                    break;
                  }
                }
              } else {
                /***/
                out->array[offset] = 0;
              }
            } else { /* boundary pixels */
              /***/
              int i, j;
              int tmp_coord;

              for (i = 0; i < se->size; i++) {
                BOOL outside = FALSE;
                for (j = 0; j < 3; j++) {
                  tmp_coord = coord[j] + se->offset[i][j];
                  if ((tmp_coord < 0) || (tmp_coord >= stack_size[j])) {
                    outside =  TRUE;
                    break;
                  }
                }
                if (outside == FALSE) {
                  if (in->array[offset + neighbor[i]] != se->mask[i]) {
                    out->array[offset] = 0;
                    break;
                  }
                }
              }
              /***/ 
            }
          }
	  
	}
	offset++;
      }
    }
    PROGRESS_REFRESH
  }

  free(neighbor);
  Kill_IMatrix(im);

  return out;
}

Stack* Stack_Dilate_Fast(const Stack *in, Stack *out, const Struct_Element *se)
{
  //TZ_WARN(ERROR_UNTESTED_FUNC);

  if (in == NULL) {
    return NULL;
  }

  Stack *cin = Stack_Not((Stack*) in, NULL);
  out = Stack_Erode_Fast(cin, out, se);
  Stack_Not(out, out);
  Kill_Stack(cin);

  return out;
}

/* Stack_Hitmiss(): Hit-and-Miss Transform.
 *
 * Args: in - input binary stack;
 *       out - output stack, which must be initialized as a binary stack;
 *       neighbor - morphological operator shape;
 *       mask - morphological operator values;
 *
 * Return: 1 if there is any change or 0 if there is no change.
 */
int Stack_Hitmiss(const Stack *in, Stack *out, const Struct_Element *se)
{
  int area = in->width * in->height;
  int *neighbor;
  int i;

  neighbor = (int *) malloc(sizeof(int) * se->size);
  for (i = 0; i <  se->size; i++) {
    neighbor[i] = se->offset[i][0] + se->offset[i][1] * in->width + 
      se->offset[i][2] * area;
  }

  int coord[3];
  int offset = 0;
  int changed = 0;
  int value = 1;
  int stack_size[3];

  stack_size[0] = in->width;
  stack_size[1] = in->height;
  stack_size[2] = in->depth;

  for (coord[2] = 0; coord[2] < in->depth; coord[2]++) {
    PROGRESS_STATUS((100 * coord[2]) / in->depth)
    for (coord[1] = 0; coord[1] < in->height; coord[1]++) {
      for (coord[0] = 0; coord[0] < in->width; coord[0]++) {
	value = 1;

	if (hit_out(coord, se->offset, se->size, stack_size) == FALSE) {
	  for (i = 0; i < se->size; i++) {
	    if (in->array[offset + neighbor[i]] != se->mask[i]) {
	      value = 0;
	      break;
	    }
	  }
        } else {
          value = 1;
          /***/
          int i, j;
          int tmp_coord;

          for (i = 0; i < se->size; i++) {
            BOOL outside = FALSE;
            for (j = 0; j < 3; j++) {
              tmp_coord = coord[j] + se->offset[i][j];
              if ((tmp_coord < 0) || (tmp_coord >= stack_size[j])) {
                outside =  TRUE;
                break;
              }
            }
            if (outside == FALSE) {
              if (in->array[offset + neighbor[i]] != se->mask[i]) {
                value = 0;
                break;
              }
            }
          }
          /***/ 
        }
	
	if (changed == 0) {
	  if (in->array[offset] != value)
	    changed = 1;
	}

	out->array[offset++] = value;
      }
    }
    PROGRESS_REFRESH
  }

  free(neighbor);

  return changed;
}

/* Stack_Fillhole(): Fill holes in a stack.
 *
 * Args: in - input stack;
 *       out - output stack, which could be NULL.
 *       value - value filled in the holes. Be careful about the range of the 
 *               values of the input stack. The function will not check if it
 *               is out of range.
 *
 * Return: output stack.
 *
 * Note: The caller is responsible for freeing the returned stack. <out> should 
         not have overlap with <in>.
 */
Stack* Stack_Fillhole(Stack *in, Stack *out, int value)
{
  if (in == NULL) {
    return NULL;
  }

  if (out == NULL) {
    out = Copy_Stack(in);
  }

  if ((in->kind != GREY) && (in->kind != GREY16)) {
    THROW(ERROR_DATA_TYPE);
  }


  DEFINE_ARRAY(out_array, grey, uint8, out);
  DEFINE_ARRAY(out_array, grey16, uint16, out);
  DEFINE_ARRAY(in_array, grey, uint8, in);
  DEFINE_ARRAY(in_array, grey16, uint16, in);

  Stack_Label_Background(out, 1);
  int i;
  int length = Get_Stack_Size(in);

  switch (in->kind) {
  case GREY:
    for (i = 0; i < length; i++) {
      if (out_array_grey[i] == 0) {
	out_array_grey[i] = value;
      } else {
	out_array_grey[i] = in_array_grey[i];
      }
    }
    break;
  case GREY16:
    for (i = 0; i < length; i++) {
      if (out_array_grey16[i] == 0) {
	out_array_grey16[i] = value;
      } else {
	out_array_grey16[i] = in_array_grey16[i];
      }
    }
    break;
  default:
    TZ_WARN(ERROR_CODE_BUG);
    THROW(ERROR_DATA_TYPE);
  }

  return out;
}

Stack* Stack_Fill_2dhole(Stack *in, Stack *out, int value, int iscon1)
{
  if (out == NULL) {
    out = Copy_Stack(in);
  }

  int old_iscon1 = Get_Chkcon();
  int old_con2d = Get_Con2d();
  Set_Connection(iscon1, 1);
  Stack_Fillhole(in, out, value);
  Set_Connection(old_iscon1, old_con2d);

  return out;
}

Stack* Stack_Fill_Hole_N(Stack *in, Stack *out, int value, int conn, 
			 IMatrix *chord)
{
  if (in == NULL) {
    return NULL;
  }

  if ((in->kind != GREY) && (in->kind != GREY16)) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Copy_Stack(in);
  }

  STACK_CHECK_CHORD(out, chord, is_owner);
  Stack_Label_Background_N(out, 1, conn, chord);
  STACK_CHECK_CHORD_END(chord, is_owner);

  Stack_Not(out, out);
  Stack_Or(in, out, out);

  return out;  
}

Stack* Stack_Boundary(const Stack *in, Stack *out)
{
  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  }

  Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
  Stack_Erode_Fast(in, out, se);
  Free_Se(se);
  Stack_Xor((Stack *)in, out, out);

  return out;
}

Stack* Stack_Boundary_N(const Stack *in, Stack *out, int nnbr)
{
  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  }

  Struct_Element *se = NULL;
  switch (nnbr) {
  case 26:
    se = Make_Cuboid_Se(3, 3, 3);
    break;
  case 6:
    se = Make_Ball_Se(1);
    break;
  case 8:
    se = Make_Rect_Se(3, 3);
    break;
  case 4:
    se = Make_Disc_Se(1);
    break;
  default:
    Kill_Stack(out);
    THROW(ERROR_DATA_VALUE);
  }

  Stack_Erode_Fast(in, out, se);
  Free_Se(se);
  Stack_Xor((Stack *)in, out, out);

  return out;  
}

Stack* Stack_Majority_Filter(const Stack *in, Stack *out, int nnbr)
{
  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  }

  int neighbor[26];
  Stack_Neighbor_Offset(nnbr, in->width, in->height, neighbor);

  int is_in_bound[26];
  int offset = 0;
  int i, j, k;
  int b = 0;
  int n = 0;

  int half_nnbr = nnbr / 2;
  for (k = 0; k < in->depth; k++) {
    for (j = 0; j < in->height; j++) {
      for (i = 0; i < in->width; i++) {
	if (in->array[offset] > 0) {
	  n = 0;
	  int nbound = Stack_Neighbor_Bound_Test(nnbr, in->width - 1,
						 in->height - 1,
						 in->depth - 1,
						 i, j, k, is_in_bound);
	  if (nbound == nnbr) {
	    for (b = 0; b < nnbr; b++) {
	      if (in->array[offset + neighbor[b]] > 0) {
		n++;
	      }
	    }

	    if (n <= half_nnbr) {
	      out->array[offset] = 0;
	    }
	  } else {
	    for (b = 0; b < nnbr; b++) {
	      if (is_in_bound[b]) {
		if (in->array[offset + neighbor[b]] > 0) {
		  n++;
		}
	      }
	    }
	     
	    if (n <= nbound / 2) {
	      out->array[offset] = 0;
	    }
	  }
	}

	offset++;
      }
    }
  }
  
  return out;
}

Stack* Stack_Majority_Filter_R(const Stack *in, Stack *out, int conn, int mnbr)
{
  if (mnbr <= 0) {
    return Copy_Stack((Stack *) in);
  }

  if (mnbr > conn) {
    return NULL;
  }

  if (out == NULL) {
    out = Copy_Stack((Stack *) in);
  }

  int neighbor[26];
  Stack_Neighbor_Offset(conn, in->width, in->height, neighbor);

  int is_in_bound[26];
  int offset = 0;
  int i, j, k;
  int b = 0;
  int n = 0;

  for (k = 0; k < in->depth; k++) {
    for (j = 0; j < in->height; j++) {
      for (i = 0; i < in->width; i++) {
	if (in->array[offset] > 0) {
	  n = 0;
	  int nbound = Stack_Neighbor_Bound_Test(conn, in->width - 1,
						 in->height - 1,
						 in->depth - 1,
						 i, j, k, is_in_bound);
	  if (nbound == conn) {
	    for (b = 0; b < conn; b++) {
	      if (in->array[offset + neighbor[b]] > 0) {
		n++;
	      }
	    }

	    if (n < mnbr) {
	      out->array[offset] = 0;
	    }
	  } else {
	    for (b = 0; b < conn; b++) {
	      if (is_in_bound[b]) {
		if (in->array[offset + neighbor[b]] > 0) {
		  n++;
		}
	      }
	    }

	    if (n * conn < mnbr * nbound) {
	      out->array[offset] = 0;
	    }
	  }
	}

	offset++;
      }
    }
  }
  
  return out;
}

#define STACK_PERIMETER(in_array, out_array)				\
  for (k = 0; k < in->depth; k++) {					\
    for (j = 0; j < in->height; j++) {					\
      for (i = 0; i < in->width; i++) {					\
	out_array[offset] = 0;						\
	if (in_array[offset] > 0) {					\
	  n_in_bound = Stack_Neighbor_Bound_Test_S(conn, cwidth, cheight,	\
						 cdepth, i, j, k,	\
						 is_in_bound);		\
	  if (n_in_bound == conn) {					\
	    for (n = 0; n < n_in_bound; n++) {				\
	      if (in_array[offset + neighbor[n]] != in_array[offset]) {	\
		out_array[offset] = 1;					\
		break;							\
	      }								\
	    }								\
	  } else {							\
	    for (n = 0; n < n_in_bound; n++) {				\
	      if (is_in_bound[n]) {					\
		if (in_array[offset + neighbor[n]] != in_array[offset]) { \
		  out_array[offset] = 1;				\
		  break;						\
		}							\
	      }								\
	    }								\
									\
	    if (out_array[offset] == 0) {				\
	      out_array[offset] = 0;					\
	    }								\
	  }								\
	}								\
	offset++;							\
      }									\
    }									\
  }

Stack* Stack_Perimeter(const Stack *in, Stack *out, int conn)
{
  if (out == NULL) {
    out = Make_Stack(GREY, in->width, in->height, in->depth);
  } else {
    if (out->kind != GREY) {
      TZ_ERROR(ERROR_DATA_TYPE);
    }   
  }

  int offset = 0;
  int i, j, k;
  int n;
  int neighbor[26];
  int is_in_bound[26];
  int n_in_bound;
  int cwidth = in->width - 1;
  int cheight = in->height - 1;
  int cdepth = in->depth - 1;

  Stack_Neighbor_Offset(conn, in->width, in->height, neighbor);

  Image_Array ima;
  ima.array = in->array;

  switch (in->kind) {
  case GREY:
    STACK_PERIMETER(ima.array8, out->array);
    break;
  case GREY16:
    STACK_PERIMETER(ima.array16, out->array);
    break;
  case FLOAT32:
    STACK_PERIMETER(ima.array32, out->array);
    break;
  case FLOAT64:
    STACK_PERIMETER(ima.array64, out->array);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  /*
  for (k = 0; k < in->depth; k++) {
    for (j = 0; j < in->height; j++) {
      for (i = 0; i < in->width; i++) {
	out->array[offset] = 0;
	if (in->array[offset] > 0) {
	  n_in_bound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, 
						 cdepth, i, j, k, 
						 is_in_bound);
	  if (n_in_bound == conn) {
	    for (n = 0; n < n_in_bound; n++) {
	      if (in->array[offset + neighbor[n]] != in->array[offset]) {
		out->array[offset] = 1;
		break;
	      }
	    }
	  } else {
	    for (n = 0; n < n_in_bound; n++) {
	      if (is_in_bound[n]) {
		if (in->array[offset + neighbor[n]] != in->array[offset]) {
		  out->array[offset] = 1;
		  break;
		}
	      }
	    }

	    if (out->array[offset] == 0) {
	      out->array[offset] = 2;
	    }
	  }
	}
	offset++;
      }
    }
  }
  */

  return out;
}

Stack* Stack_Remove_Small_Object(Stack *in, Stack *out, 
				 int size, int conn)
{
  if (out == NULL) {
    out = Copy_Stack(in);
  }

  if (in != out) {
    if (Stack_Same_Attribute(in, out) == FALSE) {
      PRINT_EXCEPTION("Unmatched stacks", 
		      "The two stacks must have the same attributes");
      return NULL;
    } else {
      Copy_Stack_Array(out, in);
    }
  }

  int label = 2;
  Stack_Label_Large_Objects_N(out, NULL, 1, label, size, conn);
  Stack_Threshold_Binarize(out, label);

  return out;
}

#include "private/tz_stack_bwdist.c"
Stack *Stack_Bwdist_L(Stack *in, Stack *out, long int *label)
{
  if (out == NULL) {
    out = Make_Stack(FLOAT32, in->width, in->height, in->depth);
  }

  int nvoxel = Stack_Voxel_Number(in);
  int i;
  float *out_array = (float *) out->array;
  for (i = 0; i < nvoxel; i++) {
    out_array[i] = in->array[i];
  }

  long int sz[3];
  sz[0] = in->width;
  sz[1] = in->height;
  sz[2] = in->depth;
  dt3d_binary(out_array, label, sz, 1);

  return out;
}

#include "private/tz_stack_bwdist_mu16.c"
/* 
 * Stack_Bwdist_L_U16() calculates the distance transformation of <in>. The
 * result is a GREY16 stack and each voxel is the square of its shortest 
 * distance to the background. The distance is 0 if the voxel itself blongs 
 * to backgrond. The maximum distance is 255 because the limit of bit number.
 */
Stack *Stack_Bwdist_L_U16(Stack *in, Stack *out, int pad)
{
  ASSERT(in->kind == GREY, "GREY stack only");

  if (out == NULL) {
    out = Make_Stack(GREY16, in->width, in->height, in->depth);
  }

  int nvoxel = Stack_Voxel_Number(in);
  int i;
  uint16 *out_array = (uint16 *) out->array;
  for (i = 0; i < nvoxel; i++) {
    out_array[i] = in->array[i];
  }

  long int sz[3];
  sz[0] = in->width;
  sz[1] = in->height;
  sz[2] = in->depth;
  //dt3d_u16(out_array, sz, pad);
  /* The meaning of pad is different in the private function */
  dt3d_binary_mu16(out_array, sz, !pad);

  return out;
}

Stack *Stack_Bwdist_L_U16P(Stack *in, Stack *out, int pad)
{
  ASSERT(in->kind == GREY, "GREY stack only");

  if (out == NULL) {
    out = Make_Stack(GREY16, in->width, in->height, in->depth);
  }

  int nvoxel = Stack_Voxel_Number(in);
  int i;
  uint16 *out_array = (uint16 *) out->array;
  for (i = 0; i < nvoxel; i++) {
    out_array[i] = in->array[i];
  }

  long int sz[3];
  sz[0] = in->width;
  sz[1] = in->height;
  sz[2] = in->depth;
  /* The meaning of pad is different in the private function */
  dt3d_binary_mu16_p(out_array, sz, !pad);

  return out;
}

Stack_Seed_Workspace* New_Stack_Seed_Workspace()
{
  Stack_Seed_Workspace *ws = (Stack_Seed_Workspace *)Guarded_Malloc(sizeof(Stack_Seed_Workspace),
					    "New_Stack_Seed_Workspace");
  ws->method = 1;
  ws->with_boundary = FALSE;
  ws->seed_mask = NULL;
  ws->seed_dist = NULL;
  ws->weight = NULL;

  return ws;
}

void Clean_Stack_Seed_Workspace(Stack_Seed_Workspace *ws)
{
  if (ws->seed_mask != NULL) {
    Kill_Stack(ws->seed_mask);
    ws->seed_mask = NULL;
  }
  if (ws->seed_dist != NULL) {
    Kill_Stack(ws->seed_dist);
    ws->seed_dist = NULL;
  }

}

void Kill_Stack_Seed_Workspace(Stack_Seed_Workspace *ws)
{
  Clean_Stack_Seed_Workspace(ws);
  free(ws);
}

Geo3d_Scalar_Field* Stack_Seed(Stack *mask, Stack_Seed_Workspace *ws)
{
  /* distance transformation */
  if (ws->method == 3) {
    ws->seed_dist = Stack_Bwdist_L_U16P(mask, ws->seed_dist, 0);
  } else {
    ws->seed_dist = Stack_Bwdist_L_U16(mask, ws->seed_dist, 0);
  }

  Stack *distw = ws->seed_dist;
  if (ws->weight != NULL) { /* add weight if available */
    distw = Make_Stack(FLOAT32, mask->width, mask->height, mask->depth);
    double sigma[] = {1.0, 1.0, 0.5};
    FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
    FMatrix *f = Filter_Stack_Fast_F(ws->weight, filter, NULL, 0);
    
    Stack weight;
    weight.kind = FLOAT32;
    weight.width = mask->width;
    weight.height = mask->height;
    weight.depth = mask->depth;
    weight.array = (uint8*) (f->array);

    Stack_Mul(ws->seed_dist, &weight, distw);
    Kill_FMatrix(f);
    Kill_FMatrix(filter);
  }

  /* create seed mask */
  switch(ws->method) {
  case 1:
  case 3:
    {
      ws->seed_mask = Stack_Locmax_Region(distw, 26);
    
      Object_3d_List *objs = Stack_Find_Object_N(ws->seed_mask, NULL, 1, 0, 26);
      Zero_Stack(ws->seed_mask);
      int objnum = 0;
      while (objs != NULL) {
	Object_3d *obj = objs->data;
	Voxel_t center;
	Object_3d_Central_Voxel(obj, center);
	Set_Stack_Pixel(ws->seed_mask, center[0], center[1], center[2], 0, 1);
	objs = objs->next;
	objnum++;
      }
      Kill_Object_3d_List(objs);
    }

    break;
  case 2:
    ws->seed_mask = Stack_Local_Max(distw, NULL, STACK_LOCMAX_CENTER);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }
  
  if (distw != ws->seed_dist) {
    Kill_Stack(distw);
    distw = NULL;
  }

  //Write_Stack("../data/seed.tif", ws->seed_mask);

  /* extact seed positions */
  Voxel_List *list = Stack_To_Voxel_List(ws->seed_mask);
  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);
  
  /* extract seed values */
  Pixel_Array *pa = Voxel_List_Sampling(ws->seed_dist, list);
  uint16 *pa_array = (uint16 *) pa->array;

  /* create seeds */
  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(pa->size);

  int i;
  int cwidth = ws->seed_mask->width - 1;
  int cheight = ws->seed_mask->height - 1;
  int cdepth = ws->seed_mask->depth - 1;

  if (ws->with_boundary == FALSE) {
    field->size = 0;
  }

  for (i = 0; i < pa->size; i++) {
    if (ws->with_boundary == FALSE) {
      if (IS_IN_OPEN_RANGE3(voxel_array[i]->x, voxel_array[i]->y,
			    voxel_array[i]->z, 0, cwidth,
			    0, cheight, 0, cdepth)) {
	field->points[field->size][0] = voxel_array[i]->x;
	field->points[field->size][1] = voxel_array[i]->y;
	field->points[field->size][2] = voxel_array[i]->z;
	field->values[field->size] = sqrt((double)pa_array[i]);
	field->size++;
      }
    } else {
      field->points[i][0] = voxel_array[i]->x;
      field->points[i][1] = voxel_array[i]->y;
      field->points[i][2] = voxel_array[i]->z;
      field->values[i] = sqrt((double)pa_array[i]);
    }
  }

  free(voxel_array);
  Kill_Voxel_List(list);
  Kill_Pixel_Array(pa);

  return field;
}

double Stack_Volume_Surface_Ratio(Stack *stack, int fill_hole)
{
  TZ_ASSERT(stack->kind == GREY, "Must be GREY kind");

  Stack *filled = NULL;

  if (fill_hole > 0) {
    filled = Stack_Fill_Hole_N(stack, NULL, 1, fill_hole, NULL);
  } else {
    filled = stack;
  }

  int volume = 0;
  int surface = 0;

  int offset = 0;
  int i, j, k;
  int n;
  int neighbor[26];
  int is_in_bound[26];
  int n_in_bound;
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;
  int conn = 26;

  Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);

  for (k = 0; k < stack->depth; k++) {
    for (j = 0; j < stack->height; j++) {
      for (i = 0; i < stack->width; i++) {
	if (filled->array[offset] > 0) {
	  volume++;
	  n_in_bound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, 
						 cdepth, i, j, k, 
						 is_in_bound);
	  
	  for (n = 0; n < n_in_bound; n++) {
	    if (is_in_bound[n]) {
	      if (filled->array[offset + neighbor[n]] == 0) {
		surface++;
		break;
	      }
	    }
	  }
	}
	offset++;
      }
    }
  }

  Kill_Stack(filled);

  return (double) volume / surface;  
}

#define STACK_Z_DILATE(signal_array)					\
  /* downward */							\
  for (k = support_size - 1; k < stack->depth - size - 1; k++) {	\
    for (j = 0; j < stack->height; j++) {				\
      for (i = 0; i < stack->width; i++) {				\
	if ((stack->array[offset] == 1) && (stack->array[offset + area] == 0)) { \
	  mean = signal->array[offset];					\
	  for (z = 1; z < support_size; z++) {				\
	    mean += signal->array[offset - area * z];			\
	  }								\
	  mean /= support_size;						\
	  for (z = 1; z <= size; z++) {					\
	    if (mean >= signal->array[offset + area * z]) {		\
	      out->array[offset + area * z] = 1;			\
	    }								\
	  }								\
	}								\
	offset++;							\
      }									\
    }									\
  }									\
									\
  offset = size * area;							\
									\
  /* upward */								\
  for (k = size; k < stack->depth - support_size; k++) {		\
    for (j = 0; j < stack->height; j++) {				\
      for (i = 0; i < stack->width; i++) {				\
	if ((stack->array[offset] == 1) && (stack->array[offset - area] == 0)) { \
	  mean = signal->array[offset];					\
	  for (z = 1; z < support_size; z++) {				\
	    mean += signal->array[offset + area * z];			\
	  }								\
	  mean /= support_size;						\
	  for (z = 1; z <= size; z++) {					\
	    if (mean >= signal->array[offset - area * z]) {		\
	      out->array[offset - area * z] = 1;			\
	    }								\
	  }								\
	}								\
	offset++;							\
      }									\
    }									\
  }  

Stack* Stack_Z_Dilate(const Stack *stack, int size, const Stack *signal, 
		      Stack *out)
{
  int i, j, k, z;
  int area = stack->width * stack->height;
  double mean = 0;

  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  }

  int support_size = 3;
  int offset = support_size * area;

  Image_Array ima;
  ima.array = signal->array;
  switch (signal->kind) {
  case GREY:
    STACK_Z_DILATE(ima.array8);
    break;
  case GREY16:
    STACK_Z_DILATE(ima.array16);
    break;
  case FLOAT32:
    STACK_Z_DILATE(ima.array32);
    break;
  case FLOAT64:
    STACK_Z_DILATE(ima.array64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return out;
}

Stack* Stack_Region_Expand(const Stack *stack, int conn, int r, Stack *out)
{
  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  } else {
    Copy_Stack_Array(out, stack);
  }

  int nvoxel = Stack_Voxel_Number(stack);

  int neighbor[26];
  int is_in_bound[26];
  int n_in_bound = 0;
  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);
  Stack_Neighbor_Offset(conn, width, height, neighbor);

  Int_List *queue = NULL; 
  Int_List *buffer_queue = NULL;
  
  int i;
  for (i = 0; i < nvoxel; i++) {
    int label = stack->array[i];
    if (label > 0) {
      n_in_bound = Stack_Neighbor_Bound_Test_I(conn, width, height, depth, i,
	  is_in_bound);

      int j;
      if (n_in_bound == conn) {
	for (j = 0; j < conn; j++) {
	  int nbr = i + neighbor[j];
	  if (out->array[nbr] == 0) {
	    //Int_Queue_En(&queue, nbr);
	    Int_Stack_Push(&queue, nbr);
	    out->array[nbr] = label;
	  }
	}
      } else {
	for (j = 0; j < conn; j++) {
	  if (is_in_bound[j]) {
	    int nbr = i + neighbor[j];
	    if (out->array[nbr] == 0) {
	      //Int_Queue_En(&queue, nbr);
	      Int_Stack_Push(&queue, nbr);
	      out->array[nbr] = label;
	    }
	  }
	}
      }
    }
  }

  r--;

  int k;
  Int_List *tmp_queue;
  for (k = 0; k < r; k++) {
    if (Int_List_Is_Empty(queue) == TRUE) {
      break;
    }
    
    while (Int_List_Is_Empty(queue) == FALSE) {
      //i = Int_Queue_De(&queue); 
      i = Int_Stack_Pop(&queue);

      int label = out->array[i];
      TZ_ASSERT(label > 0, "Unexpected label.");
      n_in_bound = Stack_Neighbor_Bound_Test_I(conn, width, height, depth, i,
	  is_in_bound);

      int j;
      if (n_in_bound == conn) {
	for (j = 0; j < conn; j++) {
	  int nbr = i + neighbor[j];
	  if (out->array[nbr] == 0) {
	    //Int_Queue_En(&buffer_queue, nbr);
	    Int_Stack_Push(&buffer_queue, nbr);
	    out->array[nbr] = label;
	  }
	}
      } else {
	for (j = 0; j < conn; j++) {
	  if (is_in_bound[j]) {
	    int nbr = i + neighbor[j];
	    if (out->array[nbr] == 0) {
	      //Int_Queue_En(&buffer_queue, nbr);
	      Int_Stack_Push(&buffer_queue, nbr);
	      out->array[nbr] = label;
	    }
	  }
	}
      }
    }

    SWAP2(queue, buffer_queue, tmp_queue);
  }

  Kill_Int_List(queue);
  Kill_Int_List(buffer_queue);

  return out;

}

Stack* Stack_Region_Expand_M(const Stack *stack, int conn, int r, Stack *out,
    Stack *mask)
{
  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  } else {
    Copy_Stack_Array(out, stack);
  }

  int nvoxel = Stack_Voxel_Number(stack);

  int neighbor[26];
  int is_in_bound[26];
  int n_in_bound = 0;
  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);
  Stack_Neighbor_Offset(conn, width, height, neighbor);

  Int_List *queue = NULL; 
  Int_List *buffer_queue = NULL;
  
  int i;
  for (i = 0; i < nvoxel; i++) {
    int label = stack->array[i];
    if (label > 0) {
      n_in_bound = Stack_Neighbor_Bound_Test_I(conn, width, height, depth, i,
	  is_in_bound);

      int j;
      if (n_in_bound == conn) {
	for (j = 0; j < conn; j++) {
	  int nbr = i + neighbor[j];
          BOOL available = TRUE;

          if (mask != NULL) {
            if (mask->array[nbr] == 0) {
              available = FALSE;
            }
          }

	  if ((out->array[nbr] == 0) && available) {
	    //Int_Queue_En(&queue, nbr);
	    Int_Stack_Push(&queue, nbr);
	    out->array[nbr] = label;
	  }
	}
      } else {
	for (j = 0; j < conn; j++) {
	  if (is_in_bound[j]) {
	    int nbr = i + neighbor[j];
            BOOL available = TRUE;

            if (mask != NULL) {
              if (mask->array[nbr] == 0) {
                available = FALSE;
              }
            }

            if ((out->array[nbr] == 0) && available) {
	      //Int_Queue_En(&queue, nbr);
	      Int_Stack_Push(&queue, nbr);
	      out->array[nbr] = label;
	    }
	  }
	}
      }
    }
  }

  r--;

  int k;
  Int_List *tmp_queue;
  for (k = 0; k < r; k++) {
    if (Int_List_Is_Empty(queue) == TRUE) {
      break;
    }
    
    while (Int_List_Is_Empty(queue) == FALSE) {
      //i = Int_Queue_De(&queue); 
      i = Int_Stack_Pop(&queue);

      int label = out->array[i];
      TZ_ASSERT(label > 0, "Unexpected label.");
      n_in_bound = Stack_Neighbor_Bound_Test_I(conn, width, height, depth, i,
	  is_in_bound);

      int j;
      if (n_in_bound == conn) {
	for (j = 0; j < conn; j++) {
	  int nbr = i + neighbor[j];
          BOOL available = TRUE;

          if (mask != NULL) {
            if (mask->array[nbr] == 0) {
              available = FALSE;
            }
          }
	  if ((out->array[nbr] == 0) && available) {
	    //Int_Queue_En(&buffer_queue, nbr);
	    Int_Stack_Push(&buffer_queue, nbr);
	    out->array[nbr] = label;
	  }
	}
      } else {
	for (j = 0; j < conn; j++) {
	  if (is_in_bound[j]) {
            int nbr = i + neighbor[j];
            BOOL available = TRUE;

            if (mask != NULL) {
              if (mask->array[nbr] == 0) {
                available = FALSE;
              }
            }
            if ((out->array[nbr] == 0) && available) {
	      //Int_Queue_En(&buffer_queue, nbr);
	      Int_Stack_Push(&buffer_queue, nbr);
	      out->array[nbr] = label;
	    }
	  }
	}
      }
    }

    SWAP2(queue, buffer_queue, tmp_queue);
  }

  Kill_Int_List(queue);
  Kill_Int_List(buffer_queue);

  return out;

}

Stack* Stack_Remove_Isolated_Object(const Stack *stack, Stack *out, int r,
    int dr)
{
  Struct_Element *se = Make_Disc_Se(dr);

  out = Stack_Dilate(stack, out, se);

  Kill_Struct_Element(se);

  int minSize = iround(TZ_PI * (r + dr) * (r + dr));

  Stack *out2 = Stack_Remove_Small_Object(out, NULL, minSize, 8);

  Stack_And(out2, (Stack*) stack, out);

  Kill_Stack(out2);

  return out;
}

Stack* Stack_Bwinterp(const Stack *stack, Stack *out)
{
  Stack *perim = Stack_Perimeter(stack, NULL, 4);
  Stack_Not(perim, perim);
  Stack *dist = Stack_Bwdist_L_U16P(perim, NULL, 0);

  Free_Stack(perim);

  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  } else {
    TZ_ASSERT(Stack_Same_Attribute(stack, out), "Incompitable stacks.");
    Copy_Stack_Array(out, stack);
  }

  Image bwslice1;
  Image bwslice2;
  Image slice1;
  Image slice2;
  Image outSlice;

  int area = Stack_Width(stack) * Stack_Height(stack);

  int k;
  for (k = 1; k < Stack_Depth(stack) - 1; k++) {
    slice1 = Image_View_Stack_Slice(dist, k - 1);
    bwslice1 = Image_View_Stack_Slice(stack, k - 1);
    slice2 = Image_View_Stack_Slice(dist, k + 1);
    bwslice2 = Image_View_Stack_Slice(stack, k + 1);
    outSlice = Image_View_Stack_Slice(out, k);


    uint16_t *distArray1 = (uint16_t*) slice1.array;
    uint16_t *distArray2 = (uint16_t*) slice2.array;

    int offset;
    for (offset = 0; offset < area; offset++) {
      /* Zero if both outside */
      if ((bwslice1.array[offset] ==0) && (bwslice2.array[offset] == 0)) {
        /* Do nothing */
      } else if ((bwslice1.array[offset] == 1) && 
          (bwslice2.array[offset] == 1)) {
        outSlice.array[offset] = 1;
      } else if ((bwslice1.array[offset] > bwslice2.array[offset]) == 
          (distArray1[offset] > distArray2[offset])) {
        outSlice.array[offset] = 1;
      }
    }
  }

  return out;
}

#define BWSHRINK_CRITICAL 255
#define BWSHRINK_QUEUED 128
#define BWSHRINK_REMOVED 3

static void check_neighbor(const Stack *stack, const Stack *label, size_t index,
    const int* neighbor_offset, int neighbor_index, const int*neighbor_order,
    const uint8_t *critical_flag, 
    Arrayqueue *seedQueue, uint8_t *vbr)
{
  size_t voxelIndex = index + neighbor_offset[neighbor_index];
  vbr[neighbor_order[neighbor_index]] = 0;
  if (stack->array[voxelIndex] == 1) {
    if (seedQueue != NULL) {
      if (label->array[voxelIndex] == 1) { //Unchecked
        label->array[voxelIndex] = BWSHRINK_QUEUED;
        Arrayqueue_Enqueue(seedQueue, voxelIndex);
      }
    }
          
    if (label->array[voxelIndex] != BWSHRINK_REMOVED) {
      vbr[neighbor_order[neighbor_index]] = 1;
    }
  }
}

static int neighbor_pattern_to_code(uint8_t *vbr)
{
  int code = 0;
  int i;

  for (i = 25; i >= 0; --i) {
    code <<= 1;
    code += vbr[i];
  }

  return code;
}

static uint8_t is_critical(const Stack *stack, const Stack *label, 
    size_t index, const int *neighbor_offset, const int *neighbor_order,
    const uint8_t *critical_flag, Arrayqueue *seedQueue, int *code)
{
  int is_in_bound[26];
  
  int nbound = Stack_Neighbor_Bound_Test_I(26, stack->width, stack->height, 
      stack->depth, index, is_in_bound);

  uint8_t vbr[26];
  int i;
  if (nbound == 26) {
    for (i = 0; i < 26; ++i) {
      check_neighbor(stack, label, index, neighbor_offset, i, neighbor_order,
          critical_flag, seedQueue, vbr);
    }
  } else {
    for (i = 0; i < 26; ++i) {
      if (is_in_bound[i]) {
        check_neighbor(stack, label, index, neighbor_offset, i, neighbor_order,
            critical_flag, seedQueue, vbr);
      } else {
        vbr[neighbor_order[i]] = 0;
      }
    }
  }

  /* use chordal graph detection (LBFS)
  if (vbr[10] == 1 && vbr[12] == 1 && vbr[13] == 1 && vbr[15] == 1) {
    return 1;
  }
  */

  *code = neighbor_pattern_to_code(vbr);

  /*
  if (vbr[4] == 1 && vbr[21] == 1 && 
      vbr[10] == 1 && vbr[15] == 1) {
    return 1;
  }
  if (vbr[4] == 1 && vbr[21] == 1 && 
      vbr[12] == 1 && vbr[13] == 1) {
    return 1;
  }
  if (vbr[12] == 1 && vbr[13] == 1 && 
      vbr[10] == 1 && vbr[15] == 1) {
    return 1;
  }
  */

  return critical_flag[*code]; 
}


Stack* Stack_Bwshrink(const Stack *stack, Stack *out)
{
  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  }
  Copy_Stack_Array(out, stack);


  Arrayqueue *seedQueue = Make_Arrayqueue(Stack_Voxel_Number(stack));

  int neighbor_offset[26];
  Stack_Neighbor_Offset(26, stack->width, stack->height, neighbor_offset);

  int neighbor_order[26];
  int tmp_neighbor_offset[26];
  Stack_Neighbor_Offset(26, 3, 3, tmp_neighbor_offset);
  int i;
  for (i = 0; i < 26; ++i) {
    neighbor_order[i] = 13 + tmp_neighbor_offset[i];
    if (neighbor_order[i] > 13) {
      --neighbor_order[i];
    }
#ifdef _DEBUG_2
    printf("%d: %d\n", i + 1, neighbor_order[i]);
#endif
  }

  uint8_t *critical_flag = Get_Conn26_Topology_Flag();

  /* Identify a seed */
  size_t voxelIndex;
  size_t voxelNumber = Stack_Voxel_Number(stack);
  int code;
  int iter;
  const int max_iter = 10000;
  for (iter = 0; iter < max_iter; ++iter) {
    int count = 0;
    for (voxelIndex = 0; voxelIndex < voxelNumber; ++voxelIndex) {
      if (out->array[voxelIndex] != BWSHRINK_REMOVED && 
          stack->array[voxelIndex] == 1) {
        out->array[voxelIndex] = 1;
      }
    }

    for (voxelIndex = 0; voxelIndex < voxelNumber; ++voxelIndex) {
      if (out->array[voxelIndex] == 1) {
        if (!is_critical(stack, out, voxelIndex, neighbor_offset, 
              neighbor_order, critical_flag, NULL, &code)) {
          out->array[voxelIndex] = BWSHRINK_QUEUED;
          Arrayqueue_Enqueue(seedQueue, voxelIndex);
          break;
        }
      }
    }

    while (Arrayqueue_Is_Empty(seedQueue) == FALSE) {
      int seed = Arrayqueue_Dequeue(seedQueue);

      if (!is_critical(stack, out, seed, neighbor_offset, neighbor_order,
            critical_flag, seedQueue, &code)) {
        out->array[seed] = BWSHRINK_REMOVED;
        ++count;
        if (count == 1) {
          printf("%d removed.\n", seed);
        }
      } else {
        out->array[seed] = BWSHRINK_CRITICAL;
      }
    }

    /*
    for (voxelIndex = 0; voxelIndex < voxelNumber; ++voxelIndex) {
      if (out->array[voxelIndex] != BWSHRINK_REMOVED) {
        out->array[voxelIndex] = 1;
      }
    }

    for (voxelIndex = 0; voxelIndex < voxelNumber; ++voxelIndex) {
      if (stack->array[voxelIndex] == 1) {
        if (!is_critical(stack, out, voxelIndex, neighbor_offset, 
              neighbor_order, critical_flag, NULL, &code)) {
          out->array[voxelIndex] = BWSHRINK_QUEUED;
          Arrayqueue_Enqueue(seedQueue, voxelIndex);
        }
      }
    }

    while (Arrayqueue_Is_Empty(seedQueue) == FALSE) {
      int seed = Arrayqueue_Dequeue(seedQueue);

      if (!is_critical(stack, out, seed, neighbor_offset, neighbor_order,
            critical_flag, seedQueue, &code)) {
        out->array[seed] = BWSHRINK_REMOVED;
        ++count;
      } else {
        out->array[seed] = BWSHRINK_CRITICAL;
      }
    }
    */

    printf("%d: %d voxels removed\n", iter, count);
    if (count == 0) {
      for (voxelIndex = 0; voxelIndex < voxelNumber; ++voxelIndex) {
        if (out->array[voxelIndex] == 1) {
          out->array[voxelIndex] = 255;
        }
      }
      break;
    }
  }

  return out;

#if 0
  /* while the seed queue is not empty */
  while (Arrayqueue_Is_Empty(seedQueue) == FALSE) {
    int seed = Arrayqueue_Dequeue(seedQueue);

    if (!is_critical(stack, out, seed, neighbor_offset, neighbor_order,
          critical_flag, seedQueue, &code)) {
      out->array[seed] = BWSHRINK_REMOVED;
    } else {
      out->array[seed] = BWSHRINK_CRITICAL;
    }
  }

  Kill_Arrayqueue(seedQueue);

  return out;
#endif
}

Stack* Stack_Bwshrink_Z3(const Stack *stack, Stack *out)
{
  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  }
  Copy_Stack_Array(out, stack);

  Arrayqueue *seedQueue = Make_Arrayqueue(Stack_Voxel_Number(stack));

  int neighbor_offset[26];
  Stack_Neighbor_Offset(26, stack->width, stack->height, neighbor_offset);

  int neighbor_order[26];
  int tmp_neighbor_offset[26];
  Stack_Neighbor_Offset(26, 3, 3, tmp_neighbor_offset);
  int i;
  for (i = 0; i < 26; ++i) {
    neighbor_order[i] = 13 + tmp_neighbor_offset[i];
    if (neighbor_order[i] > 13) {
      --neighbor_order[i];
    }
#ifdef _DEBUG_2
    printf("%d: %d\n", i + 1, neighbor_order[i]);
#endif
  }

  uint8_t *critical_flag = Get_Conn26_Topology_Cut_Flag();

  /* Identify a seed */
  size_t voxelIndex;
  size_t voxelNumber = Stack_Voxel_Number(stack);

  size_t voxelIndexArray[27] = {0, 2, 6, 8, 18, 20, 24, 26, 1, 5, 7, 3,
    19, 23, 25, 21, 9, 11, 15, 17, 4, 22, 10, 16, 14, 12, 13 };

  for (i = 0; i < voxelNumber; ++i) {
    voxelIndex = voxelIndexArray[i];
    if (stack->array[voxelIndex] == 1) {
      Arrayqueue_Enqueue(seedQueue, voxelIndex);
    }
  }

  int code;

  /* while the seed queue is not empty */
  while (Arrayqueue_Is_Empty(seedQueue) == FALSE) {
    int count = 0;
    while (Arrayqueue_Is_Empty(seedQueue) == FALSE) {
      int seed = Arrayqueue_Dequeue(seedQueue);

      if (!is_critical(stack, out, seed, neighbor_offset, neighbor_order,
            critical_flag, NULL, &code)) {
        out->array[seed] = BWSHRINK_REMOVED;
        ++count;
      } else {
        out->array[seed] = BWSHRINK_CRITICAL;
      }
    }

    if (count == 0) {
      break;
    }

    count = 0;
    for (i = 0; i < voxelNumber; ++i) {
      voxelIndex = voxelIndexArray[i];
      if (stack->array[voxelIndex] == 1 && 
        out->array[voxelIndex] != BWSHRINK_REMOVED) {
        Arrayqueue_Enqueue(seedQueue, voxelIndex);
        ++count;
      }
    }
    if (count <= 1) {
      break;
    }
  }

  Kill_Arrayqueue(seedQueue);

  return out;
}

BOOL Stack_Has_Hole_Z3(const Stack *stack, Stack *out)
{
  Stack *in_out = out;
  out = Stack_Bwshrink_Z3(stack, out);

  int sum = 0;
  int i;
  for (i = 0; i < 27; ++i) {
    if (out->array[i] == BWSHRINK_CRITICAL) {
      out->array[i] = 1;
    } else {
      out->array[i] = 0;
    }
    sum += out->array[i];
  }

  if (sum > 1) {
    int cross_sum = stack->array[12] + stack->array[14] + 
      stack->array[4] + stack->array[22] + 
      stack->array[10] + stack->array[16];

    if (cross_sum == 6) {
      return TRUE;
    }

    if (sum == 4) {
      if (out->array[4] == 1 && out->array[22] == 1 && 
          out->array[10] == 1 && out->array[16] == 1) {
        if (stack->array[12] == 1 || stack->array[14] == 1) {
          return FALSE;
        }
      }
      if (out->array[4] == 1 && out->array[22] == 1 && 
          out->array[12] == 1 && out->array[14] == 1) {
        if (stack->array[10] == 1 || stack->array[16] == 1) {
          return FALSE;
        }
      }
      if (out->array[12] == 1 && out->array[14] == 1 && 
          out->array[10] == 1 && out->array[16] == 1) {
        if (stack->array[4] == 1 || stack->array[22] == 1) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }

  if (in_out == NULL) {
    Kill_Stack(out);
  }

  return FALSE;
}

size_t Stack_Foreground_Size(const Stack *stack)
{
  TZ_ASSERT(stack->kind == GREY, "Invalid stack kind");

  size_t s = 0;
  size_t index;
  size_t volume = Stack_Voxel_Number(stack);
  for (index = 0; index < volume; ++index) {
    s += IS_FOREGOUND_VALUE(stack->array[index]);
  }

  return s;
}

static int Euler_Table[256] = {
  0, 1, 0, -1, 0,-1, 0,1, 0,-3, 0,-1, 0,-1, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,-3, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,-1, 0,1,
  0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,-3, 0,3, 0,-1, 0,1, 0,1, 0,3, 0,-1, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,1, 0,3, 0,3, 0,1,
  0,5, 0,3, 0,3, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,-7, 0,-1, 0,-1, 0,1, 0,-3, 0,-1, 0,-1, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1,
  0,1, 0,-1, 0,-3, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,-3, 0,3, 0,-1, 0,1, 0,1, 0,3, 0,-1, 0,1,
  0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1, 0,1, 0,3, 0,3, 0,1, 0,5, 0,3, 0,3, 0,1, 0,-1, 0,1, 0,1, 0,-1, 0,3, 0,1, 0,1, 0,-1
};

static int Bwthin_Octant[8][7] = {
  {23, 24, 14, 15, 20, 21, 12},
  {25, 22, 16, 13, 24, 21, 15},
  {17, 20, 9, 12, 18, 21, 10},
  {19, 22, 18, 21, 11, 13, 10},
  {6, 14, 7, 15, 3, 12, 4},
  {8, 7, 16, 15, 5, 4, 13},
  {0, 9, 3, 12, 1, 10, 4},
  {2, 1, 11, 10, 5, 4, 13}
};

static unsigned char Octant_Code[7] = {128, 64, 32, 16, 8, 4, 2};


static BOOL stack_bwthin_is_euler_invariant(const int *neighbor)
{
  int euler_number = 0;
  int i, j;
  for (i = 0; i < 8; ++i) {
    unsigned char euler_code = 1;
    for (j = 0; j < 7; ++j) {
      if (neighbor[Bwthin_Octant[i][j]] > 0) {
        euler_code |= Octant_Code[j];
      }
    }
    euler_number += Euler_Table[euler_code];
  }

  return (euler_number == 0);
}

static int Octant_Index[26] = {
  1, 1, 2, 1, 1, 2, 3, 3, 4, 1, 1, 2, 1, 2, 3, 3, 4, 5, 5, 6, 5, 5, 6, 7, 7, 8
};

static void stack_bwthin_octree_label(int index, int label, 
    int *neighbor_label) 
{
  switch (index) {
    case 1:
      if(neighbor_label[0] == 1) {
        neighbor_label[0] = label;
      }

      if(neighbor_label[1] == 1) {
        neighbor_label[1] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
      }

      if(neighbor_label[3] == 1) {
        neighbor_label[3] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
      }
      if(neighbor_label[4] == 1) {
        neighbor_label[4] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[9] == 1) {
        neighbor_label[9] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
      }
      if(neighbor_label[10] == 1) {
        neighbor_label[10] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[12] == 1) {
        neighbor_label[12] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      break;
    case 2:
      if(neighbor_label[1] == 1) {
        neighbor_label[1] = label;
        stack_bwthin_octree_label(1, label, neighbor_label);
      }
      if(neighbor_label[4] == 1) {
        neighbor_label[4] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[10] == 1) {
        neighbor_label[10] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[2] == 1 ) {
        neighbor_label[2] = label;  
      }
      if(neighbor_label[5] == 1) {
        neighbor_label[5] = label;        
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[11] == 1) {
        neighbor_label[11] = label;        
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[13] == 1) {
        neighbor_label[13] = label;        
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 3:
      if(neighbor_label[3] == 1) {
        neighbor_label[3] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
      }
      if(neighbor_label[4] == 1) {
        neighbor_label[4] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[12] == 1) {
        neighbor_label[12] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[6] == 1) {
        neighbor_label[6] = label;        
      }
      if(neighbor_label[7] == 1) {
        neighbor_label[7] = label;        
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[14] == 1) {
        neighbor_label[14] = label;        
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[15] == 1) {
        neighbor_label[15] = label;        
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 4:
      if(neighbor_label[4] == 1) {
        neighbor_label[4] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(3, label, neighbor_label);
      }
      if(neighbor_label[5] == 1) {
        neighbor_label[5] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
      }
      if(neighbor_label[13] == 1) {
        neighbor_label[13] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[7] == 1) {
        neighbor_label[7] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
      }
      if(neighbor_label[15] == 1) {
        neighbor_label[15] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[8] == 1) {
        neighbor_label[8] = label;
      }
      if(neighbor_label[16] == 1) {
        neighbor_label[16] = label;        
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 5:
      if(neighbor_label[9] == 1) {
        neighbor_label[9] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
      }
      if(neighbor_label[10] == 1) {
        neighbor_label[10] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[12] == 1) {
        neighbor_label[12] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[17] == 1) {
        neighbor_label[17] = label;  
      }
      if(neighbor_label[18] == 1) {
        neighbor_label[18] = label;        
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[20] == 1) {
        neighbor_label[20] = label;        
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[21] == 1) {
        neighbor_label[21] = label;        
        stack_bwthin_octree_label(6, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 6:
      if(neighbor_label[10] == 1) {
        neighbor_label[10] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
      }
      if(neighbor_label[11] == 1) {
        neighbor_label[11] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
      }
      if(neighbor_label[13] == 1) {
        neighbor_label[13] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[18] == 1) {
        neighbor_label[18] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
      }
      if(neighbor_label[21] == 1) {
        neighbor_label[21] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[19] == 1) {
        neighbor_label[19] = label;  
      }
      if(neighbor_label[22] == 1) {
        neighbor_label[22] = label;        
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 7:
      if(neighbor_label[12] == 1) {
        neighbor_label[12] = label;        
        stack_bwthin_octree_label(1, label, neighbor_label);
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(5, label, neighbor_label);
      }
      if(neighbor_label[14] == 1) {
        neighbor_label[14] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
      }
      if(neighbor_label[15] == 1) {
        neighbor_label[15] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[20] == 1) {
        neighbor_label[20] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
      }
      if(neighbor_label[21] == 1) {
        neighbor_label[21] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      if(neighbor_label[23] == 1) {
        neighbor_label[23] = label;  
      }
      if(neighbor_label[24] == 1) {
        neighbor_label[24] = label;        
        stack_bwthin_octree_label(8, label, neighbor_label);
      }
      break;
    case 8:
      if(neighbor_label[13] == 1) {
        neighbor_label[13] = label;        
        stack_bwthin_octree_label(2, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[15] == 1) {
        neighbor_label[15] = label;        
        stack_bwthin_octree_label(3, label, neighbor_label);
        stack_bwthin_octree_label(4, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[16] == 1) {
        neighbor_label[16] = label;        
        stack_bwthin_octree_label(4, label, neighbor_label);
      }
      if(neighbor_label[21] == 1) {
        neighbor_label[21] = label;        
        stack_bwthin_octree_label(5, label, neighbor_label);
        stack_bwthin_octree_label(6, label, neighbor_label);
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[22] == 1) {
        neighbor_label[22] = label;        
        stack_bwthin_octree_label(6, label, neighbor_label);
      }
      if(neighbor_label[24] == 1) {
        neighbor_label[24] = label;        
        stack_bwthin_octree_label(7, label, neighbor_label);
      }
      if(neighbor_label[25] == 1) {
        neighbor_label[25] = label;
      }
      break;
    default:
      break;
  }
}

static BOOL stack_bwthin_is_simple_connect_point(const int *neighbor)
{
  int label = 2;
  int neighbor_label[26];

  int i = 0;
  for (i = 0; i < 26; ++i) {
    neighbor_label[i] = neighbor[i] > 0;
  }

  for (i = 0; i < 26; ++i) {
    if (neighbor_label[i] == 1) {
      stack_bwthin_octree_label(Octant_Index[i], label, neighbor_label);
      ++label;
      if (label >= 4) {
        return FALSE;
      }
    }
  }

  return TRUE;
}

static void stack_bwthin_neighbor_value(const Stack *stack, size_t index,
    int *neighbor_value) 
{
  int width = stack->width;
  size_t area = stack->width * stack->height;
  int swidth = width - 2;
  size_t offset = index - area - stack->width - 1;

  neighbor_value[0] = stack->array[offset++];
  neighbor_value[1] = stack->array[offset++];
  neighbor_value[2] = stack->array[offset];
  offset += swidth;
  neighbor_value[3] = stack->array[offset++];
  neighbor_value[4] = stack->array[offset++];
  neighbor_value[5] = stack->array[offset];
  offset += swidth;
  neighbor_value[6] = stack->array[offset++];
  neighbor_value[7] = stack->array[offset++];
  neighbor_value[8] = stack->array[offset];
  offset = index - width - 1;
  neighbor_value[9] = stack->array[offset++];
  neighbor_value[10] = stack->array[offset++];
  neighbor_value[11] = stack->array[offset];
  offset += swidth;
  neighbor_value[12] = stack->array[offset++];
  offset++;
  neighbor_value[13] = stack->array[offset];
  offset += swidth;
  neighbor_value[14] = stack->array[offset++];
  neighbor_value[15] = stack->array[offset++];
  neighbor_value[16] = stack->array[offset++];
  offset = index + area - width - 1;
  neighbor_value[17] = stack->array[offset++];
  neighbor_value[18] = stack->array[offset++];
  neighbor_value[19] = stack->array[offset];
  offset += swidth;
  neighbor_value[20] = stack->array[offset++];
  neighbor_value[21] = stack->array[offset++];
  neighbor_value[22] = stack->array[offset];
  offset += swidth;
  neighbor_value[23] = stack->array[offset++];
  neighbor_value[24] = stack->array[offset++];
  neighbor_value[25] = stack->array[offset++];
}

static BOOL stack_bwthin_is_candidate_voxel(const Stack *stack, size_t index,
    int x, int y, int z, int border, Stack_Bwpeel_Option_t peeling_option)
{
  if (stack->array[index] == 0) {
    return FALSE;
  }

  int neighbor_value[26];
  stack_bwthin_neighbor_value(stack, index, neighbor_value);

  if (!(border == 5 && neighbor_value[4] == 0 /*&& neighbor_value[21] > 0*/) &&
      !(border == 0 && neighbor_value[10] == 0 /*&& neighbor_value[15] > 0*/) &&
      !(border == 3 && neighbor_value[12] == 0 /*&& neighbor_value[13] > 0*/) &&
      !(border == 2 && neighbor_value[13] == 0 /*&& neighbor_value[12] > 0*/) &&
      !(border == 1 && neighbor_value[15] == 0 /*&& neighbor_value[10] > 0*/) &&
      !(border == 4 && neighbor_value[21] == 0 /*&& neighbor_value[4] > 0)*/)) {
    return FALSE;
  }

  int offset = 0;
  int neighborOnNumber = 0;
  for (offset = 0; offset < 26; offset++) {
    if (neighbor_value[offset] > 0) {
      ++neighborOnNumber;
    }
  }

  switch (peeling_option) {
    case NORMAL_THINNING:
      if (neighborOnNumber <= 1) {
        return FALSE;
      }
      break;
    case REMOVE_ARC:
      if (neighborOnNumber <= 1) {
        return TRUE;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
  }

  if (stack_bwthin_is_euler_invariant(neighbor_value) == FALSE) {
    return FALSE;
  }

  if (stack_bwthin_is_simple_connect_point(neighbor_value) == FALSE) {
    return FALSE;
  }

  return TRUE;
}

/* Building Skeleton Models via 3-D Medial Surface/Axis Thinning Algorithms, 
 * T. Lee et al (1994) */
Stack *Stack_Bwpeel(const Stack *stack, Stack_Bwpeel_Option_t option, 
    Stack *out)
{
  TZ_ASSERT(stack->kind == GREY, "Unsupported stack type.");

  if (out == NULL) {
    out = Make_Stack(GREY, stack->width + 2, stack->height + 2, 
        stack->depth + 2);
  }

  /* Expand the stack so that no boundary test is necessary for neighbor 
   * access */
  Crop_Stack(stack, -1, -1, -1, stack->width + 2, stack->height + 2,
      stack->depth + 2, out);
  
  size_t voxelNumber = Stack_Voxel_Number(out);
  size_t foregroundSize = 0;
  size_t offset = 0;
  /* Foreground size is necessary for initializing checklist*/
  for (offset = 0; offset < voxelNumber; ++offset) {
    if (out->array[offset] > 0) {
      ++foregroundSize;
    }
  }
  
  /* alloc <checklist> */
  size_t *checklist = (size_t*) Guarded_Malloc(sizeof(size_t) * foregroundSize,
      "Stack_Bwthin");
  size_t list_length = 0;

  size_t area = out->width * out->height;
  int cwidth = out->width - 1;
  int cheight = out->height - 1;
  int cdepth = out->depth - 1;

  int changed = 6; /* six borders */
  while (changed) { /* any border has been changed */
    changed = 6;
    int border;
    int twice_width = out->width * 2;
    for (border = 0; border < 6; ++border) {
      /* For each voxel in the stack */
      size_t index = area + out->width + 1;
      int x, y, z;
      list_length = 0;
      for (z = 1; z < cdepth; ++z) {
        for (y = 1; y < cheight; ++y) {
          for (x = 1; x <cwidth; ++x) {
            /* If the voxel passes the test*/
            if (stack_bwthin_is_candidate_voxel(out, index, x, y, z, border, 
                  option)) {
              checklist[list_length++] = index;
#ifdef _DEBUG_2
              printf("list length: %d\n", (int) list_length);
#endif
            }

            ++index;
          }
          index += 2;
        }
        index += twice_width;
      }

      /* For each point in the checklist */
      int neighbor_value[26];
      BOOL voxel_removed = FALSE;
      for (index = 0; index < list_length; ++index) {
        offset = checklist[index];
        int has_on_neighbor = 0;
        stack_bwthin_neighbor_value(out, offset, neighbor_value);

        int i;
        for (i = 0; i < 26; ++i) {
          if (neighbor_value[i] > 0) {
            has_on_neighbor = 1;
            break;
          }
        }
        /* If the number of ON neighbor is greater than 1*/
        if (has_on_neighbor) {
          /* If the neighborhood is connected */
          if (stack_bwthin_is_simple_connect_point(neighbor_value) == TRUE) {
            /* Turn the point back off */
            out->array[offset] = 0;
            voxel_removed = TRUE;
          }
        } else {
          if (option == REMOVE_ARC) {
            /* Turn the point back off */
            out->array[offset] = 0;
            voxel_removed = TRUE;
          }
        }
      }
          
#ifdef _DEBUG_2
      Print_Stack_Value(out);
#endif
      if (voxel_removed == FALSE) {
        --changed;
      }
    }
  }

  /* free <checklist> */
  free(checklist);

  Stack *out2 = Crop_Stack(out, 1, 1, 1, stack->width, stack->height, 
      stack->depth, NULL); 

  Kill_Stack(out);

  return out2;
}

Stack *Stack_Bwthin(const Stack *stack, Stack *out)
{
  return Stack_Bwpeel(stack, NORMAL_THINNING, out);
}

int Stack_Bwthin_Count_Simple_Point()
{
  int neighbor[26];
  int i;
  for (i = 0; i < 26; ++i) {
    neighbor[i] = 0;
  }

  int count = 0;
  int neighbor_code = 0;
  while (neighbor_code >> 26 == 0) {
    for (i = 0; i < 26; ++i) {
      neighbor[i] = (neighbor_code >> i) % 2;
    }

    if (stack_bwthin_is_euler_invariant(neighbor) &&
        stack_bwthin_is_simple_connect_point(neighbor)) {
      ++count;
    }
    ++neighbor_code;
  }

  return count;
}
