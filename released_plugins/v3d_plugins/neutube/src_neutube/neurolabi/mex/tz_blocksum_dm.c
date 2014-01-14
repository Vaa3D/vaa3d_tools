#include <mex.h>
#include "tz_mexutils.h"
#include "tz_mexarraytrans.h"

static void check_argin(int nrhs, const mxArray *prhs[])
{
  if (nrhs != 2)
    mxErrMsgTxt("TZ_BLOCKSUM_DM takes 2 arguments.");

  if (!mxIsDouble(prhs[0]))
    mxErrMsgTxt("The first argument must be a double matrix.");

  mwSize nd = mxGetNumberOfDimensions(prhs[0]);
  if (nd > 3)
    mexErrMsgTxt("Dimensions too high");

  if (!tz_mxIsDoubleVector(prhs[1]))
    mxErrMsgTxt("The second argument must be a double vector.");

  if (tz_mxGetL(prhs[1]) != nd)
    mxErrMsgTxt("The second argument has wrong size.");
}

/* Block sum of double matrix.
 * TZ_BLOCKSUM_DM(x, bdim)
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  check_argin(nrhs, prhs);
  
  Array_Link *a = MxArray_To_DMatrix(prhs[0]);
  DMatrix *dm = Get_DMatrix_At(a);

  mwSize ndim = (mwSize) dm->ndim;
  mwSize dims[MAX_DIM];
  dim_type bdim[MAX_DIM];
  mwSize idim;

  double *bdim_in = (double *) mxGetPr(prhs[1]);
  
  for (idim = 0; idim < ndim; idim++) {
    bdim[idim] = (dim_type) bdim_in[idim];
    dims[idim] = (mwSize) (bdim[idim] + dm->dim[idim] - 1); 
  }

  plhs[0] = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxREAL);
  Array_Link *a2 = MxArray_To_DMatrix(plhs[0]);
  DMatrix_Blocksum(dm, bdim, Get_DMatrix_At(a2));
  Delete_Array_Link(a);
  Delete_Array_Link(a2);
}
