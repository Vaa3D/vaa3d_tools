#include "nspoint.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>
#include <math/nsline.h>


#define _NS_POINT2_INSIDE_AABBOX( P, B )\
	ns_assert( NULL != (P) );\
	ns_assert( NULL != (B) );\
	return\
	(P)->x >= (B)->O.x && (P)->x < (B)->O.x + (B)->width  &&\
	(P)->y <= (B)->O.y && (P)->y > (B)->O.y - (B)->height

#define _NS_POINT3_INSIDE_AABBOX( P, B )\
	ns_assert( NULL != (P) );\
	ns_assert( NULL != (B) );\
	return\
	(P)->x >= (B)->O.x && (P)->x < (B)->O.x + (B)->width  &&\
	(P)->y <= (B)->O.y && (P)->y > (B)->O.y - (B)->height &&\
	(P)->z <= (B)->O.z && (P)->z > (B)->O.z - (B)->length

nsboolean ns_point2d_inside_aabbox( const NsPoint2d *P, const NsAABBox2d *B )
	{  _NS_POINT2_INSIDE_AABBOX( P, B );  }

nsboolean ns_point3d_inside_aabbox( const NsPoint3d *P, const NsAABBox3d *B )
	{  _NS_POINT3_INSIDE_AABBOX( P, B );  }


nsboolean ns_point3i_inside_cube( const NsPoint3i *P, const NsCubei *C )
	{
	ns_assert( NULL != P );
	ns_assert( NULL != C );

	return ( C->C1.x <= P->x && P->x <= C->C2.x ) &&
			 ( C->C1.y <= P->y && P->y <= C->C2.y ) &&
			 ( C->C1.z <= P->z && P->z <= C->C2.z );
	}


nsboolean ns_point2i_inside_drag_rect( const NsPoint2i *P, const NsDragRect *R )
	{
	ns_assert( NULL != P );
	ns_assert( NULL != R );

	return ( R->start_x <= P->x && P->x <= R->end_x ) &&
			 ( R->start_y <= P->y && P->y <= R->end_y );
	}


/* NOTE: Can avoid square root by using the distance squared. Using
	<= since a point on the surface I think technically intersects. */
#define _NS_POINT_INSIDE_SPHERE( P, S )\
	ns_assert( NULL != (P) );\
	ns_assert( NULL != (S) );\
	return\
	NS_POW2( (P)->x - (S)->O.x ) +\
	NS_POW2( (P)->y - (S)->O.y ) +\
	NS_POW2( (P)->z - (S)->O.z )\
	<= NS_POW2( (S)->r )

nsboolean ns_point3d_inside_sphere( const NsPoint3d *P, const NsSphered *S )
	{  _NS_POINT_INSIDE_SPHERE( P, S );  }

nsboolean ns_point3f_inside_sphere( const NsPoint3f *P, const NsSpheref *S )
	{  _NS_POINT_INSIDE_SPHERE( P, S );  }


/* NOTE: Can avoid square root by using the distance squared. Using
	<= since a point on the surface I think technically intersects. */
#define _NS_POINT_INSIDE_CIRCLE( P, C )\
	ns_assert( NULL != (P) );\
	ns_assert( NULL != (C) );\
	return\
	NS_POW2( (P)->x - (C)->O.x ) +\
	NS_POW2( (P)->y - (C)->O.y )\
	<= NS_POW2( (C)->r )

nsboolean ns_point2d_inside_circle( const NsPoint2d *P, const NsCircled *C )
	{  _NS_POINT_INSIDE_CIRCLE( P, C );  }

nsboolean ns_point2f_inside_circle( const NsPoint2f *P, const NsCirclef *C )
	{  _NS_POINT_INSIDE_CIRCLE( P, C );  }


/* 7/29/04 http://www.mines.edu/~gmurray/ArbitraryAxisRotation/ArbitraryAxisRotation.html */
#define _NS_POINT3_AXIS_ROTATE( R, P, A, theta, t, type, one, sin_func, cos_func )\
	type x, y, z, sine, cosine, mag, inv_mag2, Axx, Ayy, Azz, AxAx, AyAy, AzAz, PxAx, PyAy, PzAz;\
	x = (R)->x; y = (R)->y; z = (R)->z;\
	sine = sin_func( (theta) ); cosine = cos_func( (theta) );\
	mag = ns_vector3##t##_mag( (A) ); inv_mag2 = (one) / ( mag*mag );\
	Axx = (A)->x*x; Ayy = (A)->y*y; Azz = (A)->z*z;\
	AxAx = (A)->x*(A)->x; AyAy = (A)->y*(A)->y; AzAz = (A)->z*(A)->z;\
	PxAx = (P)->x*(A)->x; PyAy = (P)->y*(A)->y; PzAz = (P)->z*(A)->z;\
	(R)->x = (\
			 (P)->x*( AyAy + AzAz ) +\
			 (A)->x*( -PyAy - PzAz + Axx + Ayy + Azz ) +\
			 ( ( x - (P)->x )*( AyAy + AzAz ) + (A)->x*( PyAy + PzAz - Ayy - Azz ) )*cosine +\
			 mag*( (P)->y*(A)->z - (P)->z*(A)->y - (A)->z*y + (A)->y*z )*sine\
			 )\
			 * inv_mag2;\
	(R)->y = (\
			 (P)->y*( AxAx + AzAz ) +\
			 (A)->y*( -PxAx - PzAz + Axx + Ayy + Azz ) +\
			 ( ( y - (P)->y )*( AxAx + AzAz ) + (A)->y*( PxAx + PzAz - Axx - Azz ) )*cosine +\
			 mag*( -(P)->x*(A)->z + (P)->z*(A)->x + (A)->z*x - (A)->x*z )*sine\
			 )\
			 * inv_mag2;\
	(R)->z = (\
			 (P)->z*( AxAx + AyAy ) +\
			 (A)->z*( -PxAx - PyAy + Axx + Ayy + Azz ) +\
			 ( ( z - (P)->z )*( AxAx + AyAy ) + (A)->z*( PxAx + PyAy - Axx - Ayy ) )*cosine +\
			 mag*( (P)->x*(A)->y - (P)->y*(A)->x - (A)->y*x + (A)->x*y )*sine\
			 )\
			 * inv_mag2

void ns_point3d_axis_rotate( NsPoint3d *R, const NsPoint3d *P, const NsVector3d *A, nsdouble theta )
	{  _NS_POINT3_AXIS_ROTATE( R, P, A, theta, d, nsdouble, 1.0, ns_sin, ns_cos );  }

void ns_point3f_axis_rotate( NsPoint3f *R, const NsPoint3f *P, const NsVector3f *A, nsfloat theta )
	{  _NS_POINT3_AXIS_ROTATE( R, P, A, theta, f, nsfloat, 1.0f, ns_sinf, ns_cosf );  }


/* if A . B < 0 or A . B > A.A abort since the point falls
	"outside" the line, where '.' is the dot product. */
#define _NS_POINT_PROJECTS_ONTO_LINE( P, L, t, type, zero )\
	NsVector##t  A, B;\
	type        dot_aa, dot_ab;\
	ns_assert( NULL != (P) );\
	ns_assert( NULL != (L) );\
	ns_vector##t##_sub( &A, &(L)->P2, &(L)->P1 );\
	ns_vector##t##_sub( &B, (P), &(L)->P1 );\
	dot_ab = ns_vector##t##_dot( &A, &B );\
	if( dot_ab < zero )\
		return NS_FALSE;\
	dot_aa = ns_vector##t##_dot( &A, &A );\
	if( dot_ab > dot_aa )\
		return NS_FALSE;\
	return NS_TRUE\

nsboolean ns_point3d_projects_onto_line( const NsPoint3d *P, const NsLine3d *L )
	{  _NS_POINT_PROJECTS_ONTO_LINE( P, L, 3d, nsdouble, 0.0 );  }

nsboolean ns_point3f_projects_onto_line( const NsPoint3f *P, const NsLine3f *L )
	{  _NS_POINT_PROJECTS_ONTO_LINE( P, L, 3f, nsfloat, 0.0f );  }


nsfloat ns_point3f_project_onto_line( const NsPoint3f *P, const NsLine3f *L, NsPoint3f *I )
	{
	nsfloat m, u;

	m = ns_line3f_length( L );

	u =
		(
		( ( P->x - L->P1.x ) * ( L->P2.x - L->P1.x ) ) +
		( ( P->y - L->P1.y ) * ( L->P2.y - L->P1.y ) ) +
		( ( P->z - L->P1.z ) * ( L->P2.z - L->P1.z ) )
		)
			/
		( m * m );

	I->x = L->P1.x + u * ( L->P2.x - L->P1.x );
	I->y = L->P1.y + u * ( L->P2.y - L->P1.y );
	I->z = L->P1.z + u * ( L->P2.z - L->P1.z );

	return u;
	}


nsdouble ns_point3d_project_onto_line( const NsPoint3d *P, const NsLine3d *L, NsPoint3d *I )
	{
	nsdouble m, u;

	m = ns_line3d_length( L );

	u =
		(
		( ( P->x - L->P1.x ) * ( L->P2.x - L->P1.x ) ) +
		( ( P->y - L->P1.y ) * ( L->P2.y - L->P1.y ) ) +
		( ( P->z - L->P1.z ) * ( L->P2.z - L->P1.z ) )
		)
			/
		( m * m );

	I->x = L->P1.x + u * ( L->P2.x - L->P1.x );
	I->y = L->P1.y + u * ( L->P2.y - L->P1.y );
	I->z = L->P1.z + u * ( L->P2.z - L->P1.z );

	return u;
	}


nsfloat ns_point3f_distance_to_line( const NsPoint3f *X0, const NsLine3f *L )
	{
	NsVector3f  X2mX1, X1mX0, CP;
	nsfloat     length;


	ns_vector3f_sub( &X2mX1, &L->P2, &L->P1 );
	ns_vector3f_sub( &X1mX0, &L->P1, X0 );

	ns_vector3f_cross( &CP, &X2mX1, &X1mX0 );

	length = ns_vector3f_mag( &X2mX1 );
	ns_assert( 0.0f < length );

	return ns_vector3f_mag( &CP ) / length;
	}


nsfloat ns_point3f_distance_to_point( const NsPoint3f *P1, const NsPoint3f *P2 )
	{  return ns_sqrtf( NS_DISTANCE_SQUARED( P1->x, P1->y, P1->z, P2->x, P2->y, P2->z ) );  }
