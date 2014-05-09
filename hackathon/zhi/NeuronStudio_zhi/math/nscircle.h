#ifndef __NS_MATH_CIRCLE_H__
#define __NS_MATH_CIRCLE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <math/nscircledecls.h>
#include <math/nspointdecls.h>
#include <math/nsdragrectdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsCircled* ns_circled
	(
	NsCircled  *C,
	nsdouble    x,
	nsdouble    y,
	nsdouble    r
	);

NS_IMPEXP NsCirclef* ns_circlef
	(
	NsCirclef  *C,
	nsfloat     x,
	nsfloat     y,
	nsfloat     r
	);


/* Internal. DO NOT USE! */
#define _ns_circle_center_x( C )  ( (C)->O.x )
#define _ns_circle_center_y( C )  ( (C)->O.y )
#define _ns_circle_radius( C )    ( (C)->r   )


#define ns_circled_center_x( C )  _ns_circle_center_x( (C) )
#define ns_circled_center_y( C )  _ns_circle_center_y( (C) )
#define ns_circled_radius( C )    _ns_circle_radius( (C) )

#define ns_circlef_center_x( C )  _ns_circle_center_x( (C) )
#define ns_circlef_center_y( C )  _ns_circle_center_y( (C) )
#define ns_circlef_radius( C )    _ns_circle_radius( (C) )


/* Returns 'O' */
NS_IMPEXP NsPoint2d* ns_circled_center( const NsCircled *C, NsPoint2d *O );
NS_IMPEXP NsPoint2f* ns_circlef_center( const NsCirclef *C, NsPoint2f *O );


NS_IMPEXP nsboolean ns_circled_intersects_drag_rect
	(
	const NsCircled   *C,
	const NsDragRect  *R
	);

NS_DECLS_END

#endif/* __NS_MATH_CIRCLE_H__ */
