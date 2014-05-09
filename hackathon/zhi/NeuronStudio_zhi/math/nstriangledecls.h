#ifndef __NS_MATH_TRIANGLE_DECLS_H__
#define __NS_MATH_TRIANGLE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

typedef struct _NsTriangle2d
	{
	NsPoint2d  A;
	NsPoint2d  B;
	NsPoint2d  C;
	}
	NsTriangle2d;

typedef struct _NsTriangle2f
	{
	NsPoint2f  A;
	NsPoint2f  B;
	NsPoint2f  C;
	}
	NsTriangle2f;

typedef struct _NsTriangle3d 
	{
	NsPoint3d  A;
	NsPoint3d  B;
	NsPoint3d  C;
	}
	NsTriangle3d;

typedef struct _NsTriangle3f
	{
	NsPoint3f  A;
	NsPoint3f  B;
	NsPoint3f  C;
	}
	NsTriangle3f;


#ifdef NS_OS_WINDOWS
	#pragma pack( push, 1 )
#endif

typedef struct _NsIndexTriangleul
	{
	nsulong  a;
	nsulong  b;
	nsulong  c;
	}
	NsIndexTriangleul;

typedef struct _NsIndexTriangleui
	{
	nsuint  a;
	nsuint  b;
	nsuint  c;
	}
	NsIndexTriangleui;

typedef struct _NsIndexTriangleus
	{
	nsushort  a;
	nsushort  b;
	nsushort  c;
	}
	NsIndexTriangleus;

#ifdef NS_OS_WINDOWS
	#pragma pack( pop )
#endif


NS_COMPILE_TIME_SIZE_CHECK( NsIndexTriangleul, 3*sizeof( nsulong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsIndexTriangleui, 3*sizeof( nsuint ) );
NS_COMPILE_TIME_SIZE_CHECK( NsIndexTriangleus, 3*sizeof( nsushort ) );


NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleul, a, 0*sizeof( nsulong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleul, b, 1*sizeof( nsulong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleul, c, 2*sizeof( nsulong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleui, a, 0*sizeof( nsuint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleui, b, 1*sizeof( nsuint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleui, c, 2*sizeof( nsuint ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleus, a, 0*sizeof( nsushort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleus, b, 1*sizeof( nsushort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsIndexTriangleus, c, 2*sizeof( nsushort ) );

NS_DECLS_END

#endif/* __NS_MATH_TRIANGLE_DECLS_H__ */
