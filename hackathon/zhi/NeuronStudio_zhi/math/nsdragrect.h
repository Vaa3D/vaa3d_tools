#ifndef __NS_MATH_DRAGRECT_H__
#define __NS_MATH_DRAGRECT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <math/nsdragrectdecls.h>

NS_DECLS_BEGIN

#define ns_drag_rect_start_x( R )\
	( (R)->start_x )

#define ns_drag_rect_start_y( R )\
	( (R)->start_y )

#define ns_drag_rect_end_x( R )\
	( (R)->end_x )

#define ns_drag_rect_end_y( R )\
	( (R)->end_y )

NS_IMPEXP void ns_drag_rect_bounds
	(
	NsDragRect  *R,
	nsint        min_x,
	nsint        min_y,
	nsint        max_x,
	nsint        max_y
	);

NS_IMPEXP void ns_drag_rect_set_tolerance( NsDragRect *R, nsint tolerance );
NS_IMPEXP nsint ns_drag_rect_get_tolerance( const NsDragRect *R );

NS_IMPEXP void ns_drag_rect_start( NsDragRect *R, nsint x, nsint y );
NS_IMPEXP void ns_drag_rect_end( NsDragRect *R, nsint x, nsint y );

/* These functions always maintain a normalized rectangle. */
NS_IMPEXP void ns_drag_rect_left( NsDragRect *R, nsint x );
NS_IMPEXP void ns_drag_rect_top( NsDragRect *R, nsint y );
NS_IMPEXP void ns_drag_rect_right( NsDragRect *R, nsint x );
NS_IMPEXP void ns_drag_rect_bottom( NsDragRect *R, nsint y );
NS_IMPEXP void ns_drag_rect_left_top( NsDragRect *R, nsint x, nsint y );
NS_IMPEXP void ns_drag_rect_right_top( NsDragRect *R, nsint x, nsint y );
NS_IMPEXP void ns_drag_rect_left_bottom( NsDragRect *R, nsint x, nsint y );
NS_IMPEXP void ns_drag_rect_right_bottom( NsDragRect *R, nsint x, nsint y );

NS_IMPEXP nsboolean ns_drag_rect_valid( const NsDragRect *R );

NS_IMPEXP void ns_drag_rect_normalize( NsDragRect *R );
NS_IMPEXP nsboolean ns_drag_rect_is_normalized( const NsDragRect *R );

NS_IMPEXP nsint ns_drag_rect_width( const NsDragRect *R );
NS_IMPEXP nsint ns_drag_rect_height( const NsDragRect *R );

NS_IMPEXP void ns_drag_rect_dimensions
	(
	const NsDragRect  *R,
	nsint             *width,
	nsint             *height
	);

NS_IMPEXP void ns_drag_rect_clip( NsDragRect *R );

NS_IMPEXP void ns_drag_rect_draw
	(
	NsDragRect  *R,
	void         ( *xor_line )( nspointer, nsint, nsint, nsint, nsint ),
	nspointer    user_data
	);

NS_IMPEXP void ns_drag_rect_erase
	(
	NsDragRect  *R,
	void         ( *xor_line )( nspointer, nsint, nsint, nsint, nsint ),
	nspointer    user_data
	);

NS_DECLS_END

#endif/* __NS_MATH_DRAGRECT_H__ */
