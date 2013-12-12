/* tz_alignstack.c
 *
 * 12-May-2008 Initial write: Ting Zhao
 */

#include <mex.h>
#include <string.h>
#include "tz_mexutils.h"
#include "../c/tz_fimage_lib.h"
#include "tz_mexarraytrans.h"

/*
 * Align two stacks. tz_alignstack(image1, image2) takes two images (2D or 3D) 
 * and returns the offset of image1 relative to image2 so that they have the 
 * best match.
 * It also returns the score and correlation coefficient of matching.
 */
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  if (nrhs != 2) {
    mexErrMsgTxt("The function takes 2 arguments.");
  }
  
  Array_Link *a1 = MxArray_To_Stack(prhs[0]);
  Stack *stack1 = Get_Stack_At(a1);

  Array_Link *a2 = MxArray_To_Stack(prhs[1]);
  Stack *stack2 = Get_Stack_At(a2);

  float unnorm_maxcorr;
  int offset[3];
  float score = Align_Stack_F(stack1, stack2, offset, &unnorm_maxcorr);

  plhs[0] = mxCreateNumericMatrix(1, 3, mxDOUBLE_CLASS, mxREAL);
  double *return_offset = (double *) mxGetPr(plhs[0]);
  return_offset[0] = (double) (offset[0] - stack1->width + 1);
  return_offset[1] = (double) (offset[1] - stack1->height + 1);
  return_offset[2] = (double) (offset[2] - stack1->depth + 1);

  if (nlhs > 1) {
    plhs[1] = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
    double *return_score = (double *) mxGetPr(plhs[1]);
    *return_score = score;
  }

  if (nlhs > 2) {
    plhs[2] = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
    double *return_corr = (double *) mxGetPr(plhs[2]);
    *return_corr = unnorm_maxcorr;
  }

  Delete_Array_Link(a1);
  Delete_Array_Link(a2);
}
