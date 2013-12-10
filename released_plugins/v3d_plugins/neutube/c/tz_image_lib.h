/**@file tz_image_lib.h
 * @brief general image routines
 * @author Ting Zhao
 */

#ifndef TZ_IMAGE_LIB_H_
#define TZ_IMAGE_LIB_H_

#include "tz_cdefs.h"
#include "tz_imatrix.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_lib.h"

__BEGIN_DECLS

/**@addtogropu image_lib_ General Image routines
 *
 * @{
 */

/*
 * Unpack_Image() frees the <image> data structure while keeping the array data,
 * which is returned from the function.
 */
uint8* Unpack_Image(Image *image);

/*
 * Get_Pixel_Number() returns the number of pixels in an image. This function is
 * obsoleted by Image_Pixel_Number() in tz_image_attribute.h.
 */
int Get_Pixel_Number(Image* image);

/*
 * Note: Get_Double_Array() and Get_Float_Array() are obsolete functions. Use
 * Image_Double_Array() and Image_Float_Array() instead.
 *
 * Get_Double_Array() returns the double type of the array of <image>.
 * Get_Float_Array() returns the float type of the array of <image>. The array
 * of <image> could be any valid kind and the functions will copy the values
 * to the returned array. The caller is responsible for freeing the returned
 * array.
 *
 * Image_Double_Array() and Image_Float_Array() are better versions. They also
 * allow users to pass their own arrays to get the results.
 */
double* Get_Double_Array(Image *image);
float* Get_Float_Array(Image *image);

double* Image_Double_Array(const Image *image, double *array);
float* Image_Float_Array(const Image *image, float *array);

/*
 * Scale_Float_Image() scales a float array and converts it into an image
 * with the specified kind. The values of the image will cover largest possible 
 * range except float types, which have range [0, 1]. Scale_Double_Image()
 * is a counterpart for double array.
 * The returned pointers are new objects.
 */
Image* Scale_Float_Image(float *fImage, int  width, int height, int kind);
Image* Scale_Double_Image(double *fImage, int  width, int height, int kind);

/*
 * Zero_Image() sets all pixels in an image to 0 intensity.
 */
void Zero_Image(Image *image);

/*
 * Copy_Image_Array() copies the values of <src> to <des>. <src> and <des> 
 * should have the same attributes. 
 */
void Copy_Image_Array(Image *des, const Image *src);

/*
 * Crop_Image() crops an image by a rectangle with size <width> x <height>.
 * The lefttop corner of the rectangle is specified by <left> and <top>. 0s are
 * padded when the rectangle is out range. The returned pointer is a new object
 * if <desimage> is NULL, otherwise <desimage> is used to store the result and 
 * returned too.
 */
Image* Crop_Image(const Image* image, int left, int top, int width, int height,
		  Image *desimage);

/*
 * Reflect_Image() returns the reflection of <image>. It is the same as rotating
 * the image by 180 degrees. If in_place is not 0, the result will be  stored
 * in <image>. The returned pointer is a new object if it is not in place
 * transformation, otherwise it is the same as <image>.
 */
Image* Reflect_Image(Image *image, int in_place);

/*
 * Print_Image_Info() prints the information of an image to the screen.
 * Print_Image_Value() prints the values of all pixels in <image> to the
 * screen.
 */
void Print_Image_Info(Image *image);
void Print_Image_Value(Image* image);

/*
 * Resize_Image() resizes an image to the size <width> x <height> and returns
 * the new image. The returned pointer is a new object.
 */
Image* Resize_Image(const Image* image,int width,int height);

/*
 * Image_Hist() returns the histogram of <image>. The first element of the
 * returned array is the length of the histogram and the second one is the
 * starting grey level. The returned  pointer is a new array.
 *
 * Image_Hist_M() creates the histogram of <image> in the region specified by
 * the binary image <mask>, in which 1 means in region and 0 means out of
 * region. If <mask> is NULL, it is the same as Image_Hist(), otherwise it must
 * be as large as <image> and be GREY kind.
 */
int* Image_Hist(Image *image);
int* Image_Hist_M(Image *image, Image *mask);

/*
 * Empty_Image() sets an image to empty. To use it safely, <image> should be
 * created without initialization or Make_Image() assignment.
 *
 * Is_Image_Empty() returns TURE if <image> is not empty, otherwise it returns
 * FALSE. NULL is considered as non-empty.
 */
void Empty_Image(Image *image);
BOOL Is_Image_Empty(const Image *image);

/*
 * Image_Block_Sum() calculates the sum of intensties in a local window and
 * returns the resulted image. The window size w x h.
 */
Image* Image_Block_Sum(const Image *image, int w, int h);

/**@}*/

__END_DECLS


#endif

