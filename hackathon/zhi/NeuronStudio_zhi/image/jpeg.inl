#include "jpeg.h"


void* ( *jpeg_fopen )( const char*, const char*, void* ) = NULL;

int ( *jpeg_fclose )( void*, void* ) = NULL;

int ( *jpeg_remove )( const char*, void* ) = NULL;

int ( *jpeg_setjmp )( void* ) = NULL;

void ( *jpeg_longjmp )( int, void* ) = NULL;


#define JPEG_COMPILE_TIME_SIZE_CHECK( type, size )\
   extern unsigned char _jpeg_compile_time_size_check[ ( ( size ) == sizeof( type ) ) ? 1 : -1 ]


JPEG_COMPILE_TIME_SIZE_CHECK( char, 1 );
JPEG_COMPILE_TIME_SIZE_CHECK( unsigned char, 1 );


#include <jpeglib.h>
#include <jerror.h>


struct JpegError
	{
	struct jpeg_error_mgr   mgr; /* Has to be the first member! */
	void                   *user_data;
	};


void _jpeg_error_callback( j_common_ptr jc )
	{
	struct JpegError *je = ( struct JpegError* )jc->err;
	/* ( *jc->err->output_message )( jc ); */
	jpeg_longjmp( 1, je->user_data );
	}


/* NOTE: JSAMPLE and JSAMPROW are unsigned char's. */

static JpegErrorType _jpeg_write_rgb24
	(
	void        *stream,
   size_t       width,
   size_t       height,
   const void  *pixels,
	int          quality,
	void        *user_data
	)
	{
	struct jpeg_compress_struct  jc;
	struct JpegError             je;
	JSAMPROW                     rows[1];


	jc.err            = jpeg_std_error( &je.mgr );
	je.mgr.error_exit = _jpeg_error_callback;
	je.user_data      = user_data;

	if( jpeg_setjmp( je.user_data ) )
		{
		jpeg_destroy_compress( &jc );
		return JPEG_ERROR_INTERNAL;
		}

	jpeg_create_compress( &jc );
	jpeg_stdio_dest( &jc, stream );

	jc.image_width      = width;
	jc.image_height     = height;
	jc.input_components = 3;
	jc.in_color_space   = JCS_RGB;

	jpeg_set_defaults( &jc );
	jpeg_set_quality( &jc, quality, TRUE );

	jpeg_start_compress( &jc, TRUE );

	while( jc.next_scanline < jc.image_height )
		{
		rows[0] = ( ( unsigned char* )pixels ) + ( jc.next_scanline * width * 3 );
		jpeg_write_scanlines( &jc, rows, 1 );
		}

	jpeg_finish_compress( &jc );
	jpeg_destroy_compress( &jc );

	return JPEG_NO_ERROR;
	}


JpegErrorType jpeg_file_write
   (
   const char     *file_name,
   JpegPixelType   type,
   size_t          width,
   size_t          height,
   const void     *pixels,
	int             quality,
	void           *user_data
   )
   {
   void           *stream;
   JpegErrorType   error = JPEG_NO_ERROR;
  

   assert( NULL != file_name );
   assert( NULL != pixels );
   assert( 0 < width );
   assert( 0 < height );
	assert( 0 <= quality && quality <= 100 );

   if( NULL == ( stream = jpeg_fopen( file_name, "wb", user_data ) ) )
      return JPEG_ERROR_FILE_OPEN;

   switch( type )
      {
		/*
      case JPEG_PIXEL_LUM8:
         error = _jpeg_write_lum8( stream, width, height, pixels, quality, user_data );
         break;
		*/

      case JPEG_PIXEL_RGB24:
         error = _jpeg_write_rgb24( stream, width, height, pixels, quality, user_data );
         break;

      default:
         assert( 0 );;
      }

   jpeg_fclose( stream, user_data );

   if( JPEG_NO_ERROR != error )
      jpeg_remove( file_name, user_data );

   return error;
   }
