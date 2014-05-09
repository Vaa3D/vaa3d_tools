#include "nspixels-flip.h"


NS_PRIVATE NsProc _ns_pixel_proc_flip;


NS_PRIVATE NsClosureValue _ns_pixel_proc_flip_params[ NS_PIXEL_PROC_FLIP_NUM_PARAMS ] =
	{
	{ "pixel_type", NS_VALUE_PIXEL_TYPE },
	{ "pixels",     NS_VALUE_POINTER },
	{ "width",      NS_VALUE_SIZE },
	{ "height",     NS_VALUE_SIZE },
	{ "length",     NS_VALUE_SIZE },
	{ "row_align",  NS_VALUE_SIZE },
	{ "type",       NS_VALUE_INT },
	{ "progress",   NS_VALUE_PROGRESS }
	};


typedef NsError ( *_NsPixelProcFlipFunc )
	(
	NsPixelType,
	nspointer,
	nssize,
	nssize,
	nssize,
	nssize,
	NsProgress*
	);


NS_PRIVATE NsError _ns_pixel_proc_flip_vertical_all
	(
	NsPixelType   pixel_type,
	nspointer     pixels,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	NsProgress   *progress
	)
	{
	nssize     z;
	nssize     bytes_per_row;
	nssize     bytes_per_slice;
	nspointer  row1;
	nspointer  row2;
	nspointer  temp;


	bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	if( NULL == ( temp = ns_malloc( bytes_per_row ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				break;

			ns_progress_update(
				progress,
				100.0f *
				( ( nsfloat )z ) / ( ( nsfloat )length )
				);
			}

		row1 = pixels;
		row2 = NS_OFFSET_POINTER( void, pixels, ( height - 1 ) * bytes_per_row );

		while( row1 < row2 )
			{
			ns_memcpy( temp, row1, bytes_per_row ); /* temp = row1 */
			ns_memcpy( row1, row2, bytes_per_row ); /* row1 = row2 */
			ns_memcpy( row2, temp, bytes_per_row ); /* row2 = temp */

			row1 = NS_INCREMENT_POINTER( void, row1, bytes_per_row );
			row2 = NS_DECREMENT_POINTER( void, row2, bytes_per_row );
			}

		pixels = NS_OFFSET_POINTER( void, pixels, bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );

	ns_free( temp );
	return ns_no_error();
	}


NS_PRIVATE NsError _ns_pixel_proc_flip_horizontal_all
	(
	NsPixelType   pixel_type,
	nspointer     pixels,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	NsProgress   *progress
	)
	{
	nssize             xs, xe;
	nssize             y;
	nssize             z;
	nssize             bytes_per_row;
	nssize             bytes_per_slice;
	nspointer          slice;
	nspointer          row;
	NsPixel            ps, pe;
	NsPixelRowGetFunc  row_get_func;
	NsPixelRowSetFunc  row_set_func;


	bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );
	slice           = pixels;

	ns_pixel_init( &ps, pixel_type );
	ns_pixel_init( &pe, pixel_type );

	row_get_func = ns_pixel_row_get_func( pixel_type );
	row_set_func = ns_pixel_row_set_func( pixel_type );

	ns_assert( NULL != row_get_func );
	ns_assert( NULL != row_set_func );

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				break;

			ns_progress_update(
				progress,
				100.0f *
				( ( nsfloat )z ) / ( ( nsfloat )length )
				);
			}

		row = slice;

		for( y = 0; y < height; ++y )
			{
			xs = 0;
			xe = width - 1;

			while( xs < xe )
				{
				( row_get_func )( &ps, row, xs );
				( row_get_func )( &pe, row, xe );

				( row_set_func )( &ps, row, xe );
				( row_set_func )( &pe, row, xs );

				++xs;
				--xe;
				}

			row = NS_OFFSET_POINTER( void, row, bytes_per_row );
			}

		slice = NS_OFFSET_POINTER( void, slice, bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_pixel_proc_flip_optical_axis_all
	(
	NsPixelType   pixel_type,
	nspointer     pixels,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	NsProgress   *progress
	)
	{
	nspointer  start_slice, end_slice, swap_slice;
	nssize     bytes_per_slice;
	nsfloat    iter, num_iters;


	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	if( NULL == ( swap_slice = ns_malloc( bytes_per_slice ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	start_slice = pixels;
	end_slice   = NS_INCREMENT_POINTER( void, pixels, bytes_per_slice * ( length - 1 ) );

	iter      = 0.0f;
	num_iters = ( nsfloat )length / 2.0f;

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	while( start_slice < end_slice )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				{
				ns_free( swap_slice );
				return ns_no_error();
				}

			ns_progress_update( progress, 100.0f * ( iter / num_iters ) );
			iter += 1.0f;
			}

		ns_memcpy( swap_slice, start_slice, bytes_per_slice ); /* swap  = start */
		ns_memcpy( start_slice, end_slice, bytes_per_slice );  /* start = end   */
		ns_memcpy( end_slice, swap_slice, bytes_per_slice );   /* end   = swap  */

		start_slice = NS_INCREMENT_POINTER( void, start_slice, bytes_per_slice );
		end_slice   = NS_DECREMENT_POINTER( void, end_slice, bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );

	ns_free( swap_slice );
	return ns_no_error();
	}


NS_PRIVATE _NsPixelProcFlipFunc _ns_pixel_proc_flip_func
	(
	NsPixelType  pixel_type,
	nsint        flip_type
	)
	{
	_NsPixelProcFlipFunc func = NULL;

	switch( flip_type )
		{
		case NS_PIXEL_PROC_FLIP_VERTICAL:
			switch( pixel_type )
				{
				case NS_PIXEL_LUM_U1:
				case NS_PIXEL_LUM_U8:
				case NS_PIXEL_LUM_U12:
				case NS_PIXEL_LUM_U16:
				case NS_PIXEL_LUM_ALPHA_U8_U8:
				case NS_PIXEL_RGB_U8_U8_U8:
				case NS_PIXEL_RGBA_U8_U8_U8_U8:
				case NS_PIXEL_BGR_U8_U8_U8:
				case NS_PIXEL_BGRA_U8_U8_U8_U8:
					func = _ns_pixel_proc_flip_vertical_all;
					break;
				}
			break;

		case NS_PIXEL_PROC_FLIP_HORIZONTAL:
			switch( pixel_type )
				{
				case NS_PIXEL_LUM_U1:
				case NS_PIXEL_LUM_U8:
				case NS_PIXEL_LUM_U12:
				case NS_PIXEL_LUM_U16:
				case NS_PIXEL_LUM_ALPHA_U8_U8:
				case NS_PIXEL_RGB_U8_U8_U8:
				case NS_PIXEL_RGBA_U8_U8_U8_U8:
				case NS_PIXEL_BGR_U8_U8_U8:
				case NS_PIXEL_BGRA_U8_U8_U8_U8:
					func = _ns_pixel_proc_flip_horizontal_all;
					break;
				}
			break;

		case NS_PIXEL_PROC_FLIP_OPTICAL_AXIS:
			switch( pixel_type )
				{
				case NS_PIXEL_LUM_U1:
				case NS_PIXEL_LUM_U8:
				case NS_PIXEL_LUM_U12:
				case NS_PIXEL_LUM_U16:
				case NS_PIXEL_LUM_ALPHA_U8_U8:
				case NS_PIXEL_RGB_U8_U8_U8:
				case NS_PIXEL_RGBA_U8_U8_U8_U8:
				case NS_PIXEL_BGR_U8_U8_U8:
				case NS_PIXEL_BGRA_U8_U8_U8_U8:
					func = _ns_pixel_proc_flip_optical_axis_all;
					break;
				}
			break;

		default:
			ns_assert_not_reached();
		}

	if( NULL == func )
		ns_warning(
			NS_WARNING_LEVEL_RECOVERABLE
			NS_MODULE
			" Pixel format "
			NS_FMT_STRING_QUOTED
			" not supported for procedure "
			NS_FMT_STRING_QUOTED
			".",
			ns_pixel_type_to_string( pixel_type ),
			_ns_pixel_proc_flip.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_flip_call
	(
	NsPixelType   pixel_type,
	nspointer     pixels,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nsint         flip_type,
	NsProgress   *progress
	)
	{
	_NsPixelProcFlipFunc func;

	if( NULL == ( func = _ns_pixel_proc_flip_func( pixel_type, flip_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", pixels=" NS_FMT_POINTER
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", row_align=" NS_FMT_ULONG
		", type=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		" )",
		ns_pixel_type_to_string( pixel_type ),
		pixels,
		width,
		height,
		length,
		row_align,
		ns_pixel_proc_flip_type_to_string( flip_type ),
		progress
		);

	return ( func )(
				pixel_type,
				pixels,
				width,
				height,
				length,
				row_align,
				progress
				);
	}


NS_PRIVATE NsError _ns_pixel_proc_flip_marshal
	(
	NsClosure  *closure,
	NsValue    *params,
	nssize      num_params,
	NsValue    *ret_value
	)
	{
	NS_USE_VARIABLE( closure );
	NS_USE_VARIABLE( num_params );
	NS_USE_VARIABLE( ret_value );

	return _ns_pixel_proc_flip_call(
				ns_value_get_pixel_type( params + NS_PIXEL_PROC_FLIP_PIXEL_TYPE ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_FLIP_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_FLIP_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_FLIP_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_FLIP_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_FLIP_ROW_ALIGN ),
				ns_value_get_int( params + NS_PIXEL_PROC_FLIP_TYPE ),
				ns_value_get_progress( params + NS_PIXEL_PROC_FLIP_PROGRESS )
				);
	}


const nschar* ns_pixel_proc_flip_type_to_string( NsPixelProcFlipType type )
	{
	NS_PRIVATE const nschar* _ns_pixel_proc_flip_type_strings[ NS_PIXEL_PROC_FLIP_NUM_TYPES ] =
		{
		"vertical",
		"horizontal",
		"optical-axis"
		};

	ns_assert( type < NS_PIXEL_PROC_FLIP_NUM_TYPES );
	return _ns_pixel_proc_flip_type_strings[ type ];
	}


NsProc* ns_pixel_proc_flip( void )
	{
	_ns_pixel_proc_flip.name        = NS_PIXEL_PROC_FLIP;
	_ns_pixel_proc_flip.title       = "Vertical/Horizontal/Optical-Axis Flip";
	_ns_pixel_proc_flip.version     = "1.0";
	_ns_pixel_proc_flip.help        = "Flips the pixels of an image vertically, horizontally, or along the optical axis.";
	_ns_pixel_proc_flip.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_flip.params      = _ns_pixel_proc_flip_params;
	_ns_pixel_proc_flip.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_flip_params );
	_ns_pixel_proc_flip.marshal     = _ns_pixel_proc_flip_marshal;

	_ns_pixel_proc_flip.ret_value.name = NULL;
	_ns_pixel_proc_flip.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_flip;
	}
