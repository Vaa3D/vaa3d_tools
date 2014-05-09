#include "nspixels-getpixel.h"


NS_PRIVATE NsProc _ns_pixel_proc_get_pixel;


enum
	{
	__PIXEL_TYPE,
	__PIXELS,
	__WIDTH,
	__HEIGHT,
	__LENGTH,
	__ROW_ALIGN,
	__X,
	__Y,
	__Z,
	__PIXEL,

	__NUM_PARAMS
	};


NS_PRIVATE NsClosureValue _ns_pixel_proc_get_pixel_params[ __NUM_PARAMS ] =
	{
	{ "pixel_type", NS_VALUE_PIXEL_TYPE },
	{ "pixels",     NS_VALUE_CONSTPOINTER },
	{ "width",      NS_VALUE_SIZE },
	{ "height",     NS_VALUE_SIZE },
	{ "length",     NS_VALUE_SIZE },
	{ "row_align",  NS_VALUE_SIZE },
	{ "x",          NS_VALUE_SIZE },
	{ "y",          NS_VALUE_SIZE },
	{ "z",          NS_VALUE_SIZE },
	{ "pixel",      NS_VALUE_PIXEL }
	};
	

typedef void ( *_NsPixelProcGetPixelFunc )
	(
	NsPixelType,
	nsconstpointer,
	nssize,
	nssize,
	nssize,
	nssize,
	nssize,
	nssize,
	nssize,
	NsPixel*
	);


/* CANT handle accessing pixels that cross byte boundaries! */
NS_PRIVATE void _ns_pixel_proc_get_pixel_all
	(
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nssize           x,
	nssize           y,
	nssize           z,
	NsPixel         *pixel
	)
	{
	nssize          bytes_per_row;
	nssize          bytes_per_slice;
	nsconstpointer  slice;
	nsconstpointer  row;


	NS_USE_VARIABLE( length );

	bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	slice = NS_OFFSET_POINTER( const void, pixels, bytes_per_slice * z );
	row   = NS_OFFSET_POINTER( const void, slice, bytes_per_row * y );

	ns_pixel_row_get( pixel, pixel_type, row, x );
	}


NS_PRIVATE _NsPixelProcGetPixelFunc _ns_pixel_proc_get_pixel_func( NsPixelType pixel_type )
	{
	_NsPixelProcGetPixelFunc func = NULL;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U1:
		case NS_PIXEL_LUM_U8:
		case NS_PIXEL_LUM_U12:
		case NS_PIXEL_LUM_U16:
		case NS_PIXEL_LUM_F32:
		case NS_PIXEL_LUM_ALPHA_U8_U8:
		case NS_PIXEL_RGB_U8_U8_U8:
		case NS_PIXEL_RGBA_U8_U8_U8_U8:
		case NS_PIXEL_BGR_U8_U8_U8:
		case NS_PIXEL_BGRA_U8_U8_U8_U8:
			func = _ns_pixel_proc_get_pixel_all;
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
			_ns_pixel_proc_get_pixel.name
			);

	return func;
	}


NS_PRIVATE NsError _ns_pixel_proc_get_pixel_call
	(
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nssize           x,
	nssize           y,
	nssize           z,
	NsPixel         *pixel
	)
	{
	_NsPixelProcGetPixelFunc func;

	if( NULL == ( func = _ns_pixel_proc_get_pixel_func( pixel_type ) ) )
		return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

	ns_assert( NULL != pixels );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );
	ns_assert( x < width );
	ns_assert( y < height );
	ns_assert( z < length );
	ns_assert( NULL != pixel );

	( func )(
		pixel_type,
		pixels,
		width,
		height,
		length,
		row_align,
		x,
		y,
		z,
		pixel
		);

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_pixel_proc_get_pixel_marshal
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

	return _ns_pixel_proc_get_pixel_call(
				ns_value_get_pixel_type( params + __PIXEL_TYPE ),
				ns_value_get_constpointer( params + __PIXELS ),
				ns_value_get_size( params + __WIDTH ),
				ns_value_get_size( params + __HEIGHT ),
				ns_value_get_size( params + __LENGTH ),
				ns_value_get_size( params + __ROW_ALIGN ),
				ns_value_get_size( params + __X ),
				ns_value_get_size( params + __Y ),
				ns_value_get_size( params + __Z ),
				ns_value_get_pixel( params + __PIXEL )
				);
	}


NsProc* ns_pixel_proc_get_pixel( void )
	{
	_ns_pixel_proc_get_pixel.name        = NS_PIXEL_PROC_GET_PIXEL;
	_ns_pixel_proc_get_pixel.invoke_type = NS_PROC_INVOKE_MARSHAL;
	_ns_pixel_proc_get_pixel.params      = _ns_pixel_proc_get_pixel_params;
	_ns_pixel_proc_get_pixel.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_get_pixel_params );
	_ns_pixel_proc_get_pixel.marshal     = _ns_pixel_proc_get_pixel_marshal;

	_ns_pixel_proc_get_pixel.ret_value.name = NULL;
	_ns_pixel_proc_get_pixel.ret_value.type = NS_VALUE_VOID;

	return &_ns_pixel_proc_get_pixel;
	}
