#ifndef _TZ_MEXUTILS_H_
#define _TZ_MEXUTILS_H_

#include <mex.h>

#define mexTRUE 1
#define mexFALSE 0

inline bool tz_mxIsVector(const mxArray *pm)
{
  return (mxGetM(pm)==1 || mxGetN(pm)==1);
}

inline bool tz_mxIsScalar(const mxArray *pm)
{
  return (mxGetM(pm)==1 && mxGetN(pm)==1);
}

inline bool tz_mxIsDoubleVector(const mxArray *pm)
{
  return ( mxIsDouble(pm) && !mxIsComplex(pm) && 
	   (mxGetM(pm)==1 || mxGetN(pm)==1) );
}

inline bool tz_mxIsDoubleScalar(const mxArray *pm)
{
  return ( mxIsDouble(pm) && !mxIsComplex(pm) && 
  	   mxGetM(pm)==1 && mxGetN(pm)==1 );
}

inline mwSize tz_mxGetL(const mxArray *pm)
{
  return ( mxGetM(pm) * mxGetN(pm) );
}

inline bool tz_mxSameSize(const mxArray *pm1, const mxArray *pm2)
{
  mwSize ndim = mxGetNumberOfDimensions(pm1);
  if (ndim != mxGetNumberOfDimensions(pm2)) {
    return mexFALSE;
  }

  const mwSize *dims1 = mxGetDimensions(pm1);
  const mwSize *dims2 = mxGetDimensions(pm2);
  mwSize i;
  for (i = 0; i < ndim; i++) {
    if (dims1[i] != dims2[i])
      return mexFALSE;
  }

  return mexTRUE;
}

#include "../c/tz_error.h"
INIT_EXCEPTION_MAIN(e)

#endif
  
