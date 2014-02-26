#include "nspixels-gammacorrect.h"


NS_PRIVATE NsProc _ns_pixel_proc_gamma_correct;


NS_PRIVATE NsClosureValue _ns_pixel_proc_gamma_correct_params[ NS_PIXEL_PROC_GAMMA_CORRECT_NUM_PARAMS ] =
	{
	{ "pixel_type", NS_VALUE_PIXEL_TYPE },
	{ "pixels",     NS_VALUE_POINTER },
	{ "width",      NS_VALUE_SIZE },
	{ "height",     NS_VALUE_SIZE },
	{ "length",     NS_VALUE_SIZE },
	{ "row_align",  NS_VALUE_SIZE },
	{ "gamma",      NS_VALUE_DOUBLE },
	{ "region",     NS_VALUE_PIXEL_REGION },
	{ "progress",   NS_VALUE_PROGRESS }
	};


typedef NsError ( *_NsPixelProcGammaCorrectFunc )
	(
	NsPixelType,
	nspointer,
	nssize,
	nssize,
	nssize,
	nssize,
	nsdouble,
	const NsPixelRegion*,
	NsProgress*
	);


NS_PRIVATE void _ns_pixel_proc_gamma_correct_lut
	(
	NsPixelLut  *lut,
	nsdouble     gamma
	)
	{
	nssize    i;
	nsdouble  max;
	nsdouble  value;


	max = ( nsdouble )( lut->size - 1 );

   for( i = 0; i < lut->size; ++i )
		{
		value = ns_pow( ( nsdouble )i / max, 1.0 / gamma );
		value = value * max + 0.5; /* Proper rounding! */

      ns_pixel_lut_set_value( lut, i, ( nsuint )value );
		}
	}


NS_PRIVATE NsError _ns_pixel_proc_gamma_correct_all
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsdouble              gamma,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	NsPixelInfo                    info;
	const NsPixelChannel          *channel;
	nssize                         x;
	nssize                         y;
	nssize                         z;
	nssize                         bytes_per_row;
	nssize                         bytes_per_slice;
	nspointer                      slice;
	nspointer                      row;
	nsuint                         value;
	NsPixelRowGetUintChannelFunc   row_get_uint_channel_funcs[ NS_PIXEL_NUM_CHANNELS ];
	NsPixelRowSetUintChannelFunc   row_set_uint_channel_funcs[ NS_PIXEL_NUM_CHANNELS ];
	NsPixelLut                     luts[ NS_PIXEL_NUM_CHANNELS ];
	NsError                        error;


	NS_USE_VARIABLE( length );

	error = ns_no_error();

	ns_pixel_info( &info, pixel_type );

	for( channel = info.channel; NULL != channel; channel = channel->next )
		{
		row_get_uint_channel_funcs[ channel->index ] =
			ns_pixel_row_get_uint_channel_func( pixel_type, channel->index );

		row_set_uint_channel_funcs[ channel->index ] =
			ns_pixel_row_set_uint_channel_func( pixel_type, channel->index );

		ns_assert( NULL != row_get_uint_channel_funcs[ channel->index ] );
		ns_assert( NULL != row_set_uint_channel_funcs[ channel->index ] );

		luts[ channel->index ].values = NULL;
		}

	/* NOTE: Dont affect the alpha channel, if there is one. */
	for( channel = info.channel; NULL != channel; channel = channel->next )
		if( ! ns_pixel_channel_is( channel, NS_PIXEL_CHANNEL_ALPHA_NAME ) )
			{
			if( NS_FAILURE( ns_pixel_lut_init( luts + channel->index, channel ), error ) )
				goto _NS_PIXEL_PROC_GAMMA_CORRECT_ALL_EXIT;

			_ns_pixel_proc_gamma_correct_lut( luts + channel->index, gamma );
			}

	bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	slice = pixels;
	slice = NS_OFFSET_POINTER( void, slice, region->z * bytes_per_slice );
	slice = NS_OFFSET_POINTER( void, slice, region->y * bytes_per_row );

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_BEGIN );

	for( z = 0; z < region->length; ++z )
		{
		if( NULL != progress )
			{
			if( ns_progress_cancelled( progress ) )
				break;

			ns_progress_update(
				progress,
				100.0f * ( ( nsfloat )z / ( nsfloat )region->length )
				);
			}

		row = slice;

		for( y = 0; y < region->height; ++y )
			{
			for( x = 0; x < region->width; ++x )
				for( channel = info.channel; NULL != channel; channel = channel->next )
					if( NULL != luts[ channel->index ].values )
						{
						value = ( row_get_uint_channel_funcs[ channel->index ] )( row, region->x + x );
						value = ns_pixel_lut_get_value( luts + channel->index, value );
						( row_set_uint_channel_funcs[ channel->index ] )( row, region->x + x, value );
						}

			row = NS_OFFSET_POINTER( void, row, bytes_per_row );
			}

		slice = NS_OFFSET_POINTER( void, slice, bytes_per_slice );
		}

	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );

	_NS_PIXEL_PROC_GAMMA_CORRECT_ALL_EXIT:

	for( channel = info.channel; NULL != channel; channel = channel->next )
		ns_pixel_lut_finalize( luts + channel->index );

	return error;
	}


NS_PRIVATE _NsPixelProcGammaCorrectFunc _ns_pixel_proc_gamma_correct_func( NsPixelType pixel_type )
	{
	_NsPixelProcGammaCorrectFunc func = NULL;

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
			func = _ns_pixel_proc_gamma_correct_all;
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
			_ns_pixel_proc_gamma_correct.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_gamma_correct_call
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsdouble              gamma,
	const NsPixelRegion  *sub_region,
	NsProgress           *progress
	)
	{
	_NsPixelProcGammaCorrectFunc   func;
	NsPixelRegion                  full_region;
	const NsPixelRegion           *region;


	if( NULL == ( func = _ns_pixel_proc_gamma_correct_func( pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	region = ns_pixel_region_extract( sub_region, &full_region, width, height, length );
	ns_pixel_region_verify( region, width, height, length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", pixels=" NS_FMT_POINTER
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", row_align=" NS_FMT_ULONG
		", gamma=" NS_FMT_DOUBLE
		", region={x:" NS_FMT_ULONG
			",y:" NS_FMT_ULONG
			",z:" NS_FMT_ULONG
			",width:" NS_FMT_ULONG
			",height:" NS_FMT_ULONG
			",length:" NS_FMT_ULONG
			"}"
		", progress=" NS_FMT_POINTER
		" )",
		ns_pixel_type_to_string( pixel_type ),
		pixels,
		width,
		height,
		length,
		row_align,
		gamma,
		region->x, region->y, region->z, region->width, region->height, region->length,
		progress
		);

	return ( func )(
				pixel_type,
				pixels,
				width,
				height,
				length,
				row_align,
				gamma,
				region,
				progress
				);
	}


NS_PRIVATE NsError _ns_pixel_proc_gamma_correct_marshal
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

	return _ns_pixel_proc_gamma_correct_call(
				ns_value_get_pixel_type( params + NS_PIXEL_PROC_GAMMA_CORRECT_PIXEL_TYPE ),
				ns_value_get_pointer( params + NS_PIXEL_PROC_GAMMA_CORRECT_PIXELS ),
				ns_value_get_size( params + NS_PIXEL_PROC_GAMMA_CORRECT_WIDTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_GAMMA_CORRECT_HEIGHT ),
				ns_value_get_size( params + NS_PIXEL_PROC_GAMMA_CORRECT_LENGTH ),
				ns_value_get_size( params + NS_PIXEL_PROC_GAMMA_CORRECT_ROW_ALIGN ),
				ns_value_get_double( params + NS_PIXEL_PROC_GAMMA_CORRECT_GAMMA ),
				ns_value_get_pixel_region( params + NS_PIXEL_PROC_GAMMA_CORRECT_REGION ),
				ns_value_get_progress( params + NS_PIXEL_PROC_GAMMA_CORRECT_PROGRESS )
				);
	}


NsProc* ns_pixel_proc_gamma_correct( void )
	{
	_ns_pixel_proc_gamma_correct.name        = NS_PIXEL_PROC_GAMMA_CORRECT;
	_ns_pixel_proc_gamma_correct.title       = "Gamma Correction";
	_ns_pixel_proc_gamma_correct.version     = "1.0";
	_ns_pixel_proc_gamma_correct.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_gamma_correct.params      = _ns_pixel_proc_gamma_correct_params;
	_ns_pixel_proc_gamma_correct.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_gamma_correct_params );
	_ns_pixel_proc_gamma_correct.marshal     = _ns_pixel_proc_gamma_correct_marshal;

	_ns_pixel_proc_gamma_correct.ret_value.name = NULL;
	_ns_pixel_proc_gamma_correct.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_gamma_correct;
	}
