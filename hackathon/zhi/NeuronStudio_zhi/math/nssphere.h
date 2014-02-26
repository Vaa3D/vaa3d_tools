#ifndef __NS_MATH_SPHERE_H__
#define __NS_MATH_SPHERE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <math/nsspheredecls.h>
#include <math/nsaabboxdecls.h>
#include <math/nsfrustumdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsSphered* ns_sphered
	(
	NsSphered  *S,
	nsdouble    x,
	nsdouble    y,
	nsdouble    z,
	nsdouble    r
	);

NS_IMPEXP NsSpheref* ns_spheref
	(
	NsSpheref  *S,
	nsfloat     x,
	nsfloat     y,
	nsfloat     z,
	nsfloat     r
	);


/* Internal. DO NOT USE! */
#define _ns_sphere_center_x( S )  ( (S)->O.x )
#define _ns_sphere_center_y( S )  ( (S)->O.y )
#define _ns_sphere_center_z( S )  ( (S)->O.z )
#define _ns_sphere_radius( S )    ( (S)->r   )


#define ns_sphered_center_x( S )  _ns_sphere_center_x( (S) )
#define ns_sphered_center_y( S )  _ns_sphere_center_y( (S) )
#define ns_sphered_center_z( S )  _ns_sphere_center_z( (S) )
#define ns_sphered_radius( S )    _ns_sphere_radius( (S) )

#define ns_spheref_center_x( S )  _ns_sphere_center_x( (S) )
#define ns_spheref_center_y( S )  _ns_sphere_center_y( (S) )
#define ns_spheref_center_z( S )  _ns_sphere_center_z( (S) )
#define ns_spheref_radius( S )    _ns_sphere_radius( (S) )


/* Returns 'O' */
NS_IMPEXP NsPoint3d* ns_sphered_center( const NsSphered *S, NsPoint3d *O );
NS_IMPEXP NsPoint3f* ns_spheref_center( const NsSpheref *S, NsPoint3f *O );


NS_IMPEXP nsboolean ns_sphered_intersects_aabbox
	(
	const NsSphered   *S,
	const NsAABBox3d  *B
	);


NS_IMPEXP nsboolean ns_sphered_intersects_sphere
	(
	const NsSphered  *S1,
	const NsSphered  *S2
	);

NS_IMPEXP nsboolean ns_spheref_intersects_sphere
	(
	const NsSpheref  *S1,
	const NsSpheref  *S2
	);


NS_IMPEXP nsboolean ns_sphered_intersects_conical_frustum
	(
	const NsSphered          *S,
	const NsConicalFrustumd  *F
	);

NS_DECLS_END

#endif/* __NS_MATH_SPHERE_H__ */
