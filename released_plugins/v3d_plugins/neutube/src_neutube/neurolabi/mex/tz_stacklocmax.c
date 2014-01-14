/* tz_stacklocmax.c
 *
 * 15-Oct-2007 Initial write: Ting Zhao
 */

#include <mex.h>
#include "tz_mexutils.h"
#include "tz_mexarraytrans.h"
#include "../c/tz_image_lib.h"

static void check_argin(int nrhs, const mxArray *prhs[], int *option)
{
  if ((nrhs < 1) || (nrhs > 2)) {
    mexErrMsgTxt("tz_stacklocmax takes 1~2 argument.");
  }

  if (!mxIsUint8(prhs[0]) && !mxIsUint16(prhs[0]) && !mxIsSingle(prhs[0])
      && !mxIsDouble(prhs[0]))
    mexErrMsgTxt("The first argument must be an uint8, uint16, single or double array.");

  mwSize nd = mxGetNumberOfDimensions(prhs[0]);
  if (nd > 3) {
    mexErrMsgTxt("The first argument must have no greater than 3 dimensions.");
  }

  if (nrhs == 2) {
    if (!mxIsChar(prhs[1])) {
      mexErrMsgTxt("The second argument must be a string.");
    } else {
      int strlen = tz_mxGetL(prhs[1]) + 1;
      char str[10];
      mxGetString(prhs[1], str, strlen);
      if (strcmp(str, "center") == 0) {
	*option = STACK_LOCMAX_CENTER;
      } else if (strcmp(str, "neighbor") == 0) {
	*option = STACK_LOCMAX_NEIGHBOR;
      } else if (strcmp(str, "nonflat") == 0) {
	*option = STACK_LOCMAX_NONFLAT;
      } else if (strcmp(str, "alter1") == 0) {
	*option = STACK_LOCMAX_ALTER1;
      } else if (strcmp(str, "alter2") == 0) {
	*option = STACK_LOCMAX_ALTER2;
      } else {
	mexErrMsgTxt("The second argument has an invalid value.");
      }
    }
  } else {
    *option = STACK_LOCMAX_ALTER1;
  }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  int option;

  check_argin(nrhs, prhs, &option);
  
  Array_Link *a = MxArray_To_Stack(prhs[0]);
  mwSize dims[3];
  dims[0] = Get_Stack_At(a)->width;
  dims[1] = Get_Stack_At(a)->height;
  dims[2] = Get_Stack_At(a)->depth;

  plhs[0] = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);
  Array_Link *aout = MxArray_To_Stack(plhs[0]);

  Stack_Local_Max(Get_Stack_At(a), Get_Stack_At(aout), option);

#if 0
  Print_Stack_Value(Get_Stack_At(a));
  Print_Stack_Value(Get_Stack_At(aout));
#endif

  Delete_Array_Link(a);
  Delete_Array_Link(aout);
}
