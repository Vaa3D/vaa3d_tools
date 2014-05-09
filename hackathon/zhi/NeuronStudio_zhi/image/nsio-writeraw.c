#include "nsio-writeraw.h"


NS_PRIVATE NsProc _ns_io_proc_write_raw;


NS_PRIVATE NsClosureValue _ns_io_proc_write_raw_params[ NS_IO_PROC_WRITE_RAW_NUM_PARAMS ] =
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


typedef NsError ( *_NsIoProcWriteRawFunc )
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


NS_PRIVATE void _ns_io_proc_write_raw_progress_update( nsfloat percent, NsIO *io )
   {  ns_progress_update( io->progress, percent );  }


NS_PRIVATE nsint _ns_io_proc_write_raw_progress_cancelled( NsIO *io )
   {  return ns_progress_cancelled( io->progress );  }


NS_PRIVATE NsError _ns_io_proc_write_raw_xfrm_error( RawErrorType in_error )
	{
	NsError out_error = ns_no_error();

	switch( in_error )
		{
		case RAW_NO_ERROR:
			break;

		case RAW_ERROR_FILE_OPEN:
			out_error = ns_error_noent( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case RAW_ERROR_FILE_READ:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case RAW_ERROR_FILE_WRITE:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case RAW_ERROR_FILE_SEEK:
			out_error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case RAW_ERROR_OUT_OF_MEMORY:
			out_error = ns_error_nomem( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case RAW_ERROR_NOT_ENOUGH_SPACE:
			out_error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		default:
			ns_assert_not_reached();
		}

	return out_error;
	}


RawPixelType ns_pixel_type_to_raw_pixel_type( NsPixelType pixel_type )
	{
   switch( pixel_type )
      {
		case NS_PIXEL_UNSUPPORTED:
			pixel_type = ( NsPixelType )RAW_UNSUPPORTED;
			break;

      case NS_PIXEL_LUM_U8:
			pixel_type = ( NsPixelType )RAW_PIXEL_LUM8;
			break;

      case NS_PIXEL_LUM_U12:
			pixel_type = ( NsPixelType )RAW_PIXEL_LUM12;
			break;

      case NS_PIXEL_LUM_U16:
			pixel_type = ( NsPixelType )RAW_PIXEL_LUM16;
			break;

      default:
         ns_assert_not_reached();
      }

	return ( RawPixelType )pixel_type;
	}


NS_PRIVATE NsError _ns_io_proc_write_raw_lum_u8_or_u12_or_u16
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
	NsIO          io;
	NsFile        file;
	RawRegion     raw_region;
	RawErrorType  error;


	NS_USE_VARIABLE( row_align );

   ns_file_construct( &file );

   io.progress = progress;
   io.file     = &file;

	pixel_type      = ( NsPixelType )ns_pixel_type_to_raw_pixel_type( pixel_type );
	conv_pixel_type = ( NsPixelType )ns_pixel_type_to_raw_pixel_type( conv_pixel_type );

   ns_progress_update( progress, NS_PROGRESS_BEGIN );
	ns_progress_set_title( progress, "Writing RAW file..." );

	raw_region.x      = pixel_region->x;
	raw_region.y      = pixel_region->y;
	raw_region.z      = pixel_region->z;
	raw_region.width  = pixel_region->width;
	raw_region.height = pixel_region->height;
	raw_region.length = pixel_region->length;

	error = raw_file_write_ex(
				&raw_region,
				file_name,
				NULL,
				0,
				( RawPixelType )pixel_type,
				width,
				height,
				length,
				pixels,
				( RawPixelType )conv_pixel_type,
				_ns_io_proc_write_raw_progress_cancelled,
				_ns_io_proc_write_raw_progress_update,
				&io
				);

   ns_file_destruct( &file );
   ns_progress_update( progress, NS_PROGRESS_END );

	return _ns_io_proc_write_raw_xfrm_error( error );
	}


NS_PRIVATE _NsIoProcWriteRawFunc _ns_io_proc_write_raw_func( NsPixelType pixel_type )
	{
	_NsIoProcWriteRawFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
		case NS_PIXEL_LUM_U12:
		case NS_PIXEL_LUM_U16:
			func = _ns_io_proc_write_raw_lum_u8_or_u12_or_u16;
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
			_ns_io_proc_write_raw.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_io_proc_write_raw_call
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
	_NsIoProcWriteRawFunc    func;
   NsPixelRegion            full_region;
   const NsPixelRegion     *region;


	if( NULL == ( func = _ns_io_proc_write_raw_func( pixel_type ) ) )
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


NS_PRIVATE NsError _ns_io_proc_write_raw_marshal
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

	return _ns_io_proc_write_raw_call(
				ns_value_get_pchar( params + NS_IO_PROC_WRITE_RAW_FILE_NAME ),
				ns_value_get_pixel_type( params + NS_IO_PROC_WRITE_RAW_PIXEL_TYPE ),
				ns_value_get_constpointer( params + NS_IO_PROC_WRITE_RAW_PIXELS ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_RAW_WIDTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_RAW_HEIGHT ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_RAW_LENGTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_RAW_ROW_ALIGN ),
            ns_value_get_pixel_region( params + NS_IO_PROC_WRITE_RAW_REGION ),
				ns_value_get_pixel_type( params + NS_IO_PROC_WRITE_RAW_CONV_PIXEL_TYPE ),
				ns_value_get_progress( params + NS_IO_PROC_WRITE_RAW_PROGRESS )
				);
	}


NsProc* ns_io_proc_write_raw( void )
	{
	_ns_io_proc_write_raw.name        = NS_IO_PROC_WRITE_RAW;
	_ns_io_proc_write_raw.title       = "RAW Writer";
	_ns_io_proc_write_raw.version     = "1.0";
	_ns_io_proc_write_raw.help        = "";
	_ns_io_proc_write_raw.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_io_proc_write_raw.params      = _ns_io_proc_write_raw_params;
	_ns_io_proc_write_raw.num_params  = NS_ARRAY_LENGTH( _ns_io_proc_write_raw_params );
	_ns_io_proc_write_raw.marshal     = _ns_io_proc_write_raw_marshal;

	_ns_io_proc_write_raw.ret_value.name = NULL;
	_ns_io_proc_write_raw.ret_value.type = NS_VALUE_VOID;

	return &_ns_io_proc_write_raw;
	}
