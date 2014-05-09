#include "nsio-writetiff.h"


NS_PRIVATE NsProc _ns_io_proc_write_tiff;


NS_PRIVATE NsClosureValue _ns_io_proc_write_tiff_params[ NS_IO_PROC_WRITE_TIFF_NUM_PARAMS ] =
	{
	{ "file_name",       NS_VALUE_PCHAR },
	{ "pixel_type",      NS_VALUE_PIXEL_TYPE },
	{ "pixels",          NS_VALUE_CONSTPOINTER },
	{ "width",           NS_VALUE_SIZE },
	{ "height",          NS_VALUE_SIZE },
	{ "length",          NS_VALUE_SIZE },
	{ "row_align",       NS_VALUE_SIZE },
	{ "region",          NS_VALUE_PIXEL_REGION },
	{ "conv_pixel_type", NS_VALUE_PIXEL_TYPE },
	{ "progress",        NS_VALUE_PROGRESS }
	};


typedef NsError ( *_NsIoProcWriteTiffFunc )
	(
	const nschar*,
	NsPixelType,
	nsconstpointer,
	nssize,
	nssize,
	nssize,
	nssize,
	const NsPixelRegion*,
	NsPixelType,
	NsProgress*
	);


NS_PRIVATE void _ns_io_proc_write_tiff_progress_update( nsfloat percent, NsIO *io )
   {  ns_progress_update( io->progress, percent );  }


NS_PRIVATE nsint _ns_io_proc_write_tiff_progress_cancelled( NsIO *io )
   {  return ns_progress_cancelled( io->progress );  }


NS_PRIVATE NsError _ns_io_proc_write_tiff_xfrm_error( TiffErrorType in_error )
	{
	NsError out_error = ns_no_error();

	switch( in_error )
		{
		case TIFF_NO_ERROR:
			break;

		case TIFF_ERROR_FILE_OPEN:
			out_error = ns_error_noent( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_FILE_READ:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_FILE_WRITE:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_FILE_SEEK:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_FILE_INVALID:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_VERSION:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_COMPRESSED:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_OUT_OF_MEMORY:
			out_error = ns_error_nomem( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_UNSUPPORTED:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_PARAMETER:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case TIFF_ERROR_NOT_ENOUGH_SPACE:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		default:
			ns_assert_not_reached();
		}

	return out_error;
	}


TiffPixelInfo* ns_pixel_type_to_tiff_pixel_info( NsPixelType pixel_type, TiffPixelInfo *info )
	{
	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
			info->pixel_type = TIFF_PIXEL_LUM8;
			break;

		case NS_PIXEL_LUM_U12:
			info->pixel_type = TIFF_PIXEL_LUM12;
			break;

		case NS_PIXEL_LUM_U16:
			info->pixel_type = TIFF_PIXEL_LUM16;
			break;

		default:
			ns_assert_not_reached();
		}

	info->channels_per_pixel = 1;
	info->bits_per_pixel     = ns_pixel_bits( pixel_type );
	info->luminance_bits     = ns_pixel_used_bits( pixel_type );

	return info;
	}


NS_PRIVATE NsError _ns_io_proc_write_tiff_lum_u8_or_u12_or_u16
	(
	const nschar         *file_name,
	NsPixelType           pixel_type,
	nsconstpointer        pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	const NsPixelRegion  *pixel_region,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	)
	{
	NsIO           io;
	NsFile         file;
	TiffFile       tiff;
	TiffRegion     tiff_region;
	TiffErrorType  error;


	NS_USE_VARIABLE( row_align );

   ns_file_construct( &file );

   io.progress = progress;
   io.file     = &file;

	tiff_file_construct( &tiff, &io );

   tiff.width  = width;
   tiff.height = height;
   tiff.length = length;

	ns_pixel_type_to_tiff_pixel_info( pixel_type, &tiff.pixel_info );

	if( NS_PIXEL_UNSUPPORTED != conv_pixel_type )
		ns_pixel_type_to_tiff_pixel_info( conv_pixel_type, &tiff.conv_pixel_info );

   ns_progress_update( progress, NS_PROGRESS_BEGIN );
	ns_progress_set_title( progress, "Writing TIFF file..." );

	tiff.pixels = ( nspointer )pixels;

	tiff_region.x      = pixel_region->x;
	tiff_region.y      = pixel_region->y;
	tiff_region.z      = pixel_region->z;
	tiff_region.width  = pixel_region->width;
	tiff_region.height = pixel_region->height;
	tiff_region.length = pixel_region->length;

	error = tiff_file_write_ex(
				&tiff,
				&tiff_region,
				file_name,
				_ns_io_proc_write_tiff_progress_cancelled,
				_ns_io_proc_write_tiff_progress_update,
				&io
				);

	tiff.pixels = NULL;
	tiff_file_destruct( &tiff, &io );

   ns_file_destruct( &file );

   ns_progress_update( progress, NS_PROGRESS_END );

	return _ns_io_proc_write_tiff_xfrm_error( error );
	}


NS_PRIVATE _NsIoProcWriteTiffFunc _ns_io_proc_write_tiff_func( NsPixelType pixel_type )
	{
	_NsIoProcWriteTiffFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
		case NS_PIXEL_LUM_U12:
		case NS_PIXEL_LUM_U16:
			func = _ns_io_proc_write_tiff_lum_u8_or_u12_or_u16;
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
			_ns_io_proc_write_tiff.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_io_proc_write_tiff_call
	(
	const nschar         *file_name,
	NsPixelType           pixel_type,
	nsconstpointer        pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
   const NsPixelRegion  *sub_region,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	)
	{
	_NsIoProcWriteTiffFunc   func;
   NsPixelRegion            full_region;
   const NsPixelRegion     *region;


	if( NULL == ( func = _ns_io_proc_write_tiff_func( pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

   region = ns_pixel_region_extract( sub_region, &full_region, width, height, length );
   ns_pixel_region_verify( region, width, height, length );

	return ( func )(
				file_name,
				pixel_type,
				pixels,
				width,
				height,
				length,
				row_align,
            region,
				conv_pixel_type,
				progress
				);
	}


NS_PRIVATE NsError _ns_io_proc_write_tiff_marshal
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

	return _ns_io_proc_write_tiff_call(
				ns_value_get_pchar( params + NS_IO_PROC_WRITE_TIFF_FILE_NAME ),
				ns_value_get_pixel_type( params + NS_IO_PROC_WRITE_TIFF_PIXEL_TYPE ),
				ns_value_get_constpointer( params + NS_IO_PROC_WRITE_TIFF_PIXELS ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_TIFF_WIDTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_TIFF_HEIGHT ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_TIFF_LENGTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_TIFF_ROW_ALIGN ),
            ns_value_get_pixel_region( params + NS_IO_PROC_WRITE_TIFF_REGION ),
				ns_value_get_pixel_type( params + NS_IO_PROC_WRITE_TIFF_CONV_PIXEL_TYPE ),
				ns_value_get_progress( params + NS_IO_PROC_WRITE_TIFF_PROGRESS )
				);
	}


NsProc* ns_io_proc_write_tiff( void )
	{
	_ns_io_proc_write_tiff.name        = NS_IO_PROC_WRITE_TIFF;
	_ns_io_proc_write_tiff.title       = "TIFF Writer";
	_ns_io_proc_write_tiff.version     = "1.0";
	_ns_io_proc_write_tiff.help        = "";
	_ns_io_proc_write_tiff.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_io_proc_write_tiff.params      = _ns_io_proc_write_tiff_params;
	_ns_io_proc_write_tiff.num_params  = NS_ARRAY_LENGTH( _ns_io_proc_write_tiff_params );
	_ns_io_proc_write_tiff.marshal     = _ns_io_proc_write_tiff_marshal;

	_ns_io_proc_write_tiff.ret_value.name = NULL;
	_ns_io_proc_write_tiff.ret_value.type = NS_VALUE_VOID;

	return &_ns_io_proc_write_tiff;
	}
