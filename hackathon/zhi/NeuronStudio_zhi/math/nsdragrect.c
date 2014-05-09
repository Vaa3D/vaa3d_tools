#include "nsdragrect.h"


void ns_drag_rect_bounds
	(
	NsDragRect  *R,
	nsint        min_x,
	nsint        min_y,
	nsint        max_x,
	nsint        max_y
	)
	{
	ns_assert( NULL != R );

	R->min_x = min_x;
	R->min_y = min_y;
	R->max_x = max_x;
	R->max_y = max_y;

	/* Normalize */
	if( R->max_x < R->min_x )
		NS_SWAP( nsint, R->min_x, R->max_x );

	if( R->max_y < R->min_y )
		NS_SWAP( nsint, R->min_y, R->max_y );
	}


void ns_drag_rect_set_tolerance( NsDragRect *R, nsint tolerance )
	{
	ns_assert( NULL != R );
	R->tolerance = tolerance;
	}


nsint ns_drag_rect_get_tolerance( const NsDragRect *R )
	{
	ns_assert( NULL != R );
	return R->tolerance;
	}


void ns_drag_rect_start( NsDragRect *R, nsint x, nsint y )
	{
	ns_assert( NULL != R );

	R->start_x = x;
	R->start_y = y;
	R->visible = NS_FALSE;
	}


void ns_drag_rect_end( NsDragRect *R, nsint x, nsint y )
	{
	ns_assert( NULL != R );

	R->end_x = x;
	R->end_y = y;
	}


void ns_drag_rect_left( NsDragRect *R, nsint x )
	{
	ns_assert( NULL != R );

	if( x < R->min_x )
		x = R->min_x;
	else if( R->end_x < x )
		x = R->end_x;

	ns_assert( R->min_x <= x && x <= R->end_x );
	R->start_x = x;
	}


void ns_drag_rect_top( NsDragRect *R, nsint y )
	{
	ns_assert( NULL != R );

	if( y < R->min_y )
		y = R->min_y;
	else if( R->end_y < y )
		y = R->end_y;

	ns_assert( R->min_y <= y && y <= R->end_y );
	R->start_y = y;
	}


void ns_drag_rect_right( NsDragRect *R, nsint x )
	{
	ns_assert( NULL != R );

	if( R->max_x < x )
		x = R->max_x;
	else if( x < R->start_x )
		x = R->start_x;

	ns_assert( R->start_x <= x && x <= R->max_x );
	R->end_x = x;
	}


void ns_drag_rect_bottom( NsDragRect *R, nsint y )
	{
	ns_assert( NULL != R );

	if( R->max_y < y )
		y = R->max_y;
	else if( y < R->start_y )
		y = R->start_y;

	ns_assert( R->start_y <= y && y <= R->max_y );
	R->end_y = y;
	}


void ns_drag_rect_left_top( NsDragRect *R, nsint x, nsint y )
	{
	ns_drag_rect_left( R, x );
	ns_drag_rect_top( R, y );
	}


void ns_drag_rect_right_top( NsDragRect *R, nsint x, nsint y )
	{
	ns_drag_rect_right( R, x );
	ns_drag_rect_top( R, y );
	}


void ns_drag_rect_left_bottom( NsDragRect *R, nsint x, nsint y )
	{
	ns_drag_rect_left( R, x );
	ns_drag_rect_bottom( R, y );
	}


void ns_drag_rect_right_bottom( NsDragRect *R, nsint x, nsint y )
	{
	ns_drag_rect_right( R, x );
	ns_drag_rect_bottom( R, y );
	}


nsboolean ns_drag_rect_valid( const NsDragRect *R )
	{
	ns_assert( NULL != R );

	return ( NS_ABS( R->end_x - R->start_x ) >= R->tolerance ) &&
			 ( NS_ABS( R->end_y - R->start_y ) >= R->tolerance );
	}


void ns_drag_rect_normalize( NsDragRect *R )
	{
	ns_assert( NULL != R );

	if( R->end_x < R->start_x )
		NS_SWAP( nsint, R->start_x, R->end_x );

	if( R->end_y < R->start_y )
		NS_SWAP( nsint, R->start_y, R->end_y );
	}


nsboolean ns_drag_rect_is_normalized( const NsDragRect *R )
	{
	ns_assert( NULL != R );

	return R->start_x <= R->end_x &&
			 R->start_y <= R->end_y;
	}


nsint ns_drag_rect_width( const NsDragRect *R )
	{
	ns_assert( NULL != R );
	/* ns_assert( ns_drag_rect_is_normalized( R ) ); */

	return R->end_x - R->start_x + 1;
	}


nsint ns_drag_rect_height( const NsDragRect *R )
	{
	ns_assert( NULL != R );
	/* ns_assert( ns_drag_rect_is_normalized( R ) ); */

	return R->end_y - R->start_y + 1;
	}


void ns_drag_rect_dimensions
	(
	const NsDragRect  *R,
	nsint             *width,
	nsint             *height
	)
	{
	ns_assert( NULL != R );
	ns_assert( NULL != width );
	ns_assert( NULL != height );

	*width  = ns_drag_rect_width( R );
	*height = ns_drag_rect_height( R );
	}


void ns_drag_rect_clip( NsDragRect *R )
	{
	ns_assert( NULL != R );

	if( R->start_x < R->min_x )
		R->start_x = R->min_x;
	else if( R->start_x > R->max_x )
		R->start_x = R->max_x;

	if( R->end_x < R->min_x )
		R->end_x = R->min_x;
	else if( R->end_x > R->max_x )
		R->end_x = R->max_x;

	if( R->start_y < R->min_y )
		R->start_y = R->min_y;
	else if( R->start_y > R->max_y )
		R->start_y = R->max_y;

	if( R->end_y < R->min_y )
		R->end_y = R->min_y;
	else if( R->end_y > R->max_y )
		R->end_y = R->max_y;
	}


NS_PRIVATE void _ns_drag_rect_xor
	(
	const NsDragRect  *_R,
	void               ( *xor_line )( nspointer, nsint, nsint, nsint, nsint ),
	nspointer          user_data
	)
	{
	NsDragRect R;

	ns_assert( NULL != _R );

	if( ns_drag_rect_valid( _R ) )
		{
		R = *_R;
		ns_drag_rect_normalize( &R );

		/* Top line */
		if( R.start_y >= R.min_y && R.start_y <= R.max_y )
			{
			if( ( R.start_x < R.min_x && R.end_x < R.min_x ) ||
             ( R.start_x > R.max_x && R.end_x > R.max_x )   )
				{}
			else
				( xor_line )(
					user_data,
					R.start_x >= R.min_x ? R.start_x : R.min_x,
					R.start_y,
					R.end_x <= R.max_x ? R.end_x : R.max_x,
					R.start_y
					);
			}

		/* Bottom line */
		if( R.end_y >= R.min_y && R.end_y <= R.max_y )
			{
			if( ( R.start_x < R.min_x && R.end_x < R.min_x ) ||
             ( R.start_x > R.max_x && R.end_x > R.max_x )   )
				{}
			else
				( xor_line )(
					user_data,
					R.start_x >= R.min_x ? R.start_x : R.min_x,
					R.end_y,
					R.end_x <= R.max_x ? R.end_x : R.max_x,
					R.end_y
					);
			}

		/* Left line */
		if( R.start_x >= R.min_x && R.start_x <= R.max_x )
			{
			if( ( R.start_y < R.min_y && R.end_y < R.min_y ) ||
             ( R.start_y > R.max_y && R.end_y > R.max_y )   )
				{}
			else
				( xor_line )(
					user_data,
					R.start_x,
					R.start_y >= R.min_y ? R.start_y : R.min_y,
					R.start_x,
					R.end_y <= R.max_y ? R.end_y : R.max_y
					);
			}


		/* Right line */
		if( R.end_x >= R.min_x && R.end_x <= R.max_x )
			{
			if( ( R.start_y < R.min_y && R.end_y < R.min_y ) ||
             ( R.start_y > R.max_y && R.end_y > R.max_y )   )
				{}
			else
				( xor_line )(
					user_data,
					R.end_x,
					R.start_y >= R.min_y ? R.start_y : R.min_y,
					R.end_x,
					R.end_y <= R.max_y ? R.end_y : R.max_y
					);
			}
		}
	}


void ns_drag_rect_draw
	(
	NsDragRect  *R,
	void         ( *xor_line )( nspointer, nsint, nsint, nsint, nsint ),
	nspointer    user_data
	)
	{
	ns_assert( NULL != R );
	ns_assert( NULL != xor_line );

	if( ! R->visible )
		{
		R->visible = NS_TRUE;
		_ns_drag_rect_xor( R, xor_line, user_data );
		}
	}


void ns_drag_rect_erase
	(
	NsDragRect  *R,
	void         ( *xor_line )( nspointer, nsint, nsint, nsint, nsint ),
	nspointer    user_data
	)
	{
	ns_assert( NULL != R );
	ns_assert( NULL != xor_line );

	if( R->visible )
		{
		R->visible = NS_FALSE;
		_ns_drag_rect_xor( R, xor_line, user_data );
		}
	}
