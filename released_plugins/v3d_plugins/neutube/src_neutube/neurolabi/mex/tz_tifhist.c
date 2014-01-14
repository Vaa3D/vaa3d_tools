#include <mex.h>
#include <string.h>
#include "tz_mexutils.h"
#include "../c/tz_image_lib_defs.h"
#include "tz_stack_lib.h"

/**
 * Find the background of a tif stack.
 * Output : integer
 * Input : stack file or bundle file.
 */
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  if (nrhs != 1)
    mexErrMsgTxt("The function takes 1 argument.");

  if ( !mxIsChar(prhs[0]) )
    mexErrMsgTxt("The argument must be a string");
  
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

  if( stack->kind==GREY || stack->kind==GREY16 ) {
    int *hist = Stack_Hist(stack);
    int hist_size = hist[0] + 2;
    plhs[0] = mxCreateNumericMatrix(hist_size,1,mxINT32_CLASS,mxREAL);
    int i;
    int32_t *hist_pointer = (int32_t *) mxGetPr(plhs[0]);
    for(i = 0; i < hist_size; i++)
      hist_pointer[i] = hist[i];
    free(hist);
    Kill_Stack(stack);
  } else {
    Kill_Stack(stack);
    mexErrMsgTxt("Unsupported image format.");
  }
}
