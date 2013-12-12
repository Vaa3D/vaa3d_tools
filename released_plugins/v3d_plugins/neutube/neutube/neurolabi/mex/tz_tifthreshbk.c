#include <mex.h>
#include "tz_mexutils.h"
//#include "../c/tz_image_lib.h"

/**
 * Find the threshold of a tif stack.
 * Output : integer
 * Input : stack file or directory (without extension).
 */
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  if( nrhs!=1 )
    mexErrMsgTxt("The function takes exactly 1 argument.");

  if( !mxIsChar(prhs[0]) )
    mexErrMsgTxt("The argument must be a string");

  /*
  Stack *stack = Read_Stack((char *)mxGetPr(prhs[0]));
  int *hist,com;

  if( stack->kind==GREY || stack->kind==GREY16 ) {
    hist = Stack_Hist(stack);
    com = Hist_Most_Common(hist,hist[1],hist[0]+hist[1]-1);
    free(hist);
  }
  
  uint8 *thre;
  uint16 *thre16;

  switch(stack->kind) {
  case GREY:
    plhs[0] = (mxArray *) mxCreateNumericalArray(1,1,mxINT8_CLASS,mxREAL);
    thre = (uint8 *) mxGetPr(plhs[0]);
    *thre = com;
    break;
  case GREY16:
    plhs[0] = (mxArray *) mxCreateNumericalArray(1,1,mxINT16_CLASS,mxREAL);
    thre16 = (uint16 *) mxGetPr(plhs[0]);
    *thre16 = com;
    break;
  defaut:
    Kill_Stack(stack);
    mexErrMsgTxt("Unsupported image format.");
  }

  Kill_Stack(stack);*/
}
