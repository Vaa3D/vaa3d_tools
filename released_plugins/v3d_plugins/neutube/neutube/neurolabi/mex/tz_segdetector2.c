/* tz_segdetector2.c
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
  int pos[] = {-1, -1, -1, -1};
  int range[2];
  int flag = 0;
  int i, j;
  
  plhs[0] = mxCreateNumericMatrix(mxGetM(prhs[0]), mxGetN(prhs[0]), 
				  mxINT8_CLASS, mxREAL);
  INT8_T *y = (INT8_T *) mxGetPr(plhs[0]);

  for (i = 0; i < length; i++) {
    if (x[i] == 1) {
      if (pos[0] < 0 || pos[1] < 0)
	pos[0] = i;
      else if (pos[2] < 0) {
	pos[2] = i;
	range[0] = (pos[0]+pos[1])/2;
	range[1] = (pos[1]+pos[2])/2;
	flag = 1;
      } else {
	pos[3] = i;
	range[0] = (pos[0]+pos[1])/2;
	range[1] = (pos[2]+pos[3])/2;
	flag = 1;
      }
    } else if (x[i] == -1) {
      if (pos[0] > 0) {
	if (pos[1] < 0)
	  pos[1] = i;
	else
	  pos[2] = i;
      }
    }

    if (flag) {
      for (j = range[0]; j <= range[1]; j++)
	y[j] = 1;
      pos[1] = pos[2] = pos[3] = -1;
      pos[0] = i;
      flag = 0;
    }
  }
}
