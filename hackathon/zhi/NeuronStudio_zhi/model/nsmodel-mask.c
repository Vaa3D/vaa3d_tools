#include "nsmodel-mask.h"


void ns_model_mask_construct( NsModelMask *mask )
	{
	ns_assert( NULL != mask );

	ns_image_construct( &mask->image_xy );
	ns_image_construct( &mask->image_zy );
	ns_image_construct( &mask->image_xz );
	}


void ns_model_mask_destruct( NsModelMask *mask )
	{
	ns_assert( NULL != mask );

	ns_image_destruct( &mask->image_xy );
	ns_image_destruct( &mask->image_zy );
	ns_image_destruct( &mask->image_xz );
	}


#ifdef NS_OS_WINDOWS
	#include <windows.h>
#else
	#error nsmodel-mask.c compilation error: Must implement on platforms other than Windows!
#endif


typedef struct _NsModelMaskDC
	{
#ifdef NS_OS_WINDOWS
	HDC      primary;
	HDC      memory;
	HBITMAP  save_bitmap;
	HBITMAP  curr_bitmap;
#endif

	NsImage  image;
	}
	NsModelMaskDC;


NS_PRIVATE void _ns_model_mask_dc_init( NsModelMaskDC *dc )
	{
#ifdef NS_OS_WINDOWS
	dc->primary     = NULL;
	dc->memory      = NULL;
	dc->save_bitmap = NULL;
	dc->curr_bitmap = NULL;
#endif
	}


NS_PRIVATE void _ns_model_mask_dc_construct( NsModelMaskDC *dc, NsProcDb *db )
	{
	_ns_model_mask_dc_init( dc );

	ns_image_construct( &dc->image );
	ns_image_set_pixel_proc_db( &dc->image, db );
	}


NS_PRIVATE void _ns_model_mask_dc_destruct( NsModelMaskDC *dc )
	{
#ifdef NS_OS_WINDOWS
	if( NULL != dc->memory )
		{
		SelectObject( dc->memory, dc->save_bitmap );

		if( NULL != dc->curr_bitmap )
			DeleteObject( dc->curr_bitmap );

		DeleteDC( dc->memory );
		}

	if( NULL != dc->primary )
		DeleteDC( dc->primary );
#endif

	_ns_model_mask_dc_init( dc );
	ns_image_destruct( &dc->image );
	}


NS_PRIVATE NsError _ns_model_mask_dc_create( NsModelMaskDC *dc, nssize width, nssize height )
	{
	NsError error;

#ifdef NS_OS_WINDOWS

	if( NS_FAILURE( ns_image_create( &dc->image, NS_PIXEL_LUM_U8, width, height, 1, sizeof( DWORD ) ), error ) )
		return error;

	if( NULL == ( dc->primary = CreateDC( "DISPLAY", NULL, NULL, NULL ) ) )
		return ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL == ( dc->memory = CreateCompatibleDC( dc->primary ) ) )
		return ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL == ( dc->curr_bitmap = CreateCompatibleBitmap( dc->memory, ( nsint )width, ( nsint )height ) ) )
		return ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	dc->save_bitmap = SelectObject( dc->memory, dc->curr_bitmap );
#endif

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_model_mask_dc_blit( NsModelMaskDC *dc )
	{
#ifdef NS_OS_WINDOWS
	BITMAPINFO *info;

	if( NULL == ( info = ns_malloc( sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * 256 ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	info->bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
	info->bmiHeader.biWidth       = ( nsint )ns_image_width( &dc->image );
	info->bmiHeader.biHeight      = -( ( nsint )ns_image_height( &dc->image ) );
	info->bmiHeader.biPlanes      = 1;
	info->bmiHeader.biBitCount    = 8;
	info->bmiHeader.biCompression = BI_RGB;

	GetDIBits(
		dc->memory,
		dc->curr_bitmap,
		0,
		( UINT )ns_image_height( &dc->image ),
		ns_image_pixels( &dc->image ),
		info,
		DIB_RGB_COLORS
		);

	ns_free( info );
	return ns_no_error();
#endif
	}


typedef struct _NsModelMaskRenderer
	{
	NsModelMaskDC  dc_xy;
	NsModelMaskDC  dc_zy;
	NsModelMaskDC  dc_xz;
	NsProcDb       db;
	}
	NsModelMaskRenderer;


NS_PRIVATE NsError _ns_model_mask_renderer_construct( NsModelMaskRenderer *mmr )
	{
	NsError error;

	if( NS_FAILURE( ns_proc_db_construct( &mmr->db ), error ) )
		return error;

	if( NS_FAILURE( ns_pixel_proc_db_register_std( &mmr->db ), error ) )
		{
		ns_proc_db_destruct( &mmr->db );
		return error;
		}

	_ns_model_mask_dc_construct( &mmr->dc_xy, &mmr->db );
	_ns_model_mask_dc_construct( &mmr->dc_zy, &mmr->db );
	_ns_model_mask_dc_construct( &mmr->dc_xz, &mmr->db );

	return ns_no_error();
	}


NS_PRIVATE void _ns_model_mask_renderer_destruct( NsModelMaskRenderer *mmr )
	{
	_ns_model_mask_dc_destruct( &mmr->dc_xy );
	_ns_model_mask_dc_destruct( &mmr->dc_zy );
	_ns_model_mask_dc_destruct( &mmr->dc_xz );

	ns_proc_db_destruct( &mmr->db );
	}


NS_PRIVATE NsError _ns_model_mask_renderer_create( NsModelMaskRenderer *mmr, const NsImage *volume )
	{
	NsError error;

	if( NS_FAILURE(
			_ns_model_mask_dc_create(
				&mmr->dc_xy,
				ns_image_width( volume ),
				ns_image_height( volume )
				),
			error ) )
		return error;

	if( NS_FAILURE(
			_ns_model_mask_dc_create(
				&mmr->dc_zy,
				ns_image_length( volume ),
				ns_image_height( volume )
				),
			error ) )
		return error;

	if( NS_FAILURE(
			_ns_model_mask_dc_create(
				&mmr->dc_xz,
				ns_image_width( volume ),
				ns_image_length( volume )
				),
			error ) )
		return error;

	return ns_no_error();
	}


#ifdef NS_OS_WINDOWS
	#include <render/nsrender2d-os-windows.h>
#endif

NS_PRIVATE void _ns_model_mask_init_render_state
	(
	NsRenderState     *state,
	nsint              display,
	nspointer          graphics,
	const NsModel     *model,
	const NsImage     *volume,
	const NsSettings  *settings
	)
	{
	ns_render_state_init( state );

	/* Increase radii of frustum and vertices when rendering.
		This prevents some roundoff error when rasterizing? */
	state->constants.model_mask_factor = 1.1f;

	state->constants.iface.pre_spheres_func  = ns_render2d_pre_spheres;
	state->constants.iface.pre_frustums_func = ns_render2d_pre_frustums;
	state->constants.iface.pre_ellipses_func = ns_render2d_pre_ellipses;
	state->constants.iface.pre_aabboxes_func = ns_render2d_pre_aabboxes;

	state->constants.iface.post_spheres_func  = ns_render2d_post_spheres;
	state->constants.iface.post_frustums_func = ns_render2d_post_frustums;
	state->constants.iface.post_ellipses_func = ns_render2d_post_ellipses;
	state->constants.iface.post_aabboxes_func = ns_render2d_post_aabboxes;

	state->constants.VERTEX_SHAPE_SOLID_ELLIPSES = 1;
	state->constants.EDGE_SHAPE_SOLID_FRUSTUMS   = 1;
	state->constants.SPINE_NO_SHAPE              = 1;
	state->constants.VERTEX_SINGLE_COLOR         = 1;
	state->constants.EDGE_SINGLE_COLOR           = 1;
		
   switch( display )
      {
      case NS_XY:
         state->constants.iface.sphere_func  = ns_render2d_sphere_xy;
         state->constants.iface.frustum_func = ns_render2d_frustum_xy;
         state->constants.iface.ellipse_func = ns_render2d_ellipse_xy;
         state->constants.iface.aabbox_func  = ns_render2d_aabbox_xy;
         break;

      case NS_ZY:
         state->constants.iface.sphere_func  = ns_render2d_sphere_zy;
         state->constants.iface.frustum_func = ns_render2d_frustum_zy;
         state->constants.iface.ellipse_func = ns_render2d_ellipse_zy;
         state->constants.iface.aabbox_func  = ns_render2d_aabbox_zy;
         break;

      case NS_XZ:
         state->constants.iface.sphere_func  = ns_render2d_sphere_xz;
         state->constants.iface.frustum_func = ns_render2d_frustum_xz;
         state->constants.iface.ellipse_func = ns_render2d_ellipse_xz;
         state->constants.iface.aabbox_func  = ns_render2d_aabbox_xz;
         break;
      }

	state->constants.dataset    = volume;
   state->constants.model      = ( NsModel* )model;
   state->constants.voxel_info = ns_settings_voxel_info( settings );
   state->constants.settings   = settings;

   state->constants.flicker_free_rendering = NS_TRUE;
	state->constants.is_model_mask          = NS_TRUE;

   state->constants.vertex_shape_mode = state->constants.VERTEX_SHAPE_SOLID_ELLIPSES;
   state->constants.edge_shape_mode   = state->constants.EDGE_SHAPE_SOLID_FRUSTUMS;
   state->constants.spine_shape_mode  = state->constants.SPINE_NO_SHAPE;

   state->constants.vertex_color_scheme = state->constants.VERTEX_SINGLE_COLOR;
   state->constants.vertex_single_color = NS_COLOR4UB_WHITE;

   state->constants.edge_color_scheme = state->constants.EDGE_SINGLE_COLOR;
   state->constants.edge_single_color = NS_COLOR4UB_WHITE;

   state->constants.line_size       = ( nsint )1;
   state->constants.display         = display;
   state->constants.zoom            = 1.0f;
   state->constants.graphics        = graphics;
	state->constants.polygon_borders = NS_TRUE;
	}


NS_PRIVATE NsError _ns_model_mask_renderer_blit( NsModelMaskRenderer *mmr )
	{
	NsError error;

	if( NS_FAILURE( _ns_model_mask_dc_blit( &mmr->dc_xy ), error ) )
		return error;

	if( NS_FAILURE( _ns_model_mask_dc_blit( &mmr->dc_zy ), error ) )
		return error;

	if( NS_FAILURE( _ns_model_mask_dc_blit( &mmr->dc_xz ), error ) )
		return error;

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_model_mask_renderer_run
	(
	NsModelMaskRenderer  *mmr,
	const NsModel        *model,
	const NsImage        *volume,
	const NsSettings     *settings
	)
	{
	NsRenderState state;

	_ns_model_mask_init_render_state( &state, NS_XY, mmr->dc_xy.memory, model, volume, settings );
	ns_render( &state );

	_ns_model_mask_init_render_state( &state, NS_ZY, mmr->dc_zy.memory, model, volume, settings );
	ns_render( &state );

	_ns_model_mask_init_render_state( &state, NS_XZ, mmr->dc_xz.memory, model, volume, settings );
	ns_render( &state );

	return _ns_model_mask_renderer_blit( mmr );
	}


NS_PRIVATE NsError _ns_model_mask_dc_copy( NsModelMaskDC *dc, NsImage *image )
	{
	NsError error;

	if( NS_FAILURE(
			ns_image_convert(
				&dc->image,
				ns_image_pixel_type( &dc->image ),
				1,
				image,
				NULL
				),
			error ) )
		return error;

	ns_image_clear( &dc->image );

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_model_mask_renderer_copy( NsModelMaskRenderer *mmr, NsModelMask *mask )
	{
	NsError error;

	if( NS_FAILURE( _ns_model_mask_dc_copy( &mmr->dc_xy, &mask->image_xy ), error ) )
		return error;

	if( NS_FAILURE( _ns_model_mask_dc_copy( &mmr->dc_zy, &mask->image_zy ), error ) )
		return error;

	if( NS_FAILURE( _ns_model_mask_dc_copy( &mmr->dc_xz, &mask->image_xz ), error ) )
		return error;

	ns_voxel_buffer_init( &mask->buffer_xy, &mask->image_xy );
	ns_voxel_buffer_init( &mask->buffer_zy, &mask->image_zy );
	ns_voxel_buffer_init( &mask->buffer_xz, &mask->image_xz );

	return ns_no_error();
	}


NsError ns_model_mask_create
	(
	NsModelMask       *mask,
	const NsModel     *model,
	const NsImage     *volume,
	const NsSettings  *settings
	)
	{
	NsModelMaskRenderer  mmr;
	NsError              error;


	ns_assert( NULL != mask );
	ns_assert( NULL != model );
	ns_assert( NULL != volume );

	error = ns_no_error();

	if( NS_FAILURE( _ns_model_mask_renderer_construct( &mmr ), error ) )
		return error;

	if( NS_FAILURE( _ns_model_mask_renderer_create( &mmr, volume ), error ) )
		goto _NS_MODEL_MASK_CREATE_EXIT;

	if( NS_FAILURE( _ns_model_mask_renderer_run( &mmr, model, volume, settings ), error ) )
		goto _NS_MODEL_MASK_CREATE_EXIT;

	if( NS_FAILURE( _ns_model_mask_renderer_copy( &mmr, mask ), error ) )
		goto _NS_MODEL_MASK_CREATE_EXIT;

	_NS_MODEL_MASK_CREATE_EXIT:

	_ns_model_mask_renderer_destruct( &mmr );
	return error;
	}


nsboolean ns_model_mask_at( const NsModelMask *mask, const NsVector3i *V )
	{
	return ( 0 == ns_voxel_get( &mask->buffer_xy, V->x, V->y, 0 ) ||
				0 == ns_voxel_get( &mask->buffer_zy, V->z, V->y, 0 ) ||
				0 == ns_voxel_get( &mask->buffer_xz, V->x, V->z, 0 )   );
	}
