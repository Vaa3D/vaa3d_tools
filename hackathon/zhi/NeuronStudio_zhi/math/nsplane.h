#ifndef __NS_MATH_PLANE_H__
#define __NS_MATH_PLANE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <math/nsplanedecls.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsPlaned* ns_planed( NsPlaned *plane, nsdouble a, nsdouble b, nsdouble c, nsdouble d );
NS_IMPEXP NsPlanef* ns_planef( NsPlanef *plane, nsfloat a, nsfloat b, nsfloat c, nsfloat d );


/* 'P1' assumed to be a point on the plane. No check done though. */
NS_IMPEXP void ns_planed_get_second_point
	(
	const NsPlaned   *plane,
	const NsPoint3d  *P1,
	NsPoint3d        *P2
	);


/* NOTE: The planes normal vector must be of unit length for the
	following routine to work! The returned distance can be negative. */
NS_IMPEXP nsdouble ns_planed_distance_to_point( const NsPlaned *plane, const NsPoint3d *P );
NS_IMPEXP nsfloat ns_planef_distance_to_point( const NsPlanef *plane, const NsPoint3f *P );

NS_DECLS_END

#endif/* __NS_MATH_PLANE_H__ */
