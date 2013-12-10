/*****************************************************************************************\
*                                                                                         *
*  Image and Image Stack Data Abstraction for TIF-encoded files                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*  Mods  :  November 2007: added idea of text description and read/write to               *
*              TIFFTAG_IMAGEDESCRIPTION tag of tif format                                 *
*           May 2008: Replaced libtiff with my own tiff library, necessitating            *
*              the introduction of a Tiff data type, and the addition of Advance_Tiff     *
*              and Tiff_EOF to the abstraction (cleaner than before).                     *
*                                                                                         *
\*****************************************************************************************/

#ifndef _IMAGE_LIB

#define _IMAGE_LIB

#include "tiff_io.h"

   //  There are three kinds of images or stacks:

#define GREY     1   // 1-byte grey-level image or stack
#define GREY16   2   // 2-byte grey-level image or stack
#define COLOR    3   // 3-byte RGB image or stack
#define FLOAT32  4   // 4-byte floating values

#define GREY_CHANNEL  0   // Images and stacks either have a single grey channel or 3 color channels

#define RED_CHANNEL   0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL  2

typedef unsigned char  uint8;   //  Datatypes for pixel values
typedef unsigned short uint16;
typedef float          float32;


/* IMAGE ABSTRACTION  */

typedef struct
  { int      kind;
    int      width; 
    int      height;
    char    *text;
    uint8   *array;   // Array of pixel values lexicographically ordered on (y,x,c).
  } Image;            //    Pixel (0,0) is the lower left-hand corner of an image.

  /*  The macros and routines below serve to illustrate how pixel information is organized
        in the 1-dimensional image "array".  Generally, for any extensive image manipulation
        one optimizes the computation of locations into the array.                           */

#define IMAGE_PIXEL_8(img,x,y,c) \
      ((uint8  *) ((img)->array + (((y)*(img)->width + (x))*(img)->kind + (c))))

#define IMAGE_PIXEL_16(img,x,y,c) \
      ((uint16 *) ((img)->array + (((y)*(img)->width + (x))*(img)->kind + (c))))

#define IMAGE_PIXEL_32(img,x,y,c) \
      ((float32 *) ((img)->array + (((y)*(img)->width + (x))*(img)->kind + (c))))

static inline double Get_Image_Pixel(Image *image, int x, int y, int c)
{ if (image->kind == GREY)
    return (*IMAGE_PIXEL_8(image,x,y,c));
  else if (image->kind == GREY16)
    return (*IMAGE_PIXEL_16(image,x,y,c));
  else // image->kind == FLOAT32
    return (*IMAGE_PIXEL_32(image,x,y,c));
}

static inline void Set_Image_Pixel(Image *image, int x, int y, int c, double v)
{ if (image->kind == GREY)
    *IMAGE_PIXEL_8(image,x,y,c) = v;
  else if (image->kind == GREY16)
    *IMAGE_PIXEL_16(image,x,y,c) = v;
  else // image->kind == FLOAT32
    *IMAGE_PIXEL_32(image,x,y,c) = v;
}

  /*  Basic Tiff I/O primitives:
        Open_Tiff opens the named .tif file in the specified mode (either "r" or "w").  A series
        of images can be written to 'tif' with Write_Tiff to create a stack or a single image can
        be written to create an image tif file.  One can similarly read a series of tif images from
        an open tif file with Read_Tiff, skip the next image with Advance_Tiff, and test if there
        are any more with Tiff_EOF.  Closing the tif with Close_Tiff completes the creation
        or scan of 'tif'.
  */

typedef void Tiff;

Tiff   *Open_Tiff(char *file_name, char *mode);
/* tz+ */
Tiff   *Open_Tiff_E(const char *file_name, char *mode, int lsm);
Image  *Read_Tiff(Tiff *tif);
void    Advance_Tiff(Tiff *tif);
int     Tiff_EOF(Tiff *tif);
void    Write_Tiff(Tiff *tif, Image *a_image);
void    Close_Tiff(Tiff *tif);

Tiff  *Copy_Tiff(Tiff *tif);
void   Pack_Tiff(Tiff *tif);
void   Free_Tiff(Tiff *tif);
void   Kill_Tiff(Tiff *tif);
void   Reset_Tiff();
int    Tiff_Usage();

  /*  Image Library:
         One can read, write, copy, pack, free, and kill images.  Also one can create a new
         uninitialized image with "Make_Image".  A copy of the image description tag in the
         tif file is stored in the text field of the tiff if present.  It can be modified
         with the set and append routines.
  */

Image *Read_Image(char *file_name);
Image *Make_Image(int kind, int width, int height);

void   Write_Image(char *file_name, Image *a_image);

void   Set_Image_Text(Image *image, char *text);
void   Append_To_Image_Text(Image *image, char *text);

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
    char    *text;
    uint8   *array;   // array of pixel values lexicographically ordered on (z,y,x,c)
  } Stack;

  /*  The macros and routines below serve to illustrate how pixel information is organized
        in the 1-dimensional stack "array".  Generally, for any extensive stack manipulation
        one optimizes the computation of locations into the array.                           */

#define STACK_PIXEL_8(img,x,y,z,c) \
      ((uint8  *) ((img)->array +   \
           ((((size_t)(z)*(img)->height + (size_t)(y))*(img)->width + (size_t)(x))*(img)->kind + (c))))

#define STACK_PIXEL_16(img,x,y,z,c) \
      ((uint16 *) ((img)->array +   \
           ((((size_t)(z)*(img)->height + (size_t)(y))*(img)->width + (size_t)(x))*(img)->kind + (c))))

#define STACK_PIXEL_32(img,x,y,z,c) \
      ((float32 *) ((img)->array +   \
           ((((size_t)(z)*(img)->height + (size_t)(y))*(img)->width + (size_t)(x))*(img)->kind + (c))))

static inline double Get_Stack_Pixel(Stack *stack, int x, int y, int z, int c)
{ if (stack->kind == GREY /*tz+*/ || stack->kind == COLOR /*tz++*/)
    return (*STACK_PIXEL_8(stack,x,y,z,c));
  else if (stack->kind == GREY16)
    return (*STACK_PIXEL_16(stack,x,y,z,c));
  else // stack->kind == FLOAT32
    return (*STACK_PIXEL_32(stack,x,y,z,c));
}

static inline void Set_Stack_Pixel(Stack *stack, int x, int y, int z, int c, double v)
{ if (stack->kind == GREY /*tz+*/ || stack->kind == COLOR /*tz++*/)
    *STACK_PIXEL_8(stack,x,y,z,c) = v;
  else if (stack->kind == GREY16)
    *STACK_PIXEL_16(stack,x,y,z,c) = v;
  else // stack->kind == FLOAT32
    *STACK_PIXEL_32(stack,x,y,z,c) = v;
}

  /*   Stack Library:
         One has exactly the same primitives as for images, with the addition of two primitives
         "Select_Plane" and "Set_Stack_Plane".  Select_Plane selects a given plane out of a stack
         and returns a pointer to an image record containing it.  The text of the image is empty.
         The plane is not allocated anew but is simply pointed at within the stack by the
         "Stack_Plane" record statically owned by "Select_Plane".  Set_Stack_Plane places the
         contents of a_image into the indicated z-plane of a_stack.  a_image and a_stack must
         have corresponding dimensions and kind, otherwise a non-zero value is returned.
  */

Stack *Read_Stack(char *file_name);
Stack *Make_Stack(int kind, int width, int height, int depth);

void   Write_Stack(char *file_name, Stack *a_stack);

char  *Get_Stack_Description(Image *image);
void   Set_Stack_Description(Image *image, char *text);
void   Append_To_Stack_Description(Image *image, char *text);

Stack *Copy_Stack(Stack *stack);
void   Pack_Stack(Stack *stack);
void   Free_Stack(Stack *stack);
void   Kill_Stack(Stack *stack);
void   Reset_Stack();
int    Stack_Usage();

typedef Image Stack_Plane;   //  Emphasizes this is a static return bundle

Stack_Plane *Select_Plane(Stack *a_stack, int plane);
int          Set_Stack_Plane(Stack *a_stack, int plane, Image *a_image);

  /*   File Stack Routines:

         Parse_Stack_Name parses a file name assuming it is of the form "<prefix><first_num>.tif"
     and returns the constituent parts where 'num_width' is the number of characters used to
     represent 'first_num'.  A file-based stack is encoded as a sequence of files with names of
     this form where the planes of the stack are in successively numbered files, e.g.
     x.001.tif x.002.tif ... x.037.tif.  The number part is assumed to always involve the
     same number of digits and be zero-padded as necessary.  The numbers are assumed to form
     a contiguous range (that does *not* need to start at 0 or 1).
  
     Given the parse information, one can then read or write a file-type stack.
  */

typedef struct        //  a return bundle (not an object)
  { char *prefix;
    int   num_width;  //  bundle names are "%s.%0*d",prefix,num_width,first_num
    int   first_num;
  } File_Bundle;

File_Bundle *Parse_Stack_Name(char *file_name);
Stack *Read_Stack_Planes(File_Bundle *bundle);
void   Write_Stack_Planes(File_Bundle *bundle, Stack *stack);

  /*  Scaling and Conversion Routines:

        The largest and smallest pixel value in a channel of an image is returned by
        "Image_Range" and using these one can then transform the values in an image with
        "Scale_Image" where <new_value> = factor * (<old_value> + offset).  For COLOR images
        if you pass a channel # greater than BLUE, the routines work on *all* three channels.
        These routines are mostly of value for FLOAT32 images and also for GREY16 images where
        it is often the case that the maximum value is much less than 0xFFFF (e.g. 12-bit images).
        "Scale_X_To_Range" convenientely scales to a given pixel spread [min,max].

        One can convert an image to any other type (specified by the parameter "kind") of
        image with "Translate_Image".  No scaling of values takes place except when a GREY16
        or FLOAT32 image has values larger than the target maximum, in which case values are
        scaled so that the largest values in the source becomes the largest possible values in
        the target.  If "in_place" is zero then a new image of the desired type is generated
        with an empty text, otherwise the input image is modified to be of the desired type,
        where its array is enlarged with a realloc if necessary to accomodate the result.  If
        the translated result is smaller than the original, the array is not made smaller (in
        case you want the bigger container to be recirculated).  If you want the translated
        result to be shrunk then call Pack_Image on it.
  */

typedef struct      //  A return bundle (not an object)
  { double maxval;
    double minval;
  } Pixel_Range;

Pixel_Range *Image_Range(Image *image, int channel);
void         Scale_Image(Image *image, int channel, double factor, double offset);
void         Scale_Image_To_Range(Image *image, int channel, double min, double max);

Pixel_Range *Stack_Range(Stack *stack, int channel);
void         Scale_Stack(Stack *stack, int channel, double factor, double offset);
void         Scale_Stack_To_Range(Stack *stack, int channel, double min, double max);

Image       *Translate_Image(Image *image, int kind, int in_place);
Stack       *Translate_Stack(Stack *stack, int kind, int in_place);

void         Ceiling_Image(Image *image, int channel, double ceiling);
void         Ceiling_Stack(Stack *stack, int channel, double ceiling);

void         Floor_Image(Image *image, int channel, double ceiling);
void         Floor_Stack(Stack *stack, int channel, double ceiling);

void         Threshold_Image(Image *image, int channel, double cutoff);
void         Threshold_Stack(Stack *stack, int channel, double cutoff);

void         Split_Color_Image(Image *image, Image **red, Image **green, Image **blue);
void         Split_Color_Stack(Stack *image, Stack **red, Stack **green, Stack **blue);

Image       *Make_Color_Image(Image *red, Image *green, Image *blue);
Stack       *Make_Color_Stack(Stack *red, Stack *green, Stack *blue);

int Determine_Kind(void *tif);
int Determine_Width(void *tif);
int Determine_Height(void *tif);
#endif
