#include "nsinterpolate.h"


void ns_lerpf_init( NsLerpf *lerp, nsfloat x1, nsfloat y1, nsfloat x2, nsfloat y2 )
	{
	ns_assert( NULL != lerp );
	//ns_assert( .00001f <= NS_ABS( x2 - x1 ) );

   lerp->slope     = ( y2 - y1 ) / ( x2 - x1 );
   lerp->intercept = y1 - lerp->slope * x1; /* y = mx + b, so b = y - mx */
	}


nsfloat ns_lerpf( NsLerpf *lerp, nsfloat x )
	{
	ns_assert( NULL != lerp );
   return lerp->slope * x + lerp->intercept; /* y = mx + b */
	}


void ns_lerpd_init( NsLerpd *lerp, nsdouble x1, nsdouble y1, nsdouble x2, nsdouble y2 )
	{
	ns_assert( NULL != lerp );
	//ns_assert( .00001 <= NS_ABS( x2 - x1 ) );

   lerp->slope     = ( y2 - y1 ) / ( x2 - x1 );
   lerp->intercept = y1 - lerp->slope * x1; /* y = mx + b, so b = y - mx */
	}


nsdouble ns_lerpd( NsLerpd *lerp, nsdouble x )
	{
	ns_assert( NULL != lerp );
   return lerp->slope * x + lerp->intercept; /* y = mx + b */
	}
