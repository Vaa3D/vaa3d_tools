#ifndef __NS_MATH_PLANE_DECLS_H__
#define __NS_MATH_PLANE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef struct _NsPlaned
	{
	NsVector3d  N;
	nsdouble    d;
	}
	NsPlaned;

typedef struct _NsPlanef
	{
	NsVector3f  N;
	nsfloat     d;
	}
	NsPlanef;

NS_DECLS_END

#endif/* __NS_MATH_PLANE_DECLS_H__ */
