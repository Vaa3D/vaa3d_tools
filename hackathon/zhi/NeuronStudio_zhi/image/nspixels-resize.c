#include "nspixels-resize.h"


NS_PRIVATE NsProc _ns_pixel_proc_resize;


NS_PRIVATE NsClosureValue _ns_pixel_proc_resize_params[ NS_PIXEL_PROC_RESIZE_NUM_PARAMS ] =
	{
	{ "pixel_type",     NS_VALUE_PIXEL_TYPE },
	{ "src_pixels",     NS_VALUE_CONSTPOINTER },
	{ "src_width",      NS_VALUE_SIZE },
	{ "src_height",     NS_VALUE_SIZE },
	{ "src_length",     NS_VALUE_SIZE },
	{ "src_row_align",  NS_VALUE_SIZE },
	{ "interp_type",    NS_VALUE_ENUM },
	{ "dest_pixels",    NS_VALUE_POINTER },
	{ "dest_width",     NS_VALUE_SIZE },
	{ "dest_height",    NS_VALUE_SIZE },
	{ "dest_length",    NS_VALUE_SIZE },
	{ "dest_row_align", NS_VALUE_SIZE },
	{ "progress",       NS_VALUE_PROGRESS }
	};


typedef void ( *_NsPixelProcRnssizeFunc )
	(
	NsPixelType,
	nsconstpointer,
	nssize,
	nssize,
	nssize,
	nssize,
	nspointer,
	nssize,
	nssize,
	nssize,
	nssize,
	NsProgress*
	);


NS_PRIVATE void _ns_pixel_proc_resize_nearest_neighbor_all
	(
	NsPixelType      pixel_type,
	nsconstpointer   src_pixels,
	nssize           src_width,
	nssize           src_height,
	nssize           src_length,
	nssize           src_row_align,
	nspointer        dest_pixels,
	nssize           dest_width,
	nssize           dest_height,
	nssize           dest_length,
	nssize           dest_row_align,
	NsProgress      *progress
	)
	{
	nssize               src_bytes_per_row;
	nssize               src_bytes_per_slice;
	nssize               dest_bytes_per_row;
	nssize               dest_bytes_per_slice;
	nsconstpointer       src_slice;
	nsconstpointer       src_row;
	nspointer            dest_slice;
	nspointer            dest_row;
	nssize               dest_x, dest_y, dest_z;
	nsfloat32            src_x, src_y, src_z;
	nsfloat32            x_step, y_step, z_step;
	nsint32              i32;
	nsfloat64            f64;
	nssize               i;
	NsPixel              pixel;
	NsPixelRowGetFunc    row_get_func;
	NsPixelRowSetFunc    row_set_func;


	row_get_func = ns_pixel_row_get_func( pixel_type );
	row_set_func = ns_pixel_row_set_func( pixel_type );

	ns_assert( NULL != row_get_func );
	ns_assert( NULL != row_set_func );

	ns_pixel_init( &pixel, pixel_type );

	src_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, src_width, src_row_align );
	src_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, src_width, src_height, src_row_align );

	dest_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, dest_width, dest_row_align );
	dest_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, dest_width, dest_height, dest_row_align );

	dest_slice = dest_pixels;

	x_step = ( nsfloat32 )src_width  / ( nsfloat32 )dest_width;
	y_step = ( nsfloat32 )src_height / ( nsfloat32 )dest_height;
	z_step = ( nsfloat32 )src_length / ( nsfloat32 )dest_length;

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( dest_z = 0, src_z = 0.0f;
		  dest_z < dest_length;
		  ++dest_z, src_z += z_step )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				return;

			ns_progress_update(
				progress,
				100.0f *
				( ( nsfloat )dest_z / ( nsfloat )dest_length )
				);
			}

		NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( src_z, i32, f64 );
		i = ( nssize )i32;
		ns_assert( i < src_length );
		src_slice = NS_OFFSET_POINTER( const void, src_pixels, i * src_bytes_per_slice );

		dest_row = dest_slice;

		for( dest_y = 0, src_y = 0.0f;
			  dest_y < dest_height;
			  ++dest_y, src_y += y_step )
			{
			NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( src_y, i32, f64 );
			i = ( nssize )i32;
			ns_assert( i < src_height );
			src_row = NS_OFFSET_POINTER( const void, src_slice, i * src_bytes_per_row );

			for( dest_x = 0, src_x = 0.0f;
				  dest_x < dest_width;
				  ++dest_x, src_x += x_step )
				{
				NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( src_x, i32, f64 );
				i = ( nssize )i32;
				ns_assert( i < src_width );

				( row_get_func )( &pixel, src_row, i );
				( row_set_func )( &pixel, dest_row, dest_x );
				}

			dest_row = NS_OFFSET_POINTER( void, dest_row, dest_bytes_per_row );
			}

		dest_slice = NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );
	}


NS_PRIVATE _NsPixelProcRnssizeFunc _ns_pixel_proc_resize_func
	(
	nsenum       interp_type,
	NsPixelType  pixel_type
	)
	{
	_NsPixelProcRnssizeFunc func = NULL;

	switch( interp_type )
		{
		case NS_PIXEL_PROC_RESIZE_NEAREST_NEIGHBOR:
			func = _ns_pixel_proc_resize_nearest_neighbor_all;
			break;
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
			_ns_pixel_proc_resize.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_resize_call
	(
	NsPixelType      pixel_type,
	nsconstpointer   src_pixels,
	nssize           src_width,
	nssize           src_height,
	nssize           src_length,
	nssize           src_row_align,
	nsenum           interp_type,
	nspointer        dest_pixels,
	nssize           dest_width,
	nssize           dest_height,
	nssize           dest_length,
	nssize           dest_row_align,
	NsProgress      *progress
	)
	{
	_NsPixelProcRnssizeFunc func;

	if( NULL == ( func = _ns_pixel_proc_resize_func( interp_type, pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != src_pixels );
	ns_assert( 0 < src_width );
	ns_assert( 0 < src_height );
	ns_assert( 0 < src_length );
	ns_assert( NULL != dest_pixels );
	ns_assert( 0 < dest_width );
	ns_assert( 0 < dest_height );
	ns_assert( 0 < dest_length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", src_pixels=" NS_FMT_POINTER
		", src_width=" NS_FMT_ULONG
		", src_height=" NS_FMT_ULONG
		", src_length=" NS_FMT_ULONG
		", src_row_align=" NS_FMT_ULONG
		", dest_pixels=" NS_FMT_POINTER
		", dest_width=" NS_FMT_ULONG
		", dest_height=" NS_FMT_ULONG
		", dest_length=" NS_FMT_ULONG
		", dest_row_align=" NS_FMT_ULONG
		", progress=" NS_FMT_POINTER
		" )",
		ns_pixel_type_to_string( pixel_type ),
		src_pixels,
		src_width,
		src_height,
		src_length,
		src_row_align,
		dest_pixels,
		dest_width,
		dest_height,
		dest_length,
		dest_row_align,
		progress
		);

	( func )(
		pixel_type,
		src_pixels,
		src_width,
		src_height,
		src_length,
		src_row_align,
		dest_pixels,
		dest_width,
		dest_height,
		dest_length,
		dest_row_align,
		progress
		);

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_pixel_proc_resize_marshal
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

	return _ns_pixel_proc_resize_call(
				ns_value_get_pixel_type( params + NS_PIXEL_PROC_RESIZE_PIXEL_TYPE ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_RESIZE_SRC_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_SRC_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_SRC_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_SRC_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_SRC_ROW_ALIGN ),
				ns_value_get_enum( params + NS_PIXEL_PROC_RESIZE_INTERP_TYPE ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_RESIZE_DEST_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_DEST_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_DEST_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_DEST_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_RESIZE_DEST_ROW_ALIGN ),
				ns_value_get_progress( params + NS_PIXEL_PROC_RESIZE_PROGRESS )
				);
	}


const nschar* ns_pixel_proc_resize_type_to_string( NsPixelProcResizeType type )
	{
	NS_PRIVATE const nschar* _ns_pixel_proc_resize_type_strings[ NS_PIXEL_PROC_RESIZE_NUM_TYPES ] =
		{
		"Nearest Neighbor"
		};

	ns_assert( type < NS_PIXEL_PROC_RESIZE_NUM_TYPES );
	return _ns_pixel_proc_resize_type_strings[ type ];
	}


NsProc* ns_pixel_proc_resize( void )
	{
	_ns_pixel_proc_resize.name        = NS_PIXEL_PROC_RESIZE;
	_ns_pixel_proc_resize.title       = "Resize";
	_ns_pixel_proc_resize.version     = "1.0";
	_ns_pixel_proc_resize.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_resize.params      = _ns_pixel_proc_resize_params;
	_ns_pixel_proc_resize.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_resize_params );
	_ns_pixel_proc_resize.marshal     = _ns_pixel_proc_resize_marshal;

	_ns_pixel_proc_resize.ret_value.name = NULL;
	_ns_pixel_proc_resize.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_resize;
	}
