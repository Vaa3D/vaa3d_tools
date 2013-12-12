/**@file tz_arraytrans.h
 * @brief *routines for array transformation (Obsolete)
 * @author Ting Zhao
 */


/*
 * Things to do to add a new type Tarray
 *
 * .h
 * #include "tarray.h"
 * #define TARRAY_AT     number
 * Tarray* Get_Tarray_At(Array_Link *a);
 *
 * .c
 * get_array
 * set_array
 * DEFINE_GET_DATA_AT(Tarray, Tarray, TARRAY_AT)
 * new_data
 *
 */


#ifndef _TZ_ARRAYTRANS_H_
#define _TZ_ARRAYTRANS_H_

#include "tz_cdefs.h"
#include <image_lib.h>
#include "tz_dmatrix.h"
#include "tz_fmatrix.h"
#include "tz_imatrix.h"
#include "tz_u8matrix.h"
#include "tz_matlabio.h"
#include "tz_object_3d.h"
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

__BEGIN_DECLS

#define IMAGE_AT                   1
#define STACK_AT                   2
#define DMATRIX_AT                 4
#define FMATRIX_AT                 5
#define IMATRIX_AT                 6
#define U8MATRIX_AT                7
#define GSL_MATRIX_AT              10
#define GSL_FLOAT_MATRIX_AT        11
#define GSL_UCHAR_MATRIX_AT        12
#define GSL_USHORT_MATRIX_AT       13
#define GSL_DOUBLE_MATRIX_AT       14
#define GSL_VECTOR_AT              15
#define GSL_FLOAT_VECTOR_AT        16
#define GSL_UCHAR_VECTOR_AT        17
#define GSL_USHORT_VECTOR_AT       18
#define GSL_DOUBLE_VECTOR_AT       19
#define MX_ARRAY_AT                30
#define MATLAB_ARRAY_AT            40
#define OBJECT_3D_AT               50

/**@brief Array link.
 * 
 * Note: This data structure is obsolete and tz_arrayview.h is preferred.
 *
 * This data structure is for transforming different array data structures, 
 * which have a common architecture. They all have
 * one and only one field for storing a data array, such as matrix, vector, and
 * pixel array. Other fields of them are information of the array. So we would
 * like to transform one of the data structures to another without making a new
 * copy of the array field. In Array_Link, <data> is a field referring to a
 * certain array data structure and <type> indicates what kind of array it is.
 */
typedef struct tag_Array_Link {
  void *data;
  int type;
} Array_Link;

/**@brief Attach an array.
 * 
 * Attach_Array() attaches a raw array to the structured array in an array link.
 * The old attached raw array will be returned. Detach_Array() detaches the raw
 * array and  returns it.
 */
void* Attach_Array(Array_Link *a, void *array);
void* Detach_Array(Array_Link *a);

/* 
 * New_Array_Link() news an array link with a certain type. It does not new a
 * raw array. Delete_Array_Link() frees the space of an array link, but any raw
 * array attached will be kept intact and returned.
 */
Array_Link* New_Array_Link(int type);
void* Delete_Array_Link(Array_Link *a);

/*
 * Get_xxx_At() is a set of funtions of getting a structured array from an 
 * array link.
 */
Stack* Get_Stack_At(Array_Link *a);
Image* Get_Image_At(Array_Link *a); 

#ifdef HAVE_LIBGSL
gsl_matrix_uchar* Get_Gmuc_At(Array_Link *a);
gsl_matrix_ushort* Get_Gmus_At(Array_Link *a);
gsl_matrix_float* Get_Gmf_At(Array_Link *a);
gsl_matrix* Get_Gmd_At(Array_Link *a);
gsl_vector_uchar* Get_Gvuc_At(Array_Link *a);
gsl_vector_ushort* Get_Gvus_At(Array_Link *a);
gsl_vector_float* Get_Gvf_At(Array_Link *a);
gsl_vector* Get_Gvd_At(Array_Link *a);
#endif

DMatrix* Get_DMatrix_At(Array_Link *a);
FMatrix* Get_FMatrix_At(Array_Link *a);
IMatrix* Get_IMatrix_At(Array_Link *a);
U8Matrix* Get_U8Matrix_At(Array_Link *a);
Object_3d* Get_Object_3d_At(Array_Link *a);

/*
 * xxx1_To_xxx2() is a set of transforming functions. After transformation,
 * one can use Get_xxx2_At() to get the transformed array. Note that the
 * transformed array should not be freed directly, use Free_Array_Link() to
 * free the corresponding array link instead.
 */
Array_Link* Image_To_Stack(Image *x);
Array_Link* Image_To_Gm(Image *x);
Array_Link* Image_To_Gv(Image *x);
Array_Link* Image_To_Matrix(Image *x);

Array_Link* Stack_To_Image(Stack *x);
Array_Link* Stack_Slice_To_Image(Stack *x, int slice);
Array_Link* Stack_To_Gm(Stack *x);
Array_Link* Stack_To_Gv(Stack *x);

Array_Link* Mar_To_Image(Matlab_Array *x);
Array_Link* Mar_To_Stack(Matlab_Array *x);
Array_Link* Mar_To_DMatrix(Matlab_Array *x);
Array_Link* Mar_To_Object_3d(Matlab_Array *x);

Array_Link* DMatrix_To_Gm(DMatrix *x);
Array_Link* FMatrix_To_Gm(FMatrix *x);
Array_Link* FMatrix_To_Stack(FMatrix *x);

__END_DECLS

#endif
