#ifndef _TZ_MEXARRAYTRANS_H_
#define _TZ_MEXARRAYTRANS_H_

/* tz_mexarraytrans.h
 * 
 * 12-Oct-2007  Initial write:  Ting Zhao
 */

#include <mex.h>
#include "../c/tz_arraytrans.h"


Array_Link* MxArray_To_Image(const mxArray *x);
Array_Link* MxArray_To_Stack(const mxArray *x);
Array_Link* MxArray_To_DMatrix(const mxArray *x);

#endif
