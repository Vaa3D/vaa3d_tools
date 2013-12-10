/**@file tz_stack_draw.h
 * @brief draw stack
 * @author Ting Zhao
 * @date 30-Nov-2007
 */

#ifndef _TZ_STACK_DRAW_H_
#define _TZ_STACK_DRAW_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_object_3d.h"
#include "tz_pixel_array.h"
#include "tz_object_3d_linked_list.h"
#include "tz_coordinate_3d.h"
#include "tz_color.h"

__BEGIN_DECLS

/**@addtogroup stack_draw_ Stack drawing routines
 * @{
 */

/* 
 * Stack_Draw_Object() draws a 3D object in a stack with the corresponding
 * voxel values. 
 * Stack_Draw_Object_Bw() draws a 3D object in a stack with a constant value. 
 * Stack_Draw_Object_Bwc() draws a 3D object in a stack with a constant color. 
 * Stack_Draw_Objects_Bw() and Stack_Draw_Objects_Bwc() draw a list of objects.
 */
void Stack_Draw_Object(Stack *stack, const Object_3d *obj, 
		       const Pixel_Array *pa);
void Stack_Draw_Object_Bw(Stack *stack, const Object_3d *obj, int value);
void Stack_Draw_Object_Bwc(Stack *stack, const Object_3d *obj, Rgb_Color color);
void Stack_Draw_Objects_Bw(Stack *stack, const Object_3d_List *obj, int value);
void Stack_Draw_Objects_Bwc(Stack *stack, const Object_3d_List *obj, 
			    Rgb_Color color);

/* Object_To_Stack_Bw() and Object_To_Stack_Bwm() turns an object to a stack, 
 * in which the background is set to 0 and and foreground (object) to <value>. 
 * The position of the object in the stack is not its original position. The 
 * offset to the original position is stored in <offset>, which is an array of 
 * x, y, z coordinates. So the object in the new stack moves <offset> to get to 
 * its original position. If you do not need <offset>, just pass a NULL pointer.
 *
 * Object_To_Stack_Bwm() has one more argument, <margin>, which is the margin
 * around the object in the stack. It is also an array of 3 integers, which are
 * margins of width, height and depth. The values could be negative, which 
 * means the object will be cropped in the stack. If <margin> is NULL, it is 
 * the same as Object_To_Stack_Bw().
 */
Stack* Object_To_Stack_Bw(const Object_3d *obj, uint8 value, int *offset);
Stack* Object_To_Stack_Bwm(const Object_3d *obj, uint8 value,  
			   const int *margin, int *offset);

/* Stack_Label_Bwc() labels <stack> with another stack <label> in the color 
 * <color>. <stack> must have three channels and <label> must be GREY kind.
 * They should have the same size. All voxels in <label> are considered as
 * foreground if they have intensities above 0, otherwise they are treated as
 * background.
 *
 * Stack_Label_Level() labels <stack> with <color> on the voxels that have
 * <level> in the stack <label>.
 *
 * Stack_Label_Color() labels a stack by a color with hue <h> and saturate <s>.
 * Then intensity of the color is determined by the intensity of the pixel.
 * <label> is the mask of regions to label. Here h has range [0, 6], s and v has
 * range[0, 1]. <v> is a stack to specify the values and it could be the same
 * as <label>.
 *
 * Stack_Label_Color_L() is the combination of Stack_Label_Color() and
 * Stack_Label_Level().
 */
void Stack_Label_Bwc(Stack *stack, const Stack *label, Rgb_Color color);
void Stack_Label_Level(Stack *stack, const Stack *label, uint8 level,
		       Rgb_Color color);
void Stack_Label_Color(Stack *stack, const Stack *label, double h, double s,
		       const Stack *v);
void Stack_Label_Color_L(Stack *stack, const Stack *label, int level,
			 double h, double s, const Stack *v);

/**@brief Alpha blending between a stack and a lable field
 *
 * <stack> and <label> must be GREY type. <alpha> must be within [0, 1].
 * If the maximum value of <label> is greater than or equal to color_number,
 * the modulus would be used. An internal color map is used when color_map is 
 * NULL. Label 0 will be ignored. 
 */
Stack* Stack_Blend_Label_Field(const Stack *stack, const Stack *label, 
    double alpha, const uint8_t *color_map, int color_number, Stack *out);

/**@brief Draw a point in a stack.
 *
 * Stack_Draw_Point() draws the point (<x>, <y>, <z>) in <stack>. The value of
 * the point is <v>. <mode> specifies how the value is added: 
 *   0 - replace, 1 - add, 2 - maximum.
 */
void Stack_Draw_Point(Stack *stack, double x, double y, double z, double v,
		      int mode);

/**@brief Draw a set of points in a stack.
 *
 * Stack_Draw_Points() draws an array of points <pts> in a stack. Each element
 * of <vs> correpsonds to the value of a point in <pts>. Only mode 0 
 * and 1 are supported (see Stack_Draw_Point). <n> is the number of points.
 */
void Stack_Draw_Points(Stack *stack, coordinate_3d_t *pts, double *vs, int n,
		       int mode);

/**@brief Draw a voxel.
 *
 * Stack_Draw_Voxel_C() paints a voxel in <color>. The neighbors will also
 * be painted when <conn> is a valid connectivity value 
 * (see tz_stack_neighborhood.h). None of its neibhbor will be painted when
 * <conn> is zero.
 *
 * Stack_Draw_Voxel_Mc() specifies the voxels to paint in a binary stack <mask>.
 */
void Stack_Draw_Voxel_C(Stack *stack, int x, int y, int z, int conn, 
			uint8 r, uint8 g, uint8 b);

/**@brief Draw a mask.
 *
 */
void Stack_Draw_Voxel_Mc(Stack *stack, Stack *mask, int conn, 
			 uint8 r, uint8 g, uint8 b);

void Stack_Blend_Mc(Stack *stack, const Stack *mask, double h);
void Stack_Blend_Mc2(Stack *stack, const Stack *mask1, double h1, 
		     const Stack *mask2, double h2);

void Stack_Blend_Mc_L(Stack *stack, Stack *mask, int level, double h);

/*
 */
void Stack_Draw_Object_C(Stack *stack, Object_3d *obj, uint8 r, uint8 g,
			 uint8 b);

/**@}*/


typedef struct _Stack_Draw_Workspace {
  Rgb_Color color;
  double h;
  double s;
  double v;
  int blend_mode;
  int color_mode;
  double z_scale;
} Stack_Draw_Workspace;

Stack_Draw_Workspace *New_Stack_Draw_Workspace();
void Delete_Stack_Draw_Workspace(Stack_Draw_Workspace *ws);
void Kill_Stack_Draw_Workspace(Stack_Draw_Workspace *ws);

void Default_Stack_Draw_Workspace(Stack_Draw_Workspace *ws);
void Clean_Stack_Draw_Workspace(Stack_Draw_Workspace *ws);


__END_DECLS

#endif 
