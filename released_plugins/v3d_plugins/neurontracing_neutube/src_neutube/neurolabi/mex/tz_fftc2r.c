#include <mex.h>

#include "../c/tz_darray.h"
#include  "tz_mexutils.h"

void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  fftw_complex *d1;  //input fft array
  double *xr,*xi,*d2; //xr: real part of d1; xi: real part of d1; 
                      //d2: ifft result 
  mwSize mrows,ncols,length,length2; //mrows,ncols for matrix size
                                     //length: length of the input,
                                     //length2: length of the output
  mwSize i;

  if(nlhs>1)
    mexErrMsgTxt("Too many outputs.");

  if(nrhs!=2)
    mexErrMsgTxt("Exactly 2 argument is required");

  mrows = mxGetM(prhs[0]);
  ncols = mxGetN(prhs[0]);
  if( (mrows!=1 && ncols!=1) ) {
    mexErrMsgTxt("The first input must be a 1D array.");
  }

  if( !tz_mxIsDoubleScalar(prhs[1]) )
    mexErrMsgTxt("The second input must be a double scalar.");

  length = (mrows>ncols) ? mrows : ncols;
  length2 = (mwSize) (*mxGetPr(prhs[1]));

  if( length != length2/2+1 ) {
    mexPrintf("length: %d, length2: %d\n",length,length2);
    mexErrMsgTxt("Invalid data length.");
  }

  xr = mxGetPr(prhs[0]);
  xi = mxGetPi(prhs[0]);

  d1 = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*length);

  for(i=0;i<length;i++) {
    d1[i][0] = xr[i];
    d1[i][1] = xi[i];
  }

  d2 = darray_ifft(d1,length2,1,0,0); //in place transformation

  if(mrows>1)
    mrows = length2;
  else
    ncols = length2;

  plhs[0] = mxCreateDoubleMatrix(mrows,ncols,mxREAL);
  double *yr = mxGetPr(plhs[0]);
  
  for(i=0;i<length2;i++)
    yr[i] = d2[i];

  fftw_free(d2);
}
