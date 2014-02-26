#include "raw.h"


void* ( *raw_fopen )( const char*, const char*, void* ) = NULL;

int ( *raw_fclose )( void*, void* ) = NULL;

size_t ( *raw_fread )( void*, size_t, size_t, void*, void* ) = NULL;

size_t ( *raw_fwrite )( const void*, size_t, size_t, void*, void* ) = NULL;

int ( *raw_remove )( const char*, void* ) = NULL;

void* ( *raw_malloc )( size_t, void* ) = NULL;

void ( *raw_free )( void*, void* ) = NULL;


#define RAW_COMPILE_TIME_SIZE_CHECK( type, size )\
   extern unsigned char _raw_compile_time_size_check[ ( ( size ) == sizeof( type ) ) ? 1 : -1 ]


RAW_COMPILE_TIME_SIZE_CHECK( char, 1 );
RAW_COMPILE_TIME_SIZE_CHECK( unsigned char, 1 );


static int _raw_get_machine_endian( void )
   { 
   int i = 1;
   return ( ( 1 == *( ( char* )&i ) ) ? RAW_LITTLE_ENDIAN : RAW_BIG_ENDIAN );
   }


static void _raw_reverse_all_endian( void *buffer, size_t bytes_per_element, size_t num_elements )
   {
   unsigned char  *ptr, *first, *last, temp;
   size_t          i;


   ptr = ( unsigned char* )buffer;

   for( i = 0; i < num_elements; ++i )
      {
      first = ptr;
      last  = first + bytes_per_element - 1;

      while( first < last )
         { 
         temp   = *first;
         *first = *last;
         *last  = temp;

         ++first;
         --last;
         }

      ptr += bytes_per_element;
      }
   }


static RawErrorType _raw_read_header
	(
	void    *stream,
	void    *header,
	size_t   header_size,
	void    *user_data
	)
   {
	return 1 == raw_fread( header, header_size, 1, stream, user_data )
			 ? RAW_NO_ERROR : RAW_ERROR_FILE_READ;
	}


static RawErrorType _raw_read_byte_aligned_pixels
   (
   void     *stream,
   size_t    width,
	size_t    height,
	size_t    length,
   int       endian,
   void    **pixels,
	size_t    pixels_size,
	int       allocate_pixels,
	int       ( *cancelled )( void* ),
	void      ( *update )( float, void* ),
	void     *user_data,
   size_t    bytes_per_pixel,
   int       do_reverse_endian
   )
   {
	size_t          num_pixels;
	size_t          bytes;
	size_t          bytes_per_slice;
	size_t          num_pixels_per_slice;
	size_t          i;
	unsigned char  *curr, *last;


	RAW_USE_VARIABLE( endian );

	num_pixels = width * height * length;
	bytes      = num_pixels * bytes_per_pixel;

	if( ! allocate_pixels && pixels_size < bytes )
		return RAW_ERROR_NOT_ENOUGH_SPACE;

	if( allocate_pixels )
		if( NULL == ( *pixels = raw_malloc( bytes, user_data ) ) )
			return RAW_ERROR_OUT_OF_MEMORY;

	if( NULL == update && NULL == cancelled )
		{
		if( bytes != raw_fread( *pixels, 1, bytes, stream, user_data ) )
			return RAW_ERROR_FILE_READ;

		if( do_reverse_endian )
			_raw_reverse_all_endian( *pixels, bytes_per_pixel, num_pixels );
		}
	else
		{
		num_pixels_per_slice = width * height;

		bytes_per_slice = bytes / length;
		assert( bytes_per_slice * length == bytes );

		curr = *pixels;
		last = curr + ( bytes - bytes_per_slice );

		for( i = 0; i < length; ++i )
			{
			if( NULL != cancelled )
				if( ( cancelled )( user_data ) )
					return RAW_NO_ERROR;

			assert( curr <= last );

			if( bytes_per_slice != raw_fread( curr, 1, bytes_per_slice, stream, user_data ) )
				return RAW_ERROR_FILE_READ;

			if( do_reverse_endian )
				_raw_reverse_all_endian( curr, bytes_per_pixel, num_pixels_per_slice );

			curr += bytes_per_slice;

			if( NULL != update )
				( update )( ( float )i / ( float )length * 100.0f, user_data );
			}
		}

	return RAW_NO_ERROR;
   }


static RawErrorType _raw_read_lum8
   (
   void     *stream,
   size_t    width,
	size_t    height,
	size_t    length,
   int       endian,
   void    **pixels,
	size_t    pixels_size,
	int       allocate_pixels,
	int       ( *cancelled )( void* ),
	void      ( *update )( float, void* ),
	void     *user_data
   )
   {
	return _raw_read_byte_aligned_pixels(
				stream,
				width,
				height,
				length,
				endian,
				pixels,
				pixels_size,
				allocate_pixels,
				cancelled,
				update,
				user_data,
				1,
				0
				);
	}


static unsigned int _raw_max_intensity_lum16( unsigned short *pixel, size_t num_pixels )
	{
	unsigned int  max_intensity;
	size_t        i;


	max_intensity = 0;

	for( i = 0; i < num_pixels; ++i )
		{
		if( ( ( unsigned int )*pixel ) > max_intensity )
			max_intensity = ( ( unsigned int )*pixel );

		++pixel;
		}

	return max_intensity;
	}


static RawErrorType _raw_read_lum16
   (
   void           *stream,
   size_t          width,
	size_t          height,
	size_t          length,
   int             endian,
   void          **pixels,
	size_t          pixels_size,
	int             allocate_pixels,
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   )
   {
   int           do_reverse_endian;
	RawErrorType  error;


	do_reverse_endian = ( _raw_get_machine_endian() != endian ) ? 1 : 0;

	error = _raw_read_byte_aligned_pixels(
				stream,
				width,
				height,
				length,
				endian,
				pixels,
				pixels_size,
				allocate_pixels,
				cancelled,
				update,
				user_data,
				2,
				do_reverse_endian
				);

	if( RAW_NO_ERROR != error )
		return error;

	assert( 2 == sizeof( unsigned short ) );
	*max_intensity = _raw_max_intensity_lum16( *pixels, width * height * length );

	return RAW_NO_ERROR;
   }


static RawErrorType _raw_read_rgb24
   (
   void     *stream,
   size_t    width,
	size_t    height,
	size_t    length,
   int       endian,
   void    **pixels,
	size_t    pixels_size,
	int       allocate_pixels,
	int       ( *cancelled )( void* ),
	void      ( *update )( float, void* ),
	void     *user_data
   )
   {
	return _raw_read_byte_aligned_pixels(
				stream,
				width,
				height,
				length,
				endian,
				pixels,
				pixels_size,
				allocate_pixels,
				cancelled,
				update,
				user_data,
				3,
				0
				);
	}


static RawErrorType _raw_read_rgba32
   (
   void     *stream,
   size_t    width,
	size_t    height,
	size_t    length,
   int       endian,
   void    **pixels,
	size_t    pixels_size,
	int       allocate_pixels,
	int       ( *cancelled )( void* ),
	void      ( *update )( float, void* ),
	void     *user_data
   )
	{
	return _raw_read_byte_aligned_pixels(
				stream,
				width,
				height,
				length,
				endian,
				pixels,
				pixels_size,
				allocate_pixels,
				cancelled,
				update,
				user_data,
				4,
				0
				);
	}


static RawErrorType raw_file_do_read
   (
   const char     *file_name,
   void           *header,
   size_t          header_size,
   RawPixelType    type,
   size_t          width,
   size_t          height,
   size_t          length,
   int             endian,
   void          **pixels,
	size_t          pixels_size,
	int             allocate_pixels,
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   )
   {
   void          *stream;
   RawErrorType   error = RAW_NO_ERROR;
  

   assert( NULL != file_name );
   assert( NULL != pixels );
   assert( 0 < width );
   assert( 0 < height );
   assert( 0 < length );
   assert( RAW_LITTLE_ENDIAN == endian || RAW_BIG_ENDIAN == endian );
	assert( NULL != max_intensity );

	*max_intensity = 0;

   if( NULL == ( stream = raw_fopen( file_name, "rb", user_data ) ) )
      return RAW_ERROR_FILE_OPEN;

   if( 0 < header_size )
      {
      assert( NULL != header );

      if( RAW_NO_ERROR !=
			 ( error = _raw_read_header(
							stream,
							header,
							header_size,
							user_data
							) ) )
         {
         raw_fclose( stream, user_data );
         return error;
         }
      }

   switch( type )
      {
      case RAW_PIXEL_LUM8:
         error = _raw_read_lum8(
						stream,
						width,
						height,
						length,
						endian,
						pixels,
						pixels_size,
						allocate_pixels,
						cancelled,
						update,
						user_data
						);
         break;

      case RAW_PIXEL_LUM12:
      case RAW_PIXEL_LUM16:
         error = _raw_read_lum16(
						stream,
						width,
						height,
						length,
						endian,
						pixels,
						pixels_size,
						allocate_pixels,
						max_intensity,
						cancelled,
						update,
						user_data
						);
         break;

      case RAW_PIXEL_RGB24:
      case RAW_PIXEL_BGR24:
         error = _raw_read_rgb24(
						stream,
						width,
						height,
						length,
						endian,
						pixels,
						pixels_size,
						allocate_pixels,
						cancelled,
						update,
						user_data
						);
         break;

      case RAW_PIXEL_RGBA32:
      case RAW_PIXEL_BGRA32:
         error = _raw_read_rgba32(
						stream,
						width,
						height,
						length,
						endian,
						pixels,
						pixels_size,
						allocate_pixels,
						cancelled,
						update,
						user_data
						);
         break;

      default:
         assert( 0 );;
      }

   raw_fclose( stream, user_data );

	if( ( RAW_NO_ERROR != error ) ||
		 ( NULL != cancelled && ( cancelled )( user_data ) ) )
		if( allocate_pixels )
			{
			raw_free( *pixels, user_data );
			*pixels = NULL;
			}

   return error;
   }


RawErrorType raw_file_read
   (
   const char     *file_name,
   void           *header,
   size_t          header_size,
   RawPixelType    type,
   size_t          width,
   size_t          height,
   size_t          length,
   int             endian,
   void          **pixels,
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   )
	{
	return raw_file_do_read(
				file_name,
				header,
				header_size,
				type,
				width,
				height,
				length,
				endian,
				pixels,
				0,
				1,
				max_intensity,
				cancelled,
				update,
				user_data
				);
	}


RawErrorType raw_file_read_ex
   (
   const char     *file_name,
   void           *header,
   size_t          header_size,
   RawPixelType    type,
   size_t          width,
   size_t          height,
   size_t          length,
   int             endian,
   void           *pixels,
	size_t          pixels_size,
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   )
	{
	return raw_file_do_read(
				file_name,
				header,
				header_size,
				type,
				width,
				height,
				length,
				endian,
				&pixels,
				pixels_size,
				0,
				max_intensity,
				cancelled,
				update,
				user_data
				);
	}


static RawErrorType _raw_write_header
	(
	void        *stream,
	const void  *header,
	size_t       size,
	void        *user_data
	)
   {
	return 1 == raw_fwrite( header, size, 1, stream, user_data ) ?
			 RAW_NO_ERROR : RAW_ERROR_FILE_WRITE;
	}


static size_t _raw_bytes_per_pixel( RawPixelType type )
	{
	size_t bytes = 0;

	switch( type )
		{
		case RAW_PIXEL_RGB24:
		case RAW_PIXEL_BGR24:
			bytes = 3;
			break;

		case RAW_PIXEL_RGBA32:
		case RAW_PIXEL_BGRA32:
			bytes = 4;
			break;

		case RAW_PIXEL_LUM8:
			bytes = 1;
			break;

		case RAW_PIXEL_LUM12:
		case RAW_PIXEL_LUM16:
			bytes = 2;
			break;

		default:
			assert( 0 );;
		}

	return bytes;
	}


#define _RAW_X_BIT_LUM_TO_X_BIT_LUM( func )\
	size_t x, y, width, height, first;\
	\
	width  = region->width;\
	height = region->height;\
	first  = region->x;\
	\
	for( y = 0; y < height; ++y )\
		{\
		for( x = 0; x < width; ++x )\
			func;\
		\
		src += full_width;\
		}


static void raw_16_bit_lum_to_12_bit_lum( const rawuint16 *src, rawuint16 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( *( src + first + x ) >> 4 ) );  }


static void raw_16_bit_lum_to_8_bit_lum( const rawuint16 *src, rawuint8 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( rawuint8 )( *( src + first + x ) >> 8 ) );  }


static void raw_12_bit_lum_to_8_bit_lum( const rawuint16 *src, rawuint8 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( rawuint8 )( *( src + first + x ) >> 4 ) );  }


static void raw_12_bit_lum_to_16_bit_lum( const rawuint16 *src, rawuint16 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( *( src + first + x ) << 4 ) );  }


static void raw_8_bit_lum_to_12_bit_lum( const rawuint8 *src, rawuint16 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( rawuint16 )( *( src + first + x ) << 4 ) );  }


static void raw_8_bit_lum_to_16_bit_lum( const rawuint8 *src, rawuint16 *dest, const RawRegion *region, size_t full_width )
	{  _RAW_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( rawuint16 )( *( src + first + x ) << 8 ) );  }


typedef struct _RawConv
	{
	void     ( *func )( const void*, void*, const RawRegion*, size_t );
	void    *buffer;
	size_t   buffer_size;
	}
	RawConv;


static void raw_conv_init( RawConv *conv )
	{
	conv->func        = NULL;
	conv->buffer      = NULL;
	conv->buffer_size = 0;
	}


static RawErrorType raw_conv_create
	(
	RawConv          *conv,
	const RawRegion  *region,
	RawPixelType      type,
	RawPixelType      conv_type,
	void             *user_data
	)
	{
	if( RAW_UNSUPPORTED == conv_type )
		return RAW_NO_ERROR;

	if( conv_type == type )
		return RAW_ERROR_PARAMETER;

	assert( NULL == conv->buffer );
	assert( 0 == conv->buffer_size );

	conv->buffer_size = _raw_bytes_per_pixel( conv_type ) * region->width * region->height;

	if( NULL == ( conv->buffer = raw_malloc( conv->buffer_size, user_data ) ) )
		return RAW_ERROR_OUT_OF_MEMORY;

	assert( NULL == conv->func );

	switch( type )
		{
		case RAW_PIXEL_LUM8:
			switch( conv_type )
				{
				case RAW_PIXEL_LUM12:
					conv->func = raw_8_bit_lum_to_12_bit_lum;
					break;

				case RAW_PIXEL_LUM16:
					conv->func = raw_8_bit_lum_to_16_bit_lum;
					break;

				default:
					return RAW_ERROR_PARAMETER;
				}
			break;

		case RAW_PIXEL_LUM12:
			switch( conv_type )
				{
				case RAW_PIXEL_LUM8:
					conv->func = raw_12_bit_lum_to_8_bit_lum;
					break;

				case RAW_PIXEL_LUM16:
					conv->func = raw_12_bit_lum_to_16_bit_lum;
					break;

				default:
					return RAW_ERROR_PARAMETER;
				}
			break;

		case RAW_PIXEL_LUM16:
			switch( conv_type )
				{
				case RAW_PIXEL_LUM8:
					conv->func = raw_16_bit_lum_to_8_bit_lum;
					break;

				case RAW_PIXEL_LUM12:
					conv->func = raw_16_bit_lum_to_12_bit_lum;
					break;

				default:
					return RAW_ERROR_PARAMETER;
				}
			break;

		default:
			return RAW_ERROR_PARAMETER;
		}

	return RAW_NO_ERROR;
	}


static void raw_conv_finalize( RawConv *conv, void *user_data )
	{  raw_free( conv->buffer, user_data );  }


static RawErrorType _raw_write_byte_aligned_pixels
   (
   void                 *stream,
	const RawRegion      *region,
   size_t                full_width,
   size_t                full_height,
   size_t                full_length,
   const unsigned char  *pixels,
	int                   ( *cancelled )( void* ),
	void                  ( *update )( float, void* ),
	void                 *user_data,
	RawPixelType          type,
	RawPixelType          conv_type,
	RawConv              *conv
   )
   {
	size_t                z, y;
	const unsigned char  *slice, *row;
	size_t                bytes_per_pixel;
	size_t                bytes_per_slice;
	size_t                full_bytes_per_row;
	size_t                region_bytes_per_row;


	RAW_USE_VARIABLE( full_length );

	bytes_per_pixel      = _raw_bytes_per_pixel( type );
	bytes_per_slice      = full_width * full_height * bytes_per_pixel;
	full_bytes_per_row   = full_width * bytes_per_pixel;
	region_bytes_per_row = region->width * bytes_per_pixel;

	slice = pixels + ( region->z * bytes_per_slice );

	for( z = 0; z < region->length; ++z )
		{
		if( NULL != cancelled )
			if( ( cancelled )( user_data ) )
				return RAW_NO_ERROR;
		
		if( RAW_UNSUPPORTED != conv_type )
			{
			( conv->func )( slice, conv->buffer, region, full_width );

			if( conv->buffer_size != raw_fwrite( conv->buffer, 1, conv->buffer_size, stream, user_data ) )
				return RAW_ERROR_FILE_WRITE;
			}
		else
			{
			row = slice + ( region->y * full_bytes_per_row ) + ( region->x * bytes_per_pixel );

			for( y = 0; y < region->height; ++y )
				{
				if( region_bytes_per_row != raw_fwrite( row, 1, region_bytes_per_row, stream, user_data ) )
					return RAW_ERROR_FILE_WRITE;

				row += full_bytes_per_row;
				}
			}

		slice += bytes_per_slice;

		if( NULL != update )
			( update )( ( float )z / ( float )region->length * 100.0f, user_data );
		}

	return RAW_NO_ERROR;
   }


static RawErrorType _raw_file_do_write
   (
	const RawRegion  *sub_region,
   const char       *file_name,
   const void       *header,
   size_t            header_size,
   RawPixelType      type,
   size_t            full_width,
   size_t            full_height,
   size_t            full_length,
   const void       *pixels,
	RawPixelType      conv_type,
	int               ( *cancelled )( void* ),
	void              ( *update )( float, void* ),
	void              *user_data
   )
   {
   void             *stream;
	RawRegion         full_region;
	const RawRegion  *region;
	RawConv           conv;
   RawErrorType      error;


	error  = RAW_NO_ERROR;
	stream = NULL;

	raw_conv_init( &conv );

	if( NULL == sub_region )
		{
		full_region.x      = 0;
		full_region.y      = 0;
		full_region.z      = 0;
		full_region.width  = full_width;
		full_region.height = full_height;
		full_region.length = full_length;

		region = &full_region;
		}
	else
		region = sub_region;

	if( 0 == region->width || 0 == region->height || 0 == region->length )
		{
		error = RAW_ERROR_PARAMETER;
		goto _RAW_FILE_DO_WRITE_EXIT;
		}

	if( full_width  <= region->x ||
		 full_height <= region->y ||
		 full_length <= region->z   )
		{
		error = RAW_ERROR_PARAMETER;
		goto _RAW_FILE_DO_WRITE_EXIT;
		}

	if( full_width  < region->x + region->width  ||
		 full_height < region->y + region->height ||
		 full_length < region->z + region->length   )
		{
		error = RAW_ERROR_PARAMETER;
		goto _RAW_FILE_DO_WRITE_EXIT;
		}

   if( NULL == ( stream = raw_fopen( file_name, "wb", user_data ) ) )
		{
      error = RAW_ERROR_FILE_OPEN;
		goto _RAW_FILE_DO_WRITE_EXIT;
		}

   if( 0 < header_size )
      {
      assert( NULL != header );

      if( RAW_NO_ERROR !=
			 ( error = _raw_write_header(
							stream,
							header,
							header_size,
							user_data
							) ) )
			goto _RAW_FILE_DO_WRITE_EXIT;
      }

	if( RAW_NO_ERROR != ( error =
			raw_conv_create(
				&conv,
				region,
				type,
				conv_type,
				user_data
				) ) )
		goto _RAW_FILE_DO_WRITE_EXIT;

   switch( type )
      {
      case RAW_PIXEL_LUM8:
      case RAW_PIXEL_LUM12:
      case RAW_PIXEL_LUM16:
      case RAW_PIXEL_RGB24:
      case RAW_PIXEL_BGR24:
      case RAW_PIXEL_RGBA32:
      case RAW_PIXEL_BGRA32:
			error =
				_raw_write_byte_aligned_pixels(
					stream,
					region,
					full_width,
					full_height,
					full_length,
					pixels,
					cancelled,
					update,
					user_data,
					type,
					conv_type,
					&conv
					);
         break;

      default:
         assert( 0 );;
      }

	_RAW_FILE_DO_WRITE_EXIT:

	raw_conv_finalize( &conv, user_data );

	if( NULL != stream )
		raw_fclose( stream, user_data );

   if( RAW_NO_ERROR != error )
      raw_remove( file_name, user_data );

   return error;
   }



RawErrorType raw_file_write
   (
   const char    *file_name,
   const void    *header,
   size_t         header_size,
   RawPixelType   type,
   size_t         width,
   size_t         height,
   size_t         length,
   const void    *pixels,
	RawPixelType   conv_type,
	int            ( *cancelled )( void* ),
	void           ( *update )( float, void* ),
	void          *user_data
   )
	{
   assert( NULL != file_name );
   assert( NULL != pixels );
   assert( 0 < width );
   assert( 0 < height );
   assert( 0 < length );

	return _raw_file_do_write(
				NULL,
				file_name,
				header,
				header_size,
				type,
				width,
				height,
				length,
				pixels,
				conv_type,
				cancelled,
				update,
				user_data
				);
	}


RawErrorType raw_file_write_ex
   (
	const RawRegion  *region,
   const char       *file_name,
   const void       *header,
   size_t            header_size,
   RawPixelType      type,
   size_t            width,
   size_t            height,
   size_t            length,
   const void       *pixels,
	RawPixelType      conv_type,
	int               ( *cancelled )( void* ),
	void              ( *update )( float, void* ),
	void              *user_data
   )
	{
	assert( NULL != region );
   assert( NULL != file_name );
   assert( NULL != pixels );
   assert( 0 < width );
   assert( 0 < height );
   assert( 0 < length );

	return _raw_file_do_write(
				region,
				file_name,
				header,
				header_size,
				type,
				width,
				height,
				length,
				pixels,
				conv_type,
				cancelled,
				update,
				user_data
				);
	}
