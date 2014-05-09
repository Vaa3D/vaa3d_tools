#ifndef __NS_MATH_RAY_DECLS_H__
#define __NS_MATH_RAY_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

/* O = origin, D = direction */

typedef struct _NsRay3d
	{
   NsPoint3d   O;
   NsVector3d  D;
	}
	NsRay3d;

typedef struct _NsRay3f
	{
	NsPoint3f   O;
	NsVector3f  D;
	}
	NsRay3f;

NS_DECLS_END

#endif/* __NS_MATH_RAY_DECLS_H__ */
