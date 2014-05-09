#ifndef __NS_MATH_MATRIX_DECLS_H__
#define __NS_MATH_MATRIX_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>

NS_DECLS_BEGIN

/* There shouldnt be any compiler added padding in these structures. */
#ifdef NS_OS_WINDOWS
#pragma pack( push, 1 )
#endif

typedef struct _NsMatrix33d
	{
	nsdouble
	m11, m12, m13,
	m21, m22, m23,
	m31, m32, m33;
	}
	NsMatrix33d;

typedef struct _NsMatrix44d
	{
	nsdouble
	m11, m12, m13, m14,
	m21, m22, m23, m24,
	m31, m32, m33, m34,
	m41, m42, m43, m44;
	}
	NsMatrix44d;

typedef struct _NsMatrix33f
	{
	nsfloat
	m11, m12, m13,
	m21, m22, m23,
	m31, m32, m33;
	}
	NsMatrix33f;

typedef struct _NsMatrix44f
	{
	nsfloat
	m11, m12, m13, m14,
	m21, m22, m23, m24,
	m31, m32, m33, m34,
	m41, m42, m43, m44;
	}
	NsMatrix44f;

typedef struct _NsMatrix33l
	{
	nslong
	m11, m12, m13,
	m21, m22, m23,
	m31, m32, m33;
	}
	NsMatrix33l;

typedef struct _NsMatrix44l
	{
	nslong
	m11, m12, m13, m14,
	m21, m22, m23, m24,
	m31, m32, m33, m34,
	m41, m42, m43, m44;
	}
	NsMatrix44l;

#ifdef NS_OS_WINDOWS
#pragma pack( pop )
#endif


NS_COMPILE_TIME_SIZE_CHECK( NsMatrix33d, 3*3*sizeof( nsdouble ) );
NS_COMPILE_TIME_SIZE_CHECK( NsMatrix44d, 4*4*sizeof( nsdouble ) );

NS_COMPILE_TIME_SIZE_CHECK( NsMatrix33f, 3*3*sizeof( nsfloat ) );
NS_COMPILE_TIME_SIZE_CHECK( NsMatrix44f, 4*4*sizeof( nsfloat ) );

NS_COMPILE_TIME_SIZE_CHECK( NsMatrix33l, 3*3*sizeof( nslong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsMatrix44l, 4*4*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m11, 0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m12, 1*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m13, 2*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m21, 3*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m22, 4*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m23, 5*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m31, 6*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m32, 7*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33d, m33, 8*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m11,  0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m12,  1*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m13,  2*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m14,  3*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m21,  4*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m22,  5*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m23,  6*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m24,  7*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m31,  8*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m32,  9*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m33, 10*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m34, 11*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m41, 12*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m42, 13*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m43, 14*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44d, m44, 15*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m11, 0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m12, 1*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m13, 2*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m21, 3*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m22, 4*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m23, 5*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m31, 6*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m32, 7*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33f, m33, 8*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m11,  0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m12,  1*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m13,  2*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m14,  3*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m21,  4*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m22,  5*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m23,  6*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m24,  7*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m31,  8*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m32,  9*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m33, 10*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m34, 11*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m41, 12*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m42, 13*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m43, 14*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44f, m44, 15*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m11, 0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m12, 1*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m13, 2*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m21, 3*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m22, 4*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m23, 5*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m31, 6*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m32, 7*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix33l, m33, 8*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m11,  0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m12,  1*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m13,  2*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m14,  3*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m21,  4*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m22,  5*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m23,  6*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m24,  7*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m31,  8*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m32,  9*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m33, 10*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m34, 11*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m41, 12*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m42, 13*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m43, 14*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsMatrix44l, m44, 15*sizeof( nslong ) );

NS_DECLS_END

#endif/* __NS_MATH_MATRIX_DECLS_H__ */
