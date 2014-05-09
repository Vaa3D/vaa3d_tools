#include "naas.h"


NsVector3i* get_non_axis_aligned_set
	(
	nsint        dim1,
	nsint        dim2,
	nsint        dim3,
	nsfloat      xdir,
	nsfloat      ydir,
	nsfloat      zdir,
	nssize      *count,
	NsVector3i  *corners
	)
	{
	NsVector3i  *pos, *pp;
	nsint        xdim, ydim, zdim;
	nsint        i, j, k;
	nsfloat      xdirab, ydirab, zdirab;
	nsfloat      xoff, yoff, zoff;
	nsfloat64    f64;
	nsint32      i32;
	nssize       p;
	nsint        ii, ji, ki;


	ns_assert( NULL != count );

	*count = ( nssize )( ( dim1 * 2 + 1 ) * ( dim2 * 2 + 1 ) * ( dim3 * 2 + 1 ) );

	/* allocate array of positions */
	if( NULL == ( pos = ns_malloc( sizeof( NsVector3i ) * ( *count ) ) ) )
		{
		*count = 0;
		return NULL;
		}

    /* get absolute values */
	if( xdir < 0.0f )
		xdirab = -xdir;
	else
		xdirab = xdir;

	if( ydir < 0.0f )
		ydirab = -ydir;
	else
		ydirab = ydir;

	if( zdir < 0.0f )
		zdirab = -zdir;
	else
		zdirab = zdir;

    /* orient box depending on direction */
	if( xdirab >= ydirab && xdirab >= zdirab )
		{
		xdim = dim1;
		ydim = dim2;
		zdim = dim3;
		yoff = ydir / xdir;
		zoff = zdir / xdir;
		pp   = pos;
		p    = 0;

		for( k = -zdim; k <= zdim; k++ )
			for( j = -ydim; j <= ydim; j++ )
				for( i =- xdim; i <= xdim; i++ )
					{
					pp->x = i;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( i * yoff, i32, f64 );
					pp->y = j + i32;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( i * zoff, i32, f64 );
					pp->z = k + i32;

					pp++;
					++p;
					}

		ns_assert( *count == p );

		pp = corners;
		p  = 0;

		for( ki = 0, k = -zdim; ki < 2; ++ki, k += 2 * zdim )
			for( ji = 0, j = -ydim; ji < 2; ++ji, j += 2 * ydim )
				for( ii = 0, i = -xdim; ii < 2; ++ii, i += 2 * xdim )
					{
					pp->x = i;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( i * yoff, i32, f64 );
					pp->y = j + i32;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( i * zoff, i32, f64 );
					pp->z = k + i32;

					pp++;
					++p;
					}

		ns_assert( 8 == p );
		}
	else if( ydirab >= xdirab && ydirab >= zdirab )
		{
		ydim = dim1;
		xdim = dim2;
		zdim = dim3;
		xoff = xdir / ydir;
		zoff = zdir / ydir;
		pp   = pos;
		p    = 0;

		for( k = -zdim; k <= zdim; k++ )
			for( j = -ydim; j <= ydim; j++ )
				for( i = -xdim; i <= xdim; i++ )
					{
					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( j * xoff, i32, f64 );
					pp->x = i + i32;

					pp->y = j;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( j * zoff, i32, f64 );
					pp->z = k + i32;

					pp++;
					++p;
					}

		ns_assert( *count == p );

		pp = corners;
		p  = 0;

		for( ki = 0, k = -zdim; ki < 2; ++ki, k += 2 * zdim )
			for( ji = 0, j = -ydim; ji < 2; ++ji, j += 2 * ydim )
				for( ii = 0, i = -xdim; ii < 2; ++ii, i += 2 * xdim )
					{
					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( j * xoff, i32, f64 );
					pp->x = i + i32;

					pp->y = j;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( j * zoff, i32, f64 );
					pp->z = k + i32;

					pp++;
					++p;
					}

		ns_assert( 8 == p );
		}
	else if( zdirab >= xdirab && zdirab >= ydirab )
		{
		zdim = dim1;

		if( xdirab >= ydirab )
			{
			xdim = dim3;
			ydim = dim2;
			}
		else
			{
			xdim = dim2;
			ydim = dim3;
			}

		xoff = xdir / zdir;
		yoff = ydir / zdir;
		pp   = pos;
		p    = 0;

		for( k = -zdim; k <= zdim; k++ )
			for( j = -ydim; j <= ydim; j++ )
				for( i = -xdim; i <= xdim; i++ )
					{
					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( k * xoff, i32, f64 );
					pp->x = i + i32;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( k * yoff, i32, f64 );
					pp->y = j + i32;

					pp->z = k;

					pp++;
					++p;
					}

		ns_assert( *count == p );

		pp = corners;
		p  = 0;

		for( ki = 0, k = -zdim; ki < 2; ++ki, k += 2 * zdim )
			for( ji = 0, j = -ydim; ji < 2; ++ji, j += 2 * ydim )
				for( ii = 0, i = -xdim; ii < 2; ++ii, i += 2 * xdim )
					{
					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( k * xoff, i32, f64 );
					pp->x = i + i32;

					NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( k * yoff, i32, f64 );
					pp->y = j + i32;

					pp->z = k;

					pp++;
					++p;
					}

		ns_assert( 8 == p );
		}

	return pos;
	}
