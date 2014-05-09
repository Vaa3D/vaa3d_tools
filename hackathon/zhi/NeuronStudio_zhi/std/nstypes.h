#ifndef __NS_STD_TYPES_H__
#define __NS_STD_TYPES_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>

NS_DECLS_BEGIN

/* Standard types. Size can vary. */
typedef signed char     nschar;
typedef unsigned char   nsuchar;
typedef signed short    nsshort;
typedef unsigned short  nsushort;
typedef signed int      nsint;
typedef unsigned int    nsuint;
typedef double          nsdouble;
typedef void*           nspointer;
typedef const void*     nsconstpointer;

/* Some useful definitions. */
typedef nsint  nsboolean;
typedef nsint  nsunichar;
typedef nsint  nsenum;

/* Sized integer types <= 32 bits. */
	typedef nschar    nsint8;
	typedef nsuchar   nsuint8;
	typedef nsshort   nsint16;
	typedef nsushort  nsuint16;
	typedef nsint     nsint32;
	typedef nsuint    nsuint32;

/* Sized float types. */
typedef float     nsfloat32;
typedef nsdouble  nsfloat64;

/* The 'nsfloat' size can vary. */
#if 64 == NS_FLOAT_BITS
	typedef nsfloat64  nsfloat;
#else
	typedef nsfloat32  nsfloat;
#endif

/* Set the size of the 'elong' type. Should
	always be the same size as a pointer. */
#if 64 == NS_POINTER_BITS
	typedef signed long long    nslong;
	typedef unsigned long long  nsulong;

	#define NS_HAVE_INT64
	typedef nslong   nsint64;
	typedef nsulong  nsuint64;

	#define NS_HAVE_LONGLONG
	typedef nslong   nslonglong;
	typedef nsulong  nsulonglong;

	typedef nslong   nsintmax;
	typedef nsulong  nsuintmax;
#else
	/* 32 == NS_POINTER_BITS */
	typedef signed long    nslong;
	typedef unsigned long  nsulong;

		typedef nsint32   nsintmax;
		typedef nsuint32  nsuintmax;
#endif

/* Some more useful definitions. */
typedef nsulong  nssize;
typedef nslong   nsptrdiff;

#ifdef NS_HAVE_LONGDOUBLE
	typedef long double   nslongdouble;
	typedef nslongdouble  nsfloatmax;
#else
	typedef nsfloat64     nsfloatmax;
#endif


NS_COMPILE_TIME_BOOLEAN( NS_POINTER_BITS, ==, sizeof( nspointer ) * 8 );
NS_COMPILE_TIME_BOOLEAN( NS_POINTER_BITS, ==, sizeof( nsconstpointer ) * 8 );

NS_COMPILE_TIME_BOOLEAN( NS_FLOAT_BITS, ==, sizeof( nsfloat ) * 8 );
NS_COMPILE_TIME_BOOLEAN( NS_FLOAT_BITS, ==, sizeof( nsfloat ) * 8 );


/* NOTE: long's are the same size as pointers. */
NS_COMPILE_TIME_SIZE_EQUAL( nslong,  nspointer );
NS_COMPILE_TIME_SIZE_EQUAL( nsulong, nspointer );


NS_COMPILE_TIME_SIZE_CHECK( nsint8,    1 );
NS_COMPILE_TIME_SIZE_CHECK( nsuint8,   1 );
NS_COMPILE_TIME_SIZE_CHECK( nsint16,   2 );
NS_COMPILE_TIME_SIZE_CHECK( nsuint16,  2 );
NS_COMPILE_TIME_SIZE_CHECK( nsint32,   4 );
NS_COMPILE_TIME_SIZE_CHECK( nsuint32,  4 );
NS_COMPILE_TIME_SIZE_CHECK( nsfloat32, 4 );
NS_COMPILE_TIME_SIZE_CHECK( nsfloat64, 8 );

#ifdef NS_HAVE_INT64
	NS_COMPILE_TIME_SIZE_CHECK( nsint64,  8 );
	NS_COMPILE_TIME_SIZE_CHECK( nsuint64, 8 );
#endif


/* Internal macros. DO NOT USE! 
	NOTE: Assuming 2's complement arithmetic. */
#define _NS_SIGNED_INT_MAX( stype, utype )\
	( ( stype )( ( ( utype )( stype )-1 ) >> 1 ) )

#define _NS_SIGNED_INT_MIN( stype, utype )\
	( ( stype )( ~( ( ( utype )( stype )-1 ) >> 1 ) ) )

#define _NS_UNSIGNED_INT_MAX( stype, utype )\
	( ( utype )( stype )-1 )


#define NS_CHAR_MAX      _NS_SIGNED_INT_MAX( nschar, nsuchar )
#define NS_CHAR_MIN      _NS_SIGNED_INT_MIN( nschar, nsuchar )

#define NS_SHORT_MAX     _NS_SIGNED_INT_MAX( nsshort, nsushort )
#define NS_SHORT_MIN     _NS_SIGNED_INT_MIN( nsshort, nsushort )

#define NS_INT_MAX       _NS_SIGNED_INT_MAX( nsint, nsuint )
#define NS_INT_MIN       _NS_SIGNED_INT_MIN( nsint, nsuint )

#define NS_LONG_MAX      _NS_SIGNED_INT_MAX( nslong, nsulong )
#define NS_LONG_MIN      _NS_SIGNED_INT_MIN( nslong, nsulong )

#define NS_INTMAX_MAX    _NS_SIGNED_INT_MAX( nsintmax, nsuintmax )
#define NS_INTMAX_MIN    _NS_SIGNED_INT_MIN( nsintmax, nsuintmax )

#define NS_UCHAR_MAX     _NS_UNSIGNED_INT_MAX( nschar,   nsuchar   )
#define NS_USHORT_MAX    _NS_UNSIGNED_INT_MAX( nsshort,  nsushort  )
#define NS_UINT_MAX      _NS_UNSIGNED_INT_MAX( nsint,    nsuint    )
#define NS_ULONG_MAX     _NS_UNSIGNED_INT_MAX( nslong,   nsulong   )
#define NS_UINTMAX_MAX   _NS_UNSIGNED_INT_MAX( nsintmax, nsuintmax )

#ifdef NS_HAVE_LONGLONG
	#define NS_LONGLONG_MAX   _NS_SIGNED_INT_MAX( nslonglong, nsulonglong )
	#define NS_LONGLONG_MIN   _NS_SIGNED_INT_MIN( nslonglong, nsulonglong )

	#define NS_ULONGLONG_MAX  _NS_UNSIGNED_INT_MAX( nslonglong, nsulonglong )
#endif

#if 64 == NS_FLOAT_BITS
	#define NS_FLOAT_MAX  1.7976931348623158e+308
	#define NS_FLOAT_MIN  2.2250738585072014e-308
#else
	#define NS_FLOAT_MAX  3.402823466e+38F
	#define NS_FLOAT_MIN  1.175494351e-38F
#endif

#define NS_DOUBLE_MAX    1.7976931348623158e+308
#define NS_DOUBLE_MIN    2.2250738585072014e-308

#define NS_POINTER_MAX       NS_ULONG_MAX
#define NS_CONSTPOINTER_MAX  NS_POINTER_MAX

#define NS_BOOLEAN_MAX  NS_TRUE
#define NS_BOOLEAN_MIN  NS_FALSE

#define NS_SIZE_MAX      NS_ULONG_MAX

#define NS_PTRDIFF_MAX   NS_LONG_MAX
#define NS_PTRDIFF_MIN   NS_LONG_MIN

#define NS_UNICHAR_MAX   NS_INT_MAX
#define NS_UNICHAR_MIN   NS_INT_MIN

#define NS_ENUM_MAX      NS_INT_MAX
#define NS_ENUM_MIN      NS_INT_MIN

#define NS_INT8_MAX      _NS_SIGNED_INT_MAX( nsint8, nsuint8 )
#define NS_INT8_MIN      _NS_SIGNED_INT_MIN( nsint8, nsuint8 )

#define NS_INT16_MAX     _NS_SIGNED_INT_MAX( nsint16, nsuint16 )
#define NS_INT16_MIN     _NS_SIGNED_INT_MIN( nsint16, nsuint16 )

#define NS_INT32_MAX     _NS_SIGNED_INT_MAX( nsint32, nsuint32 )
#define NS_INT32_MIN     _NS_SIGNED_INT_MIN( nsint32, nsuint32 )

#define NS_UINT8_MAX     _NS_UNSIGNED_INT_MAX( nsint8, nsuint8 )
#define NS_UINT16_MAX    _NS_UNSIGNED_INT_MAX( nsint16, nsuint16 )
#define NS_UINT32_MAX    _NS_UNSIGNED_INT_MAX( nsint32, nsuint32 )

#define NS_FLOAT32_MAX   3.402823466e+38F
#define NS_FLOAT32_MIN   1.175494351e-38F

#define NS_FLOAT64_MAX   1.7976931348623158e+308
#define NS_FLOAT64_MIN   2.2250738585072014e-308

#ifdef NS_HAVE_INT64
	#define NS_INT64_MAX   _NS_SIGNED_INT_MAX( nsint64, nsuint64 )
	#define NS_INT64_MIN   _NS_SIGNED_INT_MIN( nsint64, nsuint64 )

	#define NS_UINT64_MAX  _NS_UNSIGNED_INT_MAX( nsint64, nsuint64 )
#endif

#ifdef NS_HAVE_LONGDOUBLE
	#define NS_FLOATMAX_MAX  NS_LONGDOUBLE_MAX
	#define NS_FLOATMAX_MIN  NS_LONGDOUBLE_MIN
#else
	#define NS_FLOATMAX_MAX  NS_FLOAT64_MAX
	#define NS_FLOATMAX_MIN  NS_FLOAT64_MIN
#endif


/* Internal macros. DO NOT USE! */

#define _NS_SIGNED_INT_TO_POINTER( i )\
	( ( nspointer )( nslong )(i) )

#define _NS_UNSIGNED_INT_TO_POINTER( i )\
	( ( nspointer )(i) )

#define _NS_POINTER_TO_SIGNED_INT( type, p )\
	( ( ns##type )( nslong )(p) )

#define _NS_POINTER_TO_UNSIGNED_INT( type, p )\
	( ( ns##type )( nsulong )(p) )


NS_COMPILE_TIME_SIZE_LESS_EQUAL( nschar, nspointer );
#define NS_CHAR_TO_POINTER( i )     _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_CHAR( p )     _NS_POINTER_TO_SIGNED_INT( char, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuchar, nspointer );
#define NS_UCHAR_TO_POINTER( i )    _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UCHAR( p )    _NS_POINTER_TO_UNSIGNED_INT( uchar, (p) )
 
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsshort, nspointer );
#define NS_SHORT_TO_POINTER( i )    _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_SHORT( p )    _NS_POINTER_TO_SIGNED_INT( short, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsushort, nspointer );
#define NS_USHORT_TO_POINTER( i )   _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_USHORT( p )   _NS_POINTER_TO_UNSIGNED_INT( ushort, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint, nspointer );
#define NS_INT_TO_POINTER( i )      _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_INT( p )      _NS_POINTER_TO_SIGNED_INT( int, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint, nspointer );
#define NS_UINT_TO_POINTER( i )     _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UINT( p )     _NS_POINTER_TO_UNSIGNED_INT( uint, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nslong, nspointer );
#define NS_LONG_TO_POINTER( i )     _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_LONG( p )     _NS_POINTER_TO_SIGNED_INT( long, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsulong, nspointer );
#define NS_ULONG_TO_POINTER( i )    _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_ULONG( p )    _NS_POINTER_TO_UNSIGNED_INT( ulong, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsboolean, nspointer );
#define NS_BOOLEAN_TO_POINTER( i )  NS_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_BOOLEAN( p )  NS_POINTER_TO_INT( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nssize, nspointer );
#define NS_SIZE_TO_POINTER( i )     NS_ULONG_TO_POINTER( (i) )
#define NS_POINTER_TO_SIZE( p )     NS_POINTER_TO_ULONG( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsptrdiff, nspointer );
#define NS_PTRDIFF_TO_POINTER( i )  NS_LONG_TO_POINTER( (i) )
#define NS_POINTER_TO_PTRDIFF( p )  NS_POINTER_TO_LONG( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsunichar, nspointer );
#define NS_UNICHAR_TO_POINTER( i )  NS_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UNICHAR( p )  NS_POINTER_TO_INT( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsenum, nspointer );
#define NS_ENUM_TO_POINTER( i )     NS_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_ENUM( p )     NS_POINTER_TO_INT( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint8, nspointer );
#define NS_INT8_TO_POINTER( i )      _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_INT8( p )      _NS_POINTER_TO_SIGNED_INT( int8, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint8, nspointer );
#define NS_UINT8_TO_POINTER( i )     _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UINT8( p )     _NS_POINTER_TO_UNSIGNED_INT( uint8, (p) )
 
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint16, nspointer );
#define NS_INT16_TO_POINTER( i )     _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_INT16( p )     _NS_POINTER_TO_SIGNED_INT( int16, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint16, nspointer );
#define NS_UINT16_TO_POINTER( i )    _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UINT16( p )    _NS_POINTER_TO_UNSIGNED_INT( uint16, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint32, nspointer );
#define NS_INT32_TO_POINTER( i )     _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_INT32( p )     _NS_POINTER_TO_SIGNED_INT( int32, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint32, nspointer );
#define NS_UINT32_TO_POINTER( i )    _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UINT32( p )    _NS_POINTER_TO_UNSIGNED_INT( uint32, (p) )


/* Internal. DO NOT USE! */
NS_IMPEXP NS_INTERNAL nspointer _ns_float32_to_pointer( nsfloat32 f );
NS_IMPEXP NS_INTERNAL nsfloat32 _ns_pointer_to_float32( nsconstpointer p );

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsfloat32, nspointer );
#define NS_FLOAT32_TO_POINTER( f )   _ns_float32_to_pointer( (f) )
#define NS_POINTER_TO_FLOAT32( p )   _ns_pointer_to_float32( (p) )


#if 64 == NS_POINTER_BITS
	#if ! defined( NS_HAVE_LONGLONG ) || ! defined( NS_HAVE_INT64 )
		#error nstypes.h compilation error: Invalid library build flags
	#endif

/* Internal. DO NOT USE! */
NS_IMPEXP NS_INTERNAL nspointer  _ns_float64_to_pointer( nsfloat64 f );
NS_IMPEXP NS_INTERNAL nsfloat64  _ns_pointer_to_float64( nsconstpointer p );

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsdouble, nspointer );
#define NS_DOUBLE_TO_POINTER( f )     _ns_float64_to_pointer( (f) )
#define NS_POINTER_TO_DOUBLE( p )     _ns_pointer_to_float64( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsfloat64, nspointer );
#define NS_FLOAT64_TO_POINTER( f )    _ns_float64_to_pointer( (f) )
#define NS_POINTER_TO_FLOAT64( p )    _ns_pointer_to_float64( (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nslonglong, nspointer );
#define NS_LONGLONG_TO_POINTER( i )   _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_LONGLONG( p )   _NS_POINTER_TO_SIGNED_INT( longlong, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsulonglong, nspointer );
#define NS_ULONGLONG_TO_POINTER( i )  _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_ULONGLONG( p )  _NS_POINTER_TO_UNSIGNED_INT( ulonglong, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint64, nspointer );
#define NS_INT64_TO_POINTER( i )      _NS_SIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_INT64( p )      _NS_POINTER_TO_SIGNED_INT( int64, (p) )

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint64, nspointer );
#define NS_UINT64_TO_POINTER( i )     _NS_UNSIGNED_INT_TO_POINTER( (i) )
#define NS_POINTER_TO_UINT64( p )     _NS_POINTER_TO_UNSIGNED_INT( uint64, (p) )

#endif/* 64 == NS_POINTER_BITS */


/*  Float | Pointer
	 ---------------
      32  |  32      Yes   case 1
		32  |  64      Yes   case 2
		64  |  32      No    case 3
		64  |  64      Yes   case 4
*/
#if 64 == NS_FLOAT_BITS
	#if 64 == NS_POINTER_BITS
		#define NS_FLOAT_TO_POINTER( f )  NS_FLOAT64_TO_POINTER( (f) )
		#define NS_POINTER_TO_FLOAT( p )  NS_POINTER_TO_FLOAT64( (p) )
	#endif
#else
	#define NS_FLOAT_TO_POINTER( f )  NS_FLOAT32_TO_POINTER( (f) )
	#define NS_POINTER_TO_FLOAT( p )  NS_POINTER_TO_FLOAT32( (p) )
#endif


/* *Hack* so that some internal macros will work. */
#define NS_POINTER_TO_POINTER( p )       ( ( nspointer )( p ) )
#define NS_CONSTPOINTER_TO_POINTER( p )  ( ( nspointer )( p ) )

NS_DECLS_END

#endif/* __NS_STD_TYPES_H__ */
