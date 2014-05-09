#include "nsline.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


#define _NS_LINE3( _L, _op, _x1, _y1, _z1, _x2, _y2, _z2 )\
	( _L )->P1.x _op ( _x1 );\
	( _L )->P1.y _op ( _y1 );\
	( _L )->P1.z _op ( _z1 );\
	( _L )->P2.x _op ( _x2 );\
	( _L )->P2.y _op ( _y2 );\
	( _L )->P2.z _op ( _z2 );\
	return ( _L )


#define _NS_LINE3_INIT( L, x1, y1, z1, x2, y2, z2 )\
	ns_assert( NULL != (L) );\
	_NS_LINE3( (L), =, (x1), (y1), (z1), (x2), (y2), (z2) )

NsLine3d* ns_line3d
	(
	NsLine3d  *L,
	nsdouble   x1,
	nsdouble   y1,
	nsdouble   z1,
	nsdouble   x2,
	nsdouble   y2,
	nsdouble   z2
	)
	{  _NS_LINE3_INIT( L, x1, y1, z1, x2, y2, z2 );  }

NsLine3f* ns_line3f
	(
	NsLine3f  *L,
	nsfloat    x1,
	nsfloat    y1,
	nsfloat    z1,
	nsfloat    x2,
	nsfloat    y2,
	nsfloat    z2
	)
	{  _NS_LINE3_INIT( L, x1, y1, z1, x2, y2, z2 );  }


NsRay3d* ns_line3d_to_ray( const NsLine3d *line, NsRay3d *ray )
	{
	ns_assert( NULL != line );
	ns_assert( NULL != ray );

	ray->O = line->P1;
	ns_vector3d_norm( ns_vector3d_sub( &ray->D, &line->P2, &line->P1 ) );

	return ray;
	}


NS_PRIVATE void _ns_line3d_do_intersects_line
	(
	const NsPoint3d  *P1,
	const NsPoint3d  *P2,
	const NsPoint3d  *P3,
	const NsPoint3d  *P4,
	NsPoint3d        *Pa,
	NsPoint3d        *Pb,
	nsdouble         *mua,
	nsdouble         *mub,
	nsboolean        *has_solution
	)
	{
	NsPoint3d  P13, P43, P21;
	nsdouble   d1343, d4321, d1321, d4343, d2121;
	nsdouble   numer, denom;


	*has_solution = NS_TRUE;

	P13.x = P1->x - P3->x;
	P13.y = P1->y - P3->y;
	P13.z = P1->z - P3->z;
	P43.x = P4->x - P3->x;
	P43.y = P4->y - P3->y;
	P43.z = P4->z - P3->z;

	if( NS_ABS( P43.x ) < NS_DOUBLE_EPSILON &&
		 NS_ABS( P43.y ) < NS_DOUBLE_EPSILON &&
		 NS_ABS( P43.z ) < NS_DOUBLE_EPSILON   )
		*has_solution = NS_FALSE;

	P21.x = P2->x - P1->x;
	P21.y = P2->y - P1->y;
	P21.z = P2->z - P1->z;

	if( NS_ABS( P21.x ) < NS_DOUBLE_EPSILON &&
		 NS_ABS( P21.y ) < NS_DOUBLE_EPSILON &&
		 NS_ABS( P21.z ) < NS_DOUBLE_EPSILON   )
		*has_solution = NS_FALSE;

	d1343 = P13.x * P43.x + P13.y * P43.y + P13.z * P43.z;
	d4321 = P43.x * P21.x + P43.y * P21.y + P43.z * P21.z;
	d1321 = P13.x * P21.x + P13.y * P21.y + P13.z * P21.z;
	d4343 = P43.x * P43.x + P43.y * P43.y + P43.z * P43.z;
	d2121 = P21.x * P21.x + P21.y * P21.y + P21.z * P21.z;

	denom = d2121 * d4343 - d4321 * d4321;

	if( NS_ABS( denom ) < NS_DOUBLE_EPSILON )
		*has_solution = NS_FALSE;

	numer = d1343 * d4321 - d1321 * d4343;

	*mua = numer / denom;
	*mub = ( d1343 + d4321 * (*mua) ) / d4343;

	Pa->x = P1->x + (*mua) * P21.x;
	Pa->y = P1->y + (*mua) * P21.y;
	Pa->z = P1->z + (*mua) * P21.z;
	Pb->x = P3->x + (*mub) * P43.x;
	Pb->y = P3->y + (*mub) * P43.y;
	Pb->z = P3->z + (*mub) * P43.z;
	}


nsboolean ns_line3d_intersects_line
	(
	const NsLine3d  *L1,
	const NsLine3d  *L2,
	NsLine3d        *isect_line,
	nsdouble        *ret_mua,
	nsdouble        *ret_mub
	)
	{
	nsdouble   mua, mub;
	nsboolean  has_solution;


	ns_assert( NULL != L1 );
	ns_assert( NULL != L2 );
	ns_assert( NULL != isect_line );

	_ns_line3d_do_intersects_line(
		&L1->P1,
		&L1->P2,
		&L2->P1,
		&L2->P2,
		&isect_line->P1,
		&isect_line->P2,
		&mua,
		&mub,
		&has_solution
		);

	if( NULL != ret_mua )*ret_mua = mua;
	if( NULL != ret_mub )*ret_mub = mub;

	return has_solution;
	}


void ns_line3d_find_shortest_between
	(
	const NsLine3d  *L1,
	const NsLine3d  *L2,
	NsLine3d        *L3
	)
	{
	nsdouble   mua, mub;
	nsboolean  has_solution;


	ns_assert( NULL != L1 );
	ns_assert( NULL != L2 );
	ns_assert( NULL != L3 );

	_ns_line3d_do_intersects_line(
		&L1->P1,
		&L1->P2,
		&L2->P1,
		&L2->P2,
		&L3->P1,
		&L3->P2,
		&mua,
		&mub,
		&has_solution
		);
	}


/* Based on code found at http://www.cit.gu.edu.au/~anthony/info/graphics/bresenham.procs */
void ns_line3i_raster
	(
	const NsLine3i  *L,
	void             ( *raster_func )( const NsVector3i*, nspointer ),
	nspointer        user_data
	)
	{
	NsVector3i  point;
	NsVector3i  step;
	NsVector3i  delta;
	NsVector3i  abs_delta;
	NsVector3i  twice_abs_delta;
	nsint       i;
	nsint       error1, error2;


	point = L->P1;

	ns_vector3i_sub( &delta, &L->P2, &L->P1 );

	ns_vector3i( &step, delta.x < 0 ? -1 : 1, delta.y < 0 ? -1 : 1, delta.z < 0 ? -1 : 1 );

	ns_vector3i_abs( &abs_delta, &delta );

	ns_vector3i_scale( &twice_abs_delta, &abs_delta, 2 );

	#define _NS_LINE3_BRESENHAM( X, Y, Z )\
		error1 = twice_abs_delta.Y - abs_delta.X;\
		error2 = twice_abs_delta.Z - abs_delta.X;\
		\
		for( i = 0; i < abs_delta.X; i++ )\
			{\
			( raster_func )( &point, user_data );\
			\
			if( error1 > 0 )\
				{\
				point.Y += step.Y;\
				error1 -= twice_abs_delta.X;\
				}\
			\
			if( error2 > 0 )\
				{\
				point.Z += step.Z;\
				error2 -= twice_abs_delta.X;\
				}\
			\
			error1 += twice_abs_delta.Y;\
			error2 += twice_abs_delta.Z;\
			\
			point.X += step.X;\
			}

	/* Is the x difference the greatest? i.e growing the fastest. */
	if( ( abs_delta.x >= abs_delta.y ) && ( abs_delta.x >= abs_delta.z ) )
		{  _NS_LINE3_BRESENHAM( x, y, z );  }
	else if( ( abs_delta.y >= abs_delta.x ) && ( abs_delta.y >= abs_delta.z ) )
		{  _NS_LINE3_BRESENHAM( y, x, z );  }
	else
		{  _NS_LINE3_BRESENHAM( z, y, x );  }

	( raster_func )( &point, user_data );
	}


nsfloat ns_line3f_length( const NsLine3f *L )
	{
	NsVector3f D;

	ns_vector3f_sub( &D, &L->P2, &L->P1 );
	return ns_vector3f_mag( &D );
	}


nsdouble ns_line3d_length( const NsLine3d *L )
	{
	NsVector3d D;

	ns_vector3d_sub( &D, &L->P2, &L->P1 );
	return ns_vector3d_mag( &D );
	}
