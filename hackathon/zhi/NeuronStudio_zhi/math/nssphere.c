#include "nssphere.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>
#include <math/nsaabbox.h>


#define _NS_SPHERE( _S, _op1, _x, _y, _z, _op2, _r )\
	(_S)->O.x _op1 (_x);\
	(_S)->O.y _op1 (_y);\
	(_S)->O.z _op1 (_z);\
	(_S)->r   _op2 (_r);\
	return (_S)


#define _NS_SPHERE_INIT( S, x, y, z, r )\
	ns_assert( NULL != (S) );\
	_NS_SPHERE( (S), =, (x), (y), (z), =, (r) )

NsSphered* ns_sphered
	(
	NsSphered  *S,
	nsdouble    x,
	nsdouble    y,
	nsdouble    z,
	nsdouble    r
	)
	{  _NS_SPHERE_INIT( S, x, y, z, r );  }
	

NsSpheref* ns_spheref
	(
	NsSpheref  *S,
	nsfloat     x,
	nsfloat     y,
	nsfloat     z,
	nsfloat     r
	)
	{  _NS_SPHERE_INIT( S, x, y, z, r );  }


#define _NS_SPHERE_CENTER( S, _O )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (_O) );\
	*(_O) = (S)->O;\
	return (_O)

NsPoint3d* ns_sphered_center( const NsSphered *S, NsPoint3d *O )
	{  _NS_SPHERE_CENTER( S, O );  }

NsPoint3f* ns_spheref_center( const NsSpheref *S, NsPoint3f *O )
	{  _NS_SPHERE_CENTER( S, O );  }


/* 8/12/04 http://www.acm.org/tog/GraphicsGems/gems/BoxSphere.c */
#define _NS_SPHERE_INTERSECTS_AABBOX( type, t, S, B, zero )\
	type           d2, r2;\
	NsVector3##t   min, max;\
	const type    *O, *bmin, *bmax;\
	nssize         i;\
	\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (B) );\
	\
	d2   = zero;\
	r2   = NS_POW2( (S)->r );\
	O    = ns_vector3##t##_const_array( &(S)->O );\
	bmin = ns_vector3##t##_const_array( ns_aabbox3##t##_min( (B), &min ) );\
	bmax = ns_vector3##t##_const_array( ns_aabbox3##t##_max( (B), &max ) );\
	\
	for( i = 0; i < 3; ++i )\
		if( O[i] < bmin[i] )\
			d2 += ( NS_POW2( O[i] - bmin[i] ) );\
		else if( O[i] > bmax[i] )\
			d2 += ( NS_POW2( O[i] - bmax[i] ) );\
	\
   return ( d2 <= r2 ) ? NS_TRUE : NS_FALSE


nsboolean ns_sphered_intersects_aabbox
	(
	const NsSphered   *S,
	const NsAABBox3d  *B
	)
	{  _NS_SPHERE_INTERSECTS_AABBOX( nsdouble, d, S, B, 0.0 );  }


#define _NS_SPHERE_INTERSECTS_SPHERE( S1, S2 )\
	ns_assert( NULL != (S1) );\
	ns_assert( NULL != (S2) );\
	return NS_POW2( (S1)->O.x - (S2)->O.x ) +\
			 NS_POW2( (S1)->O.y - (S2)->O.y ) +\
			 NS_POW2( (S1)->O.z - (S2)->O.z )\
			 < NS_POW2( (S1)->r + (S2)->r )

nsboolean ns_sphered_intersects_sphere
	(
	const NsSphered  *S1,
	const NsSphered  *S2
	)
	{  _NS_SPHERE_INTERSECTS_SPHERE( S1, S2 );  }

nsboolean ns_spheref_intersects_sphere
	(
	const NsSpheref  *S1,
	const NsSpheref  *S2
	)
	{  _NS_SPHERE_INTERSECTS_SPHERE( S1, S2 );  }


nsboolean ns_sphered_intersects_conical_frustum
	(
	const NsSphered          *S,
	const NsConicalFrustumd  *F
	)
	{
   NsVector3d   N;
   NsVector3d   C;
   nsdouble     radius;
   nsdouble     length;
   nsdouble     d;
   nsdouble     u;
   NsSphered    T;
   nsboolean    done;


	ns_assert( NULL != S );
	ns_assert( NULL != F );

   /* Get direction vector between the two endpoints. */
   ns_vector3d_sub( &N, &F->C2, &F->C1 );
   length = ns_vector3d_mag( &N );

   if( length <= NS_FLOAT_ALMOST_ZERO )
      {
      /* If the frustum has no length, just take the larger of
         the spheres and use as the intersection test. */
      if( F->r1 > F->r2 )
         ns_sphered( &T, F->C1.x, F->C1.y, F->C1.z, F->r1 );
      else
         ns_sphered( &T, F->C2.x, F->C2.y, F->C2.z, F->r2 );

      if( ns_sphered_intersects_sphere( S, &T ) )
			return NS_TRUE;
      }
   else
      {
      ns_vector3d_norm( &N );

      /* Check intersection between spheres, which approximate the
         conical frustum, and the passed sphere since there is no easy
         intersection test for a conical frustum and a sphere. */

      done = NS_FALSE;
      d    = 0.0;

      while( ! done )
         {
         /* Create a sphere by interpolating the current coordinate and radius. */
         ns_vector3d_scale( &C, &N, d );
         ns_vector3d_cmpd_add( &C, &F->C1 );

         u = d / length;

         if( 1.0 <= u )
            {
            u    = 1.0;
            done = NS_TRUE;
            }

         /* If u = 0, then radius == radius1
            If u = 1, then radius == radius2 */
         radius = ( 1.0 - u ) * F->r1 + u * F->r2;

         /* Increase radius by 5% to account for "space" between spheres. */
         ns_sphered( &T, C.x, C.y, C.z, radius + radius * .05 );

         if( ns_sphered_intersects_sphere( S, &T ) )
            return NS_TRUE;

			/* The 'd' variable wont advance if the radius goes to zero and
				we will end up in an infinite loop. */
			if( NS_FLOAT_EQUAL( radius, 0.0f ) )
				return NS_FALSE;

         /* Move half the radius of the previous sphere. */
         d += radius / 2.0;
         }
      }

   return NS_FALSE;
	}
