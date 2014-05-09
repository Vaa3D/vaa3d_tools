#ifndef __NS_MATH_CIRCLE_DECLS_H__
#define __NS_MATH_CIRCLE_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef struct _NsCircled
	{
   NsVector2d  O;
   nsdouble    r;
	}
	NsCircled;

typedef struct _NsCirclef
	{
   NsVector2f  O;
   nsfloat     r;
	}
	NsCirclef;

typedef struct _NsCirclei
	{
   NsVector2i  O;
   nsint       r;
	}
	NsCirclei;

NS_DECLS_END

#endif/* __NS_MATH_CIRCLE_DECLS_H__ */
