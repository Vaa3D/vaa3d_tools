#include "nsio-writejpeg.h"


NS_PRIVATE NsProc _ns_io_proc_write_jpeg;


NS_PRIVATE NsClosureValue _ns_io_proc_write_jpeg_params[ NS_IO_PROC_WRITE_JPEG_NUM_PARAMS ] =
	{
	{ "file_name",  NS_VALUE_PCHAR },
	{ "pixel_type", NS_VALUE_PIXEL_TYPE },
	{ "pixels",     NS_VALUE_CONSTPOINTER },
	{ "width",      NS_VALUE_SIZE },
	{ "height",     NS_VALUE_SIZE },
	{ "length",     NS_VALUE_SIZE },
	{ "row_align",  NS_VALUE_SIZE },
	{ "quality",    NS_VALUE_INT },
	{ "progress",   NS_VALUE_PROGRESS }
	};


typedef NsError ( *_NsIoProcWriteJpegFunc )
	(
	const nschar*,
	NsPixelType,
	nsconstpointer,
	nssize,
	nssize,
	nssize,
	nssize,
	nsint,
	NsProgress*
	);


NS_PRIVATE NsError _ns_io_proc_write_jpeg_xfrm_error( JpegErrorType in_error )
	{
	NsError out_error = ns_no_error();

	switch( in_error )
		{
		case JPEG_NO_ERROR:
			break;

		case JPEG_ERROR_FILE_OPEN:
			out_error = ns_error_noent( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		case JPEG_ERROR_INTERNAL:
			out_error = ns_error_unknown( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			break;

		default:
			ns_assert_not_reached();
		}

	return out_error;
	}


NS_PRIVATE NsError _ns_io_proc_write_jpeg_rgb_or_bgr_u8_u8_u8
	(
	const nschar    *file_name,
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nsint            quality,
	NsProgress      *progress
	)
	{
	NsIO           io;
	NsFile         file;
	JpegErrorType  error;


	NS_USE_VARIABLE( pixel_type );
	NS_USE_VARIABLE( length );
	NS_USE_VARIABLE( row_align );
	NS_USE_VARIABLE( progress );

	ns_file_construct( &file );
	io.file = &file;


/*TEMP: Dont need these if jpeg support enabled. */
NS_USE_VARIABLE( file_name );
NS_USE_VARIABLE( pixels );
NS_USE_VARIABLE( width );
NS_USE_VARIABLE( height );
NS_USE_VARIABLE( quality );


	error = jpeg_file_write(
				file_name,
				JPEG_PIXEL_RGB24,
				width,
				height,
				pixels,
				quality,
				&io
				);

	ns_file_destruct( &file );

	return _ns_io_proc_write_jpeg_xfrm_error( error );
	}


NS_PRIVATE _NsIoProcWriteJpegFunc _ns_io_proc_write_jpeg_func( NsPixelType pixel_type )
	{
	_NsIoProcWriteJpegFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_RGB_U8_U8_U8:
		case NS_PIXEL_BGR_U8_U8_U8:
			func = _ns_io_proc_write_jpeg_rgb_or_bgr_u8_u8_u8;
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
			_ns_io_proc_write_jpeg.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_io_proc_write_jpeg_call
	(
	const nschar    *file_name,
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nsint            quality,
	NsProgress      *progress
	)
	{
	_NsIoProcWriteJpegFunc func;

	if( NULL == ( func = _ns_io_proc_write_jpeg_func( pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 1 == length );
	ns_assert( 1 == row_align );

	return ( func )(
				file_name,
				pixel_type,
				pixels,
				width,
				height,
				length,
				row_align,
				quality,
				progress
				);
	}


NS_PRIVATE NsError _ns_io_proc_write_jpeg_marshal
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

	return _ns_io_proc_write_jpeg_call(
				ns_value_get_pchar( params + NS_IO_PROC_WRITE_JPEG_FILE_NAME ),
				ns_value_get_pixel_type( params + NS_IO_PROC_WRITE_JPEG_PIXEL_TYPE ),
				ns_value_get_constpointer( params + NS_IO_PROC_WRITE_JPEG_PIXELS ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_JPEG_WIDTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_JPEG_HEIGHT ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_JPEG_LENGTH ),
				ns_value_get_size( params + NS_IO_PROC_WRITE_JPEG_ROW_ALIGN ),
				ns_value_get_int( params + NS_IO_PROC_WRITE_JPEG_QUALITY ),
				ns_value_get_progress( params + NS_IO_PROC_WRITE_JPEG_PROGRESS )
				);
	}


NsProc* ns_io_proc_write_jpeg( void )
	{
	_ns_io_proc_write_jpeg.name        = NS_IO_PROC_WRITE_JPEG;
	_ns_io_proc_write_jpeg.title       = "JPEG Writer";
	_ns_io_proc_write_jpeg.version     = "1.0";
	_ns_io_proc_write_jpeg.help        = "";
	_ns_io_proc_write_jpeg.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_io_proc_write_jpeg.params      = _ns_io_proc_write_jpeg_params;
	_ns_io_proc_write_jpeg.num_params  = NS_ARRAY_LENGTH( _ns_io_proc_write_jpeg_params );
	_ns_io_proc_write_jpeg.marshal     = _ns_io_proc_write_jpeg_marshal;

	_ns_io_proc_write_jpeg.ret_value.name = NULL;
	_ns_io_proc_write_jpeg.ret_value.type = NS_VALUE_VOID;

	return &_ns_io_proc_write_jpeg;
	}
