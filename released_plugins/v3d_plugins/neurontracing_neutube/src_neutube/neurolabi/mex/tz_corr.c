/*********************************
 * tz_convolve.c
 *
 * Mex function of the convolution of 2 double arrays
 *
 * Created by T. Zhao, 17-Sep-2007
 *
 * Version: 1.0
 *
 *********************************/

#include <mex.h>
#include <string.h>

#include "../c/tz_darray.h"
#include "tz_mexutils.h"

void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  double *conv; //temporary array to store convolution
  mwSize length; //length of the convolution

  if(nlhs>1)
    mexErrMsgTxt("Too many outputs.");

  if(nrhs!=2)
    mexErrMsgTxt("Exactly 2 argument is required");
  
  if( !tz_mxIsDoubleVector(prhs[0]) ) {
    mexErrMsgTxt("The first input must be a 1D double array.");
  }

  if( !tz_mxIsDoubleVector(prhs[0]) ) {
    mexErrMsgTxt("The second input must be a 1D array.");
  }
  
  length = tz_mxGetL(prhs[0]) + tz_mxGetL(prhs[1]) - 1; 
  conv = (double *) fftw_malloc(sizeof(fftw_complex)*R2C_LENGTH(length));  
  darray_convolve(mxGetPr(prhs[0]),tz_mxGetL(prhs[0]),
		  mxGetPr(prhs[1]),tz_mxGetL(prhs[1]),1,conv);
  plhs[0] = mxCreateDoubleMatrix(1,length,mxREAL);
  memcpy(mxGetPr(plhs[0]),conv,sizeof(double)*length);
  fftw_free(conv);
}
