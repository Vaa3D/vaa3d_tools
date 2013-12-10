/* tz_stack_draw.c
 *
 * 30-Nov-2007 Ting Zhao
 */

#include <string.h>
#include <math.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_attribute.h"
#include "tz_stack_relation.h"
#include "tz_stack_draw.h"
#include "tz_stack_utils.h"
#include "tz_stack_neighborhood.h"
#include "tz_math.h"
#include "tz_color.h"

INIT_EXCEPTION

/* Stack_Draw_Object(): Draw an object in a stack.
 *
 * Args: stack - canvas stack;
 *       obj - 3d object;
 *       pa - voxel values.
 *
 * Return: void.
 */
void Stack_Draw_Object(Stack *stack, const Object_3d *obj, 
		       const Pixel_Array *pa)
{
  if (stack->kind != pa->kind) {
    THROW(ERROR_DATA_COMPTB);
  }

#ifdef INT_VOXEL_TYPE
  int i;
  int offset1 = 0;
  int offset2 = 0;
  int area = stack->width * stack->height;

  for (i = 0; i < obj->size; i++) {
    offset1 = area * obj->voxels[i][2] + stack->width * obj->voxels[i][1] +
      obj->voxels[i][0];
    memcpy(stack->array + offset1, BYTE_ARRAY(pa->array) + offset2, 
	   stack->kind);
    offset2 += stack->kind;
  }
#else
  THROW(ERROR_DATA_TYPE);
#endif
}

/* Stack_Draw_Object_Bwc(): Draw an objecct in a stack with a certain color.
 * 
 * Args: stack - canvas stack;
 *       obj -  object to draw;
 *       color - color.
 *
 * Return: void.
 */
void Stack_Draw_Object_Bwc(Stack *stack, const Object_3d *obj, Rgb_Color color)
{
#ifdef INT_VOXEL_TYPE
  int i;
  for (i = 0; i < obj->size; i++) {
    Set_Stack_Pixel(stack, obj->voxels[i][0], obj->voxels[i][1], 
		    obj->voxels[i][2], 0, color.r);
    Set_Stack_Pixel(stack, obj->voxels[i][0], obj->voxels[i][1], 
		    obj->voxels[i][2], 1, color.g);
    Set_Stack_Pixel(stack, obj->voxels[i][0], obj->voxels[i][1], 
		    obj->voxels[i][2], 2, color.b);
  }
#else
  THROW(ERROR_DATA_TYPE);
#endif  
}

/* Stack_Draw_Object_Bw(): Draw an object in a stack.
 *
 * Note: 
 */
void Stack_Draw_Object_Bw(Stack *stack, const Object_3d *obj, int value)
{
#ifdef INT_VOXEL_TYPE
  int i;
  for (i = 0; i < obj->size; i++) {
    Set_Stack_Pixel(stack, obj->voxels[i][0], obj->voxels[i][1], 
		    obj->voxels[i][2], 0, value);
  }
#else
  THROW(ERROR_DATA_TYPE);
#endif
}

/* Stack_Draw_Objects_Bw(): Draw a list of objects in a stack.
 *
 * Args: stack - stack to be drawed;
 *       obj - object list;
 *       value - drawing value. If it is negative, the function will try to draw
 *               different objects in different grey levels and -<value> is the
 *               maximum grey level.
 */
void Stack_Draw_Objects_Bw(Stack *stack, const Object_3d_List *obj, int value)
{
  if (value >= 0) {
    while (obj != NULL) {
      Stack_Draw_Object_Bw(stack, obj->data, value);
      obj = obj->next;
    }
  } else {
    int max_value = -value;
    value = 1;
    while (obj != NULL) {
      Stack_Draw_Object_Bw(stack, obj->data, value);
      obj = obj->next;
      value++;
      if (value > max_value) {
	value = 1;
      }
    }
  }
}

/* Stack_Draw_Object_Bwc(): Draw an objecct in a stack with a certain color.
 */
void Stack_Draw_Objects_Bwc(Stack *stack, const Object_3d_List *obj, 
			    Rgb_Color color)
{
  while (obj != NULL) {
    Stack_Draw_Object_Bwc(stack, obj->data, color);
    obj = obj->next;
  }
}

/* Object_To_Stack_Bw(): Turn an object to a stack.
 *
 * Input: obj - the input object;
 *        value - value of the object pixels;
 *        offset - offset of the object, which could be NULL if the caller does
 *                 not need the information returned.
 *
 * Return: a stack with GREY kind, which contains the input object.
 */
Stack* Object_To_Stack_Bw(const Object_3d *obj, uint8 value, int *offset)
{
  int corners[6];
  Object_3d_Range(obj, corners);

  if (offset != NULL) {
    int i;
    for (i = 0; i < 3; i++) {
      offset[i] = corners[i];
    }
  }

  Object_3d *tmpobj = Copy_Object_3d(obj);
  Object_3d_Translate(tmpobj, -corners[0], -corners[1], -corners[2]);

  int width, height, depth;
  width = corners[3] - corners[0] + 1;
  height = corners[4] - corners[1] + 1;
  depth = corners[5] - corners[2] + 1;

  Stack *stack = Make_Stack(GREY, width, height, depth);
  Zero_Stack(stack);
  Stack_Draw_Object_Bw(stack, tmpobj, value);

  Kill_Object_3d(tmpobj);

  return stack;
}

/* Object_To_Stack_Bwm(): Turn an object to a stack.
 *
 * Input: obj - the input object;
 *        value - value of the object pixels;
 *        margin - margin around the object;
 *        offset - offset of the object, which could be NULL if the caller does
 *                 not need the information returned.
 *
 * Return: a stack with GREY kind, which contains the input object.
 */
Stack* Object_To_Stack_Bwm(const Object_3d *obj, uint8 value,  
			   const int *margin, int *offset)
{
  int corners[6];
  Object_3d_Range(obj, corners);

  /* recalculate the corners for margin */
  if (margin != NULL) {
    int i;
    for (i = 0; i < 3; i++) {
      corners[i] -= margin[i];
    }
    for (i = 3; i < 6; i++) {
      corners[i] += margin[i - 3];
    }
  }

  if (offset != NULL) {
    int i;
    for (i = 0; i < 3; i++) {
      offset[i] = corners[i];
    }
  }

  Object_3d *tmpobj = Copy_Object_3d(obj);
  Object_3d_Translate(tmpobj, -corners[0], -corners[1], -corners[2]);

  int width, height, depth;
  width = corners[3] - corners[0] + 1;
  height = corners[4] - corners[1] + 1;
  depth = corners[5] - corners[2] + 1;

  Stack *stack = Make_Stack(GREY, width, height, depth);
  Zero_Stack(stack);
  Stack_Draw_Object_Bw(stack, tmpobj, value);

  Kill_Object_3d(tmpobj);

  return stack;
}


/* Stack_Label_Bwc(): Label a stack by a binary stack with a certain color.
 * 
 * Args: stack - canvas stack;
 *       label - object stack;
 *       color - color.
 *
 * Return: void.
 */
void Stack_Label_Bwc(Stack *stack, const Stack *label, Rgb_Color color)
{
  if (Stack_Channel_Number(stack) != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Same_Size(stack, label) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (Stack_Voxel_Bsize(label) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  size_t i;
  size_t offset = 0;
  size_t nvoxel = Stack_Voxel_Number(stack);

  for (i = 0; i < nvoxel; i++) {
    if (label->array[i] > 0) {
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }  
}

void Stack_Label_Level(Stack *stack, const Stack *label, uint8 level,
		       Rgb_Color color)
{
  if (Stack_Channel_Number(stack) != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Same_Size(stack, label) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (Stack_Voxel_Bsize(label) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  int i;
  int offset = 0;
  int nvoxel = Stack_Voxel_Number(stack);

  for (i = 0; i < nvoxel; i++) {
    if (label->array[i] == level) {
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }  
}

void Stack_Label_Color(Stack *stack, const Stack *label, double h, double s,
		       const Stack *v)
{
  if (Stack_Channel_Number(stack) != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Same_Size(stack, label) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (Stack_Voxel_Bsize(label) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Voxel_Bsize(v) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  int i;
  int offset = 0;
  int nvoxel = Stack_Voxel_Number(stack);
  Rgb_Color color;

  for (i = 0; i < nvoxel; i++) {
    if (label->array[i] > 0) {
      Set_Color_Hsv(&color, h, s, (double) v->array[i] / 255.0);
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }
}

void Stack_Label_Color_L(Stack *stack, const Stack *label, int level,
			 double h, double s, const Stack *v)
{
  if (Stack_Channel_Number(stack) != 3) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Same_Size(stack, label) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (Stack_Voxel_Bsize(label) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  if (Stack_Voxel_Bsize(v) != 1) {
    THROW(ERROR_DATA_TYPE);
  }

  ASSERT(label->kind == GREY, "Current version only supports GREY kind.");

  int i;
  int offset = 0;
  int nvoxel = Stack_Voxel_Number(stack);
  Rgb_Color color;

  for (i = 0; i < nvoxel; i++) {
    if (label->array[i] == level) {
      Set_Color_Hsv(&color, h, s, (double) v->array[i] / 255.0);
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }
}

#define STACK_REPLACE_POINT(array, type, value, min, max)		\
  if (is_finite == TRUE) {						\
    if (value < min) {							\
      value = min;							\
    } else if (value > max) {						\
      value = max;							\
    }									\
  }									\
  array[offset] = (type) value;

#define STACK_ADD_POINT(array, type, value, min, max)	\
  value += array[offset];				\
  STACK_REPLACE_POINT(array, type, value, min, max);

#define STACK_MAX_POINT(array, type, value, min, max)			\
  value = (value > array[offset]) ? value : array[offset];		\
  STACK_REPLACE_POINT(array, type, value, min, max);

#define STACK_SET_POINT(array, type, value, min, max, mode)		\
  if (offset < nvoxel) {						\
    switch (mode) {							\
    case 0:								\
      STACK_REPLACE_POINT(array, type, value, min, max);		\
      break;								\
    case 1:								\
      STACK_ADD_POINT(array, type, value, min, max);			\
      break;								\
    case 2:								\
      STACK_MAX_POINT(array, type, value, min, max);			\
      break;								\
    default:								\
      TZ_ERROR(ERROR_DATA_VALUE);					\
    }									\
  }

#define STACK_DRAW_POINT(stack_array, type, min, max, mode)		\
  /* If the point is out of range */					\
  if ((x < stack->width) && (x >= 0) && (y < stack->height) && (y >= 0)	\
      && (z < stack->depth) && (z >= 0)) {				\
    /* Get weights of surrounded voxels. */				\
    double x_low = floor(x);						\
    double wx_high = x - x_low;						\
    double wx_low = 1.0 - wx_high;					\
    double y_low = floor(y);						\
    double wy_high = y - y_low;						\
    double wy_low = 1.0 - wy_high;					\
    double z_low = floor(z);						\
    double wz_high = z - z_low;						\
    double wz_low = 1.0 - wz_high;					\
									\
    int area = stack->width * stack->height;				\
									\
    int offset =  area * ((int) z_low) + stack->width * ((int) y_low) + \
      ((int) x_low);							\
									\
    /* Calculate the weighted sum */					\
    if (wx_high > 0.0) {						\
      if (wy_high > 0.0) {						\
	if (wz_high > 0.0) {						\
	  value = v * wx_low * wy_low * wz_low;				\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset++;							\
	  value = v * wx_high * wy_low * wz_low;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += stack->width;					\
	  value = v * wx_high * wy_high * wz_low;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset--;							\
	  value = v * wx_low * wy_high * wz_low;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += area;						\
	  value = v * wx_low * wy_high * wz_high;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset++;							\
	  value = v * wx_high * wy_high * wz_high;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset -= stack->width;					\
	  value = v * wx_high * wy_low * wz_high;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset --;							\
	  value = v * wx_low * wy_low * wz_high;			\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	} else { /* wz_high == 0.0 */					\
	  value = v * wx_low * wy_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset++;							\
	  value = v * wx_high * wy_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += stack->width;					\
	  value = v * wx_high * wy_high;				\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset--;							\
	  value = v * wx_low * wy_high;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	}								\
      } else { /* wy_high == 0.0 */					\
	if (wz_high > 0.0) {						\
	  value = v * wx_low * wz_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset++;							\
	  value = v * wx_high * wz_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += area;						\
	  value = v * wx_high * wz_high;				\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset --;							\
	  value = v * wx_low * wz_high;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	} else { /* wz_high == 0.0 */					\
	  value = v * wx_low;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset++;							\
	  value = v * wx_high;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	}								\
      }									\
    } else { /* wx_high == 0.0 */					\
      if (wy_high > 0.0) {						\
	if (wz_high > 0.0) {						\
	  value = v * wy_low * wz_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += stack->width;					\
	  value = v * wy_high * wz_low;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += area;						\
	  value = v * wy_high * wz_high;				\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset -= stack->width;					\
	  value = v * wy_low * wz_high;					\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	} else { /* wz_high == 0.0 */					\
	  value = v * wy_low;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += stack->width;					\
	  value = v * wy_high;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	}								\
      } else { /* wy_high == 0.0 */					\
	if (wz_high > 0.0) {						\
	  value = v * wz_low;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
									\
	  offset += area;						\
	  value = v * wz_high;						\
	  STACK_SET_POINT(stack_array, type, value, min, max, mode);	\
	} else { /* wz_high == 0.0 */					\
	  STACK_SET_POINT(stack_array, type, v, min, max, mode);	\
	}								\
      }									\
    }									\
  }  

void Stack_Draw_Point(Stack *stack, double x, double y, double z, double v,
		      int mode)
{
  if (stack == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (Stack_Channel_Number(stack) != 1) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  int is_finite = TRUE;
  double value;

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  int nvoxel = Stack_Voxel_Number(stack);

  switch(stack->kind) {
  case GREY:
    STACK_DRAW_POINT(array_grey, uint8, 0, 255, mode);
    break;
  case GREY16:
    STACK_DRAW_POINT(array_grey16, uint16, 0, 65535, mode);
    break;
  case FLOAT32:
    is_finite = FALSE;
    STACK_DRAW_POINT(array_float32, float32, 0, 0, mode);
    break;
  case FLOAT64:
    is_finite = FALSE;
    STACK_DRAW_POINT(array_float64, float64, 0, 0, mode);
    break;
  default:
    PRINT_EXCEPTION("Unsupported image kind",
		    "Nothing is drawn.");
    break;
  }
}

/* Stack_Draw_Points(): Draw an array of points in a stack.
 * 
 * Args: stack - stack for drawing;
 *       pts - an array of 3d points;
 *       vs - an array of values;
 *       n - number of points;
 *       mode - drawing mode. 0 for replace and 1 for add.
 *
 * Return: void.
 */
void Stack_Draw_Points(Stack *stack, coordinate_3d_t *pts, double *vs, int n,
		       int mode)
{
  int i;
  /* If it is a replacement mode */
  if (mode == 0) {
    /* Erase the orginal values first */
    for (i = 0; i < n; i++) {
      Stack_Draw_Point(stack, pts[i][0], pts[i][1], pts[i][2], 0.0, 0);
    }
  }

  for (i = 0; i < n; i++) {
    Stack_Draw_Point(stack, pts[i][0], pts[i][1], pts[i][2], vs[i], 1);
  }
}

void Stack_Draw_Voxel_C(Stack *stack, int x, int y, int z, int conn, 
			uint8 r, uint8 g, uint8 b)
{
  ASSERT(stack->kind == COLOR, "color stack only");

  color_t *array = (color_t *) stack->array;

  int index = Stack_Util_Offset(x, y, z, stack->width, stack->height, 
				stack->depth);
  array[index][0] = r;
  array[index][1] = g;
  array[index][2] = b;
  
  if (conn > 0) {
    int neighbor[26];
    int is_in_bound[26];
    Stack_Neighbor_Offset(conn, stack->width, stack->height, neighbor);
    int nbound = Stack_Neighbor_Bound_Test(conn, stack->width - 1, 
					   stack->height - 1,
					   stack->depth - 1,
					   x, y, z, is_in_bound);
    if (nbound == conn) {
      int i;
      for (i = 0; i < conn; i++) {
	array[index + neighbor[i]][0] = r;
	array[index + neighbor[i]][1] = g;
	array[index + neighbor[i]][2] = b;
      }
    } else {
      int i;
      for (i = 0; i < conn; i++) {
	if (is_in_bound[i]) {
	  array[index + neighbor[i]][0] = r;
	  array[index + neighbor[i]][1] = g;
	  array[index + neighbor[i]][2] = b;
	}
      }
    }
  }
}

void Stack_Draw_Voxel_Mc(Stack *stack, Stack *mask, int conn, 
			 uint8 r, uint8 g, uint8 b)
{
  ASSERT(mask->kind == GREY, "GREY mask only");

  int x, y, z;
  size_t offset = 0;
  for (z = 0; z < stack->depth; z++) {
    for (y = 0; y < stack->height; y++) {
      for (x = 0; x < stack->width; x++) {
	if (mask->array[offset] == 1) {
	  Stack_Draw_Voxel_C(stack, x, y, z, conn, r, g, b);
	}
	offset++;
      }
    }
  }
}

void Stack_Blend_Mc(Stack *stack, const Stack *mask, double h)
{
  ASSERT(mask->kind == GREY, "GREY mask only");
  ASSERT(stack->kind == COLOR, "color canvas only");
  ASSERT(Stack_Same_Size(stack, mask), "Unmatched stack size");

  size_t i;
  size_t offset = 0;
  size_t nvoxel = Stack_Voxel_Number(stack);
  Rgb_Color color;

  for (i = 0; i < nvoxel; i++) {
    if (mask->array[i] == 1) {
      double v = 0.5 + (double) stack->array[offset] / 510.0;
      if (v > 1.0) {
	v = 1.0;
      }
      Set_Color_Hsv(&color, h, v, v);
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;  
    }
    offset += 3;
  }  
}

void Stack_Blend_Mc2(Stack *stack, const Stack *mask1, double h1, 
		     const Stack *mask2, double h2)
{
  ASSERT(mask1->kind == GREY, "GREY mask only");
  ASSERT(stack->kind == COLOR, "color canvas only");

  if (mask2 == NULL) {
    Stack_Blend_Mc(stack, mask1, h1);
    return;
  }

  ASSERT(mask2->kind == GREY, "GREY mask only");

  size_t i;
  size_t offset = 0;
  size_t nvoxel = Stack_Voxel_Number(stack);
  Rgb_Color color;
  Rgb_Color color2;

  int r, g, b;

  for (i = 0; i < nvoxel; i++) {
    uint8 code = (mask1->array[i] << 1) + mask2->array[i];
    double v = 0.0;
    if (code > 0) {
      v = 0.5 + (double) stack->array[offset] / 510.0;
      if (v > 1.0) {
	v = 1.0;
      }

      switch (code) {
      case 1:
	Set_Color_Hsv(&color, h2, v, v);
	break;
      case 2:
	Set_Color_Hsv(&color, h1, v, v);	
	break;
      case 3:
	Set_Color_Hsv(&color, h1, v, v);
	Set_Color_Hsv(&color2, h2, v, v);

	r = color.r;
	r += color2.r;
	color.r = imin2(255, r);
	g = color.g;
	g += color2.g;
	color.g = imin2(255, g);
	b = color.b;
	b += color2.b;
	color.b = imin2(255, b);
	break;
      }
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }  
}

void Stack_Blend_Mc_L(Stack *stack, Stack *mask, int level, double h)
{
  ASSERT(mask->kind == GREY, "GREY mask only");
  ASSERT(stack->kind == COLOR, "color canvas only");

  size_t i;
  size_t offset = 0;
  size_t nvoxel = Stack_Voxel_Number(stack);
  Rgb_Color color;

  for (i = 0; i < nvoxel; i++) {
    if (mask->array[i] == level) {
      double s = 0.5 +  (double) stack->array[offset] / 510.0;
      if (s > 1.0) {
	s = 1.0;
      }
      Set_Color_Hsv(&color, h, s, s);
      stack->array[offset] = color.r;
      stack->array[offset + 1] = color.g;
      stack->array[offset + 2] = color.b;
    }
    offset += 3;
  }  
}

void Stack_Draw_Object_C(Stack *stack, Object_3d *obj, uint8 r, uint8 g,
			 uint8 b)
{
  ASSERT(stack->kind == COLOR, "color canvas only");
  
  int offset;

  int i;
  int area = stack->width * stack->height;
  color_t *array = (color_t *) stack->array;

  for (i = 0; i < obj->size; i++) {
    if (IS_IN_OPEN_RANGE(obj->voxels[i][0], -1, Stack_Width(stack)) &&
	IS_IN_OPEN_RANGE(obj->voxels[i][1], -1, Stack_Height(stack)) &&
	IS_IN_OPEN_RANGE(obj->voxels[i][0], -1, Stack_Width(stack))) {
      offset = STACK_UTIL_OFFSET(obj->voxels[i][0], obj->voxels[i][1],
	  obj->voxels[i][2], stack->width, area);
      array[offset][0] = r;
      array[offset][1] = g;
      array[offset][2] = b;
    }
  }
}

Stack_Draw_Workspace *New_Stack_Draw_Workspace()
{
  Stack_Draw_Workspace *ws = (Stack_Draw_Workspace*) 
    Guarded_Malloc(sizeof(Stack_Draw_Workspace), "New_Stack_Draw_Workspace");

  Default_Stack_Draw_Workspace(ws);

  return ws;
}

void Default_Stack_Draw_Workspace(Stack_Draw_Workspace *ws)
{
  ws->color.r = 255;
  ws->color.g = 255;
  ws->color.b = 255;
  ws->h = 1.0;
  ws->s = 1.0;
  ws->v = 1.0;
  ws->blend_mode = 0;
  ws->color_mode = 0;
  ws->z_scale = 1.0;
}

void Clean_Stack_Draw_Workspace(Stack_Draw_Workspace *ws)
{
  /* reserved for future revision */
}

void Delete_Stack_Draw_Workspace(Stack_Draw_Workspace *ws)
{
  free(ws);
}

void Kill_Stack_Draw_Workspace(Stack_Draw_Workspace *ws)
{
  Clean_Stack_Draw_Workspace(ws);
  free(ws);
}

Stack* Stack_Blend_Label_Field(const Stack *stack, const Stack *label, 
    double alpha, const uint8_t *color_map, int color_number, Stack *out)
{
  TZ_ASSERT(stack->kind == GREY, "Unsupported kind");
  TZ_ASSERT(label->kind == GREY || label->kind == GREY16, "Unsupported kind");
  TZ_ASSERT(Stack_Same_Size(stack, label), "Unmatched size");

  if (alpha < 0.0) {
    TZ_WARN(ERROR_DATA_VALUE);
    alpha = 0.0;
  }
  if (alpha > 1.0) {
    TZ_WARN(ERROR_DATA_VALUE);
    alpha = 1.0;
  }

  if (out == NULL) {
    out = Make_Stack(COLOR, stack->width, stack->height, stack->depth);
  }

  TZ_ASSERT(out->kind == COLOR, "Unsupported kind");

  color_t *out_array = (color_t*) out->array;

  if (color_map == NULL) {
    color_map = Jet_Colormap;
    color_number = Jet_Color_Number;
  }

  size_t voxel_number = Stack_Voxel_Number(stack);
  size_t offset;
  Image_Array ima;
  ima.array = label->array;
  for (offset = 0; offset < voxel_number; ++offset) {
    int label_value;
    if (label->kind == GREY) {
      label_value = ima.array8[offset];
    } else {
      label_value = ima.array16[offset];
    }
    
    int gray_value = stack->array[offset];
    if (label_value > 0) {
      label_value = (label_value - 1) % color_number;
      int k;
      for (k = 0; k < 3; ++k) {
        int value = iround((1.0 - alpha) * gray_value +
            alpha * color_map[label_value * 3 + k]);
        if (value < 0) {
          value = 0;
        } else if (value > 255) {
          value = 255;
        }
        out_array[offset][k] = value;
      }
    } else {
      int k;
      for (k = 0; k < 3; ++k) {
        out_array[offset][k] = gray_value;
      }
    }
#ifdef _DEBUG_2
      if (out_array[offset][0] == 0 && out_array[offset][1] == 0 &&
          out_array[offset][2] == 0) {
        printf("debug here\n");
      }
#endif
  }

  return out;
}

