#ifndef __NS_MATH_CUBE_DECLS_H__
#define __NS_MATH_CUBE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

/* C1 = a corner, C2 = the opposite corner */

typedef struct _NsCubei
	{
	NsPoint3i  C1;
	NsPoint3i  C2;
	}
	NsCubei;

typedef struct _NsCubef
	{
	NsPoint3f  C1;
	NsPoint3f  C2;
	}
	NsCubef;

NS_DECLS_END

#endif/* __NS_MATH_CUBE_DECLS_H__ */
