#ifndef __JPEG_H__
#define __JPEG_H__
/*=============================================================================   
     Module  : jpeg.h 1.0.0
     Purpose : Define routines to read and write .jpg files.
     Author  : Doug Ehlenberger, CNIC Mt. Sinai School of Medicine
==============================================================================*/


#include <stddef.h>
#include <assert.h>


#ifdef __cplusplus
extern "C"{
#endif


#define JPEG_USE_VARIABLE( x ) (x)


typedef enum
   {
   JPEG_NO_ERROR,
   JPEG_ERROR_FILE_OPEN,
   JPEG_ERROR_INTERNAL
   }
   JpegErrorType;


typedef enum
   {
   JPEG_UNSUPPORTED,
   JPEG_PIXEL_RGB24,
   JPEG_PIXEL_LUM8
   }
   JpegPixelType;


/* NOTE: JPEG quality ranges from 0 to 100. */


/* IMPORTANT: 'user_data' is passed to both the
	progress callback functions and the global
	functions defined below. */

/*
JpegErrorType jpeg_file_write
   (
   const char     *file_name,
   JpegPixelType   type,
   size_t          width,
   size_t          height,
   const void     *pixels,
	int             quality,
	void           *user_data
   );
*/

/* TEMP? Use above if JPEG support needed. */
#define jpeg_file_write( file_name, type, width, height, pixels, quality, user_data )\
	JPEG_NO_ERROR

/* IMPORTANT: Global function pointers that MUST be replaced
	by the user. The first parameter is always a user-defined
	object. */
extern void* ( *jpeg_fopen )( const char*, const char*, void* );

extern int ( *jpeg_fclose )( void*, void* );

extern int ( *jpeg_remove )( const char*, void* );

extern int ( *jpeg_setjmp )( void* );

extern void ( *jpeg_longjmp )( int, void* );

#ifdef __cplusplus
}
#endif

#endif/* __JPEG_H__ */
