/* tz_fcmp.c
 *
 * 19-Nov-2007 Initial write: Ting Zhao
 */

#include <gsl/gsl_math.h>
#include "tz_mexutils.h"

void check_argin(int nrhs, const mxArray *prhs[])
{
  if (nrhs > 3  || nrhs < 2)
    mxErrMsgTxt("TZ_FCMP takes 2~3 arguments.");

  if (!(mxIsDouble(prhs[0]) && mxIsDouble(prhs[1])))
    mxErrMsgTxt("The first two arguments must be both double arrays.");

  if (!tz_mxSameSize(prhs[0], prhs[1]) && !tz_mxIsScalar(prhs[0]) &&
      !tz_mxIsScalar(prhs[1]))
    mxErrMsgTxt("The first two arguments must have the same size if neither of them is a scalar.");

  if (nrhs == 3) {
    if (!tz_mxIsDoubleScalar(prhs[2]))
      mxErrMsgTxt("The third arguments must be a double scalar.");
  }
}

/* tz_fcmp(x1, x2) compare two double float numbers.
 * tz_fcmp(x1, x2, ep) compare the double float numbers at the accuracy ep, 
 * which is 1e-5 by default.
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  check_argin(nrhs, prhs);
  
  double epsilon = 1e-5;

  if (nrhs == 3) {
    epsilon = *mxGetPr(prhs[2]);
  }

  mwSize length;
  mwSize length1 = tz_mxGetL(prhs[0]);
  mwSize length2 = tz_mxGetL(prhs[1]);
  const mxArray *array = NULL;

  if (mxIsScalar(prhs[0]) && mxIsScalar(prhs[1])) {
    plhs[0] = mxCreateNumericMatrix(1, 1, mxINT8_CLASS, mxREAL);
    length = 1;
  } else if (!mxIsScalar(prhs[0]) && !mxIsScalar(prhs[1])) {
    plhs[0] = mxCreateNumericArray(mxGetNumberOfDimensions(prhs[0]), 
				   mxGetDimensions(prhs[0]), mxINT8_CLASS,
				   mxREAL);
    length = length1;
  } else {
    if (length1 == 1) {
      array = prhs[1];
      length = length2;
    } else {
      array = prhs[0];
      length = length1;
    }
    
    plhs[0] = mxCreateNumericArray(mxGetNumberOfDimensions(array), 
				   mxGetDimensions(array), mxINT8_CLASS,
				   mxREAL);
  }

  INT8_T *y = (INT8_T *) mxGetPr(plhs[0]);
  double *x1 = mxGetPr(prhs[0]);
  double *x2 = mxGetPr(prhs[1]);

  mwSize i;
  if (length1 == length2) {
    for (i = 0; i < length; i++) {
      y[i] = (INT8_T) gsl_fcmp(x1[i], x2[i], epsilon);
    }
  } else if (length1 > length2) {
    for (i = 0; i < length; i++) {
      y[i] = (INT8_T) gsl_fcmp(x1[i], x2[0], epsilon);
    }
  } else {
    for (i = 0; i < length; i++) {
      y[i] = (INT8_T) gsl_fcmp(x1[0], x2[i], epsilon);
    }
  }
}
