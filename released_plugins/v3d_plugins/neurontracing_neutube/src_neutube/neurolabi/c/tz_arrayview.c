/* tz_arrayview.c
 *
 * 14-Feb-2008 Initial write: Ting Zhao
 */

#include <string.h>
#include "tz_sys.h"
#include "tz_error.h"
#include "tz_matlabdefs.h"
#include "tz_image_lib.h"
#include "tz_stack_attribute.h"
#include "tz_arrayview.h"

/* Image_View_Stack(): Create an image view from a stack.
 *
 * Args: x - the stack to view.
 *   
 * Return: an image view.
 */
Image Image_View_Stack(const Stack *x)
{
  //Image_View iv;

  Image image;

  if (x == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    Empty_Image(&image);
  } else {
    image.kind = x->kind;
    image.width = x->width;
    image.height = x->height * x->depth;
    image.array = x->array;
    image.text = NULL;
  }

  return image;
}

Image Image_View_Stack_Slice(const Stack *x, int slice)
{
  //Image_View iv;
  Image image;

  if (x == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    Empty_Image(&(image));
  } else {
    if ((slice < 0) || (slice >= x->depth)) {
      TZ_WARN(ERROR_DATA_VALUE);
      Empty_Image(&(image));
    } else {
      image.kind = x->kind;
      image.width = x->width;
      image.height = x->height;
      image.array = x->array + x->width * x->height * slice *
	Stack_Voxel_Bsize(x);
      image.text = NULL;
    }
  }
  
  return image;
}

Image Image_View_Mar(const Matlab_Array *x)
{
  //Image_View iv;
  Image image;
  
  if (x == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    Empty_Image(&(image));
  } else {
    if (x->ndim > 2) {
      TZ_WARN(ERROR_DATA_COMPTB);
      Empty_Image(&(image));
    } else if ((x->type != mxUINT8_CLASS) && (x->type != mxUINT16_CLASS) && 
	       (x->type != mxSINGLE_CLASS)) {
      TZ_WARN(ERROR_DATA_COMPTB);
      Empty_Image(&(image));
    } else {
      image.width = x->dim[0];

      if (x->ndim == 1) {
	image.height = 1;
      } else {
	image.height = x->dim[1];
      }

      switch (x->type) {
      case mxUINT8_CLASS:
	image.kind = GREY;
	break;
      case mxUINT16_CLASS:
	image.kind = GREY16;
	break;
      case mxSINGLE_CLASS:
	image.kind = FLOAT32;
	break;
      default:
	TZ_WARN(ERROR_DATA_COMPTB);
	Empty_Image(&(image));
      }
    }
    image.text = NULL;
  }
   
  return image;
}

Stack Stack_View_DMatrix(const DMatrix *x)
{
  //Stack_View sv;
  Stack stack;

  if (x == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    Empty_Stack(&(stack));
  } else {
    if (x->ndim > 3) {
      TZ_WARN(ERROR_DATA_COMPTB);
      Empty_Stack(&(stack));
    } else {
      stack.array = (uint8 *) x->array;
      stack.kind = FLOAT64;
      stack.width = x->dim[0];
      if (x->ndim >=2) {
	stack.height = x->dim[1];
      } else {
	stack.height = 1;
      }

      if (x->ndim == 3) {
	stack.depth = x->dim[2];
      } else {
	stack.depth = 1;
      }
      stack.text = NULL;
    }
  }

  return stack;
}

Stack Stack_View_FMatrix(const FMatrix *x)
{
  //Stack_View sv;
  Stack stack;

  if (x == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    Empty_Stack(&(stack));
  } else {
    if (x->ndim > 3) {
      TZ_WARN(ERROR_DATA_COMPTB);
      Empty_Stack(&(stack));
    } else {
      stack.array = (uint8 *) x->array;
      stack.kind = FLOAT32;
      stack.width = x->dim[0];
      if (x->ndim >=2) {
	stack.height = x->dim[1];
      } else {
	stack.height = 1;
      }

      if (x->ndim == 3) {
	stack.depth = x->dim[2];
      } else {
	stack.depth = 1;
      }
      stack.text = NULL;
    }
  }

  return stack;
}

Stack Stack_View_Image(const Image *image)
{
  //Stack_View sv;
  Stack stack;

  stack.kind = image->kind;
  stack.width = image->width;
  stack.height = image->height;
  stack.depth = 1;
  stack.array = image->array;
  stack.text = NULL;
  return stack;
}

Matlab_Array Matlab_Array_View_Int_Arraylist(const Int_Arraylist *a)
{
  //Matlab_Array_View mav;
  Matlab_Array matlab_array;

  switch (INT_BIT) {
  case 8:
    matlab_array.type = mxINT8_CLASS;
    break;
  case 16:
    matlab_array.type = mxINT16_CLASS;
    break;
  case 32:
    matlab_array.type = mxINT32_CLASS;
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  matlab_array.flag = 0;
  matlab_array.ndim = 1;
  matlab_array.dim[0] = a->length;
  strcpy(matlab_array.name, "matlab_array_view");
  matlab_array.array = a->array;

  return matlab_array;
}

DMatrix DMatrix_View_Matlab_Array(const Matlab_Array *ma)
{
  DMatrix dm;

  dm.ndim = ma->ndim;

  int i;
  for (i = 0; i < ma->ndim; i++) {
    dm.dim[i] = ma->dim[i];
  }

  dm.array = (double*)ma->array;
  
  return dm;
}
