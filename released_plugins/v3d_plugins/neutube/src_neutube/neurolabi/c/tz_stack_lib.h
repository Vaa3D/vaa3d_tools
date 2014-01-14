/**@file tz_stack_lib.h
 * @brief stack routines
 * @author Ting Zhao
 * @date 14-Jan-2008
 */

#ifndef _TZ_STACK_LIB_H_
#define _TZ_STACK_LIB_H_

#include "tz_cdefs.h"
#include "tz_imatrix.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_attribute.h"
#include "tz_cuboid_i.h"
#include "tz_file_list.h"

__BEGIN_DECLS

void Set_Chkcon(int iscon4);
int Get_Chkcon();
void Set_Connection(int iscon1, int con_2d);
int Get_Con2d();

/**@addtogroup unctg_stack_lib_ Uncategorized stack rountines (tz_stack_lib.h)
 * @{
 */

/**@addtogroup init_stack_ stack initialization
 * @{
 */

/**@brief set a stack to zero.
 *
 * Zero_Stack() sets the values of all voxels in <stack> to zero. One_Stack()
 * sets the values of all voxels in <stack> to one. Stack_Set_Constant() sets
 * the values of all voxels in <stack> to a constant stored in \a value, which 
 * is interpreted based on the kind of \a stack. 
 */
void Zero_Stack(Stack *stack);
void One_Stack(Stack *stack);
void Stack_Set_Constant(Stack  *stack, const void *value);

/**@} */

/**@brief copy stack values.
 *
 * Copy_Stack_Array() copies the values of the array of <src> to <dst>. <dst> 
 * and <src> should have the same attributes. 
 */
void Copy_Stack_Array(Stack *dst, const Stack *src);

/**@brief copy a stack with NULL text.
 *
 * Copy_Stack_T() is similar to Copy_Stack(). But it allows the text of <stack> to
 * be NULL. The text field was added later to the structure and has caused
 * compatability problems because it does not support NULL text.
 */
Stack* Copy_Stack_T(Stack *stack);

/**@brief Convert a stack into a binary matrix.
 *
 * Get_White_Matrix3() returns a binary integer matrix from the stack <stack>.
 * <stack> must be GREY or GREY16 kind. The functions maps non-zero pixels
 * to 1 and other pixels to 0. 
 */
IMatrix* Get_White_Matrix3(Stack *stack);

/**@brief Count foreground size in blocks.
 */
IMatrix* Count_Stack_Fg(Stack *stack, dim_type bdim[]);

IMatrix* Stack_Foreoverlap(Stack *stack1,Stack *stack2);
IMatrix* Stack_Foreunion(Stack *stack1,Stack *stack2);
IMatrix* Stack_Foreunion_cthr(Stack *stack1,Stack *stack2,int minol);

/**@brief Crop a stack.
 *
 * Crop_Stack() crops a stack by a cuboid with the size 
 * \a width x \a height x \a depth.
 * The left-top-front corner of the rectangle is specified by \a left and 
 * \a top and \a front. 0s are padded when the cuboid is out of range. The 
 * returned pointer is a new object if <desstack> is NULL, otherwise the 
 * function returns <desstack>, where the result is stored.
 */
Stack* Crop_Stack(const Stack* stack,int left,int top,int front,
		  int width,int height,int depth, Stack *desstack);

Stack* Reflect_Stack(Stack* stack,int in_place);

/**@brief Flip a stack on Z axis
 *
 * Flip_Stack_Z() flips the planes of \a stack, i.e. the ith plane couting from
 * the first plane becomes the ith plane counting from the last plane. It
 * supports in-place operation and NULL \a out.
 */
Stack* Flip_Stack_Z(Stack *stack, Stack *out);

/**@brief Flip a stack on Y axis
 *
 * Flip_Stack_Y() flips each plane of \a stack at the vertical direction. It
 * supports in-place operation and NULL \a out.
 */
Stack* Flip_Stack_Y(const Stack *stack, Stack *out);

/*
 * Print_Stack_Value() prints value of the stack to the screen.
 *
 * Note: Print_Stack_Value() is obsoleted by Print_Stack().
 */
void Print_Stack_Value(Stack* stack);

double Stack_Array_Value(const Stack *stack, size_t index);

Stack* Scale_Float_Stack(float *fStack, int  width, int height, 
			 int depth, int kind);
Stack* Scale_Double_Stack(double *fStack, int  width, int height, 
			  int depth, int kind);
Stack* Scale_Double_Stack_P(double *fStack, int  width, int height, 
			    int depth, int kind);

void Stretch_Stack_Value(Stack *stack);
void Stretch_Stack_Value_Q(Stack *stack, double q);
void Stack_Brighten_Bw(Stack *stack);
void Stack_Brighten_Level(Stack *stack, int level);

/**@brief Get a binary stack for a certain grey level.
 *
 * Stack_Level_Mask() turns <stack> into a binary stack by setting its voxels
 * with intersity level to 1 and other voxels to 0. It returns the number of 
 * voxles that are labeled as 1.
 */
size_t Stack_Level_Mask(Stack *stack, int level);

size_t Stack_Level_Mask2(const Stack *stack, Stack *out, int level);

Stack* Resize_Stack(const Stack* stack,int width,int height,int depth);
Stack* Downsample_Stack(const Stack* stack,int wintv,int hintv,int dintv);
Stack* Downsample_Stack_Mean(Stack *stack,int wintv,int hintv,int dintv,
			     Stack *stack2);
Stack* Downsample_Stack_Mean_F(Stack *stack,int wintv,int hintv,int dintv,
    Stack *stack2);
void Downsample_Stack_Max_Size(int width, int height, int depth, 
    int wintv,int hintv,int dintv, int *wd, int *hd, int *dd);
Stack* Downsample_Stack_Max(Stack* stack,int wintv,int hintv,int dintv,
    Stack *out);

Stack* Upsample_Stack(const Stack *stack, int wintv, int hintv, int dintv,
		      Stack *out);

/*
 * Resample_Stack_Depth() resamples a stack along Z-axis with the interval
 * <dintv>. The result is stored in <dst> if it is not NULL and the return
 * pointer is the same as <dst> too. If <dst> is NULL, the result is returned
 * as a new stack object.
 */
Stack* Resample_Stack_Depth(const Stack* src, Stack *dst, double dintv);

void Stack_Overlap(const int dim[],const int bdim[],const int offset[],
		   int start1[],int end1[],int start2[],int end2[]);
int Hist_Tpthre2(int *hist, int low, int high);
int* Stack_Hist(const Stack *stack);
int* Stack_Hist_M(const Stack *stack, Stack *mask);

/* Obsolete. Use Stack_Voxel_Number() instead. */
size_t Get_Stack_Size(const Stack *stack);

void Stack_Binarize(Stack *stack);
void Stack_Binarize_Level(Stack *stack, double level);

void Stack_Sub_Common(Stack* stack,int low,int high);

int Hist_Rcthre(int *hist,int low,int high);

double Stack_Corrcoef(Stack *stack1,Stack *stack2);

Stack* Stack_Locmax(Stack *stack, Stack *stack_out);

/*
 * Options: STACK_LOCMAX_CENTER STACK_LOCMAX_NEIGHBOR STACK_LOCMAX_NONFLAT
 *          STACK_LOCMAX_ALTER1 STACK_LOCMAX_ALTER2 STACK_LOCMAX_SINGLE
 */
Stack* Stack_Local_Max(const Stack *stack, Stack *stack_out, int option);

int Stack_Label_Object(Stack *stack, IMatrix *chord, int seed, int flag,
		       int label);
int Stack_Label_Object_Constraint(Stack *stack, IMatrix *chord, int seed, 
				  int flag, int label, Stack *code, int max);
/*
 * Proj_Stack_Zmax() 
 */
Image* Proj_Stack_Zmax(const Stack *stack);
Image* Proj_Stack_Zmin(const Stack *stack);
Image* Proj_Stack_Xmax(const Stack *stack);
Image* Proj_Stack_Ymax(const Stack *stack);

double Stack_Hittest_Z(const Stack *stack, int x, int y);

/*
 * Stack_Channel_Extraction() turns a channel of <stack> into a separated
 * stack. <channel> is the index of the channel to extract. The result is
 * stored in <out> and the returned pointer is <out> too if <out> is not NULL.
 * Otherwise it returns a new pointer.
 */
Stack* Stack_Channel_Extraction(const Stack *stack, int channel, Stack *out);

/*
 * Empty_Stack() sets an stack to empty. To use it safely, <stack> should be
 * created without initialization or Make_Stack() assignment.
 */
void Empty_Stack(Stack *stack);

/**@brief Obsolete function (Use \ref Stack_Is_Empty instead).
 */
BOOL Is_Stack_Empty(const Stack *stack);

/*
 */
void Stack_Clean_Locmax(Stack *stack, Stack *locmax);

/**@brief invert a stack intensity
 *
 * Stack_Invert_Value() inverts the intensity of each pixel \a stack. After the
 * operation, min is mapped to max and max is mapped to min.
 */
void Stack_Invert_Value(Stack *stack);
/**@}*/

double Stack_Pixel(const Stack *stack, int x, int y, int z, int c);

Stack *Stack_Merge(const Stack *stack1, const Stack *stack2, 
		   int x_offset, int y_offset, int z_offset, int option);
Stack *Stack_Merge_M(Stack **stack, int n, int **offset, int option,
		     Stack *new_stack);
Stack *Stack_Merge_M_FC(Stack **stack, int n, int **offset, int option, 
			Stack *new_stack, int *corner1, int *corner2);


Tiff_Image* Stack_Tiff_Image(const Stack *stack, int s, Tiff_Image *image);

Stack* Stack_Locmax_Enhance(Stack *stack, Stack *out);

Stack* Stack_Running_Max(const Stack *stack, int dim_idx, Stack *out);
Stack* Stack_Running_Min(const Stack *stack, int dim_idx, Stack *out);
Stack* Stack_Running_Median(const Stack *stack, int dim_idx, Stack *out);

Stack* Stack_Median_Filter_N(const Stack *stack, int conn, Stack *out);

/**@brief Obtain the bound box of a stack.
 *
 * The bound box of <stack> is defined as the smallest box that contains all
 * foreground voxels (I(x)>0). The result is stored in <bound_box>;
 */
void Stack_Bound_Box(const Stack *stack, Cuboid_I *bound_box);

/**@brief Crop a stack using its bound box.
 *
 * Stack_Bound_Crop() crops <stack> by the bound box of <stack> with an
 * expansion from all directions by <margin>
 */
Stack* Stack_Bound_Crop(const Stack *stack, int margin);

void Stack_Bound_Box_F(File_List *list, Cuboid_I *bound_box);
void Stack_Bound_Box_F_M(File_List *list, Cuboid_I *bound_box);

Stack* Stack_Mip_F(File_List *list);

__END_DECLS

#endif
