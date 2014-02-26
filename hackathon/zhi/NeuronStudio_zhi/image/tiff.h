#ifndef __TIFF_H__
#define __TIFF_H__
/*=============================================================================   
     Module  : tiff.h ( version 1.9.8 )
     Purpose : Define routines to read and write tiff files.
     Author  : Doug Ehlenberger, CNIC Mt. Sinai School of Medicine
==============================================================================*/
/* 
	Version 1.1
	- Fixed bug when short values are packed into long values.

	Version 1.2
	- Removed the requirement that the compression tag be present.
	The field defaults to "no compression".

	Version 1.3
	- Can now read 3D images. Added 'imageLength' field to the
	TIFFINFO structure.
	- Added TIFF_MAJOR_VERSION, TIFF_MINOR_VERSION, TIFF_REVISION_VERSION,
	and TIFF_VERSION_STRING macros.

	Version 1.4
	- Added typedef's for primitive types.
	- Added TiffPixelType enumeration.
	- Complete change of interface, now object-oriented. TiffFile structure
	now replaces the old TIFFINFO structure.
	- Thread safe.
	- Can now read just the file info., the whole 3D image, or just one
	2D slice of the 3D image.

	Version 1.5
	- The read function now takes callback's so that progress can be measured.

	Version 1.6
	- Eliminated useless/unused typedef's, definitions, functions, etc...
	- User can now set global function pointers. Added 'user_data' as last
	parameter to the functions.

	Version 1.7
	- Added ...read_ex() function to allow a pre-allocated buffer to be used
	instead of the library allocating one on its own.

	Version 1.7.1
	- Now passing the size of the pre-allocated buffer and created a new error
	code if the buffer isnt big enough.

	Version 1.7.2
	- Added support for fseek() and ftell() to surpass the 2GB limit.
	- Got rid of default implementations for functions. User must provide!

	Veraion 1.8.0
	- Replaced FILE* with void* to avoid having to include stdio.h.
	- Added support for reading various other tags including strings.

	Version 1.8.1
	- Fixed small memory leak. Reader and writer structures were not being free'd.

	Version 1.8.2
	- Added TIFF_USE_VARIABLE to avoid "unreferenced variable" warning(s).

	Version 1.9.0
	- Added tiff_file_write_ex() which allows a rectangular region of an image
	to be written instead of the whole thing.

	Version 1.9.2
	- Added max_intensity field to the TiffFile structure. The library will traverse
	the pixels (if 16-bit grayscale image) and find the maximum intensity value.
	This only runs for images marked as TIFF_PIXEL_LUM16 since this helps determine if the
	dynamic range of the image is really within the 12-bit or 16-bit range. In other
	words if for example the 'max_intensity' is returned as 3300, then the image is
	is actually 12-bit since that is within the 12-bit range of 0 to 4095.

	Version 1.9.4
	- Added support for reading Zeiss LSM files which are basically TIFF's with a few
	differences. The 'is_lsm' member of the TiffFile structure must be set to non-zero
	before calling any of the read functions. You can use the tiff_file_construct_lsm()
	function as well which sets this member to non-zero.

	Version 1.9.6
	- Added TiffLsmInfo structure that is filled if reading an LSM file. This structure is
	only allocated if the file is an LSM and is free'd in the call to tiff_file_delete()
	or tiff_file_destruct().

	Version 1.9.8
	- Added 'pixel_info' and 'conv_pixel_info'. Must use 'pixel_info' now to specify the
	type of pixels. Set members of 'conv_pixel_info' before writing any files to set
	the type of the pixels to be written. Any conversion (e.g. 16-bit to 8-bit) is handled
	internally by this library.
*/


#include <stddef.h>
#include <assert.h>


#ifdef __cplusplus
extern "C"{
#endif


/* Define for extensive output. */
/* #define TIFF_VERBOSE */


typedef signed char     tiffint8;
typedef unsigned char   tiffuint8;
typedef signed short    tiffint16;
typedef unsigned short  tiffuint16;
typedef signed int      tiffint32;
typedef unsigned int    tiffuint32;


#define TIFF_COMPILE_TIME_SIZE_CHECK( type, size )\
   extern unsigned char _tiff_compile_time_size_check[ ( ( size ) == sizeof( type ) ) ? 1 : -1 ]


TIFF_COMPILE_TIME_SIZE_CHECK( tiffint8,   1 );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffuint8,  1 );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffint16,  2 );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffuint16, 2 );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffint32,  4 );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffuint32, 4 );
TIFF_COMPILE_TIME_SIZE_CHECK( double, 8 );


/* NOTE!! The TIFF 5.0 spec. long byte size is 4.*/
#define TIFF_TYPE_USHORT_SIZE  2
#define TIFF_TYPE_ULONG_SIZE   4

TIFF_COMPILE_TIME_SIZE_CHECK( tiffint16,  TIFF_TYPE_USHORT_SIZE );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffuint16, TIFF_TYPE_USHORT_SIZE );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffint32,  TIFF_TYPE_ULONG_SIZE );
TIFF_COMPILE_TIME_SIZE_CHECK( tiffuint32, TIFF_TYPE_ULONG_SIZE );


#define TIFF_USE_VARIABLE( x ) (x)


typedef enum
	{
	TIFF_PIXEL_UNSUPPORTED,
	TIFF_PIXEL_LUM8,
	TIFF_PIXEL_LUM12,  /* Assuming 12 bits packed into 16. */
	TIFF_PIXEL_LUM16
	}
	TiffPixelType;


#define TIFF_PIXEL_LUM_NUM_CHANNELS  1


#define TIFF_PIXEL_LUM8_NUM_BITS    8
#define TIFF_PIXEL_LUM12_NUM_BITS  12
#define TIFF_PIXEL_LUM16_NUM_BITS  16


typedef enum
   {
   TIFF_NO_ERROR,
   TIFF_ERROR_FILE_OPEN,
   TIFF_ERROR_FILE_READ,
   TIFF_ERROR_FILE_WRITE,
   TIFF_ERROR_FILE_SEEK,
   TIFF_ERROR_FILE_INVALID,
   TIFF_ERROR_VERSION,
   TIFF_ERROR_COMPRESSED,
   TIFF_ERROR_OUT_OF_MEMORY,
   TIFF_ERROR_UNSUPPORTED,
	TIFF_ERROR_PARAMETER,
	TIFF_ERROR_NOT_ENOUGH_SPACE
   }
   TiffErrorType;


typedef struct _TiffRational
	{
	tiffint32  numerator;
	tiffint32  denominator;
	}
	TiffRational;


typedef struct _TiffLsmInfo
	{
	tiffuint32  magic_number;
	tiffint32   structure_size;
	tiffint32   dimension_x;
	tiffint32   dimension_y;
	tiffint32   dimension_z;
	tiffint32   dimension_channels;
	tiffint32   dimension_time;
	tiffint32   s_data_type;
	tiffint32   thumbnail_x;
	tiffint32   thumbnail_y;
	double      voxel_size_x;
	double      voxel_size_y;
	double      voxel_size_z;
	tiffuint32  scan_type;
	tiffuint32  u_data_type;
	tiffuint32  offset_vector_overlay;
	tiffuint32  offset_input_lut;
	tiffuint32  offset_output_lut;
	tiffuint32  offset_channel_colors;
	double      time_interval;
	tiffuint32  offset_channel_data_types;
	tiffuint32  offset_scan_information;
	tiffuint32  offset_ks_data;
	tiffuint32  offset_time_stamps;
	tiffuint32  offset_event_list;
	tiffuint32  offset_roi;
	tiffuint32  offset_bleach_roi;
	tiffuint32  offset_next_recording;
	}
	TiffLsmInfo;


typedef struct _TiffPixelInfo
	{
	TiffPixelType  pixel_type;
	tiffuint32     channels_per_pixel;
	tiffuint32     bits_per_pixel;
	tiffuint32     luminance_bits;
	tiffuint32     red_bits;
	tiffuint32     green_bits;
	tiffuint32     blue_bits;
	tiffuint32     alpha_bits;
	}
	TiffPixelInfo;


typedef struct _TiffFile
   {
   size_t          width;
   size_t          height;
   size_t          length;
   void           *pixels;
	TiffPixelInfo   pixel_info;
	TiffPixelInfo   conv_pixel_info;
	size_t          max_intensity;
	const char     *document_name;
	const char     *image_description;
	const char     *make;
	const char     *model;
	const char     *page_name;
	const char     *software;
	const char     *date_time;
	const char     *artist;
	const char     *host_computer;
	tiffint8        is_lsm;
	TiffLsmInfo    *lsm_info;
   }
   TiffFile;



/* IMPORTANT: 'user_data' is passed to both the
	progress callback functions and the global
	functions defined below. */


/* Structure set to all bits zero. */
TiffFile* tiff_file_new( void *user_data );

void tiff_file_construct( TiffFile *tiff_file, void *user_data );
void tiff_file_construct_lsm( TiffFile *tiff_file, void *user_data );


/* NOTE: delete() not only free's pixels and strings but
	the passed structure as well. Obviously any allocated
	member should be set to NULL if freeing is not allowed. */
void tiff_file_delete( TiffFile *tiff_file, void *user_data );
void tiff_file_destruct( TiffFile *tiff_file, void *user_data );


/* Reads everything except the actual pixels. */
TiffErrorType tiff_file_read_info
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	void        *user_data
	);

/* Reads the whole 3D image. 2D image just has length 1. */
TiffErrorType tiff_file_read
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void         *user_data
	);

TiffErrorType tiff_file_read_ex
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	void        *pixels, /* Pre-allocated buffer */
	size_t       pixels_size, /* Size of the buffer */
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	);

/* Reads only one 2D slice of the 3D image.
	z is zero-based index, i.e. [0,length-1]. */
TiffErrorType tiff_file_read_at
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	size_t       z,
	void        *user_data
	);


TiffErrorType tiff_file_write
	(
	const TiffFile  *tiff_file,
	const char      *file_name,
	int              ( *cancelled )( void* ),
	void             ( *update )( float, void* ),
	void            *user_data
	);


typedef struct _TiffRegion
	{
   size_t  x;
   size_t  y;
   size_t  z;
   size_t  width;
   size_t  height;
   size_t  length;
	}
	TiffRegion;

TiffErrorType tiff_file_write_ex
	(
	const TiffFile    *tiff_file,
	const TiffRegion  *tiff_region,
	const char        *file_name,
	int                ( *cancelled )( void* ),
	void               ( *update )( float, void* ),
	void              *user_data
	);


/* IMPORTANT: Global function pointers that MUST be replaced
	by the user. The last parameter is usually a user-defined
	object. */
extern void* ( *tiff_fopen )( const char*, const char*, void* );

extern int ( *tiff_fclose )( void*, void* );

extern size_t ( *tiff_fread )( void*, size_t, size_t, void*, void* );

extern size_t ( *tiff_fwrite )( const void*, size_t, size_t, void*, void* );

extern int ( *tiff_remove )( const char*, void* );

extern void* ( *tiff_malloc )( size_t, void* );

extern void* ( *tiff_calloc )( size_t, size_t, void* );

extern void ( *tiff_free )( void*, void* );

/* For tiff_print() the first parameter is the user-defined object. */
extern void ( *tiff_print )( void*, const char*, ... );

/* fseek() and ftell() only allow a 32-bit signed long type so it
	limits traversing files greater than 2GB. To overcome this we
	can define our own prototypes. */

extern int TIFF_SEEK_SET;
extern int TIFF_SEEK_CUR;
extern int TIFF_SEEK_END;

/* Returns the same as fseek(). The second parameter specifies
	the offset as a 'size_t'. The third parameter is a boolean
	indicates whether to travel forwards. */
extern int ( *tiff_fseek )( void*, size_t, int, int, void* );

/* Returns the same as ftell() on error. However the file
	position is returned to a pointer to 'size_t'. */
extern int ( *tiff_ftell )( void*, size_t*, void* );


#ifdef __cplusplus
}
#endif


#endif/* __TIFF_H__ */
