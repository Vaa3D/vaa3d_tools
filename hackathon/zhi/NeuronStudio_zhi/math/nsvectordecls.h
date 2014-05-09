#ifndef __NS_MATH_VECTOR_DECLS_H__
#define __NS_MATH_VECTOR_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>

NS_DECLS_BEGIN

/* There shouldnt be any compiler added padding in these structures. */
#ifdef NS_OS_WINDOWS
	#pragma pack( push, 1 )
#endif

typedef struct _NsVector2d
	{
   nsdouble  x;
   nsdouble  y;
	}
	NsVector2d;

typedef struct _NsVector3d
	{
   nsdouble  x;
   nsdouble  y;
   nsdouble  z;
	}
	NsVector3d;

typedef struct _NsVector4d
	{
   nsdouble  x;
   nsdouble  y;
   nsdouble  z;
   nsdouble  w;
	}
	NsVector4d;

typedef struct _NsVector2f
	{
   nsfloat  x;
   nsfloat  y;
	}
	NsVector2f;

typedef struct _NsVector3f
	{
   nsfloat  x;
   nsfloat  y;
   nsfloat  z;
	}
	NsVector3f;

typedef struct _NsVector4f
	{
   nsfloat  x;
   nsfloat  y;
   nsfloat  z;
   nsfloat  w;
	}
	NsVector4f;

typedef struct _NsVector2l
	{
	nslong  x;
	nslong  y;
	}
	NsVector2l;

typedef struct _NsVector3l
	{
	nslong  x;
	nslong  y;
	nslong  z;
	}
	NsVector3l;

typedef struct _NsVector4l
	{
	nslong  x;
	nslong  y;
	nslong  z;
	nslong  w;
	}
	NsVector4l;

typedef struct _NsVector2i
	{
	nsint  x;
	nsint  y;
	}
	NsVector2i;

typedef struct _NsVector3i
	{
	nsint  x;
	nsint  y;
	nsint  z;
	}
	NsVector3i;

typedef struct _NsVector4i
	{
	nsint  x;
	nsint  y;
	nsint  z;
	nsint  w;
	}
	NsVector4i;

typedef struct _NsVector2s
	{
	nsshort  x;
	nsshort  y;
	}
	NsVector2s;

typedef struct _NsVector3s
	{
	nsshort  x;
	nsshort  y;
	nsshort  z;
	}
	NsVector3s;

typedef struct _NsVector4s
	{
	nsshort  x;
	nsshort  y;
	nsshort  z;
	nsshort  w;
	}
	NsVector4s;

typedef struct _NsVector3b
	{
	nschar  x;
	nschar  y;
	nschar  z;
	}
	NsVector3b;

typedef struct _NsVector3ub
	{
	nsuchar  x;
	nsuchar  y;
	nsuchar  z;
	}
	NsVector3ub;

typedef struct _NsVector4ub
	{
	nsuchar  x;
	nsuchar  y;
	nsuchar  z;
	nsuchar  w;
	}
	NsVector4ub;

#ifdef NS_OS_WINDOWS
	#pragma pack( pop )
#endif


NS_COMPILE_TIME_SIZE_CHECK( NsVector2d, 2*sizeof( nsdouble ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector3d, 3*sizeof( nsdouble ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4d, 4*sizeof( nsdouble ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector2f, 2*sizeof( nsfloat ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector3f, 3*sizeof( nsfloat ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4f, 4*sizeof( nsfloat ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector2l, 2*sizeof( nslong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector3l, 3*sizeof( nslong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4l, 4*sizeof( nslong ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector2i, 2*sizeof( nsint ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector3i, 3*sizeof( nsint ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4i, 4*sizeof( nsint ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector2s, 2*sizeof( nsshort ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector3s, 3*sizeof( nsshort ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4s, 4*sizeof( nsshort ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector3b, 3*sizeof( nschar ) );

NS_COMPILE_TIME_SIZE_CHECK( NsVector3ub, 3*sizeof( nsuchar ) );
NS_COMPILE_TIME_SIZE_CHECK( NsVector4ub, 4*sizeof( nsuchar ) );


/*NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2d, x, 0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2d, y, 1*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3d, x, 0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3d, y, 1*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3d, z, 2*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4d, x, 0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4d, y, 1*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4d, z, 2*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4d, w, 3*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2f, x, 0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2f, y, 1*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3f, x, 0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3f, y, 1*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3f, z, 2*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4f, x, 0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4f, y, 1*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4f, z, 2*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4f, w, 3*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2l, x, 0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2l, y, 1*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3l, x, 0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3l, y, 1*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3l, z, 2*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4l, x, 0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4l, y, 1*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4l, z, 2*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4l, w, 3*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2i, x, 0*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2i, y, 1*sizeof( nsint ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3i, x, 0*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3i, y, 1*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3i, z, 2*sizeof( nsint ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4i, x, 0*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4i, y, 1*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4i, z, 2*sizeof( nsint ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4i, w, 3*sizeof( nsint ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2s, x, 0*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector2s, y, 1*sizeof( nsshort ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3s, x, 0*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3s, y, 1*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3s, z, 2*sizeof( nsshort ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4s, x, 0*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4s, y, 1*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4s, z, 2*sizeof( nsshort ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4s, w, 3*sizeof( nsshort ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3b, x, 0*sizeof( nschar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3b, y, 1*sizeof( nschar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3b, z, 2*sizeof( nschar ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3ub, x, 0*sizeof( nsuchar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3ub, y, 1*sizeof( nsuchar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector3ub, z, 2*sizeof( nsuchar ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4ub, x, 0*sizeof( nsuchar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4ub, y, 1*sizeof( nsuchar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4ub, z, 2*sizeof( nsuchar ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsVector4ub, w, 3*sizeof( nsuchar ) );*/

NS_DECLS_END

#endif/* __NS_MATH_VECTOR_DECLS_H__ */
