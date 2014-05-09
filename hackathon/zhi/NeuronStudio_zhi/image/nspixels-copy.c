#include "nspixels-copy.h"


NS_PRIVATE NsProc _ns_pixel_proc_copy;


NS_PRIVATE NsClosureValue _ns_pixel_proc_copy_params[ NS_PIXEL_PROC_COPY_NUM_PARAMS ] =
	{
	{ "pixel_type",  NS_VALUE_PIXEL_TYPE },
	{ "src_pixels",  NS_VALUE_CONSTPOINTER },
	{ "width",       NS_VALUE_SIZE },
	{ "height",      NS_VALUE_SIZE },
	{ "length",      NS_VALUE_SIZE },
	{ "row_align",   NS_VALUE_SIZE },
	{ "dest_pixels", NS_VALUE_POINTER },
	{ "region",      NS_VALUE_PIXEL_REGION },
	{ "progress",    NS_VALUE_PROGRESS }
	};


typedef void ( *_NsPixelProcCopyFunc )
	(
	NsPixelType,
	nsconstpointer,
	nssize,
	nssize,
	nssize,
	nssize,
	nspointer,
	const NsPixelRegion*,
	NsProgress*
	);


/* Fast copy. Do an entire slice at a time. */
NS_PRIVATE void _ns_pixel_proc_copy_func_fast
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	nssize z, bytes_per_slice;

	ns_verify( NULL == region );

	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				return;

			ns_progress_update(
				progress,
				100.0f * ( ( nsfloat )z / ( nsfloat )length )
				);
			}

		ns_memcpy( dest_pixels, src_pixels, bytes_per_slice );

		src_pixels  = NS_OFFSET_POINTER( void, src_pixels,  bytes_per_slice );
		dest_pixels = NS_OFFSET_POINTER( void, dest_pixels, bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );
	}


/* Medium speed assign. Copy one row at a time. Only works for pixels
	that dont cross byte boundaries! */
NS_PRIVATE void _ns_pixel_proc_copy_func_medium
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	nssize              y;
	nssize              z;
	nssize              src_bytes_per_row;
	nssize              src_bytes_per_slice;
	nsconstpointer      src_slice;
	nsconstpointer      src_row;
	nssize              dest_bytes_per_row;
	nssize              dest_bytes_per_slice;
	nspointer           dest_slice;
	nspointer           dest_row;
	nssize              bytes_per_pixel;


	ns_assert( NULL != region );
	ns_pixel_region_verify( region, width, height, length );

	NS_USE_VARIABLE( length );

	ns_assert( ns_pixel_byte_bounded( pixel_type ) );
	bytes_per_pixel = ns_pixel_bits( pixel_type ) / 8;

	src_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	src_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );
	src_slice           = src_pixels;

	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->z * src_bytes_per_slice );
	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->y * src_bytes_per_row );

	dest_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, region->width, row_align );
	dest_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, region->width, region->height, row_align );
	dest_slice           = dest_pixels;

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < region->length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				return;

			ns_progress_update(
				progress,
				100.0f * ( ( nsfloat )z / ( nsfloat )length )
				);
			}

		src_row  = src_slice;
		dest_row = dest_slice;

		for( y = 0; y < region->height; ++y )
			{
			ns_memcpy(
				dest_row,
				NS_OFFSET_POINTER( const void, src_row, region->x * bytes_per_pixel ),
				dest_bytes_per_row
				);

			src_row  = NS_OFFSET_POINTER( const void, src_row, src_bytes_per_row );
			dest_row = NS_OFFSET_POINTER( void, dest_row, dest_bytes_per_row );
			}

		src_slice  = NS_OFFSET_POINTER( const void, src_slice, src_bytes_per_slice );
		dest_slice = NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );
	}


/* Slow copy. Do one pixel at a time. */
NS_PRIVATE void _ns_pixel_proc_copy_func_slow
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
   nssize              x;
	nssize              y;
	nssize              z;
	nssize              src_bytes_per_row;
	nssize              src_bytes_per_slice;
	nsconstpointer      src_slice;
	nsconstpointer      src_row;
	nssize              dest_bytes_per_row;
	nssize              dest_bytes_per_slice;
	nspointer           dest_slice;
	nspointer           dest_row;
	NsPixel             pixel;
	NsPixelRowGetFunc   row_get_func;
	NsPixelRowSetFunc   row_set_func;


	ns_assert( NULL != region );
	ns_pixel_region_verify( region, width, height, length );

	NS_USE_VARIABLE( length );

	src_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	src_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );
	src_slice           = src_pixels;

	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->z * src_bytes_per_slice );
	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->y * src_bytes_per_row );

	dest_bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, region->width, row_align );
	dest_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, region->width, region->height, row_align );
	dest_slice           = dest_pixels;

	ns_pixel_init( &pixel, pixel_type );

	row_get_func = ns_pixel_row_get_func( pixel_type );
	row_set_func = ns_pixel_row_set_func( pixel_type );

	ns_assert( NULL != row_get_func );
	ns_assert( NULL != row_set_func );

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < region->length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				return;

			ns_progress_update(
				progress,
				100.0f * ( ( nsfloat )z / ( nsfloat )region->length )
				);
			}

		src_row  = src_slice;
		dest_row = dest_slice;

		for( y = 0; y < region->height; ++y )
			{
			for( x = 0; x < region->width; ++x )
				{
				( row_get_func )( &pixel, src_row, region->x + x );
				( row_set_func )( &pixel, dest_row, x );
				}

			src_row  = NS_OFFSET_POINTER( const void, src_row, src_bytes_per_row );
			dest_row = NS_OFFSET_POINTER( void, dest_row, dest_bytes_per_row );
			}

		src_slice  = NS_OFFSET_POINTER( const void, src_slice, src_bytes_per_slice );
		dest_slice = NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );
	}


NS_PRIVATE _NsPixelProcCopyFunc _ns_pixel_proc_copy_func
	(
	NsPixelType           pixel_type,
	const NsPixelRegion  *region
	)
	{
	_NsPixelProcCopyFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U1:
			func = ( NULL == region ) ?
					 _ns_pixel_proc_copy_func_fast : _ns_pixel_proc_copy_func_slow;
			break;

		case NS_PIXEL_LUM_U8:
		case NS_PIXEL_LUM_U12:
		case NS_PIXEL_LUM_U16:
		case NS_PIXEL_LUM_F32:
		case NS_PIXEL_LUM_ALPHA_U8_U8:
		case NS_PIXEL_RGB_U8_U8_U8:
		case NS_PIXEL_RGBA_U8_U8_U8_U8:
		case NS_PIXEL_BGR_U8_U8_U8:
		case NS_PIXEL_BGRA_U8_U8_U8_U8:
			func = ( NULL == region ) ?
					 _ns_pixel_proc_copy_func_fast : _ns_pixel_proc_copy_func_medium;
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
			_ns_pixel_proc_copy.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_copy_call
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	_NsPixelProcCopyFunc func;

	if( NULL == ( func = _ns_pixel_proc_copy_func( pixel_type, region ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != src_pixels );
	ns_assert( NULL != dest_pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	( func )(
		pixel_type,
		src_pixels,
		width,
		height,
		length,
		row_align,
		dest_pixels,
		region,
		progress
		);

	return ns_no_error();
	}


NsError _ns_pixel_proc_copy_marshal
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

	return _ns_pixel_proc_copy_call(
				ns_value_get_pixel_type( params + NS_PIXEL_PROC_COPY_PIXEL_TYPE ),
				ns_value_get_constpointer( params + NS_PIXEL_PROC_COPY_SRC_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_COPY_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_COPY_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_COPY_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_COPY_ROW_ALIGN ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_COPY_DEST_PIXELS ),
				ns_value_get_pixel_region( params + NS_PIXEL_PROC_COPY_REGION ),
				ns_value_get_progress( params + NS_PIXEL_PROC_COPY_PROGRESS )
				);
	}


NsProc* ns_pixel_proc_copy( void )
	{
	_ns_pixel_proc_copy.name        = NS_PIXEL_PROC_COPY;
	_ns_pixel_proc_copy.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_copy.params      = _ns_pixel_proc_copy_params;
	_ns_pixel_proc_copy.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_copy_params );
	_ns_pixel_proc_copy.marshal     = _ns_pixel_proc_copy_marshal;

	_ns_pixel_proc_copy.ret_value.name = NULL;
	_ns_pixel_proc_copy.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_copy;
	}
