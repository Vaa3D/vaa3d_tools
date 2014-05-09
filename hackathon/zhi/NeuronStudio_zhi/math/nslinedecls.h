#ifndef __NS_MATH_LINE_DECLS_H__
#define __NS_MATH_LINE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef struct _NsLine3d
	{
	NsVector3d  P1;
	NsVector3d  P2;
	}
	NsLine3d;

typedef struct _NsLine3f
	{
	NsVector3f  P1;
	NsVector3f  P2;
	}
	NsLine3f;

typedef struct _NsLine3i
	{
	NsVector3i  P1;
	NsVector3i  P2;
	}
	NsLine3i;

NS_DECLS_END

#endif/* __NS_MATH_LINE_DECLS_H__ */
