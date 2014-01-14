/**@file tz_stack_bwmorph.h
 * @brief morphologial operation for binary stack
 * @author Ting Zhao
 * @date05-Nov-2007
 */

#ifndef _TZ_STACK_BWMORPH_H_
#define _TZ_STACK_BWMORPH_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_imatrix.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

typedef int Coordinate[3];

typedef struct tagStruct_Element {
  int size;
  Coordinate *offset;
  uint8 *mask;
}  Struct_Element;

enum {
  STACK_CORNER_1 = 10, STACK_CORNER_2, STACK_CORNER_3, STACK_CORNER_4,
  STACK_CORNER_5, STACK_CORNER_6, STACK_CORNER_7, STACK_CORNER_8,
  
  STACK_LINE_1, STACK_LINE_2, STACK_LINE_3, STACK_LINE_4, STACK_LINE_5,
  STACK_LINE_6, STACK_LINE_7, STACK_LINE_8, STACK_LINE_9, STACK_LINE_10,
  STACK_LINE_11, STACK_LINE_12,

  STACK_FACE_1, STACK_FACE_2, STACK_FACE_3, STACK_FACE_4, STACK_FACE_5,
  STACK_FACE_6
};

/**@addtogroup bwmorph_ Morphological operation (tz_stack_bwmorph.h)
 * @{
 */

void Print_Se(Struct_Element *se);

Struct_Element *New_Se(int size);
void Free_Se(Struct_Element *se);
void Kill_Struct_Element(Struct_Element *se);

Stack *Se_To_Stack(Struct_Element *se);

/**************** Make structrural element ****************
 * Stack_To_Se() turns a stack to a structural element.
 *
 * Make_Corner_Se_2D() returns an oriented 2D corner structural element.
 * Make_Ball_Se() returns a ball-shape structural element with radius <r>.
 * Make_Disc_Se() returns a disc-shape structural element with radius <r>.
 * Make_Cuboid_Se() returns a cuboid-shape structural element with size
 * <width> x <height> x <depth>.
 * Make_Rect_Se() returns a rectangle-shape structural element with size
 * <width> x <height>.
 */
Struct_Element *Stack_To_Se(const Stack *stack, Coordinate center, int set);
Struct_Element *Make_Corner_Se_2D(int orient);
Struct_Element *Make_Ball_Se(int r);
Struct_Element *Make_Disc_Se(int r);
Struct_Element *Make_Ring_Se(int ri, int ro);
Struct_Element *Make_Cuboid_Se(int width, int height, int depth);
Struct_Element *Make_Rect_Se(int width, int height);
Struct_Element *Make_Cross_Se(BOOL is_2d);
Struct_Element *Make_Zline_Se(int length);

/***************** Erode and Dilate *************************************
 * Stack_Erode() and Stack_Dilate() perform erosion and dilation of the input
 * stack <in> and store the result in <out>. If <out> is NULL, a new stack will
 * be created. The returned value also refers to the result. <se> is the 
 * structural element, which must have all 1s in its mask and contain (0,0)
 * offset. No aliasing is allowed between the arrays of <in> and <out>.
 *
 * The caller is responsible for freeing the returned pointer.
 *
 * Stack_Erode_Fast() and Stack_Dilate_Fast() are the fast version of erosion
 * and dilation. But they may take more memory.
 ***********************************************************************/
Stack* Stack_Erode(const Stack *in, Stack *out, const Struct_Element *se);
Stack* Stack_Dilate(const Stack *in, Stack *out, const Struct_Element *se);
Stack* Stack_Dilate_Rm(const Stack *in, Stack *out, const Struct_Element *se,
		       const Stack *region_mask, int region);

Stack* Stack_Erode_Fast(const Stack *in, Stack *out, const Struct_Element *se);
Stack* Stack_Dilate_Fast(const Stack *in, Stack *out, const Struct_Element *se);

/*
 * Stack_Hitmiss() does Hit-miss operation for the stack <in> with the structure
 * element <se>. The result is stored in <out> and returned as <out> if it is 
 * not NULL. Otherwise it returns a new stack pointer and the caller is 
 * responsible for freeing the new pointer.
 */
int Stack_Hitmiss(const Stack *in, Stack *out, const Struct_Element *se);

/***************** skeletonnization (under developing) ******************/
Stack* Stack_Skeletonize(Stack *in, Stack *out, int iscon1);

/***************** Hole Filling ******************************/
Stack* Stack_Fillhole(Stack *in, Stack *out, int value);
Stack* Stack_Fill_2dhole(Stack *in, Stack *out, int value, int iscon1);
Stack* Stack_Fill_Hole_N(Stack *in, Stack *out, int value, int conn, 
			 IMatrix *chord);

/***************** Boundary Extraction *******************************
 * Note: The functions are obseleted. Stack_Perimeter is preferred.
 *
 * Stack_Boundary() extract boundary pixels in a stack. A boundary pixel
 * is defined as a pixel in foreground but connected with at least one
 * background pixel. <out> is used to store the result and it must not be
 * aliased with <in>. If <out> is NULL, a new stack will be created.
 *
 * Stack_Boundary_N() does the same thing but has an additional parameter 
 * <conn> to specify the neiborhood system. If it is 4 or 8, the boundary is
 * defined in the 2D space.
 *
 * The caller is responsible for freeing the returned pointer.
 *********************************************************************/
Stack* Stack_Boundary(const Stack *in, Stack *out);
Stack* Stack_Boundary_N(const Stack *in, Stack *out, int conn);

/*
 * Stack_Majority_Filter() filters a binary stack <in> by a majority filter. So
 * every white voxel of <in> will be set to 0 when and only when no more than 
 * half of its neighbors are white. <conn> specifies the neighborhood system.
 * <out> should not refer to the same stack as <in> does. It can be NULL and
 * a new stack will be returned from the function.
 *
 * Stack_Majority_Filter_R() has an additional parameter <mnbr> to specify
 * how many neighbors for meeting the 'majority' condition, i.e. every white
 * voxel of <in> will be set to 0 when and only when fewer than <mnbr> its
 * neighbors are white. It returns NULL wehn <mnbr> is greater than <conn>.
 */
Stack* Stack_Majority_Filter(const Stack *in, Stack *out, int conn);
Stack* Stack_Majority_Filter_R(const Stack *in, Stack *out, int conn, int mnbr);

/*
 * Stack_Perimeter() extracts the perimeter of the stack <in>. The result is
 * stored in <out>, which is returned as well, if <out> is not NULL; otherwise
 * it returns the result as a new stack.
 */
Stack* Stack_Perimeter(const Stack *in, Stack *out, int conn);

/*
 * Stack_Remove_Small_Object() remove objects that have sizes smaller than 
 * <size> in the stack <in>. The result is stored in <out> if <out> is not
 * NULL. <out> can be the same pointer as <in> for in-place operation.
 */
Stack* Stack_Remove_Small_Object(Stack *in, Stack *out, 
				 int size, int conn);

/**@addtogroup dist_map distance transformation
 */
/**@brief distance transformation
 * 
 * Stack_Bwdist_L() does distance transformation for foreground voxels of \a in
 * and stores the result in \a out. \a label is used to store the nearest voxel
 * indices. Set it to NULL if you do not need such information. <out> must be
 * FLOAT32 kind.
 */
Stack* Stack_Bwdist_L(Stack *in, Stack *out, long int *label);

/**@brief economic distance transformation
 *
 * Stack_Bwdist_L_U16() is an economic version of distance transformation. It
 * takes much less memory than Stack_Bwdist_L() does and can be much faster
 * too. The drawback is that the maximum distance is 254 because of the limit 
 * of bit number. The result \a out is a GREY16 stack and each voxel is the 
 * square of its shortest distance to the background.  The value of a point 
 * will be 254 * 254 when its actual distance is greater than 254. The 
 * distance is 0 if the voxel itself blongs to background.  If \a out is NULL, 
 * the result is stored in the returned stack. \a pad is the value of 
 * out-of-range field, which can be 0 or 1.
 * 
 * example:
 *   Stack *stack = Read_Stack("../data/fly_neuron/mask.tif");
 *   Stack *out = Stack_Bwdist_L_U16(stack, NULL, 0);
 *
 * Note: The result is the square of distance.
 */
Stack* Stack_Bwdist_L_U16(Stack *in, Stack *out, int pad);

/**@brief Plane-by-plane distance transform
 *
 * Stack_Bwdist_L_U16P() is similiar to Stack_Bwdist_L_U16. However, it does
 * 2D distance transform for each slice of <in> instead of performing the
 * 3D distance transform.
 */
Stack *Stack_Bwdist_L_U16P(Stack *in, Stack *out, int pad);

/**@}*/

typedef struct _Stack_Seed_Workspace {
  int method;
  BOOL with_boundary;
  Stack *seed_mask;
  Stack *seed_dist;
  Stack *weight;
} Stack_Seed_Workspace;

Stack_Seed_Workspace* New_Stack_Seed_Workspace();
void Clean_Stack_Seed_Workspace(Stack_Seed_Workspace *ws);
void Kill_Stack_Seed_Workspace(Stack_Seed_Workspace *ws);

Geo3d_Scalar_Field* Stack_Seed(Stack *mask, Stack_Seed_Workspace *ws);

/**@Brief Calucate volume-area ratio
 *
 * Stack_Volume_Surface_Ratio() returns the ratio between the volume and surface
 * area of the foreground of \a stack. If \a fill_hole is one of the 
 * neighborhood connection values, the holes of the foreground will be filled
 * based on the connection before calculation. \a stack will be modified in an
 * undefined way if fill_hole is not 0.
 */
double Stack_Volume_Surface_Ratio(Stack *stack, int fill_hole);

/**@}*/

Stack* Stack_Z_Dilate(const Stack *stack, int size, const Stack *signal, 
		      Stack *out);

Stack* Stack_Region_Expand(const Stack *stack, int conn, int r, Stack *out);
Stack* Stack_Region_Expand_M(const Stack *stack, int conn, int r, Stack *out,
    Stack *mask);

typedef struct _Stack_Region_Expand_Workspace {
  int conn;
  int r;
  IMatrix *chord;
} Stack_Region_Expand_Workspace;

/**@brief Remove small isolated objects.
 *
 */
Stack* Stack_Remove_Isolated_Object(const Stack *stack, Stack *out, int r,
    int dr);

size_t Stack_Foreground_Size(const Stack *stack);

/**@brief Interpolate a binary stack.
 */
Stack* Stack_Bwinterp(const Stack *stack, Stack *out);

/**@brief Shrink a binary stack.
 */
Stack* Stack_Bwshrink(const Stack *stack, Stack *out);

/**@brief Shrink 3x3x3 stack.
 */
Stack* Stack_Bwshrink_Z3(const Stack *stack, Stack *out);

BOOL Stack_Has_Hole_Z3(const Stack *stack, Stack *out);

/* Thinning */
typedef enum { NORMAL_THINNING, REMOVE_ARC } Stack_Bwpeel_Option_t;

Stack *Stack_Bwpeel(const Stack *stack, Stack_Bwpeel_Option_t option, 
    Stack *out);
Stack *Stack_Bwthin(const Stack *stack, Stack *out);

/* for unit test */
int Stack_Bwthin_Count_Simple_Point();

__END_DECLS

#endif
