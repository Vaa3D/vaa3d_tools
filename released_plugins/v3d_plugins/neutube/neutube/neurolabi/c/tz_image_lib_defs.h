/**@file tz_image_lib_defs.h
 * @brief definitions for image processing
 * @author Ting Zhao
 * @date 14-Jan-2008
 */

#ifndef _TZ_IMAGE_LIB_DEFS_H_
#define _TZ_IMAGE_LIB_DEFS_H_

#include "tz_cdefs.h"
#if GENELIB_VER < 3
__BEGIN_DECLS
  #include <image_lib.h>
__END_DECLS
#else
  #include <image.h>
#endif

#include "tz_tiff_image.h"
#include "tz_tiff_io.h"
#include "tz_mc_stack.h"
#include "tz_utilities.h"

#define MAXVALUE_GREY 255
#define MAXVALUE_GREY16 65535
#define STACK_DEPTH_MAX 1000

/* Extended image kinds */
#define FLOAT64 8
#define FCOLOR 12
#define DCOLOR 24
#define GREY8 GREY
#define GREY32 FLOAT32
#define GREY64 FLOAT64
#define SGREY -1
#define SGREY16 -2
#define UNKNOWN_IMAGE_KIND -100

//typedef uint32_t uint32;

typedef double float64;
typedef uint8 color_t[3];
typedef float fcolor_t[3];
typedef double dcolor_t[3];

#define SAME_COLOR(c1, c2) ((c1[0] == c2[0]) && (c1[1] == c2[1]) && (c1[2] == c2[2]))

/**@union _Image_Array tz_image_lib_defs.h
 *
 * Image array union
 */
typedef union _Image_Array {
  uint8 *array;         /**< uint8 array */
  uint8 *array8;        /**< uint8 array */
  uint16 *array16;      /**< uint16 array */
  float *array32;       /**< float32 array */
  double *array64;      /**< double array */
  color_t *arrayc;      /**< rgb color array */
} Image_Array;

#define CREATE_IMAGE_ARRAY(ima, image)	\
  Image_Array ima;			\
  ima.array = image->array;

#define VALIDATE_INTENSITY_GREY(value)	\
  if (value < 0) {				\
    value = 0;					\
  } else if (value > 255) {			\
    value = 255;				\
  }

#define VALIDATE_INTENSITY_GREY8 VALIDATE_INTENSITY_GREY  

#define VALIDATE_INTENSITY_GREY16(value)	\
  if (value < 0) {				\
    value = 0;					\
  } else if (value > 65535) {			\
    value = 65535;				\
  }

#define VALIDATE_INTENSITY_FLOAT32(value) 
#define VALIDATE_INTENSITY_GREY32(value) 
#define VALIDATE_INTENSITY_FLOAT64(value) 
#define VALIDATE_INTENSITY_GREY64(value) 

#define VALIDATE_INTENSITY_COLOR(value) VALIDATE_INTENSITY_GREY(value)

/**@brief Maximum possible grey level for a certain kind.
 *
 * Only GREY and GREY16 are supported.
 */
#define MAXGREY(kind)					\
  ((kind==GREY16)?MAXVALUE_GREY16:MAXVALUE_GREY)

/**@brief Maximum pixel value of an image.
 *
 * The result is stored in \a max_value.
 */
#define IMAGE_MAX(image,max_value)				\
  {								\
    if(image->kind==GREY16) {					\
      uint16* array16 = (uint16 *)image->array;			\
      long length = image->width*image->height;			\
      MAX(array16,length,max_value);				\
    }								\
    else if(image->kind==GREY){					\
      long length = image->width*image->height;			\
      MAX(image->array,length,max_value);			\
    } else							\
      fprintf(stderr,"IMAGE_MAX: Unsupported image format.\n");	\
  }

/**@brief Minimum pixel value of an image.
 *
 * The result is stored in \a min_value.
 */
#define IMAGE_MIN(image,min_value)				\
  {								\
    if(image->kind==GREY16) {					\
      uint16* array16 = (uint16 *)image->array;			\
      long length = image->width*image->height;			\
      MIN(array16,length,min_value);				\
    }								\
    else if(image->kind==GREY){					\
      long length = image->width*image->height;			\
      MIN(image->array,length,min_value);			\
    } else							\
      fprintf(stderr,"IMAGE_MIN: Unsupported image format.\n");	\
  }

/**@brief Define a pixel variable.
 *
 * The variable has the type \a data_type and name \a prefix_\a suffix.
 */
#define DEFINE_PIXEL(prefix, suffix, data_type)	\
  data_type TZ_CONCATU(prefix, suffix);

/**@brief Define pixel variable with all possible types.
 */
#define DEFINE_PIXEL_ALL(prefix)				\
  DEFINE_PIXEL(prefix, grey, uint8)				\
  DEFINE_PIXEL(prefix, grey16, uint16)				\
  DEFINE_PIXEL(prefix, color, color_t)				\
  DEFINE_PIXEL(prefix, float32, float32)			\
  DEFINE_PIXEL(prefix, float64, double)

/**@brief Create a pointer with a certain type.
 *
 * The point has the same address as that of the data in \a obj.
 */
#define DEFINE_ARRAY(prefix, suffix, data_type, obj)			\
  data_type *TZ_CONCATU(prefix, suffix) = (data_type *) obj->array;

/**@brief Create pointers for scalar array.
 */
#define DEFINE_SCALAR_ARRAY_ALL(prefix, obj)			\
  DEFINE_ARRAY(prefix, grey, uint8, obj)			\
  DEFINE_ARRAY(prefix, grey16, uint16, obj)			\
  DEFINE_ARRAY(prefix, float32, float32, obj)			\
  DEFINE_ARRAY(prefix, float64, double, obj)

/* The defined arrays are: prefix_grey, prefix_grey16, prefix_float32,
 * prefix_float64, prefix_color */
#define DEFINE_ARRAY_ALL(prefix, obj)				\
  DEFINE_SCALAR_ARRAY_ALL(prefix, obj)				\
  DEFINE_ARRAY(prefix, color, color_t, obj)

/**@brief Stack operation template.
 *
 * This macro applies \a static_func on \a stack. \a stack_func is usually
 * a macro function.
 */
#define SCALAR_STACK_OPERATION(stack, stack_func)		\
  CREATE_IMAGE_ARRAY(ima__, stack);				\
  switch (stack->kind) {					\
  case GREY:							\
    stack_func(ima__.array8);					\
    break;							\
  case GREY16:							\
    stack_func(ima__.array16);					\
    break;							\
  case FLOAT32:							\
    stack_func(ima__.array32);					\
    break;							\
  case FLOAT64:							\
    stack_func(ima__.array64);					\
    break;							\
  default:							\
    PRINT_EXCEPTION("Unsuppoted stack kind",			\
		    "GREY, GREY16, FLOAT32, FLOAT64 only");	\
  }

#define STACK_ARRAY_OPERATION(ima, stack_func)			\
  switch (stack->kind) {					\
  case GREY:							\
    stack_func(ima.array8);					\
    break;							\
  case GREY16:							\
    stack_func(ima.array16);					\
    break;							\
  case FLOAT32:							\
    stack_func(ima.array32);					\
    break;							\
  case FLOAT64:							\
    stack_func(ima.array64);					\
    break;							\
  default:							\
    PRINT_EXCEPTION("Unsuppoted stack kind",			\
		    "GREY, GREY16, FLOAT32, FLOAT64 only");	\
  }

typedef struct tagTree_Code {
  tz_uint16 level;
  tz_uint16 branch;
  int link; /* level link */
} Tree_Code;

typedef struct tagLevel_Code {
  tz_uint16 level;
  int link; /* level link */
} Level_Code;

enum {
  STACK_LOCMAX_CENTER, STACK_LOCMAX_NEIGHBOR, STACK_LOCMAX_NONFLAT,
  STACK_LOCMAX_FLAT, STACK_LOCMAX_ALTER1, STACK_LOCMAX_ALTER2,
  STACK_LOCMAX_SINGLE
};

#define STACK_CHECK_CHORD(stack, chord, is_owner)			\
  BOOL is_owner = FALSE;						\
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

#define STACK_CHECK_CHORD_END(chord, is_owner)	\
  if (is_owner == TRUE) {			\
    Kill_IMatrix(chord);			\
  }

#endif
