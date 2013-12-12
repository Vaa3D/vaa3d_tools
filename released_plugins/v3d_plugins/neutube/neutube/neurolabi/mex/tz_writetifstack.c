/* tz_writetifstack.c
 *
 * 12-Dec-2007 Initial write: Ting Zhao
 */

#include <mex.h>
#include "image_lib.h"
#include "tz_mexutils.h"
#include "tz_mexarraytrans.h"

/* tz_writetifstack(stack, filepath)
 *
 * Write tif stack. There is no output.
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 2) {
    mexErrMsgTxt("The function takes 2 argument exactly.");
  }

  if (!mxIsNumeric(prhs[0])) {
    mexErrMsgTxt("The first argument must be a numeric array.");
  }

  if (!mxIsUint8(prhs[0]) && !mxIsUint16(prhs[0])) {
    mexErrMsgTxt("The first argument must be a uint8 or uint16 array.");
  }

  if (!mxIsChar(prhs[1])) {
    mexErrMsgTxt("The second argument must be a string.");
  }
  
  char *file_path = mxArrayToString(prhs[1]);
  Array_Link *a = MxArray_To_Stack(prhs[0]);
  Stack *stack = Get_Stack_At(a);
  if (stack->text == NULL) {
    stack->text = "\0";
  }
  Write_Stack(file_path, stack);
  Delete_Array_Link(a);
}
