/* tz_writetifstack.c
 *
 * 12-Dec-2007 Initial write: Ting Zhao
 */

#include <mex.h>
#include <string.h>
#include "image_lib.h"
#include "tz_image_lib_defs.h"
#include "tz_utilities.h"
#include "tz_mexutils.h"
#include "tz_mexarraytrans.h"

/* tz_readtifstack(filepath)
 *
 * Write a tif stack.
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 1) {
    mexErrMsgTxt("The function takes 1 argument exactly.");
  }

  if (!mxIsChar(prhs[0])) {
    mexErrMsgTxt("The first argument must be a string.");
  }
  
  char *file_path = mxArrayToString(prhs[0]);

  if (!fexist(file_path)) {
    mexErrMsgTxt("Cannot find the file.");
  }

  Stack *stack = Read_Stack(file_path);

  mwSize dims[4];
  mwSize dim_offset = 0;
  dims[0] = stack->kind;
  dims[1] = stack->width;
  dims[2] = stack->height;
  dims[3] = stack->depth;

  mwSize ndim = 3;
  if (stack->depth == 1) {
    ndim--;
  }
  
  if (stack->kind != COLOR) {
    dim_offset++;
  } else {
    ndim++;
  }

  switch (stack->kind) {
  case GREY:
    plhs[0] = mxCreateNumericArray(ndim, dims + dim_offset, 
				   mxUINT8_CLASS, mxREAL);
    break;
  case GREY16:
    plhs[0] = mxCreateNumericArray(ndim, dims + dim_offset, 
				   mxUINT16_CLASS, mxREAL);
    break;
  case FLOAT32:
    plhs[0] = mxCreateNumericArray(ndim, dims + dim_offset, 
				   mxSINGLE_CLASS, mxREAL);
    break;
  case FLOAT64:
    plhs[0] = mxCreateNumericArray(ndim, dims + dim_offset, 
				   mxDOUBLE_CLASS, mxREAL);
    break;
  case COLOR:
    plhs[0] = mxCreateNumericArray(ndim, dims, mxUINT8_CLASS, mxREAL);
    break;
  default:
    Kill_Stack(stack);
    mexErrMsgTxt("Unsupported image kind.");
  }

  void *dst = (void *) mxGetPr(plhs[0]);
  memcpy(dst, stack->array, mxGetElementSize(plhs[0]) *
	 mxGetNumberOfElements(plhs[0]));

  Kill_Stack(stack);
}
