#include "nsray.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>
#include <math/nsaabbox.h>


#define _NS_RAY3_CAST( R, t, P )\
	ns_assert( NULL != (R) );\
	ns_assert( NULL != (P) );\
	(P)->x = (R)->O.x + (R)->D.x * (t);\
	(P)->y = (R)->O.y + (R)->D.y * (t);\
	(P)->z = (R)->O.z + (R)->D.z * (t);\
	return (P)

NsPoint3d* ns_ray3d_cast( const NsRay3d *R, nsdouble t, NsPoint3d *P )
	{  _NS_RAY3_CAST( R, t, P );  }

NsPoint3f* ns_ray3f_cast( const NsRay3f *R, nsfloat t, NsPoint3f *P )
	{  _NS_RAY3_CAST( R, t, P );  }


/* Code based (with minor changes) off:
	Ray/AABB: Andrew Woo, Fast Ray-Box Intersection, Graphics Gems, pp. 395-396 */
nsboolean ns_ray3d_intersects_aabbox
	(
	const NsRay3d     *ray,
	const NsAABBox3d  *B,
	NsPoint3d         *isect_pt
	)
	{
	NsPoint3d        v_min;
	NsPoint3d        v_max;
	const nsdouble  *box_min;
	const nsdouble  *box_max;
	nsdouble         plane[3];
	nsdouble         tmax[3];
	const nsdouble  *direction;
	const nsdouble  *origin;
	nsdouble        *coord;
	nsboolean        middle[3];
	nsboolean        inside;
	nssize           candidate;
	nssize           i;


	ns_assert( NULL != ray );
	ns_assert( NULL != B );
	ns_assert( NULL != isect_pt );

	/* NOTE: Convert to arrays so we can loop. */
	box_min   = ns_vector3d_const_array( ns_aabbox3d_min( B, &v_min ) );
	box_max   = ns_vector3d_const_array( ns_aabbox3d_max( B, &v_max ) );
	origin    = ns_vector3d_const_array( &ray->O );
	direction = ns_vector3d_const_array( &ray->D );

	inside = NS_TRUE;

	/* Make the planes. Note that since this is an axis aligned
		bounding box, only one component of the planes normal is
		necessary. */
	for( i = 0; i < 3; ++i )
		if( origin[i] < box_min[i] )
			{
			middle[i] = NS_FALSE;
			plane[i]  = box_min[i];
			inside    = NS_FALSE;
			}
		else if( box_max[i] < origin[i] )
			{
			middle[i] = NS_FALSE;
			plane[i]  = box_max[i];
			inside    = NS_FALSE;
			}
		else
			middle[i] = NS_TRUE;

	if( inside )
		{
		*isect_pt = ray->O;
		return NS_TRUE;
		}

	/* Calculate T distances to planes. */
	for( i = 0; i < 3; ++i )
		if( ! middle[i] && ! NS_DOUBLE_EQUAL( direction[i], 0.0 ) )
			tmax[i] = ( plane[i] - origin[i] ) / direction[i];
		else
			tmax[i] = -1.0;

	/* Get the largest of the T distances. */
	candidate = 0;

	for( i = 1; i < 3; ++i )
		if( tmax[ candidate ] < tmax[i] )
			candidate = i;

	/* Check that the final candidate is actually inside the box. */
	if( tmax[ candidate ] < 0.0 )
		return NS_FALSE;

	coord = ns_vector3d_array( isect_pt );

	for( i = 0; i < 3; ++i )
		if( i != candidate )
			{
			coord[i] = origin[i] + tmax[ candidate ] * direction[i];

			if( coord[i] < box_min[i] || box_max[i] < coord[i] )
				return NS_FALSE;
			}
		else
			coord[i] = plane[i];

	return NS_TRUE;
	}


/* http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm */
/* TODO: Make macro template! */
nsboolean ns_ray3f_intersects_sphere
	(
	const NsRay3f    *R,
	const NsSpheref  *S,
	NsPoint3f        *isect_pt
	)
	{
	nsfloat     b, c, discriminant, sqrt_of_discriminant, t0, t1, t;
	NsVector3f  V;


	ns_assert( NULL != R );
	ns_assert( NULL != S );

	ns_vector3f_sub( &V, &R->O, &S->O );

	/* NOTE: a = R->D dot R->D, which is assumed to be 1! */
	b = ns_vector3f_dot( &R->D, &V ) * 2.0f;
	c = ns_vector3f_dot( &V, &V ) - NS_POW2( S->r );
	
	discriminant = NS_POW2( b ) - 4.0f * c; /* b*b - 4*a*c */

	/* NOTE: discriminant == 0.0 is a tangential hit! */
	if( discriminant < 0.0f )
		return NS_FALSE;
	else if( discriminant >= 0.0f )
		{
		sqrt_of_discriminant = ns_sqrtf( discriminant );

		/* Quadratic formula:
			( -b +- sqrt( b*b - 4*a*c  ) ) / 2*a =
			( -b +- sqrt( discriminant ) ) / 2  =
			( -b +- sqrt( discriminant ) ) * .5 */
		t0 = ( -b + sqrt_of_discriminant ) * .5f;
		t1 = ( -b - sqrt_of_discriminant ) * .5f;

		if( t0 < 0.0f )
			{
			if( t1 < 0.0f )
				return NS_FALSE;

			t = t1;
			}
		else if( t1 < 0.0f )
			{
			if( t0 < 0.0f )
				return NS_FALSE;

			t = t0;
			}
		else
			t = ( t0 < t1 ) ? t0 : t1;

		ns_ray3f_cast( R, t, isect_pt );
		}

	return NS_TRUE;
	}


nsboolean ns_ray3d_intersects_sphere
	(
	const NsRay3d    *R,
	const NsSphered  *S,
	NsPoint3d        *isect_pt
	)
	{
	nsdouble    b, c, discriminant, sqrt_of_discriminant, t0, t1, t;
	NsVector3d  V;


	ns_assert( NULL != R );
	ns_assert( NULL != S );

	ns_vector3d_sub( &V, &R->O, &S->O );

	/* NOTE: a = R->D dot R->D, which is assumed to be 1! */
	b = ns_vector3d_dot( &R->D, &V ) * 2.0;
	c = ns_vector3d_dot( &V, &V ) - NS_POW2( S->r );
	
	discriminant = NS_POW2( b ) - 4.0 * c; /* b*b - 4*a*c */

	/* NOTE: discriminant == 0.0 is a tangential hit! */
	if( discriminant < 0.0 )
		return NS_FALSE;
	else if( discriminant >= 0.0 )
		{
		sqrt_of_discriminant = ns_sqrt( discriminant );

		/* Quadratic formula:
			( -b +- sqrt( b*b - 4*a*c  ) ) / 2*a =
			( -b +- sqrt( discriminant ) ) / 2  =
			( -b +- sqrt( discriminant ) ) * .5 */
		t0 = ( -b + sqrt_of_discriminant ) * .5;
		t1 = ( -b - sqrt_of_discriminant ) * .5;

		if( t0 < 0.0 )
			{
			if( t1 < 0.0 )
				return NS_FALSE;

			t = t1;
			}
		else if( t1 < 0.0 )
			{
			if( t0 < 0.0 )
				return NS_FALSE;

			t = t0;
			}
		else
			t = ( t0 < t1 ) ? t0 : t1;

		ns_ray3d_cast( R, t, isect_pt );
		}

	return NS_TRUE;
	}
