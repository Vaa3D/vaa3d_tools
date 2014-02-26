#ifndef __NS_MATH_RAY_H__
#define __NS_MATH_RAY_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmath.h>
#include <math/nsraydecls.h>
#include <math/nsaabboxdecls.h>
#include <math/nsplanedecls.h>
#include <math/nsspheredecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsPoint3d* ns_ray3d_cast( const NsRay3d *R, nsdouble t, NsPoint3d *P );
NS_IMPEXP NsPoint3f* ns_ray3f_cast( const NsRay3f *R, nsfloat t, NsPoint3f *P );

NS_IMPEXP nsboolean ns_ray3d_intersects_aabbox
	(
	const NsRay3d     *ray,
	const NsAABBox3d  *B,
	NsPoint3d         *isect_pt
	);

NS_IMPEXP nsboolean ns_ray3f_intersects_sphere
	(
	const NsRay3f    *R,
	const NsSpheref  *S,
	NsPoint3f        *isect_pt
	);

NS_IMPEXP nsboolean ns_ray3d_intersects_sphere
	(
	const NsRay3d    *R,
	const NsSphered  *S,
	NsPoint3d        *isect_pt
	);

NS_DECLS_END

#endif/* __NS_MATH_RAY_H__ */
