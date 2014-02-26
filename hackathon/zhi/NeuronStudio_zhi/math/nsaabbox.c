#include "nsaabbox.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


#define _NS_AABBOX2( _B, _op1, _x, _y, _op2, _w, _h )\
	( _B )->O.x    _op1 ( _x );\
	( _B )->O.y    _op1 ( _y );\
	( _B )->width  _op2 ( _w );\
	( _B )->height _op2 ( _h );\
	return ( _B )

#define _NS_AABBOX3( _B, _op1, _x, _y, _z, _op2, _w, _h, _l )\
	( _B )->O.x    _op1 ( _x );\
	( _B )->O.y    _op1 ( _y );\
	( _B )->O.z    _op1 ( _z );\
	( _B )->width  _op2 ( _w );\
	( _B )->height _op2 ( _h );\
	( _B )->length _op2 ( _l );\
	return ( _B )


#define _NS_AABBOX2_INIT( B, x, y, w, h )\
	ns_assert( NULL != (B) );\
	_NS_AABBOX2( (B), =, (x), (y), =, (w), (h) )

#define _NS_AABBOX3_INIT( B, x, y, z, w, h, l )\
	ns_assert( NULL != (B) );\
	_NS_AABBOX3( (B), =, (x), (y), (z), =, (w), (h), (l) )

NsAABBox2d* ns_aabbox2d
	(
	NsAABBox2d  *B,
	nsdouble     x_left,
	nsdouble     y_top,
	nsdouble     width,
	nsdouble     height
	)
	{  _NS_AABBOX2_INIT( B, x_left, y_top, width, height );  }

NsAABBox3d* ns_aabbox3d
	(
	NsAABBox3d  *B,
	nsdouble     x_left,
	nsdouble     y_top,
	nsdouble     z_near,
	nsdouble     width,
	nsdouble     height,
	nsdouble     length
	)
	{  _NS_AABBOX3_INIT( B, x_left, y_top, z_near, width, height, length );  }


NS_PRIVATE void _ns_aabbox3d_do_render
	(
	const NsAABBox3d  *B,
	void              ( *line_func )( nspointer, const NsVector3d*, const NsVector3d* ),
	nsdouble          ox,
	nsdouble          oy,
	nsdouble          oz,
	nspointer         user_data
	)
	{
	NsVector3d  corners[8];
	NsVector3d  D;


	ns_aabbox3d_corners( B, corners );

	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_NEAR ]; D.x += ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_NEAR ]; D.y -= oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_NEAR ]; D.z -= oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_NEAR, &D );

	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_NEAR ]; D.x -= ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_NEAR ]; D.y -= oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_NEAR]; D.z -= oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_NEAR, &D );

	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR ]; D.x += ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR ]; D.y += oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR ]; D.z -= oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR, &D );

	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR ]; D.x -= ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR ]; D.y += oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR ]; D.z -= oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR, &D );

	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_FAR ]; D.x += ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_FAR ]; D.y -= oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_TOP_FAR ]; D.z += oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_TOP_FAR, &D );

	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_FAR ]; D.x -= ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_FAR ]; D.y -= oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_TOP_FAR ]; D.z += oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_TOP_FAR, &D );

	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR ]; D.x += ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR ]; D.y += oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR ]; D.z += oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR, &D );

	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR ]; D.x -= ox;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR ]; D.y += oy;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR, &D );
	D = corners[ NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR ]; D.z += oz;
	( line_func )( user_data, corners + NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR, &D );
	}


void ns_aabbox3d_render
	(
	const NsAABBox3d  *B,
	void              ( *line_func )( nspointer, const NsVector3d*, const NsVector3d* ),
	nspointer         user_data
	)
	{
	ns_assert( NULL != B );
	ns_assert( NULL != line_func );

	_ns_aabbox3d_do_render(
		B,
		line_func,
		ns_aabbox3d_width( B )  / 4.0,
		ns_aabbox3d_height( B ) / 4.0,
		ns_aabbox3d_length( B ) / 4.0,
		user_data
		);
	}


void ns_aabbox3d_render_ex
	(
	const NsAABBox3d   *B,
	void               ( *line_func )( nspointer, const NsVector3d*, const NsVector3d* ),
	const NsVector3d  *line_length,
	nspointer          user_data
	)
	{
	ns_assert( NULL != B );
	ns_assert( NULL != line_func );
	ns_assert( NULL != line_length );

	_ns_aabbox3d_do_render(
		B,
		line_func,
		line_length->x,
		line_length->y,
		line_length->z,
		user_data
		);
	}


#define _NS_AABBOX_ORIGIN( B, _O )\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (_O) );\
	*(_O) = box->O;\
	return (_O)

NsPoint2d* ns_aabbox2d_origin( const NsAABBox2d *box, NsPoint2d *O )
	{  _NS_AABBOX_ORIGIN( box, O );  }

NsPoint3d* ns_aabbox3d_origin( const NsAABBox3d *box, NsPoint3d *O )
	{  _NS_AABBOX_ORIGIN( box, O );  }


#define _NS_AABBOX3_CORNER_LTN( B, C, t )\
	ns_vector3##t( (C), (B)->O.x,              (B)->O.y,               (B)->O.z               )

#define _NS_AABBOX3_CORNER_RTN( B, C, t )\
	ns_vector3##t( (C), (B)->O.x + (B)->width, (B)->O.y,               (B)->O.z               )

#define _NS_AABBOX3_CORNER_LBN( B, C, t )\
	ns_vector3##t( (C), (B)->O.x,              (B)->O.y - (B)->height, (B)->O.z               )

#define _NS_AABBOX3_CORNER_RBN( B, C, t )\
	ns_vector3##t( (C), (B)->O.x + (B)->width, (B)->O.y - (B)->height, (B)->O.z               )

#define _NS_AABBOX3_CORNER_LTF( B, C, t )\
	ns_vector3##t( (C), (B)->O.x,              (B)->O.y,               (B)->O.z - (B)->length )

#define _NS_AABBOX3_CORNER_RTF( B, C, t )\
	ns_vector3##t( (C), (B)->O.x + (B)->width, (B)->O.y,               (B)->O.z - (B)->length )

#define _NS_AABBOX3_CORNER_LBF( B, C, t )\
	ns_vector3##t( (C), (B)->O.x,              (B)->O.y - (B)->height, (B)->O.z - (B)->length )

#define _NS_AABBOX3_CORNER_RBF( B, C, t )\
	ns_vector3##t( (C), (B)->O.x + (B)->width, (B)->O.y - (B)->height, (B)->O.z - (B)->length )


#define _NS_AABBOX3_CORNER( B, which, C, t )\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	switch( (which) )\
		{\
		case NS_AABBOX3_CORNER_LEFT_TOP_NEAR:     _NS_AABBOX3_CORNER_LTN( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_RIGHT_TOP_NEAR:    _NS_AABBOX3_CORNER_RTN( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR:  _NS_AABBOX3_CORNER_LBN( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR: _NS_AABBOX3_CORNER_RBN( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_LEFT_TOP_FAR:      _NS_AABBOX3_CORNER_LTF( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_RIGHT_TOP_FAR:     _NS_AABBOX3_CORNER_RTF( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR:   _NS_AABBOX3_CORNER_LBF( (B), (C), t ); break;\
		case NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR:  _NS_AABBOX3_CORNER_RBF( (B), (C), t ); break;\
		default: ns_assert_not_reached();\
		}\
	return (C)

NsPoint3d* ns_aabbox3d_corner
	(
	const NsAABBox3d     *box,
	NsAABBox3CornerType   which,
	NsPoint3d            *corner
	)
	{  _NS_AABBOX3_CORNER( box, which, corner, d );  }


#define _NS_AABBOX3_CORNERS( B, C, t )\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_AABBOX3_CORNER_LTN( (B), (C) + NS_AABBOX3_CORNER_LEFT_TOP_NEAR,     t );\
	_NS_AABBOX3_CORNER_RTN( (B), (C) + NS_AABBOX3_CORNER_RIGHT_TOP_NEAR,    t );\
	_NS_AABBOX3_CORNER_LBN( (B), (C) + NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR,  t );\
	_NS_AABBOX3_CORNER_RBN( (B), (C) + NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR, t );\
	_NS_AABBOX3_CORNER_LTF( (B), (C) + NS_AABBOX3_CORNER_LEFT_TOP_FAR,      t );\
	_NS_AABBOX3_CORNER_RTF( (B), (C) + NS_AABBOX3_CORNER_RIGHT_TOP_FAR,     t );\
	_NS_AABBOX3_CORNER_LBF( (B), (C) + NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR,   t );\
	_NS_AABBOX3_CORNER_RBF( (B), (C) + NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR,  t )

void ns_aabbox3d_corners
	(
	const NsAABBox3d  *box,
	NsPoint3d          corners[ 8 ]
	)
	{  _NS_AABBOX3_CORNERS( box, corners, d );  }


#define _NS_AABBOX3_MIN( B, P, t )\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (P) );\
	return _NS_AABBOX3_CORNER_LBF( (B), (P), t )
	

NsPoint3d* ns_aabbox3d_min( const NsAABBox3d *box, NsPoint3d *P )
	{  _NS_AABBOX3_MIN( box, P, d );  }


#define _NS_AABBOX3_MAX( B, P, t )\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (P) );\
	return _NS_AABBOX3_CORNER_RTN( (B), (P), t )

NsPoint3d* ns_aabbox3d_max( const NsAABBox3d *box, NsPoint3d *P )
	{  _NS_AABBOX3_MAX( box, P, d );  }


#define _NS_AABBOX2_SCALE( C, A, sw, sh )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX2(\
		(C),\
		=,\
		(A)->O.x,\
		(A)->O.y,\
		=,\
		(A)->width  * (sw),\
		(A)->height * (sh)\
		)

#define _NS_AABBOX3_SCALE( C, A, sw, sh, sl )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX3(\
		(C),\
		=,\
		(A)->O.x,\
		(A)->O.y,\
		(A)->O.z,\
		=,\
		(A)->width  * (sw),\
		(A)->height * (sh),\
		(A)->length * (sl)\
		)

NsAABBox2d* ns_aabbox2d_scale
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           s
	)
	{  _NS_AABBOX2_SCALE( C, A, s, s );  }

NsAABBox3d* ns_aabbox3d_scale
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           s
	)
	{  _NS_AABBOX3_SCALE( C, A, s, s, s );  }

NsAABBox2d* ns_aabbox2d_non_uni_scale
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	const NsVector2d  *S
	)
	{  _NS_AABBOX2_SCALE( C, A, S->x, S->y );  }

NsAABBox3d* ns_aabbox3d_non_uni_scale
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	const NsVector3d  *S
	)
	{  _NS_AABBOX3_SCALE( C, A, S->x, S->y, S->z );  }

NsAABBox2d* ns_aabbox2d_scale_xy
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           sx,
	nsdouble           sy
	)
	{  _NS_AABBOX2_SCALE( C, A, sx, sy );  }

NsAABBox3d* ns_aabbox3d_scale_xyz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           sx,
	nsdouble           sy,
	nsdouble           sz
	)
	{  _NS_AABBOX3_SCALE( C, A, sx, sy, sz );  }


#define _NS_AABBOX2_TRANSLATE( C, A, _x, _y )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX2(\
		(C),\
		=,\
		(A)->O.x + (_x),\
		(A)->O.y + (_y),\
		=,\
		(A)->width,\
		(A)->height\
		)

#define _NS_AABBOX3_TRANSLATE( C, A, _x, _y, _z )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX3(\
		(C),\
		=,\
		(A)->O.x + (_x),\
		(A)->O.y + (_y),\
		(A)->O.z + (_z),\
		=,\
		(A)->width,\
		(A)->height,\
		(A)->length\
		)

NsAABBox2d* ns_aabbox2d_translate
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	const NsVector2d  *V
	)
	{  _NS_AABBOX2_TRANSLATE( C, A, V->x, V->y );  }

NsAABBox3d* ns_aabbox3d_translate
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	const NsVector3d  *V
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, V->x, V->y, V->z );  }	

NsAABBox2d* ns_aabbox2d_translate_xy
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           x,
	nsdouble           y
	)
	{  _NS_AABBOX2_TRANSLATE( C, A, x, y );  }

NsAABBox3d* ns_aabbox3d_translate_xyz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           y,
	nsdouble           z
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, x, y, z );  }

NsAABBox2d* ns_aabbox2d_translate_x
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           x
	)
	{  _NS_AABBOX2_TRANSLATE( C, A, x, 0.0 );  }

 NsAABBox2d* ns_aabbox2d_translate_y
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           y
	)
	{  _NS_AABBOX2_TRANSLATE( C, A, 0.0, y );  }

NsAABBox3d* ns_aabbox3d_translate_x
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, x, 0.0, 0.0 );  }

NsAABBox3d* ns_aabbox3d_translate_y
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           y
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, 0.0, y, 0.0 );  }

NsAABBox3d* ns_aabbox3d_translate_z
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           z
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, 0.0, 0.0, z );  }

NsAABBox3d* ns_aabbox3d_translate_xy
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           y
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, x, y, 0.0 );  }

NsAABBox3d* ns_aabbox3d_translate_xz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           z
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, x, 0.0, z );  }

NsAABBox3d* ns_aabbox3d_translate_yz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           y,
	nsdouble           z
	)
	{  _NS_AABBOX3_TRANSLATE( C, A, 0.0, y, z );  }


#define _NS_AABBOX2_XFLATE( C, A, l, r, t, b )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX2(\
		(C),\
		=,\
		(A)->O.x - (l),\
		(A)->O.y + (t),\
		=,\
		(A)->width + (l) + (r),\
		(A)->height + (t) + (b)\
		)

#define _NS_AABBOX3_XFLATE( C, A, l, r, t, b, n, f )\
	ns_assert( NULL != (C) );\
	ns_assert( NULL != (A) );\
	_NS_AABBOX3(\
		(C),\
		=,\
		(A)->O.x - (l),\
		(A)->O.y + (t),\
		(A)->O.z + (n),\
		=,\
		(A)->width + (l) + (r),\
		(A)->height + (t) + (b),\
		(A)->length + (n) + (f)\
		)

NsAABBox2d* ns_aabbox2d_xflate( NsAABBox2d *C, const NsAABBox2d *A, nsdouble v )
	{  _NS_AABBOX2_XFLATE( C, A, v, v, v, v );  }

NsAABBox3d* ns_aabbox3d_xflate( NsAABBox3d *C, const NsAABBox3d *A, nsdouble v )
	{  _NS_AABBOX3_XFLATE( C, A, v, v, v, v, v, v );  }


#define _NS_AABBOX2_CMPD_XFLATE( B, l, r, t, b )\
	ns_assert( NULL != (B) );\
	(B)->O.x    -= (l);\
	(B)->O.y    += (t);\
	(B)->width  += ( (l) + (r) );\
	(B)->height += ( (t) + (b) );\
	return (B)

#define _NS_AABBOX3_CMPD_XFLATE( B, l, r, t, b, n, f )\
	ns_assert( NULL != (B) );\
	(B)->O.x    -= (l);\
	(B)->O.y    += (t);\
	(B)->O.z    += (n);\
	(B)->width  += ( (l) + (r) );\
	(B)->height += ( (t) + (b) );\
	(B)->length += ( (n) + (f) );\
	return (B)

NsAABBox2d* ns_aabbox2d_cmpd_xflate( NsAABBox2d *B, nsdouble v )
	{  _NS_AABBOX2_CMPD_XFLATE( B, v, v, v, v );  }

NsAABBox3d* ns_aabbox3d_cmpd_xflate( NsAABBox3d *B, nsdouble v )
	{  _NS_AABBOX3_CMPD_XFLATE( B, v, v, v, v, v, v );  }


nsuint8 ns_aabbox3d_cohen_sutherland_code
	(
	const NsAABBox3d  *B,
	const NsPoint3d   *P
	)
	{
	nsuint8 code = 0;

	ns_assert( NULL != B );
	ns_assert( NULL != P );

	if( P->x < B->O.x )
		code |= NS_COHEN_SUTHERLAND_LEFT;
	else if( P->x >= B->O.x + B->width )
		code |= NS_COHEN_SUTHERLAND_RIGHT;

	if( P->y > B->O.y )
		code |= NS_COHEN_SUTHERLAND_TOP;
	else if( P->y <= B->O.y - B->height )
		code |= NS_COHEN_SUTHERLAND_BOTTOM;

	if( P->z > B->O.z )
		code |= NS_COHEN_SUTHERLAND_NEAR;
	else if( P->z <= B->O.z - B->length )
		code |= NS_COHEN_SUTHERLAND_FAR;

	return code;
	}


#define _NS_AABBOX2_INTERSECTS_AABBOX( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return\
	( _ns_aabbox_left(A)   <= _ns_aabbox_right(B) && _ns_aabbox_right(A) >= _ns_aabbox_left(B)   ) &&\
	( _ns_aabbox_bottom(A) <= _ns_aabbox_top(B)   && _ns_aabbox_top(A)   >= _ns_aabbox_bottom(B) )

#define _NS_AABBOX3_INTERSECTS_AABBOX( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return\
	( _ns_aabbox_left(A)   <= _ns_aabbox_right(B) && _ns_aabbox_right(A) >= _ns_aabbox_left(B)   ) &&\
	( _ns_aabbox_bottom(A) <= _ns_aabbox_top(B)   && _ns_aabbox_top(A)   >= _ns_aabbox_bottom(B) ) &&\
	( _ns_aabbox_far(A)    <= _ns_aabbox_near(B)  && _ns_aabbox_near(A)  >= _ns_aabbox_far(B)    )

nsboolean ns_aabbox2d_intersects_aabbox
	(
	const NsAABBox2d  *A,
	const NsAABBox2d  *B
	)
	{  _NS_AABBOX2_INTERSECTS_AABBOX( A, B );  }

nsboolean ns_aabbox3d_intersects_aabbox
	(
	const NsAABBox3d  *A,
	const NsAABBox3d  *B
	)
	{  _NS_AABBOX3_INTERSECTS_AABBOX( A, B );  }
