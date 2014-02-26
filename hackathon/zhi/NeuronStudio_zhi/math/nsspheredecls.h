#ifndef __NS_MATH_SPHERE_DECLS_H__
#define __NS_MATH_SPHERE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef struct _NsSphered
	{
   NsVector3d  O;
   nsdouble    r;
	}
	NsSphered;

typedef struct _NsSpheref
	{
   NsVector3f  O;
   nsfloat     r;
	}
	NsSpheref;

NS_DECLS_END

#endif/* __NS_MATH_SPHERE_DECLS_H__ */
