#include "nsrender2d-os-windows.h"


NS_PRIVATE void _ns_render_state_delete_pen( nspointer pen )
	{  DeleteObject( pen );  }


NS_PRIVATE void _ns_render_state_init_pen( NsRenderState *state )
	{  state->variables.curr_pen = NULL;  }


NS_PRIVATE void _ns_render_state_finalize_pen( NsRenderState *state )
	{
	if( NULL != state->variables.curr_pen )
		{
		SelectObject( state->constants.graphics, state->variables.prev_pen );
		state->variables.curr_pen = NULL;
		}
	}


NS_PRIVATE nsint _ns_render_state_size_pen( NsRenderState *state )
	{
	return ( ! state->variables.filled ||
				/* state->constants.polygon_borders || */
				ns_render_state_is_line_art( state ) )
			 ? ( nsint )state->constants.line_size : 1;
	}


NS_PRIVATE nsuint _ns_render_state_color_pen( NsRenderState *state )
	{
	/* NOTE: Assuming 0 is the color black. */
	return ( state->variables.filled &&
				state->constants.polygon_borders &&
				ns_render_state_is_polygon_art( state ) )
			 ? 0 : NS_COLOR4UB_TO_UINT32( state->variables.color );
	}


NS_PRIVATE void _ns_render_state_on_pen( NsRenderState *state )
	{
	if( NULL == state->constants.pen_db )
		{
		state->variables.curr_pen = GetStockObject( WHITE_PEN );
		state->variables.prev_pen = SelectObject( state->constants.graphics, state->variables.curr_pen );
		}
	else			
		{
		nsint   width;
		nsuint  color;


		width = _ns_render_state_size_pen( state );
		color = _ns_render_state_color_pen( state );

		/* IMPORTANT Pack on the width to the upper byte.
			Assuming width less than 256 so its OK. */
		state->variables.curr_pen_color = color | ( ( nsuint )width << 24 );

		/* Check if the color has changed from the previous one,
			or if this is the first one being used. */
		if( NULL == state->variables.curr_pen ||
			 state->variables.curr_pen_color != state->variables.prev_pen_color )
			{
			state->variables.prev_pen_color = state->variables.curr_pen_color;

			_ns_render_state_finalize_pen( state );

			state->variables.curr_pen =
				( state->constants.get_pen )(
					state->constants.pen_db,
					state->variables.curr_pen_color
					);

			if( NULL == state->variables.curr_pen )
				{
				/* NOTE: Ignoring failure of creation and setting it
					in the database. */

				state->variables.curr_pen = CreatePen( PS_SOLID, width, color );

				( state->constants.set_pen )(
					state->constants.pen_db,
					state->variables.curr_pen_color,
					state->variables.curr_pen,
					_ns_render_state_delete_pen
					);
				}

			state->variables.prev_pen = SelectObject( state->constants.graphics, state->variables.curr_pen );
			}
		}
	}


NS_PRIVATE void _ns_render_state_delete_brush( nspointer brush )
	{  DeleteObject( brush );  }


NS_PRIVATE void _ns_render_state_init_brush( NsRenderState *state )
	{  state->variables.curr_brush = NULL;  }


NS_PRIVATE void _ns_render_state_finalize_brush( NsRenderState *state )
	{
	if( NULL != state->variables.curr_brush )
		{
		SelectObject( state->constants.graphics, state->variables.prev_brush );
		state->variables.curr_brush = NULL;
		}
	}


NS_PRIVATE void _ns_render_state_on_brush( NsRenderState *state )
	{
	if( NULL == state->constants.brush_db )
		{
		state->variables.curr_brush = GetStockObject( WHITE_BRUSH );
		state->variables.prev_brush = SelectObject( state->constants.graphics, state->variables.curr_brush );
		}
	else			
		{
		state->variables.curr_brush_color = NS_COLOR4UB_TO_UINT32( state->variables.color );

		/* Check if the color has changed from the previous one,
			or if this is the first one being used. */
		if( NULL == state->variables.curr_brush ||
			 state->variables.curr_brush_color != state->variables.prev_brush_color )
			{
			state->variables.prev_brush_color = state->variables.curr_brush_color;

			_ns_render_state_finalize_brush( state );

			state->variables.curr_brush =
				( state->constants.get_brush )(
					state->constants.brush_db,
					state->variables.curr_brush_color
					);

			if( NULL == state->variables.curr_brush )
				{
				/* NOTE: Ignoring failure of creation and setting it
					in the database. */

				state->variables.curr_brush = CreateSolidBrush( state->variables.curr_brush_color );

				( state->constants.set_brush )(
					state->constants.brush_db,
					state->variables.curr_brush_color,
					state->variables.curr_brush,
					_ns_render_state_delete_brush
					);
				}

			state->variables.prev_brush = SelectObject( state->constants.graphics, state->variables.curr_brush );
			}
		}
	}




NS_COMPILE_TIME_SIZE_EQUAL( NsColor4ub, RGBQUAD );

NS_PRIVATE void _ns_render2d_create_palette( NsRenderState *state )
	{
	BITMAPINFOHEADER  *hdr;
	nssize             bytes;


	bytes = sizeof( BITMAPINFOHEADER ) + sizeof( NsColor4ub ) * state->constants.palette_size;

	state->variables.palette = ns_malloc( bytes + sizeof( nscookie ) );

	if( NULL == ( hdr = state->variables.palette ) )
		return;

	ns_cookie_set( state->variables.palette, bytes );

	ns_memcpy(
		( ( nsuint8* )state->variables.palette ) + sizeof( BITMAPINFOHEADER ),
		state->constants.palette_colors,
		sizeof( NsColor4ub ) * state->constants.palette_size
		);

	ns_cookie_valid( state->variables.palette, bytes );

	hdr->biSize          = sizeof( BITMAPINFOHEADER );
	hdr->biPlanes        = 1;
	hdr->biCompression   = BI_RGB;
	hdr->biSizeImage     = 0;
	hdr->biXPelsPerMeter = 0;
	hdr->biYPelsPerMeter = 0;
	hdr->biClrUsed       = 0;
	hdr->biClrImportant  = 0;
	}


void ns_render2d_pre_images( NsRenderState *state )
	{  state->variables.palette = NULL;  }


void ns_render2d_post_images( NsRenderState *state )
	{
	ns_delete( state->variables.palette );
	state->variables.palette = NULL;
	}


NS_PRIVATE void _ns_render2d_image
	(
	NsRenderState  *state,
	nsint           x,
	nsint           y,
	nsint           dest_width,
	nsint           dest_height,
	const NsImage  *image
	)
	{
	BITMAPINFOHEADER  *hdr;
	nsint              src_width, src_height;


	/* NOTE: Defer allocation until an image needs to be rendered.
		This saves creating and initializing a palette that wont
		be used. */

	if( NULL == state->variables.palette )	
		_ns_render2d_create_palette( state );

	if( NULL == ( hdr = state->variables.palette ) )
		return;

	src_width  = ( nsint )ns_image_width( image );
	src_height = ( nsint )ns_image_height( image );

	hdr->biWidth    = src_width;
	hdr->biHeight   = -src_height;
	hdr->biBitCount = ( nsushort )ns_pixel_bits( ns_image_pixel_type( image ) );

	StretchDIBits(
		state->constants.graphics,
		x,
		y,
		dest_width,
		dest_height,
		0, 
		0, 
		src_width,
		src_height,
		ns_image_pixels( image ),
		state->variables.palette,
		DIB_RGB_COLORS,
		SRCCOPY
		);
	}


#define _NS_RENDER2D_IMAGE( c1, c2 )\
	NsVector2f A, B, C;\
	\
	A.x = ( nsfloat )V->c1;\
	A.y = ( nsfloat )V->c2;\
	\
	if( 0 == ns_image_size( image_##c1##c2 ) )\
		return;\
	\
	B.x = A.x + ( nsfloat )ns_image_width( image_##c1##c2 );\
	B.y = A.y + ( nsfloat )ns_image_height( image_##c1##c2 );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, B, C );\
	\
	_ns_render2d_image(\
		state,\
		( nsint )A.x,\
		( nsint )A.y,\
		( nsint )( B.x - A.x ),\
		( nsint )( B.y - A.y ),\
		image_##c1##c2\
		)


void ns_render2d_image_xy
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_IMAGE( x, y );

	NS_USE_VARIABLE( image_zy );
	NS_USE_VARIABLE( image_xz );
	}


void ns_render2d_image_zy
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_IMAGE( z, y );

	NS_USE_VARIABLE( image_xy );
	NS_USE_VARIABLE( image_xz );
	}


void ns_render2d_image_xz
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_IMAGE( x, z );

	NS_USE_VARIABLE( image_xy );
	NS_USE_VARIABLE( image_zy );
	}




void ns_render2d_pre_texts( NsRenderState *state )
	{
	state->variables.bk_mode =
		SetBkMode(
			state->constants.graphics,
			TRANSPARENT
			);

	state->variables.text_color =
		SetTextColor(
			state->constants.graphics,
			NS_COLOR4UB_TO_UINT32( state->variables.color )
			);
	}


void ns_render2d_post_texts( NsRenderState *state )
	{
	SetBkMode( state->constants.graphics, state->variables.bk_mode );
	SetTextColor( state->constants.graphics, state->variables.text_color );
	}


#define _NS_RENDER2D_TEXT( c1, c2 )\
	NsVector2f A, C;\
	\
	A.x = NS_TO_IMAGE_SPACE( V->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	A.y = NS_TO_IMAGE_SPACE( V->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	\
	TextOut(\
		state->constants.graphics,\
		( nsint )A.x,\
		( nsint )A.y,\
		text,\
		( nsint )ns_ascii_strlen( text )\
		)


void ns_render2d_text_xy
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_TEXT( x, y );  }


void ns_render2d_text_zy
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_TEXT( z, y );  }


void ns_render2d_text_xz
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_TEXT( x, z );  }




void ns_render2d_pre_borders( NsRenderState *state )
	{  _ns_render_state_init_brush( state );  }


void ns_render2d_post_borders( NsRenderState *state )
	{  _ns_render_state_finalize_brush( state );  }


NS_PRIVATE void _ns_render2d_border
	(
	NsRenderState  *state,
	nsint           x,
	nsint           y,
	nsint           width,
	nsint           height
	)
	{
	RECT rc;

	_ns_render_state_on_brush( state );

	rc.left   = x;
	rc.top    = y;
	rc.right  = x + width;
	rc.bottom = y + height;

	FrameRect( state->constants.graphics, &rc, state->variables.curr_brush );
	}


#define _NS_RENDER2D_BORDER( c1, c2 )\
	NsVector2f A, B, C;\
	\
	A.x = ( nsfloat )V1->c1;\
	A.y = ( nsfloat )V1->c2;\
	B.x = ( nsfloat )V2->c1 + 1.0f;\
	B.y = ( nsfloat )V2->c2 + 1.0f;\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, B, C );\
	\
	_ns_render2d_border(\
		state,\
		( nsint )A.x,\
		( nsint )A.y,\
		( nsint )( B.x - A.x ),\
		( nsint )( B.y - A.y )\
		)


void ns_render2d_border_xy
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_BORDER( x, y );  }


void ns_render2d_border_zy
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_BORDER( z, y );  }


void ns_render2d_border_xz
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_BORDER( x, z );  }




void ns_render2d_pre_lines( NsRenderState *state )
	{  _ns_render_state_init_pen( state );  }


void ns_render2d_post_lines( NsRenderState *state )
	{  _ns_render_state_finalize_pen( state );  }


NS_PRIVATE void _ns_render2d_line( NsRenderState *state, nsint x1, nsint y1, nsint x2, nsint y2 )
	{
	_ns_render_state_on_pen( state );

	MoveToEx( state->constants.graphics, x1, y1, NULL );
	LineTo( state->constants.graphics, x2, y2 );
	}


#define _NS_RENDER2D_LINE( c1, c2 )\
	NsVector2f A, B, C;\
	\
	A.x = NS_TO_IMAGE_SPACE( V1->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	A.y = NS_TO_IMAGE_SPACE( V1->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	B.x = NS_TO_IMAGE_SPACE( V2->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	B.y = NS_TO_IMAGE_SPACE( V2->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, B, C );\
	\
	_ns_render2d_line( state, ( nsint )A.x, ( nsint )A.y, ( nsint )B.x, ( nsint )B.y )


void ns_render2d_line_xy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_LINE( x, y );  }


void ns_render2d_line_zy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_LINE( z, y );  }


void ns_render2d_line_xz
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_LINE( x, z );  }




void ns_render2d_pre_points( NsRenderState *state )
	{  _ns_render_state_init_brush( state );  }


void ns_render2d_post_points( NsRenderState *state )
	{  _ns_render_state_finalize_brush( state );  }


NS_PRIVATE void _ns_render2d_point
	(
	NsRenderState  *state,
	nsint           left,
	nsint           top,
	nsint           right,
	nsint           bottom
	)
	{
	RECT rc;

	_ns_render_state_on_brush( state );

	rc.left   = left;
	rc.top    = top;
	rc.right  = right;
	rc.bottom = bottom;

	if( 1 == state->constants.point_size )
		SetPixel( state->constants.graphics, left, top, state->variables.curr_brush_color );
	else
		FillRect( state->constants.graphics, &rc, state->variables.curr_brush );
	}


#define _NS_RENDER2D_POINT( c1, c2 )\
	NsVector2f  Vf, C;\
	NsVector2i  Vi;\
	nsint       diameter, radius;\
	\
	Vf.x = NS_TO_IMAGE_SPACE( P->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	Vf.y = NS_TO_IMAGE_SPACE( P->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, Vf, C );\
	\
	Vi.x = ( nsint )Vf.x;\
	Vi.y = ( nsint )Vf.y;\
	\
	diameter = ( nsint )state->constants.point_size;\
	radius   = diameter / 2;\
	\
	_ns_render2d_point(\
		state,\
		Vi.x - radius,\
		Vi.y - radius,\
		Vi.x - radius + diameter,\
		Vi.y - radius + diameter\
		)


void ns_render2d_point_xy
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_POINT( x, y );
	NS_USE_VARIABLE( N );
	}


void ns_render2d_point_zy
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_POINT( z, y );
	NS_USE_VARIABLE( N );
	}


void ns_render2d_point_xz
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_POINT( x, z );
	NS_USE_VARIABLE( N );
	}




void ns_render2d_pre_spheres( NsRenderState *state )
	{  ns_render2d_pre_ellipses( state );  }


void ns_render2d_post_spheres( NsRenderState *state )
	{  ns_render2d_post_ellipses( state );  }


NS_PRIVATE void _ns_render2d_ellipse( NsRenderState *state, nsint ox, nsint oy, nsint rx, nsint ry )
	{
	_ns_render_state_on_brush( state );
	_ns_render_state_on_pen( state );

	if( state->variables.filled )
		Ellipse(
			state->constants.graphics,
			ox - rx,
			oy - ry,
			ox + rx + 1,
			oy + ry + 1
			);
	else
		Arc(
			state->constants.graphics,
			ox - rx,
			oy - ry,
			ox + rx + 1,
			oy + ry + 1,
			ox - rx,
			oy,
			ox - rx,
			oy
			);
	}


#define _NS_RENDER2D_SPHERE( c1, c2 )\
	NsVector2f V, R, C;\
	\
	V.x = NS_TO_IMAGE_SPACE( P->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V.y = NS_TO_IMAGE_SPACE( P->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	R.x = NS_TO_IMAGE_SPACE( radius * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	R.y = NS_TO_IMAGE_SPACE( radius * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	R.x -= .5f;\
	R.y -= .5f;\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V, C );\
	ns_vector2f_cmpd_scale( &R, state->constants.zoom );\
	\
	_ns_render2d_ellipse( state, ( nsint )V.x, ( nsint )V.y, ( nsint )R.x, ( nsint )R.y )


void ns_render2d_sphere_xy
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_SPHERE( x, y );  }


void ns_render2d_sphere_zy
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_SPHERE( z, y );  }


void ns_render2d_sphere_xz
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_SPHERE( x, z );  }




void ns_render2d_pre_aabboxes( NsRenderState *state )
	{  _ns_render_state_init_pen( state );  }


void ns_render2d_post_aabboxes( NsRenderState *state )
	{  _ns_render_state_finalize_pen( state );  }


NS_PRIVATE void _ns_render2d_aabbox
	(
	NsRenderState  *state,
	nsint           x,
	nsint           y,
	nsint           width,
	nsint           height
	)
	{
	nsint ox, oy;

	if( width < NS_RENDER_AABBOX_MIN_CLIENT_SIZE )
		{
		x -= ( NS_RENDER_AABBOX_MIN_CLIENT_SIZE - width ) / 2;
		width = NS_RENDER_AABBOX_MIN_CLIENT_SIZE;
		}

	if( height < NS_RENDER_AABBOX_MIN_CLIENT_SIZE )
		{
		y -= ( NS_RENDER_AABBOX_MIN_CLIENT_SIZE - height ) / 2;
		height = NS_RENDER_AABBOX_MIN_CLIENT_SIZE;
		}

	_ns_render_state_on_pen( state );

	ox = width  / 4;
	oy = height / 4;

	#define __L  (x)
	#define __R  (x+width-1)
	#define __T  (y)
	#define __B  (y+height-1)

	MoveToEx( state->constants.graphics, __L,      __T,     NULL );
	LineTo  ( state->constants.graphics, __L+ox+1, __T           );

	MoveToEx( state->constants.graphics, __L,      __T,     NULL );
	LineTo  ( state->constants.graphics, __L,      __T+oy+1      );

	MoveToEx( state->constants.graphics, __R-ox,   __T,     NULL );
	LineTo  ( state->constants.graphics, __R+1,    __T           );

	MoveToEx( state->constants.graphics, __R,      __T,     NULL );
	LineTo  ( state->constants.graphics, __R,      __T+oy+1      );

	MoveToEx( state->constants.graphics, __L,      __B-oy,  NULL );
	LineTo  ( state->constants.graphics, __L,      __B+1         );

	MoveToEx( state->constants.graphics, __L,      __B,     NULL );
	LineTo  ( state->constants.graphics, __L+ox+1, __B           );

	MoveToEx( state->constants.graphics, __R,      __B-oy,  NULL );
	LineTo  ( state->constants.graphics, __R,      __B+1         );

	MoveToEx( state->constants.graphics, __R-ox,   __B,     NULL );
	LineTo  ( state->constants.graphics, __R+1,    __B           );

	#undef __L
	#undef __R
	#undef __T
	#undef __B
	}


NS_PRIVATE void _ns_render2d_aabbox_corners
	(
	const NsAABBox3d  *B,
	NsVector3f        *C1f,
	NsVector3f        *C2f
	)
	{
	NsVector3d C1d, C2d;

	ns_aabbox3d_min( B, &C1d );
	ns_aabbox3d_max( B, &C2d );

	ns_vector3d_to_3f( &C1d, C1f );
	ns_vector3d_to_3f( &C2d, C2f );
	}


#define _NS_RENDER2D_AABBOX( c1, c2 )\
	A.x = NS_TO_IMAGE_SPACE( V1.c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	A.y = NS_TO_IMAGE_SPACE( V1.c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	B.x = NS_TO_IMAGE_SPACE( V2.c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	B.y = NS_TO_IMAGE_SPACE( V2.c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, B, C );\
	\
	_ns_render2d_aabbox(\
		state,\
		( nsint )A.x,\
		( nsint )A.y,\
		( nsint )( B.x - A.x + 0.5f ),\
		( nsint )( B.y - A.y + 0.5f )\
		)


void ns_render2d_aabbox_xy
	(
	const NsAABBox3d  *box,
	NsRenderState     *state
	)
	{
	NsVector3f  V1, V2;
	NsVector2f  A, B, C;

	_ns_render2d_aabbox_corners( box, &V1, &V2 );
	_NS_RENDER2D_AABBOX( x, y );
	}


void ns_render2d_aabbox_zy
	(
	const NsAABBox3d  *box,
	NsRenderState     *state
	)
	{
	NsVector3f  V1, V2;
	NsVector2f  A, B, C;

	_ns_render2d_aabbox_corners( box, &V1, &V2 );
	_NS_RENDER2D_AABBOX( z, y );
	}


void ns_render2d_aabbox_xz
	(
	const NsAABBox3d  *box,
	NsRenderState     *state
	)
	{
	NsVector3f  V1, V2;
	NsVector2f  A, B, C;

	_ns_render2d_aabbox_corners( box, &V1, &V2 );
	_NS_RENDER2D_AABBOX( x, z );
	}




void ns_render2d_pre_frustums( NsRenderState *state )
	{
	_ns_render_state_init_brush( state );
	_ns_render_state_init_pen( state );
	}


void ns_render2d_post_frustums( NsRenderState *state )
	{
	_ns_render_state_finalize_brush( state );
	_ns_render_state_finalize_pen( state );
	}


NS_PRIVATE void _ns_render2d_frustum
	(
	NsRenderState     *state,
	const NsVector2f  *_V1,
	const NsVector2f  *_R1,
	const NsVector2f  *_V2,
	const NsVector2f  *_R2
	)
	{
	NsVector3f  V1, V2, R1, R2, O, A, B, U, S, S1, S2, Sa, Sb;
	POINT       points[4];


	_ns_render_state_on_brush( state );
	_ns_render_state_on_pen( state );

	ns_vector2f_to_3f( _V1, &V1 );
	ns_vector2f_to_3f( _V2, &V2 );
	ns_vector2f_to_3f( _R1, &R1 );
	ns_vector2f_to_3f( _R2, &R2 );

	ns_vector3f_add( &O, &V1, &R1 );

	ns_vector3f_sub( &A, &V2, &V1 );
	ns_vector3f_sub( &B, &O, &V1 );
	ns_vector3f_cross( &U, &B, &A );
	ns_vector3f_cross( &S, &U, &A );

	S1 = S2 = S;

	ns_vector3f_norm( &S1 );
	ns_vector3f_norm( &S2 );

	ns_vector3f_cmpd_non_uni_scale( &S1, &R1 );
	ns_vector3f_cmpd_non_uni_scale( &S2, &R2 );

	ns_vector3f_add( &Sa, &V1, &S1 );
	ns_vector3f_cmpd_rev( &S1 );
	ns_vector3f_add( &Sb, &V1, &S1 );

	points[0].x = ( LONG )( nslong )Sa.x;
	points[0].y = ( LONG )( nslong )Sa.y;
	points[1].x = ( LONG )( nslong )Sb.x;
	points[1].y = ( LONG )( nslong )Sb.y;

	ns_vector3f_add( &Sa, &V2, &S2 );
	ns_vector3f_cmpd_rev( &S2 );
	ns_vector3f_add( &Sb, &V2, &S2 );

	points[3].x = ( LONG )( nslong )Sa.x;
	points[3].y = ( LONG )( nslong )Sa.y;
	points[2].x = ( LONG )( nslong )Sb.x;
	points[2].y = ( LONG )( nslong )Sb.y;

	if( state->variables.filled )
		Polygon( state->constants.graphics, points, 4 );
	else
		{
		MoveToEx( state->constants.graphics, points[0].x, points[0].y, NULL );
		LineTo( state->constants.graphics, points[1].x, points[1].y );

		MoveToEx( state->constants.graphics, points[1].x, points[1].y, NULL );
		LineTo( state->constants.graphics, points[2].x, points[2].y );

		MoveToEx( state->constants.graphics, points[2].x, points[2].y, NULL );
		LineTo( state->constants.graphics, points[3].x, points[3].y );

		MoveToEx( state->constants.graphics, points[3].x, points[3].y, NULL );
		LineTo( state->constants.graphics, points[0].x, points[0].y );
		}
	}


#define _NS_RENDER2D_FRUSTUM( c1, c2 )\
	NsVector2f V1, V2, R1, R2, C;\
	\
	V1.x = NS_TO_IMAGE_SPACE( P1->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V1.y = NS_TO_IMAGE_SPACE( P1->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	V2.x = NS_TO_IMAGE_SPACE( P2->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V2.y = NS_TO_IMAGE_SPACE( P2->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	R1.x = NS_TO_IMAGE_SPACE( radius1 * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	R1.y = NS_TO_IMAGE_SPACE( radius1 * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	R2.x = NS_TO_IMAGE_SPACE( radius2 * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	R2.y = NS_TO_IMAGE_SPACE( radius2 * state->constants.model_mask_factor, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	R1.x -= .5f;\
	R1.y -= .5f;\
	R2.x -= .5f;\
	R2.y -= .5f;\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V1, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V2, C );\
	\
	ns_vector2f_cmpd_scale( &R1, state->constants.zoom );\
	ns_vector2f_cmpd_scale( &R2, state->constants.zoom );\
	\
	_ns_render2d_frustum( state, &V1, &R1, &V2, &R2 )


void ns_render2d_frustum_xy
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_FRUSTUM( x, y );  }


void ns_render2d_frustum_zy
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_FRUSTUM( z, y );  }


void ns_render2d_frustum_xz
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_FRUSTUM( x, z );  }




void ns_render2d_pre_triangles( NsRenderState *state )
	{
	_ns_render_state_init_brush( state );
	_ns_render_state_init_pen( state );
	}


void ns_render2d_post_triangles( NsRenderState *state )
	{
	_ns_render_state_finalize_brush( state );
	_ns_render_state_finalize_pen( state );
	}


NS_PRIVATE void _ns_render2d_triangle
	(
	NsRenderState  *state,
	nsint           x1,
	nsint           y1,
	nsint           x2,
	nsint           y2,
	nsint           x3,
	nsint           y3
	)
	{
	POINT points[3];

	_ns_render_state_on_brush( state );
	_ns_render_state_on_pen( state );

	points[0].x = x1;
	points[0].y = y1;
	points[1].x = x2;
	points[1].y = y2;
	points[2].x = x3;
	points[2].y = y3;

	Polygon( state->constants.graphics, points, 3 );
	}


#define _NS_RENDER2D_TRIANGLE( c1, c2 )\
	NsVector2f V1, V2, V3, C;\
	\
	V1.x = NS_TO_IMAGE_SPACE( P1->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V1.y = NS_TO_IMAGE_SPACE( P1->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	V2.x = NS_TO_IMAGE_SPACE( P2->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V2.y = NS_TO_IMAGE_SPACE( P2->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	V3.x = NS_TO_IMAGE_SPACE( P3->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V3.y = NS_TO_IMAGE_SPACE( P3->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V1, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V2, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V3, C );\
	\
	_ns_render2d_triangle(\
		state,\
		( nsint )V1.x,\
		( nsint )V1.y,\
		( nsint )V2.x,\
		( nsint )V2.y,\
		( nsint )V3.x,\
		( nsint )V3.y\
		)


void ns_render2d_triangle_xy
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_TRIANGLE( x, y );

	NS_USE_VARIABLE( N1 );
	NS_USE_VARIABLE( N2 );
	NS_USE_VARIABLE( N3 );
	}


void ns_render2d_triangle_zy
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_TRIANGLE( z, y );

	NS_USE_VARIABLE( N1 );
	NS_USE_VARIABLE( N2 );
	NS_USE_VARIABLE( N3 );
	}


void ns_render2d_triangle_xz
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	)
	{
	_NS_RENDER2D_TRIANGLE( x, z );

	NS_USE_VARIABLE( N1 );
	NS_USE_VARIABLE( N2 );
	NS_USE_VARIABLE( N3 );
	}




void ns_render2d_pre_ellipses( NsRenderState *state )
	{
	_ns_render_state_init_brush( state );
	_ns_render_state_init_pen( state );
	}


void ns_render2d_post_ellipses( NsRenderState *state )
	{
	_ns_render_state_finalize_brush( state );
	_ns_render_state_finalize_pen( state );
	}


#define _NS_RENDER2D_ELLIPSE( c1, c2 )\
	NsVector2f V, _R, C;\
	\
	V.x = NS_TO_IMAGE_SPACE( P->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	V.y = NS_TO_IMAGE_SPACE( P->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	_R.x = NS_TO_IMAGE_SPACE( R->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	_R.y = NS_TO_IMAGE_SPACE( R->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	_R.x -= .5f;\
	_R.y -= .5f;\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, V, C );\
	ns_vector2f_cmpd_scale( &_R, state->constants.zoom );\
	\
	_ns_render2d_ellipse( state, ( nsint )V.x, ( nsint )V.y, ( nsint )_R.x, ( nsint )_R.y )


void ns_render2d_ellipse_xy
	(
	const NsVector3f  *P,
	const NsVector3f  *R,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_ELLIPSE( x, y );  }


void ns_render2d_ellipse_zy
	(
	const NsVector3f  *P,
	const NsVector3f  *R,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_ELLIPSE( z, y );  }


void ns_render2d_ellipse_xz
	(
	const NsVector3f  *P,
	const NsVector3f  *R,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_ELLIPSE( x, z );  }




void ns_render2d_pre_rectangles( NsRenderState *state )
	{
	_ns_render_state_init_brush( state );
	_ns_render_state_init_pen( state );
	}


void ns_render2d_post_rectangles( NsRenderState *state )
	{
	_ns_render_state_finalize_brush( state );
	_ns_render_state_finalize_pen( state );
	}


NS_PRIVATE void _ns_render2d_rectangle
	(
	NsRenderState  *state,
	nsint           x,
	nsint           y,
	nsint           width,
	nsint           height
	)
	{
	RECT rc;

	_ns_render_state_on_brush( state );
	_ns_render_state_on_pen( state );

	if( width < 3 )
		width = 3;

	if( height < 3 )
		height = 3;

	rc.left   = x;
	rc.top    = y;
	rc.right  = x + width;
	rc.bottom = y + height;

	if( ! state->constants.polygon_borders )
		{
		--rc.left;
		--rc.top;
		++rc.right;
		++rc.bottom;
		}

	Rectangle( state->constants.graphics, rc.left, rc.top, rc.right, rc.bottom );
	}


#define _NS_RENDER2D_RECTANGLE( c1, c2 )\
	NsVector2f A, B, C;\
	\
	A.x = NS_TO_IMAGE_SPACE( V1->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	A.y = NS_TO_IMAGE_SPACE( V1->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	B.x = NS_TO_IMAGE_SPACE( V2->c1, ns_voxel_info_one_over_size_##c1( state->constants.voxel_info ) );\
	B.y = NS_TO_IMAGE_SPACE( V2->c2, ns_voxel_info_one_over_size_##c2( state->constants.voxel_info ) );\
	\
	C.x = ( nsfloat )state->constants.corner_x;\
	C.y = ( nsfloat )state->constants.corner_y;\
	\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, A, C );\
	_NS_RENDER2D_TO_CLIENT_SPACE( state, B, C );\
	\
	_ns_render2d_rectangle(\
		state,\
		( nsint )A.x,\
		( nsint )A.y,\
		( nsint )( B.x - A.x ),\
		( nsint )( B.y - A.y )\
		)


void ns_render2d_rectangle_xy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_RECTANGLE( x, y );  }


void ns_render2d_rectangle_zy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_RECTANGLE( z, y );  }


void ns_render2d_rectangle_xz
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{  _NS_RENDER2D_RECTANGLE( x, z );  }
