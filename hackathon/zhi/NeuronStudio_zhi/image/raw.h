#ifndef __RAW_H__
#define __RAW_H__
/*=============================================================================   
     Module  : raw.h 1.2.6
     Purpose : Define routines to read and write .raw files.
     Author  : Doug Ehlenberger, CNIC Mt. Sinai School of Medicine
==============================================================================*/
/* Version 1.1

   - Updated syntax.
	- Changed 'unsigned' to 'size_t' to allow for 64-bit.
	- Added progress callback functions.
	- User can now set global function pointers. Added 'user_data' as last
	parameter to the functions.

	Version 1.2
	- Added ...read_ex() function to allow a pre-allocated buffer to be used
	instead of the library allocating one on its own.

	Version 1.2.1
	- Now passing the size of the pre-allocated buffer and created a new error
	code if the buffer isnt big enough.

	Version 1.2.2
	- Got rid of default implementations for functions. User must provide!

	Version 1.2.3
	- Added RAW_USE_VARIABLE to avoid "unreferenced variable" warning(s).

	Version 1.2.4
	- Added raw_file_write_ex() which allows a rectangular region of an image
	to be written instead of the whole thing.

	Version 1.2.5
	- Added max_intensity field to the read calls. The library will traverse
	the pixels (if 16-bit grayscale image) and find the maximum intensity value.
	This only runs for images marked as RAW_PIXEL_LUM16 since this helps determine if
	the dynamic range of the image is really within the 12-bit or 16-bit range. In other
	words if for example the 'max_intensity' is returned as 3300, then the image is
	is actually 12-bit since that is within the 12-bit range of 0 to 4095.

	Version 1.2.6
	- Added 'conv_type' to writing routines. Set this to some value or RAW_UNSUPPORTED
	if no conversion is required. Any conversion (e.g. 16-bit to 8-bit) is handled
	internally by this library.
*/


#include <stddef.h>
#include <assert.h>


#ifdef __cplusplus
extern "C"{
#endif


typedef unsigned char   rawuint8;
typedef unsigned short  rawuint16;

#define RAW_COMPILE_TIME_SIZE_CHECK( type, size )\
   extern unsigned char _raw_compile_time_size_check[ ( ( size ) == sizeof( type ) ) ? 1 : -1 ]


RAW_COMPILE_TIME_SIZE_CHECK( rawuint8,  1 );
RAW_COMPILE_TIME_SIZE_CHECK( rawuint16, 2 );


#define RAW_USE_VARIABLE( x ) (x)


typedef enum
   {
   RAW_NO_ERROR,
   RAW_ERROR_FILE_OPEN,
   RAW_ERROR_FILE_READ,
   RAW_ERROR_FILE_WRITE,
   RAW_ERROR_FILE_SEEK,
   RAW_ERROR_OUT_OF_MEMORY,
	RAW_ERROR_NOT_ENOUGH_SPACE,
	RAW_ERROR_PARAMETER
   }
   RawErrorType;


typedef enum
   {
   RAW_UNSUPPORTED,
   RAW_PIXEL_RGB24,
   RAW_PIXEL_RGBA32,
   RAW_PIXEL_BGR24,
   RAW_PIXEL_BGRA32,
   RAW_PIXEL_LUM8,
	RAW_PIXEL_LUM12, /* Packed into 16 bits! */
   RAW_PIXEL_LUM16
   }
   RawPixelType;


enum{ RAW_LITTLE_ENDIAN, RAW_BIG_ENDIAN };


/* IMPORTANT: 'user_data' is passed to both the
	progress callback functions and the global
	functions defined below. */

RawErrorType raw_file_read
   (
   const char     *file_name,
   void           *header, /* NULL if no header. */
   size_t          header_size, /* Should be zero if no header. */
   RawPixelType    type,
   size_t          width,
   size_t          height,
   size_t          length, /* Should be 1 if 2D image. */
   int             endian,
   void          **pixels,
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   );

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
   void           *pixels, /* Pre-allocated buffer */
	size_t          pixels_size, /* Size of the buffer */
	unsigned int   *max_intensity,
	int             ( *cancelled )( void* ),
	void            ( *update )( float, void* ),
	void           *user_data
   );


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
   );


typedef struct _RawRegion
	{
   size_t  x;
   size_t  y;
   size_t  z;
   size_t  width;
   size_t  height;
   size_t  length;
	}
	RawRegion;

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
   );


/* IMPORTANT: Global function pointers that MUST be replaced
	by the user. The last parameter is always a user-defined
	object. */
extern void*  ( *raw_fopen )( const char*, const char*, void* );

extern int ( *raw_fclose )( void*, void* );

extern size_t ( *raw_fread )( void*, size_t, size_t, void*, void* );

extern size_t ( *raw_fwrite )( const void*, size_t, size_t, void*, void* );

extern int ( *raw_remove )( const char*, void* );

extern void* ( *raw_malloc )( size_t, void* );

extern void ( *raw_free )( void*, void* );

#ifdef __cplusplus
}
#endif

#endif/* __RAW_H__ */
