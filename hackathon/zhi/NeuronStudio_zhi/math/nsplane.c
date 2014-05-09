#include "nsplane.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


#define _NS_PLANE( P, _a, _b, _c, _d, t )\
   ns_assert( NULL != (P) );\
   ns_vector##t( &(P)->N, (_a), (_b), (_c) );\
   (P)->d = (_d);\
	return (P)

NsPlaned* ns_planed( NsPlaned *plane, nsdouble a, nsdouble b, nsdouble c, nsdouble d )
   {  _NS_PLANE( plane, a, b, c, d, 3d );  }

NsPlanef* ns_planef( NsPlanef *plane, nsfloat a, nsfloat b, nsfloat c, nsfloat d )
   {  _NS_PLANE( plane, a, b, c, d, 3f );  }


void ns_planed_get_second_point
	(
	const NsPlaned   *plane,
	const NsPoint3d  *P1,
	NsPoint3d        *P2
	)
	{
	NsVector3d  P3;
	NsVector3d  CP;
	NsVector3d  T;


	ns_assert( NULL != plane );
	ns_assert( NULL != P1 );
	ns_assert( NULL != P2 );

	/* Displace P1 by the smallest component of the normal(a new point P3) and
		take cross product N x P3-P1 to get a vector parallel to the plane. Add
		this vector to P1 to get a point on the plane P2. 

     (P2)    (P3)
       \     /
     CP \   /
         \ /
        (P1)-------
                  N
	*/

	ns_vector3d( &T, NS_ABS( plane->N.x ), NS_ABS( plane->N.y ), NS_ABS( plane->N.z ) );

	P3 = *P1;

	if( T.x < T.y )
		{
		if( T.x < T.z )
			P3.x += 1.0;
		else
			P3.z += 1.0;
		}
	else if( T.y < T.z )
		P3.y += 1.0;
	else
		P3.z += 1.0;

	/* P2 = P1 + ( N x ( P3 - P1 ) ) */
	ns_vector3d_add( P2, P1, ns_vector3d_cross( &CP, &plane->N, ns_vector3d_sub( &T, &P3, P1 ) ) );
	}

#define _NS_PLANE_DISTANCE_TO_POINT( plane, P, t )\
   ns_assert( NULL != (plane) );\
   ns_assert( NULL != (P) );\
   return ns_vector##t##_dot( &(plane)->N, (P) ) + (plane)->d

nsdouble ns_planed_distance_to_point( const NsPlaned *plane, const NsPoint3d *P )
   {  _NS_PLANE_DISTANCE_TO_POINT( plane, P, 3d );  }

nsfloat ns_planef_distance_to_point( const NsPlanef *plane, const NsPoint3f *P )
   {  _NS_PLANE_DISTANCE_TO_POINT( plane, P, 3f );  }
