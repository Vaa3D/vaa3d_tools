#ifndef __NS_MATH_INTERPOLATE_H__
#define __NS_MATH_INTERPOLATE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>

NS_DECLS_BEGIN

/* Linear interpolation. */
typedef struct _NsLerpf
	{
   nsfloat  slope;
   nsfloat  intercept;
	}
	NsLerpf;

   
NS_IMPEXP void ns_lerpf_init( NsLerpf *lerp, nsfloat x1, nsfloat y1, nsfloat x2, nsfloat y2 );

/* NOTE: x should range from x1 and x2 given in the init function. */
NS_IMPEXP nsfloat ns_lerpf( NsLerpf *lerp, nsfloat x );



/* Linear interpolation. */
typedef struct _NsLerpd
	{
   nsdouble  slope;
   nsdouble  intercept;
	}
	NsLerpd;

   
NS_IMPEXP void ns_lerpd_init( NsLerpd *lerp, nsdouble x1, nsdouble y1, nsdouble x2, nsdouble y2 );

/* NOTE: x should range from x1 and x2 given in the init function. */
NS_IMPEXP nsdouble ns_lerpd( NsLerpd *lerp, nsdouble x );

NS_DECLS_END

#endif/* __NS_MATH_INTERPOLATE_H__ */
