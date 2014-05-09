#ifndef __NS_STD_MATH_H__
#define __NS_STD_MATH_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

#define NS_PI  NS_PI_D


/* Epsilon is largest number such that 1.0 + epsilon != 1.0. */
#define NS_DOUBLE_EPSILON  2.2204460492503131E-16
#define NS_PI_D            3.1415926535897932384626433832795

#define ns_pow( f, p )    _ns_pow( (f), (p) )
#define ns_ceil( f )      _ns_ceil( (f) )
#define ns_floor( f )     _ns_floor( (f) )
#define ns_sin( f )       _ns_sin( (f) )
#define ns_cos( f )       _ns_cos( (f) )
#define ns_tan( f )       _ns_tan( (f) )
#define ns_asin( f )      _ns_asin( (f) )
#define ns_acos( f )      _ns_acos( (f) )
#define ns_atan2( x, y )  _ns_atan2( (x), (y) )
#define ns_sqrt( f )      _ns_sqrt( (f) )
#define ns_fabs( f )      _ns_fabs( (f) )
#define ns_log( f )       _ns_log( (f) )
#define ns_log10( f )     _ns_log10( (f) )


#if 64 == NS_FLOAT_BITS
	NS_COMPILE_TIME_SIZE_EQUAL( nsfloat, nsdouble );

	#define NS_FLOAT_EPSILON  NS_DOUBLE_EPSILON
	#define NS_PI_F           NS_PI_D

	#define ns_powf( f, p )    ns_pow( (f), (p) )
	#define ns_ceilf( f )      ns_ceil( (f) )
	#define ns_floorf( f )     ns_floor( (f) )
	#define ns_sinf( f )       ns_sin( (f) )
	#define ns_cosf( f )       ns_cos( (f) )
	#define ns_tanf( f )       ns_tan( (f) )
	#define ns_asinf( f )      ns_asin( (f) )
	#define ns_acosf( f )      ns_acos( (f) )
	#define ns_atan2f( x, y )  ns_atan2( (x), (y) )
	#define ns_sqrtf( f )      ns_sqrt( (f) )
	#define ns_fabsf( f )      ns_fabs( (f) )
	#define ns_logf( f )       ns_log( (f) )
	#define ns_log10f( f )     ns_log10( (f) )
#else
	#define NS_FLOAT_EPSILON  1.192092896E-07F
	#define NS_PI_F           3.141592654F

	NS_IMPEXP nsfloat ns_powf( nsfloat f, nsfloat p );
	NS_IMPEXP nsfloat ns_ceilf( nsfloat f );
	NS_IMPEXP nsfloat ns_floorf( nsfloat f );
	NS_IMPEXP nsfloat ns_sinf( nsfloat f );
	NS_IMPEXP nsfloat ns_cosf( nsfloat f );
	NS_IMPEXP nsfloat ns_tanf( nsfloat f );
	NS_IMPEXP nsfloat ns_asinf( nsfloat f );
	NS_IMPEXP nsfloat ns_acosf( nsfloat f );
	NS_IMPEXP nsfloat ns_atan2f( nsfloat x, nsfloat y );
	NS_IMPEXP nsfloat ns_sqrtf( nsfloat f );
	NS_IMPEXP nsfloat ns_fabsf( nsfloat f );
	NS_IMPEXP nsfloat ns_logf( nsfloat f );
	NS_IMPEXP nsfloat ns_log10f( nsfloat f );
#endif/* 64 == NS_FLOAT_BITS */

NS_DECLS_END

#endif/* __NS_STD_MATH_H__ */
