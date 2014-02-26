#include "nspixels-subsample.h"


NS_PRIVATE NsProc _ns_pixel_proc_subsample;


NS_PRIVATE NsClosureValue _ns_pixel_proc_subsample_params[ NS_PIXEL_PROC_SUBSAMPLE_NUM_PARAMS ] =
	{
	{ "pixel_type",     NS_VALUE_PIXEL_TYPE },
	{ "src_pixels",     NS_VALUE_CONSTPOINTER },
	{ "src_width",      NS_VALUE_SIZE },
	{ "src_height",     NS_VALUE_SIZE },
	{ "src_length",     NS_VALUE_SIZE },
	{ "src_row_align",  NS_VALUE_SIZE },
	{ "dest_pixels",    NS_VALUE_POINTER },
	{ "dest_width",     NS_VALUE_SIZE },
	{ "dest_height",    NS_VALUE_SIZE },
	{ "dest_length",    NS_VALUE_SIZE },
	{ "dest_row_align", NS_VALUE_SIZE },
	{ "scale_x",        NS_VALUE_DOUBLE },
	{ "scale_y",        NS_VALUE_DOUBLE },
	{ "scale_z",        NS_VALUE_DOUBLE },
	{ "progress",       NS_VALUE_PROGRESS }
	};


typedef NsError ( *_NsPixelProcSubsampleFunc )
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
	nsdouble,
	nsdouble,
	nsdouble,
	NsProgress*
	);


NS_PRIVATE NsError _ns_pixel_proc_subsample_lum
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
	nsdouble         scale_x,
	nsdouble         scale_y,
	nsdouble         scale_z,
	NsProgress      *progress
	)
	{
	SUBSAMPLER      ss;
	int             ( *func )( SUBSAMPLER* );
	nsconstpointer  src_slice;
	nssize          src_bytes_per_slice;
	nspointer       dest_slice;
	nspointer       dest_end;
	nssize          dest_bytes_per_slice;
	nsint           index;
	nssize          z;


	/* This implementation cannot handle aligned scan lines. */
	ns_verify( 1 == src_row_align );
	ns_verify( 1 == dest_row_align );

	InitSubsampler(
		&ss,
		( nsint )src_width,
		( nsint )src_height,
		( nsint )src_length,
		scale_x / 100.0,
		scale_y / 100.0,
		scale_z / 100.0
		);

	ns_verify( ( nsint )dest_width  == ss.newxdim );
	ns_verify( ( nsint )dest_height == ss.newydim );
	ns_verify( ( nsint )dest_length == ss.newzdim );

	func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
			func = SubsampleLum1ub;
			break;

		case NS_PIXEL_LUM_U12:
			func = SubsampleLum1us12;
			break;

		case NS_PIXEL_LUM_U16:
			func = SubsampleLum1us16;
			break;

		default:
			ns_assert_not_reached();
		}

	ns_assert( NULL != func );

	dest_slice = dest_pixels;

	src_bytes_per_slice  = ns_pixel_buffer_slice_size( pixel_type, src_width, src_height, src_row_align );
	dest_bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, dest_width, dest_height, dest_row_align );

	dest_end = NS_OFFSET_POINTER( void, dest_pixels, dest_length * dest_bytes_per_slice );

	ns_progress_set_title( progress, _ns_pixel_proc_subsample.title );
	ns_progress_update( progress, NS_PROGRESS_BEGIN );

	z = 0;

	/* TODO: The subsampler library has been altered to handle a volume that is completely in
		memory. The original code was designed to be fed a series of images from disk and to
		save a new subsampled volume (as slices) to disk. The library could be re-written to
		better handle this and could also be multi-threaded, but this was not done to save time. */

	while( ! SubsamplerIsComplete( &ss ) )
		{
		if( ns_progress_cancelled( progress ) )
			break;

		ns_progress_update(
			progress,
			100.0f * ( ( nsfloat )z / ( nsfloat )dest_length )
			);

		++z;

		while( ( index = SubsamplerNextIndex( &ss ) ) != -1 )
			{
/*TEMP!!!!!!!!!!!*///ns_print( "%d ", index );

			ns_assert( ( nssize )index < src_length );
			src_slice = NS_OFFSET_POINTER( const void, src_pixels, ( nssize )index * src_bytes_per_slice );

			if( SubsamplerAddSrcImage( &ss, src_slice ) )
				{
				CleanSubsampler( &ss );
				return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
				}
			}

		ns_assert( NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice ) <= dest_end );
		SubsamplerAddDestImage( &ss, dest_slice );

		if( ( func )( &ss ) )
			{
			CleanSubsampler( &ss );
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}

/*TEMP!!!!!!!!!!!*///ns_print( "SUB%d ", ss.lastout );

		dest_slice = NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice );
		}

	CleanSubsampler( &ss );

	ns_progress_update( progress, NS_PROGRESS_END );
	return ns_no_error();
	}


NS_PRIVATE _NsPixelProcSubsampleFunc _ns_pixel_proc_subsample_func( NsPixelType pixel_type )
	{
	_NsPixelProcSubsampleFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
		case NS_PIXEL_LUM_U12:
		case NS_PIXEL_LUM_U16:
			func = _ns_pixel_proc_subsample_lum;
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
			_ns_pixel_proc_subsample.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_subsample_call
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
	nsdouble         scale_x,
	nsdouble         scale_y,
	nsdouble         scale_z,
	NsProgress      *progress
	)
	{
	_NsPixelProcSubsampleFunc func;

	if( NULL == ( func = _ns_pixel_proc_subsample_func( pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != src_pixels );
	ns_assert( 0 < src_width );
	ns_assert( 0 < src_height );
	ns_assert( 0 < src_length );
	ns_assert( NULL != dest_pixels );
	ns_assert( 0 < dest_width );
	ns_assert( 0 < dest_height );
	ns_assert( 0 < dest_length );

	#ifdef NS_DEBUG
	{
	nsint check_width, check_height, check_length;

	SubsamplerSizes(
		( nsint )src_width,
		( nsint )src_height,
		( nsint )src_length,
		scale_x / 100.0,
		scale_y / 100.0,
		scale_z / 100.0,
		&check_width,
		&check_height,
		&check_length
		);

	ns_assert( ( nsint )dest_width  == check_width );
	ns_assert( ( nsint )dest_height == check_height );
	ns_assert( ( nsint )dest_length == check_length );
	}
	#endif

	return ( func )(
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
				scale_x,
				scale_y,
				scale_z,
				progress
				);
	}

NsError _ns_pixel_proc_subsample_marshal
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

	return _ns_pixel_proc_subsample_call(
				ns_value_get_pixel_type( params + NS_PIXEL_PROC_SUBSAMPLE_PIXEL_TYPE ),
				ns_value_get_constpointer( params + NS_PIXEL_PROC_SUBSAMPLE_SRC_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_SRC_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_SRC_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_SRC_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_SRC_ROW_ALIGN ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_SUBSAMPLE_DEST_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_DEST_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_DEST_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_DEST_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_SUBSAMPLE_DEST_ROW_ALIGN ),
				ns_value_get_double( params + NS_PIXEL_PROC_SUBSAMPLE_SCALE_X ),
				ns_value_get_double( params + NS_PIXEL_PROC_SUBSAMPLE_SCALE_Y ),
				ns_value_get_double( params + NS_PIXEL_PROC_SUBSAMPLE_SCALE_Z ),
				ns_value_get_progress( params + NS_PIXEL_PROC_SUBSAMPLE_PROGRESS )
				);
	}


NsProc* ns_pixel_proc_subsample( void )
	{
	_ns_pixel_proc_subsample.name        = NS_PIXEL_PROC_SUBSAMPLE;
   _ns_pixel_proc_subsample.title       = "Subsample";
   _ns_pixel_proc_subsample.version     = "1.0";
	_ns_pixel_proc_subsample.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_subsample.params      = _ns_pixel_proc_subsample_params;
	_ns_pixel_proc_subsample.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_subsample_params );
	_ns_pixel_proc_subsample.marshal     = _ns_pixel_proc_subsample_marshal;

	_ns_pixel_proc_subsample.ret_value.name = NULL;
	_ns_pixel_proc_subsample.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_subsample;
	}
