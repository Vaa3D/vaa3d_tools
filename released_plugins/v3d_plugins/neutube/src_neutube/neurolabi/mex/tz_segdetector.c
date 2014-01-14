/* tz_segdetector.c
 *
 * 16-Oct-2007
 */

#include <mex.h>
#include "tz_mexutils.h"

void check_argin(int nrhs, const mxArray *prhs[])
{
  if (nrhs > 2  || nrhs < 1)
    mxErrMsgTxt("TZ_SEGDETECTOR takes 1~3 arguments.");

  if (!mxIsInt8(prhs[0]))
    mxErrMsgTxt("The first argument must be an int8 array.");

  if (!tz_mxIsVector(prhs[0]))
    mxErrMsgTxt("The first argument must be a vector.");

  if (nrhs >= 2)
    if (!mxIsInt8(prhs[1]))
      mxErrMsgTxt("The first argument must be an int8 scalar.");
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  check_argin(nrhs, prhs);
  
  INT8_T b = 0;

  if (nrhs == 2)
    b = *((INT8_T *) mxGetPr(prhs[1]));

  INT8_T *x = (INT8_T *) mxGetPr(prhs[0]);
  mwSize length = tz_mxGetL(prhs[0]);
  int pos[] = {0, 0};
  int flag = 0;
  int i, j;
  
  plhs[0] = mxCreateNumericMatrix(mxGetM(prhs[0]), mxGetN(prhs[0]), 
				  mxINT8_CLASS, mxREAL);
  INT8_T *y = (INT8_T *) mxGetPr(plhs[0]);

  for (i = 0; i < length; i++) {
    if (flag) {
      y[i] = 1;
      if (x[i] == 1) {
	flag = 0;
	pos[1] = i;
	if (b > 0) {
	  for (j = pos[0]; j < pos[0] + b; j++)
	    y[j] = 0;
	  for (j = pos[1] - b + 1; j <= pos[1]; j++)
	    y[j] = 0;
	}
      }
    } else {
      if (x[i] == 1) {
	pos[0] = i;
      } else if (x[i] == -1) {
	if (pos[0] > 0) {
	  for (j = pos[0]; j <= i; j++)
	    y[j] = 1;
	  flag = 1;
	}
      }
    }
  }
}
