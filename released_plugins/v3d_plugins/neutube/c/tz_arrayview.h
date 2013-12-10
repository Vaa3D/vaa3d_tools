/**@file tz_arrayview.h
 * @brief array view
 * @author Ting Zhao
 * @date 13-Feb-2008 
 */

#ifndef _TZ_ARRAYVIEW_H_
#define _TZ_ARRAYVIEW_H_

#include "tz_cdefs.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_matrix.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include <image_lib.h>
#include "tz_dmatrix.h"
#include "tz_fmatrix.h"
#include "tz_imatrix.h"
#include "tz_u8matrix.h"
#include "tz_matlabio.h"
#include "tz_object_3d.h"
#include "tz_int_arraylist.h"

__BEGIN_DECLS

/*
typedef struct tagImage_View {
  Image image;
} Image_View;
*/

/* Image_View routines
 *
 * Image_View_Stack() returns an image view of a stack. The width of the image
 * is the same as the width of the stack, while the height of the image is 
 * the product of the height and depth of the stack.
 *
 * Image_View_Stack_Slice() returns an image view of a slice of a stack.
 * <slice> specifies the index of the slice for viewing and the first slice
 * has the index 0.
 *
 * Image_View_Mar() returns an image view of a matlab array.
 *
 * They will all return an empty image view if the view cannot be created 
 * normally.
 */
Image Image_View_Stack(const Stack *x);
Image Image_View_Stack_Slice(const Stack *x, int slice);
Image Image_View_Mar(const Matlab_Array *x);

/*
typedef struct tagStack_View {
  Stack stack;
} Stack_View;
*/

Stack Stack_View_DMatrix(const DMatrix *x);
Stack Stack_View_FMatrix(const FMatrix *x);
Stack Stack_View_Image(const Image *image);

/*
typedef struct tagMatlab_Array_View {
  Matlab_Array matlab_array;
} Matlab_Array_View;
*/

/* Matlab_Array_View routines
 *
 * Matlab_Array_View_Int_Arraylist() returns a matlab array view of an integer
 * arraylist.
 */
Matlab_Array Matlab_Array_View_Int_Arraylist(const Int_Arraylist *a);

DMatrix DMatrix_View_Matlab_Array(const Matlab_Array *ma);

__END_DECLS

#endif
