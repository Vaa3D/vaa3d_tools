#include <mex.h>
#include <string.h>
#include "tz_mexutils.h"
#include "../c/tz_image_lib.h"
#include "tz_mexarraytrans.h"

/**
 * Find the threshold of a stack.
 * Output : integer
 * Input : stack file or bundle file.
 */
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  if (nrhs < 1 || nrhs > 3)
    mexErrMsgTxt("The function takes 1~3 arguments.");

  if ( !mxIsUint8(prhs[0]) && !mxIsUint16(prhs[0]) )
    mexErrMsgTxt("The argument must be an uint8 or uint16 array.");
  
  if (nrhs == 2) {
    if ( !tz_mxIsDoubleScalar(prhs[1]) )
      mexErrMsgTxt("The second argument must be a doulbe scalar");
  }

  if (nrhs == 3) {
    if ( !tz_mxIsDoubleScalar(prhs[2]) )
      mexErrMsgTxt("The third argument must be a doulbe scalar");
  }

  Array_Link *a = MxArray_To_Stack(prhs[0]);
  Stack *stack = Get_Stack_At(a);
  
  int *hist,com;
  int low,high;

  if( stack->kind==GREY || stack->kind==GREY16 ) {
    hist = Stack_Hist(stack);

    if (nrhs == 1) {
      low = hist[1];
      high = hist[0]+hist[1]-1;
    } else if (nrhs == 2) {
      low = (int) (*mxGetPr(prhs[1]));
      high = hist[0]+hist[1]-1;
    } else {
      low = (int) (*mxGetPr(prhs[1]));
      high = (int) (*mxGetPr(prhs[2]));
    }
    
    com = Hist_Rcthre(hist, low, high);

    free(hist);
  } else {
    Delete_Array_Link(a);
    mexErrMsgTxt("Unsupported image format.");
  }
  
  uint8 *thre;
  uint16 *thre16;

  switch(stack->kind) {
  case GREY:
    plhs[0] = mxCreateNumericMatrix(1,1,mxUINT8_CLASS,mxREAL);
    thre = (uint8 *) mxGetPr(plhs[0]);
    *thre = com;

    break;
  case GREY16:
    plhs[0] = mxCreateNumericMatrix(1,1,mxUINT16_CLASS,mxREAL);
    thre16 = (uint16 *) mxGetPr(plhs[0]);
    *thre16 = com;
    break;
  defaut:
    Delete_Array_Link(a);
    mexErrMsgTxt("Unsupported image format.");
  }

  Delete_Array_Link(a);
}
