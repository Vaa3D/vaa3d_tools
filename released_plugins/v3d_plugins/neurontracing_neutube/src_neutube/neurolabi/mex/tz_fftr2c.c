#include <mex.h>
#include "../c/tz_darray.h"
#include "tz_mexutils.h"

void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  double *d1,*yr,*yi;
  fftw_complex *d2;
  mwSize mrows,ncols,length,length2;
  mwSize i;

  if(nlhs>1)
    mexErrMsgTxt("Too many outputs.");

  if(nrhs!=1)
    mexErrMsgTxt("Exactly one argument is required");
  
  if(!tz_mxIsDoubleVector(prhs[0])) {
    mexErrMsgTxt("Input must be a noncomplex double array.");
  }

  mrows = mxGetM(prhs[0]);
  ncols = mxGetN(prhs[0]);

  d1 = mxGetPr(prhs[0]);
  length = (mrows>ncols) ? mrows : ncols;

  d2 = darray_fft(d1,length,0,1);
  length2 = length/2+1;
  
  if(mrows>1)
    mrows = length2;
  else
    ncols = length2;

  plhs[0] = mxCreateDoubleMatrix(mrows,ncols,mxCOMPLEX);
  yr = mxGetPr(plhs[0]);
  yi = mxGetPi(plhs[0]);
  
  for(i=0;i<length2;i++) {
    yr[i] = d2[i][0];
    yi[i] = d2[i][1];
  }

  fftw_free(d2);
}
