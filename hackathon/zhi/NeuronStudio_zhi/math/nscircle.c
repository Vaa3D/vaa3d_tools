#include "nscircle.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


#define _NS_CIRCLE( _S, _op1, _x, _y, _op2, _r )\
	(_S)->O.x _op1 (_x);\
	(_S)->O.y _op1 (_y);\
	(_S)->r   _op2 (_r);\
	return (_S)


#define _NS_CIRCLE_INIT( C, x, y, r )\
	ns_assert( NULL != (C) );\
	_NS_CIRCLE( (C), =, (x), (y), =, (r) )

NsCircled* ns_circled
	(
	NsCircled  *C,
	nsdouble    x,
	nsdouble    y,
	nsdouble    r
	)
	{  _NS_CIRCLE_INIT( C, x, y, r );  }
	

NsCirclef* ns_circlef
	(
	NsCirclef  *C,
	nsfloat     x,
	nsfloat     y,
	nsfloat     r
	)
	{  _NS_CIRCLE_INIT( C, x, y, r );  }


#define _NS_CIRCLE_CENTER( C, _O )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (_O) );\
	*(_O) = (C)->O;\
	return (_O)

NsPoint2d* ns_circled_center( const NsCircled *C, NsPoint2d *O )
	{  _NS_CIRCLE_CENTER( C, O );  }

NsPoint2f* ns_circlef_center( const NsCirclef *C, NsPoint2f *O )
	{  _NS_CIRCLE_CENTER( C, O );  }


nsboolean ns_circled_intersects_drag_rect
	(
	const NsCircled   *C,
	const NsDragRect  *R
	)
	{
	nsdouble         d2, r2;
	const nsdouble  *O;
	nsdouble         bmin[2], bmax[2];
	nssize           i;

	ns_assert( NULL != C );
	ns_assert( NULL != R );

	d2      = 0.0;
	r2      = NS_POW2( C->r );
	O       = ns_vector2d_const_array( &C->O );
	bmin[0] = ( nsdouble )R->start_x;
	bmin[1] = ( nsdouble )R->start_y;
	bmax[0] = ( nsdouble )R->end_x;
	bmax[1] = ( nsdouble )R->end_y;

	for( i = 0; i < 2; ++i )
		if( O[i] < bmin[i] )
			d2 += ( NS_POW2( O[i] - bmin[i] ) );
		else if( O[i] > bmax[i] )
			d2 += ( NS_POW2( O[i] - bmax[i] ) );

   return ( d2 <= r2 ) ? NS_TRUE : NS_FALSE;
	}
