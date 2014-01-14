#include <mex.h>
#include <string.h>
#include "tz_mexutils.h"
#include "../c/tz_image_lib.h"

/**
 * Find the threshold of a tif stack.
 * Output : integer
 * Input : stack file or bundle file.
 */
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  if (nrhs < 1 || nrhs > 3)
    mexErrMsgTxt("The function takes 1~3 arguments.");

  if ( !mxIsChar(prhs[0]) )
    mexErrMsgTxt("The argument must be a string");
  
  if (nrhs == 2) {
    if ( !tz_mxIsDoubleScalar(prhs[1]) )
      mexErrMsgTxt("The second argument must be a doulbe scalar");
  }

  if (nrhs == 3) {
    if ( !tz_mxIsDoubleScalar(prhs[2]) )
      mexErrMsgTxt("The third argument must be a doulbe scalar");
  }

  char *filePath = mxArrayToString(prhs[0]);

  Stack *stack = NULL;

  if( strlen(filePath)>4 ) {
    if( Is_Tiff(filePath) ) /* Read file */
      stack = Read_Stack(filePath);
    else if ( Is_Fbdf(filePath) ) { /* Read directory */
      File_Bundle fb;
      initfb(&fb);
      if( Load_Fbdf(filePath,&fb) )
	stack = Read_Stack_Planes(&fb);
      freefb(&fb);
    } else
      mexErrMsgTxt("Unrecongnized file format.");
  } else {
    mexErrMsgTxt("Unrecongnized file format.");
  }
      
  if( stack==NULL )
    mexErrMsgTxt("Unable to load the stack.");

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
    
    printf("Pick threshold between %d and %d\n", low, high);
    com = Hist_Rcthre(hist, low, high);

    free(hist);
    Kill_Stack(stack);
  } else {
    Kill_Stack(stack);
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
    Kill_Stack(stack);
    mexErrMsgTxt("Unsupported image format.");
  }
}
