/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*****************************************************************************************\
*                                                                                         *
*  Image and Image Stack Data Abstraction for TIF-encoded files                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*                                                                                         *
\*****************************************************************************************/
/* changed the include file name by PHC 060816*/
/* comment uint8 and uint16 types on 070808 */
//2010-05-20: add the path of tiffio.h, PHC

#ifndef MG_IMAGE_LIB

#define MG_IMAGE_LIB

#include "tiffio.h"

   //  There are three kinds of images or stacks:

#define GREY   1   // 1-byte grey-level image or stack
#define GREY16 2   // 2-byte grey-level image or stack
#define COLOR  3   // 3-byte RGB image or stack

#define GREY_CHANNEL  0   // Images and stacks either have a single grey channel or 3 color channels
#define RED_CHANNEL   0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL  2

/*These two variables have been defined in tiffio.h. Thus comment. */
/*
typedef unsigned char  uint8;   //  Datatypes for 1 byte and 2 byte pixel values
typedef unsigned short uint16;
*/


/* IMAGE ABSTRACTION  */

typedef struct
  { int      kind;
    int      width; 
    int      height;
    uint8   *array;   // Array of pixel values lexicographically ordered on (y,x,c).
  } Image;            //    Pixel (0,0) is the lower left-hand corner of an image.

  /*  The macros and routines below serve to illustrate how pixel information is organized
        in the 1-dimensional image "array".  Generally, for any extensive image manipulation
        one optimizes the computation of locations into the array.                           */

#define IMAGE_PIXEL_8(img,x,y,c) \
      ((uint8  *) ((img)->array + (((y)*(img)->width + (x))*(img)->kind + (c))))

#define IMAGE_PIXEL_16(img,x,y,c) \
      ((uint16 *) ((img)->array + (((y)*(img)->width + (x))*(img)->kind + (c))))

static inline int Get_Image_Pixel(Image *image, int x, int y, int c)
{ if (image->kind == GREY16)
    return (*IMAGE_PIXEL_16(image,x,y,c));
  else
    return (*IMAGE_PIXEL_8(image,x,y,c));
}

static inline void Set_Image_Pixel(Image *image, int x, int y, int c, int v)
{ if (image->kind == GREY16)
    *IMAGE_PIXEL_16(image,x,y,c) = v;
  else
    *IMAGE_PIXEL_8(image,x,y,c) = v;
}

  /*  Basic I/O primitives:
        Open_Tiff opens the named .tif file in the specified mode (typically "r" or "w").  A series
      of images can be written to 'tif' with Write_Tiff to create a stack or a single image can
      be written to create an image tif file.  One can similarly read a series of tif images from
      an open tif file, when the last one is read, the flag at the location pointed to by 'last'
      is set to non-zero.  Closing the tif with Close_Tiff completes the creation or scan of 'tif'.
  */

TIFF   *Open_Tiff(char *file_name, char *mode);
Image  *Read_Tiff(TIFF *tif, int *last);
void   Write_Tiff(TIFF *tif, Image *a_image);
void   Close_Tiff(TIFF *tif);

  /*  Image Library:
         One can read, write, copy, pack, free, and kill images.  Also one can create a new
      uninitialized image with "New_Image".  The largest and smallest pixel value in a channel
      of an image is returned by "Image_Range" and using these one can then transform the values
      in an image with "Scale_Image" where <new_value> = <old_value>*factor + offset.  These
      returns are mostly of value for GREY16 images where is is often the case that the maximum
      value is much less than 0xFFFF (e.g. 12-bit images).
        One can convert an image to any other type (specified by the parameter "kind") of image
      with "Translate_Image".  No scaling of values takes place except when a GREY16 stack
      containing values greater than 255 is converted to a GREY or COLOR image.  In these cases
      values are scaled so the maximum GREY16 value in the image becomes 255 in the converted
      GREY or COLOR image.  If "in_place" is zero then a new image of the desired type is
      generated, otherwise the input image is modified to be of the desired type, where its
      array is enlarged with a realloc if necessary to accomodate the result.  If the translated
      result is smaller than the original, the array is not made smaller (in case you want the
      bigger container to be recirculated).  If you want the translated result to be shrunk
      then call Pack_Image on it.
  */

Image *Read_Image(char *file_name);
Image *Make_Image(int kind, int width, int height);
Image *Translate_Image(Image *image, int kind, int in_place);

void   Write_Image(char *file_name, Image *a_image);

typedef struct
  { int maxval;
    int minval;
  } Pixel_Range;

Pixel_Range *Image_Range(Image *image, int channel);
void         Scale_Image(Image *image, int channel, double factor, double offset);

Image *Copy_Image(Image *image);
void   Pack_Image(Image *image);
void   Free_Image(Image *image);
void   Kill_Image(Image *image);
void   Reset_Image();
int    Image_Usage();


/* STACK ABSTRACTION */

typedef struct
  { int      kind;
    int      width;
    int      height;
    int      depth;
    uint8   *array;   // array of pixel values lexicographically ordered on (z,y,x,c)
  } Stack;

  /*  The macros and routines below serve to illustrate how pixel information is organized
        in the 1-dimensional stack "array".  Generally, for any extensive stack manipulation
        one optimizes the computation of locations into the array.                           */

#define STACK_PIXEL_8(img,x,y,z,c) \
      ((uint8  *) ((img)->array +   \
           ((((z)*(img)->height + (y))*(img)->width + (x))*(img)->kind + (c))))

#define STACK_PIXEL_16(img,x,y,z,c) \
      ((uint16 *) ((img)->array +   \
           ((((z)*(img)->height + (y))*(img)->width + (x))*(img)->kind + (c))))

static inline int Get_Stack_Pixel(Stack *stack, int x, int y, int z, int c)
{ if (stack->kind == GREY16)
    return (*STACK_PIXEL_16(stack,x,y,z,c));
  else
    return (*STACK_PIXEL_8(stack,x,y,z,c));
}

static inline void Set_Stack_Pixel(Stack *stack, int x, int y, int z, int c, int v)
{ if (stack->kind == GREY16)
    *STACK_PIXEL_16(stack,x,y,z,c) = v;
  else
    *STACK_PIXEL_8(stack,x,y,z,c) = v;
}

  /*   Stack Library:
         One has exactly the same primitives as for images, with the addition of a primitive
       "Select_Plane" that selects a given plane out of a stack and returns a pointer to an
       image record containing it.  The plane is not allocated anew but is simply pointed at
       within the stack by the "Stack_Plane" record statically owned by "Select_Plane".
  */

Stack *Read_Stack(char *file_name);
Stack *Read_Stack_location(char *file_name, int zs,int ze);
Stack *Make_Stack(int kind, int width, int height, int depth);
Stack *Translate_Stack(Stack *stack, int kind, int in_place);

Stack *Read_LSM_Stack(char *file_name);/*070713*/

void   Write_Stack(char *file_name, Stack *a_stack);

Pixel_Range *Stack_Range(Stack *stack, int channel);
void         Scale_Stack(Stack *stack, int channel, double factor, double offset);

Stack *Copy_Stack(Stack *stack);
void   Pack_Stack(Stack *stack);
void   Free_Stack(Stack *stack);
void   Kill_Stack(Stack *stack);
void   Reset_Stack();
int    Stack_Usage();

typedef Image Stack_Plane;

Stack_Plane *Select_Plane(Stack *a_stack, int plane);

  /*   File Stack Routines;

         Parse_Stack_Name parses a file name assuming it is of the form "<prefix><first_num>.tif"
     and returns the constituent parts where 'num_width' is the number of characters used to
     represent 'first_num'.  A file-based stack is encoded as a sequence of files with names of
     this form where the planes of the stack are in successively numbered files, e.g.
     x.001.tif x.002.tif ... x.037.tif.  The number part is assumed to always involve the
     same number of digits and be zero-padded as necessary.  The numbers are assumed to form
     a contiguous range (that does *not* need to start at 0 or 1).
  
     Given the parse information, one can then read or write a file-type stack.
  */

void   Parse_Stack_Name(char *file_name, char **prefix, int *num_width, int *first_num);
Stack *Read_Stack_Planes(char *prefix, int num_width, int first_num);
void   Write_Stack_Planes(char *prefix, int num_width, int first_num, Stack *a_stack);

#endif
