#ifndef __NS_MATH_AABBOX_DECLS_H__
#define __NS_MATH_AABBOX_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <math/nsvectordecls.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

/* NOTE: The 'O' vector is the left, top, near
	corner of the box. Not using a 'max' vector
	since this would allow for the box to NOT be
	axis-aligned. */

typedef struct _NsAABBox2d
	{
	NsPoint2d  O;
	nsdouble   width;
	nsdouble   height;
	}
	NsAABBox2d;

typedef struct _NsAABBox3d
	{
	NsPoint3d  O;
	nsdouble   width;
	nsdouble   height;
	nsdouble   length;
	}
	NsAABBox3d;


typedef struct _NsAABBox2i
	{
	NsPoint2i  O;
	nsint      width;
	nsint      height;
	}
	NsAABBox2i;

NS_DECLS_END

#endif/* __NS_MATH_AABBOX_DECLS_H__ */
