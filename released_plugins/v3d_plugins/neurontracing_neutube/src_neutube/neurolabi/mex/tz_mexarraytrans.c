/* tz_mexarraytrans.c
 * 
 * 12-Oct-2007  Initial write:  Ting Zhao
 */

#include "tz_mexarraytrans.h"

#define FLOAT64 8

/* MxArray_To_Image(): Transforam a matlab array to an image.
 *
 * Args: x - input array, which must be mxUINT8_CLASS, mxUINT16_CLASS or  
 *           mxSINGLE_CLASS.
 *
 * Return: an array link.
 */
Array_Link* MxArray_To_Image(const mxArray *x)
{
  Array_Link *a = NULL; /* array link to return */

  if (mxIsEmpty(x))
    mexErrMsgTxt("Empty matrix");

  mwSize d = mxGetNumberOfDimensions(x);
  if (d > 2)
    mexErrMsgTxt("Dimentions too high");

  mwSize m = mxGetM(x);
  mwSize n = mxGetN(x);
  Image *imagex = NULL;

  a = New_Array_Link(IMAGE_AT);
  imagex = Get_Image_At(a);
  imagex->width = m;
  imagex->height = n;
  imagex->text = NULL;

  switch (mxGetClassID(x)) {
  case mxUINT8_CLASS:
    imagex->kind = GREY;
    break;
  case mxUINT16_CLASS:
    imagex->kind = GREY16;
    break;
  case mxSINGLE_CLASS:
    imagex->kind = FLOAT32;
    break;
  case mxDOUBLE_CLASS:
    imagex->kind = FLOAT64;
    break;
  default:
    mexErrMsgTxt("Invalid data class.");
  }

  Attach_Array(a, mxGetPr(x));
  
  return a;
}


/* MxArray_To_Stack(): Transforam a matlab array to a stack.
 *
 * Args: x - input array, which must be mxUINT8_CLASS, mxUINT16_CLASS or  
 *           mxSINGLE_CLASS.
 *
 * Return: an array link.
 */
Array_Link* MxArray_To_Stack(const mxArray *x)
{
  Array_Link *a = NULL; /* array link to return */

  if (mxIsEmpty(x))
    mexErrMsgTxt("Empty matrix");

  mwSize nd = mxGetNumberOfDimensions(x);
  if (nd > 4) {
    mexErrMsgTxt("Dimensions too high");
  } else if (nd == 4) {
    if (mxGetClassID(x) != mxUINT8_CLASS) {
      mexErrMsgTxt("Unsupported data type: should be uint8 for 4D array.");
    }
  }

  const mwSize *ds = mxGetDimensions(x);
  
  a = New_Array_Link(STACK_AT);
  Stack *stackx = Get_Stack_At(a);

  if (nd == 4) {
    ds++;
  }

  stackx->width = ds[0];
  stackx->height = 1;
  stackx->depth = 1;
  stackx->text = NULL;

  if (nd >= 2)
    stackx->height = ds[1];
  
  if (nd >= 3)
    stackx->depth = ds[2];

  switch (mxGetClassID(x)) {
  case mxUINT8_CLASS:
    if (nd == 4) {
      stackx->kind = COLOR;
    } else {
      stackx->kind = GREY;
    }
    break;
  case mxUINT16_CLASS:
    stackx->kind = GREY16;
    break;
  case mxSINGLE_CLASS:
    stackx->kind = FLOAT32;
    break;
  case mxDOUBLE_CLASS:
    stackx->kind = FLOAT64;
    break;
  default:
    Delete_Array_Link(a);
    mexErrMsgTxt("Invalid data class.");
  }

  Attach_Array(a, mxGetPr(x));
  return a;
}

/* MxArray_To_DMatrix(): Transform a matlab array to a double matrix.
 *
 * Args: x - input matlab array.
 *
 * Return: an array link of DMatrix.
 */
Array_Link* MxArray_To_DMatrix(const mxArray *x)
{
  Array_Link *a = NULL; /* array link to return */
  
  if (mxIsEmpty(x))
    mexErrMsgTxt("Empty matrix");

  mwSize nd = mxGetNumberOfDimensions(x);
  if (nd > MAX_DIM)
    mexErrMsgTxt("Dimensions too high");
  
  if (!mxIsDouble(x))
    mexErrMsgTxt("The input argument must be a double array.");

  a = New_Array_Link(DMATRIX_AT);

  DMatrix *dm = Get_DMatrix_At(a);
  dm->ndim = (ndim_type) nd;
  const mwSize *ds = mxGetDimensions(x);
  ndim_type i;
  for (i = 0; i < dm->ndim; i++)
    dm->dim[i] = ds[i];
  
  Attach_Array(a, mxGetPr(x));

  return a;
}
