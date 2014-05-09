#ifndef __NS_MATH_FRUSTUM_DECLS_H__
#define __NS_MATH_FRUSTUM_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef struct _NsConicalFrustumd
	{
	NsVector3d  C1;
	NsVector3d  C2;
	nsdouble    r1;
	nsdouble    r2;
	}
	NsConicalFrustumd;

NS_DECLS_END

#endif/* __NS_MATH_FRUSTUM_DECLS_H__ */
