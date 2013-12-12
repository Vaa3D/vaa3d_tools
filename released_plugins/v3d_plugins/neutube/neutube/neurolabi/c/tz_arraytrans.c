#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_matlabdefs.h"
#include "tz_image_lib.h"
#include "tz_arraytrans.h"

/* get_array(): Get the attached array.
 *
 * Args: x - array data;
 *       type - data type.
 *
 * Return: a void pointer to the array.
 */
static void* get_array(void *x, int type)
{
  switch (type) {
  case IMAGE_AT:
    return ((Image*) x)->array;
  case STACK_AT:
    return ((Stack*) x)->array;
  case DMATRIX_AT:
    return ((DMatrix*) x)->array;
  case FMATRIX_AT:
    return ((FMatrix*) x)->array;
  case IMATRIX_AT:
    return ((IMatrix*) x)->array;
  case U8MATRIX_AT:
    return ((U8Matrix*) x)->array;
#ifdef HAVE_LIBGSL
  case GSL_UCHAR_MATRIX_AT:
    return ((gsl_matrix_uchar*) x)->data;
#endif
  case MATLAB_ARRAY_AT:
    return ((Matlab_Array*) x)->array;
  case OBJECT_3D_AT:
    return ((Object_3d*) x)->voxels;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return NULL;
}

/* set_array(): Attach an array.
 *
 * Args: x - array data;
 *       type - data type;
 *       array - array to attach.
 *
 * Return: void.
 */
static void set_array(void *x, int type, void *array)
{
  switch (type) {
  case IMAGE_AT:
    ((Image*) x)->array = (uint8*)array;
    break;
  case STACK_AT:
    ((Stack*) x)->array = (uint8*)array;
    break;
  case DMATRIX_AT:
    ((DMatrix*) x)->array = (double*)array;
    break;
  case FMATRIX_AT:
    ((FMatrix*) x)->array = (float*)array;
    break;
  case IMATRIX_AT:
    ((IMatrix*) x)->array = (int*)array;
    break;
  case U8MATRIX_AT:
    ((U8Matrix*) x)->array = (tz_uint8*)array;
    break;
  case MATLAB_ARRAY_AT:
    ((Matlab_Array*) x)->array = array;
    break;
  case OBJECT_3D_AT:
    ((Object_3d*) x)->voxels = (Voxel_t (*))array;
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }  
}

/* Get_XX_At: Get array data from an array link.
 *
 * Note: Try to avoid assigning the returned value to a lvalue.
 *
 * Args: a - input array link.
 *
 * Return: a T type pointer.
 */

#define DEFINE_GET_DATA_AT(T, name, flag)			\
  T * TZ_CONCAT3(Get_, name, _At) (Array_Link *a)		\
  {								\
    if (a->type != flag)					\
      TZ_ERROR(ERROR_DATA_TYPE);				\
    else {							\
      return (T *) a->data;					\
    }								\
    return NULL;						\
  }    

DEFINE_GET_DATA_AT(Stack, Stack, STACK_AT)
DEFINE_GET_DATA_AT(Image, Image, IMAGE_AT)
#ifdef HAVE_LIBGSL
DEFINE_GET_DATA_AT(gsl_matrix_uchar, Gmuc, GSL_UCHAR_MATRIX_AT)
DEFINE_GET_DATA_AT(gsl_matrix_ushort, Gmus, GSL_USHORT_MATRIX_AT)
DEFINE_GET_DATA_AT(gsl_matrix_float, Gmf, GSL_FLOAT_MATRIX_AT)
DEFINE_GET_DATA_AT(gsl_matrix, Gmd, GSL_DOUBLE_MATRIX_AT)
DEFINE_GET_DATA_AT(gsl_vector_uchar, Gvuc, GSL_UCHAR_VECTOR_AT)
DEFINE_GET_DATA_AT(gsl_vector_ushort, Gvus, GSL_USHORT_VECTOR_AT)
DEFINE_GET_DATA_AT(gsl_vector_float, Gvf, GSL_FLOAT_VECTOR_AT)
DEFINE_GET_DATA_AT(gsl_vector, Gvd, GSL_DOUBLE_VECTOR_AT)
#endif
DEFINE_GET_DATA_AT(DMatrix, DMatrix, DMATRIX_AT)
DEFINE_GET_DATA_AT(FMatrix, FMatrix, FMATRIX_AT)
DEFINE_GET_DATA_AT(IMatrix, IMatrix, IMATRIX_AT)
DEFINE_GET_DATA_AT(U8Matrix, U8Matrix, U8MATRIX_AT)
DEFINE_GET_DATA_AT(Matlab_Array, Mar, MATLAB_ARRAY_AT)
DEFINE_GET_DATA_AT(Object_3d, Object_3d, OBJECT_3D_AT)

/**
 * Attach an array to a data structure. The old array is returned.
 */
void* Attach_Array(Array_Link *a, void *array)
{
  void *oldArray = get_array(a->data, a->type);
  set_array(a->data, a->type, array);

  return oldArray;
}

/* Detach an array from a data structure. The pointer to the 
 * detached array is returned.
 */
void* Detach_Array(Array_Link *a) 
{
  return Attach_Array(a, NULL);
}

/* new_data(): New an array data.
 *
 * Note: No initialization is done for the data.
 *
 * Args: type - data type.
 *
 * Return: a void pointer to the new data.
 */
static void* new_data(int type)
{
  switch (type) {
  case IMAGE_AT:
    return Guarded_Malloc(sizeof(Image), "new_data");
  case STACK_AT:
    return Guarded_Malloc(sizeof(Stack), "new_data");
  case DMATRIX_AT:
    return Guarded_Malloc(sizeof(DMatrix), "new_data");
  case FMATRIX_AT:
    return Guarded_Malloc(sizeof(FMatrix), "new_data");
  case IMATRIX_AT:
    return Guarded_Malloc(sizeof(IMatrix), "new_data");
  case U8MATRIX_AT:
    return Guarded_Malloc(sizeof(U8Matrix), "new_data");  
#ifdef HAVE_LIBGSL
  case GSL_UCHAR_MATRIX_AT:
    return Guarded_Malloc(sizeof(gsl_matrix_uchar), "new_data");
#endif
  case MATLAB_ARRAY_AT:
    return Guarded_Malloc(sizeof(Matlab_Array), "new_data");
  case OBJECT_3D_AT:
    return Guarded_Malloc(sizeof(Object_3d), "new_data");
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return NULL;
}

/* New_Array_Link(): New an array link.
 *
 * Args: type - type of the array.
 *
 * Return: an array link pointer.
 */
Array_Link* New_Array_Link(int type)
{
  Array_Link *a = (Array_Link *) Guarded_Malloc(sizeof(Array_Link), 
						"New_Array_Link");
  a->type = type;
  a->data = new_data(a->type);
  
  return a;
}

/* Delete_Array_Link(): Free an array link.
 *
 * Note: The memory of the  attached array is not freed.
 *
 * Args: a - input array link.
 *
 * Return: a pointer to the attached array.
 */
void* Delete_Array_Link(Array_Link *a)
{
  void *array = get_array(a->data, a->type); /* array to return */
  free(a->data);
  free(a);
  return array;
}

/* Image_To_Stack(): Transform a stack to an image. 
 *
 * Note: After transformation, the stack will be attached with the data in the
 *       image. The transformed data will not be put into object management, so
 *       Free_Stack or Kill_Stack cannot be used for the stack. 
 *
 * Args: x - input image
 *
 * Return: the pointer to the stack transformed from the input image.  
 *
 */
Array_Link* Image_To_Stack(Image *x)
{
  Array_Link *a = New_Array_Link(STACK_AT);
  Stack *stack = (Stack *) a->data;

  stack->width = x->width;
  stack->height = x->height;
  stack->kind = x->kind;
  stack->depth = 1;
  set_array(stack, STACK_AT, get_array(x, IMAGE_AT));

  return a;
}

#define TZ_SET_MATRIX(m, nd, dim)			\
  {							\
    m->ndim = nd;					\
    memcpy(m->dim, dim, sizeof(dim_type) * nd);	\
  }

/* Image_To_Matrix(): Transform an image to a matrix.
 *
 * Args: x - image to transform.
 * 
 * Return: an array link of a matrix (FMatrix or U8Matrix).
 */
Array_Link* Image_To_Matrix(Image *x)
{
  Array_Link *a = NULL;

  dim_type dim[3];

  switch (x->kind) {
  case GREY:
    dim[0] = x->width;
    dim[1] = x->height;
    a = New_Array_Link(U8MATRIX_AT);
    TZ_SET_MATRIX(Get_U8Matrix_At(a), 2, dim);
    set_array(a, U8MATRIX_AT, get_array(x, IMAGE_AT));
    break;
  case FLOAT32:					
    dim[0] = x->width;
    dim[1] = x->height;
    a = New_Array_Link(FMATRIX_AT);
    TZ_SET_MATRIX(Get_FMatrix_At(a), 2, dim);
    set_array(a, FMATRIX_AT, get_array(x, IMAGE_AT));
    break;
  case COLOR:
    dim[0] = 3;
    dim[1] = x->width;
    dim[2] = x->height;
    a = New_Array_Link(U8MATRIX_AT);
    TZ_SET_MATRIX(Get_U8Matrix_At(a), 3, dim);
    set_array(a, U8MATRIX_AT, get_array(x, IMAGE_AT));
    break;
  default:
    TZ_WARN(ERROR_DATA_COMPTB);
  }

  return a;
}

#ifdef HAVE_LIBGSL

#define TZ_SET_GM(gm, set_size1, set_size2, set_data, T)	\
  {								\
    gm->size1 = (size_t) set_size1;				\
    gm->size2 = (size_t) set_size2;				\
    gm->tda = gm->size2;					\
    gm->data = (T *) set_data;					\
    gm->block->size = set_size1 * set_size2;			\
    gm->block->data = (void *) gm->data;			\
    gm->owner = 0;						\
  }

/* Image_To_Gm(): Convert an image to a gsl matrix
 *
 * Args: x - input image, whose kind should not be COLOR.
 *
 * Return: an array link.
 */
Array_Link* Image_To_Gm(Image *x)
{
  Array_Link *a = NULL; /* array link to return */

  switch (x->kind) {
  case GREY:
    a = New_Array_Link(GSL_UCHAR_MATRIX_AT);
    TZ_SET_GM(Get_Gmuc_At(a), x->width, x->height, x->array, unsigned char);
    break;
  case GREY16:
    a = New_Array_Link(GSL_USHORT_MATRIX_AT);
    TZ_SET_GM(Get_Gmus_At(a), x->width, x->height, x->array, unsigned short);
    break;
  case FLOAT32:
    a = New_Array_Link(GSL_FLOAT_MATRIX_AT);
    TZ_SET_GM(Get_Gmf_At(a), x->width, x->height, x->array, float);
    break;
  default:
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  return a;
}

#define TZ_SET_GV(gv, set_size, set_data, T)			\
  {								\
    gv->size = (size_t) set_size;				\
    gv->stride = 0;						\
    gv->data = (T *) set_data;					\
    gv->block->size = gv->size;					\
    gv->block->data = (void *) gv->data;			\
    gv->owner = 0;						\
  }

/* Image_To_Gv(): Convert an image to a gsl vector.
 *
 * Args: x - input image.
 *
 * Return: an array link.
 */
Array_Link* Image_To_Gv(Image *x)
{
  Array_Link *a = NULL; /* array link to return */

  switch (x->kind) {
  case GREY:
    a = New_Array_Link(GSL_UCHAR_VECTOR_AT);
    TZ_SET_GV(Get_Gvuc_At(a), x->width * x->height, x->array, unsigned char);
    break;
  case GREY16:
    a = New_Array_Link(GSL_USHORT_VECTOR_AT);
    TZ_SET_GV(Get_Gvus_At(a), x->width * x->height, x->array, unsigned short);
    break;
  case FLOAT32:
    a = New_Array_Link(GSL_FLOAT_VECTOR_AT);
    TZ_SET_GV(Get_Gvf_At(a), x->width * x->height, x->array, float);
    break;
  case COLOR:
    a = New_Array_Link(GSL_FLOAT_VECTOR_AT);
    TZ_SET_GV(Get_Gvuc_At(a), x->width * x->height * x->kind, x->array, 
	      unsigned char);
    break;
  default:
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  return a;
}

#endif

/* Stack_To_Image(): Transform a stack to an image
 *
 * Note: After transformation, the image will be attached with the data in the
 *       stack. The transformed data will not be put into object management, so
 *       Free_Image or Kill_Image cannot be used for the stack. 
 *       
*  Args: x - input stack. The depth of the stack must be only one.
 *
 * Return: an array link.
 */
Array_Link* Stack_To_Image(Stack *x)
{
  Array_Link *a = New_Array_Link(IMAGE_AT);
  Image *image = (Image*)(a->data);

  if (x->depth != 1) {
    TZ_WARN(ERROR_DATA_COMPTB);
    set_array(image, IMAGE_AT, NULL);
  }
  else {
    image->width = x->width;
    image->height = x->height;
    image->kind = x->kind;
    set_array(image, IMAGE_AT, get_array(x, STACK_AT));
  }

  return a;
}

/* Stack_Slice_To_Image(): Turn a slice of a stack to an image.
 */
Array_Link* Stack_Slice_To_Image(Stack *x, int slice)
{
  Array_Link *a = New_Array_Link(IMAGE_AT);
  Image *image = (Image*)(a->data);

  image->width = x->width;
  image->height = x->height;
  image->kind = x->kind;
  set_array(image, IMAGE_AT, 
	    (byte_t*)get_array(x, STACK_AT) + 
	    x->width * x->height * x->kind * slice);
 
  return a;  
}

/* Stack_To_Matrix(): Transform an stack to a matrix.
 *
 * Args: x - stack to transform.
 * 
 * Return: an array link of a matrix (FMatrix or U8Matrix).
 */
Array_Link* Stack_To_Matrix(Stack *x)
{
  Array_Link *a = NULL;

  dim_type dim[4];

  switch (x->kind) {
  case GREY:
    dim[0] = x->width;
    dim[1] = x->height;
    dim[2] = x->depth;
    a = New_Array_Link(U8MATRIX_AT);
    TZ_SET_MATRIX(Get_U8Matrix_At(a), 2, dim);
    set_array(a, U8MATRIX_AT, get_array(x, STACK_AT));
    break;
  case FLOAT32:					
    dim[0] = x->width;
    dim[1] = x->height;
    dim[2] = x->depth;
    a = New_Array_Link(FMATRIX_AT);
    TZ_SET_MATRIX(Get_FMatrix_At(a), 2, dim);
    set_array(a, FMATRIX_AT, get_array(x, STACK_AT));
    break;
  case COLOR:
    dim[0] = 3;
    dim[1] = x->width;
    dim[2] = x->height;
    dim[3] = x->depth;
    a = New_Array_Link(U8MATRIX_AT);
    TZ_SET_MATRIX(Get_U8Matrix_At(a), 3, dim);
    set_array(a, U8MATRIX_AT, get_array(x, STACK_AT));
    break;
  default:
    TZ_WARN(ERROR_DATA_COMPTB);
  }

  return a;
}

#ifdef HAVE_LIBGSL
/* Stack_To_Gm(): Tranform a stack to a gsl matrix.
 * 
 * Args: x - input  stack, whose depth must be 1.
 *
 * Return: an array link.
 */
Array_Link* Stack_To_Gm(Stack *x)
{
  Array_Link *a = Stack_To_Image(x);
  Array_Link *a2 = Image_To_Gm(Get_Image_At(a));
  Delete_Array_Link(a);
  return a2;
}

/* Stack_To_Gv(): Transform a stack to an image
 *       
 * Args: x - input stack. 
 *
 * Return: an array link.
 */
Array_Link* Stack_To_Gv(Stack *x)
{
  Array_Link *a = NULL; /* array link to return */
  int array_length = x->width * x->height *x->depth;

  switch (x->kind) {
  case GREY:
    a = New_Array_Link(GSL_UCHAR_VECTOR_AT);
    TZ_SET_GV(Get_Gvuc_At(a), array_length, x->array, unsigned char);
    break;
  case GREY16:
    a = New_Array_Link(GSL_USHORT_VECTOR_AT);
    TZ_SET_GV(Get_Gvus_At(a), array_length, x->array, unsigned short);
    break;
  case FLOAT32:
    a = New_Array_Link(GSL_FLOAT_VECTOR_AT);
    TZ_SET_GV(Get_Gvf_At(a), array_length, x->array, float);
    break;
  case COLOR:
    a = New_Array_Link(GSL_FLOAT_VECTOR_AT);
    TZ_SET_GV(Get_Gvuc_At(a), array_length * x->kind, x->array, unsigned char);
    break;
  default:
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  return a;
}

#endif

/* Mar_To_Image(): Transform a matlab array to an image.
 *
 * Args: x - input matlab array.
 *
 * Return: an array link.
 */
Array_Link* Mar_To_Image(Matlab_Array *x)
{
  Array_Link *a = NULL; /* array link to return */

  /* check if the transformation is possible */
  if (x->ndim > 2)
    TZ_ERROR(ERROR_DATA_COMPTB);

  if ((x->type != mxUINT8_CLASS) && (x->type != mxUINT16_CLASS) && 
      (x->type != mxSINGLE_CLASS))
    TZ_ERROR(ERROR_DATA_COMPTB);
  
  a = New_Array_Link(IMAGE_AT);
  Get_Image_At(a)->width = x->dim[0];
  
  if (x->ndim == 1) {
    Get_Image_At(a)->height = 1;
  } else {
    Get_Image_At(a)->height = x->dim[1];
  }

  switch (x->type) {
  case mxUINT8_CLASS:
    Get_Image_At(a)->kind = GREY;
    break;
  case mxUINT16_CLASS:
    Get_Image_At(a)->kind = GREY16;
    break;
  case mxSINGLE_CLASS:
    Get_Image_At(a)->kind = FLOAT32;
    break;
  default:
    Delete_Array_Link(a);
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  Attach_Array(a, get_array(x, MATLAB_ARRAY_AT));

  return a;
}

/* Mar_To_Stack(): Transform a matlab array to a stack.
 *
 * Args: x - input matlab array.
 *
 * Return: an array link.
 */
Array_Link* Mar_To_Stack(Matlab_Array *x)
{
  Array_Link *a = NULL; /* array link to return */

  /* check if the transformation is possible */
  if (x->ndim > 3)
    TZ_ERROR(ERROR_DATA_COMPTB);

  if ((x->type != mxUINT8_CLASS) && (x->type != mxUINT16_CLASS) && 
      (x->type != mxSINGLE_CLASS) && (x->type != mxDOUBLE_CLASS))
    TZ_ERROR(ERROR_DATA_COMPTB);
  
  a = New_Array_Link(STACK_AT);
  Get_Stack_At(a)->width = x->dim[0];
  
  if (x->ndim == 1) {
    Get_Stack_At(a)->height = 1;
  } else {
    Get_Stack_At(a)->height = x->dim[1];
  }

  if (x->ndim == 2) {
    Get_Stack_At(a)->depth = 1;
  } else {
    Get_Stack_At(a)->depth = x->dim[2];
  }

  switch (x->type) {
  case mxUINT8_CLASS:
    Get_Stack_At(a)->kind = GREY;
    break;
  case mxUINT16_CLASS:
    Get_Stack_At(a)->kind = GREY16;
    break;
  case mxSINGLE_CLASS:
    Get_Stack_At(a)->kind = FLOAT32;
    break;
  case mxDOUBLE_CLASS:
    Get_Stack_At(a)->kind = FLOAT64;
    break;
  default:
    Delete_Array_Link(a);
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  Attach_Array(a, get_array(x, MATLAB_ARRAY_AT));

  return a;
}

/* Mar_To_DMatrix(): Transform a matlab array to a double matrix.
 *
 * Args: x - input matlab array.
 *
 * Return: an array link.
 */
Array_Link* Mar_To_DMatrix(Matlab_Array *x)
{
  Array_Link *a = NULL; /* array link to return */

  /* check if the transformation is possible */
  if (x->ndim > TZ_MATRIX_MAX_DIM)
    TZ_ERROR(ERROR_DATA_COMPTB);

  if (x->type != mxDOUBLE_CLASS)
    TZ_ERROR(ERROR_DATA_COMPTB);
  
  a = New_Array_Link(DMATRIX_AT);
  Get_DMatrix_At(a)->ndim = x->ndim;

  int i;
  for (i = 0; i < x->ndim; i++) {
    Get_DMatrix_At(a)->dim[i] = x->dim[i];
  }

  Attach_Array(a, get_array(x, MATLAB_ARRAY_AT));

  return a;
}

/* Mar_To_Object_3d(): Transform a matlab array to a 3d object.
 *
 * Args: x - input matlab array.
 *
 * Return: an array link.
 *
 * Note: The function assumes a voxel has int32 array type.
 */
Array_Link* Mar_To_Object_3d(Matlab_Array *x)
{
  Array_Link *a = NULL; /* array link to return */

  /* check if the transformation is possible */
  if (x->ndim != 2)
    TZ_ERROR(ERROR_DATA_COMPTB);

  if (x->dim[0] != 3)
    TZ_ERROR(ERROR_DATA_COMPTB);

  if (x->type != mxINT32_CLASS)
    TZ_ERROR(ERROR_DATA_COMPTB);
  /*****************************************/
  
  a = New_Array_Link(OBJECT_3D_AT);
  Get_Object_3d_At(a)->size = x->dim[1];

  Attach_Array(a, get_array(x, MATLAB_ARRAY_AT));

  return a;
}

#ifdef HAVE_LIBGSL
/* DMatrix_To_Gm(): Transform a double matrix to a gsl matrix.
 *
 * Arguments: x - double matrix to transform.
 *
 * Return: an array link of gsl double matrix.
 */
Array_Link* DMatrix_To_Gm(DMatrix *x)
{
  Array_Link *a = New_Array_Link(GSL_MATRIX_AT);
  int size1 = x->dim[0];
  int size2 = 1;
  if (x->ndim > 1) {
    dim_type d;
    for (d = 1; d < x->ndim; d++) {
      size2 *= x->dim[d];
    }
  }
  TZ_SET_GM(Get_Gmd_At(a), size1, size2, get_array(x, DMATRIX_AT), double);  

  return a;
}
#endif

Array_Link* FMatrix_To_Stack(FMatrix *x)
{
  Array_Link *a = NULL; /* array link to return */

  /* check if the transformation is possible */
  if (x->ndim > 3) {
    TZ_ERROR(ERROR_DATA_COMPTB);
  }
  
  a = New_Array_Link(STACK_AT);
  Get_Stack_At(a)->width = x->dim[0];
  Get_Stack_At(a)->height = x->dim[1];
  Get_Stack_At(a)->depth = x->dim[2];
  Get_Stack_At(a)->kind = FLOAT32;
  
  Attach_Array(a, get_array(x, FMATRIX_AT));

  return a;
}
