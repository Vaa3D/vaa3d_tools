#include "tiff.h"


void* ( *tiff_fopen )( const char*, const char*, void* ) = NULL;

int ( *tiff_fclose )( void*, void* ) = NULL;

size_t ( *tiff_fread )( void*, size_t, size_t, void*, void* ) = NULL;

size_t ( *tiff_fwrite )( const void*, size_t, size_t, void*, void* ) = NULL;

int ( *tiff_remove )( const char*, void* ) = NULL;

void* ( *tiff_malloc )( size_t, void* ) = NULL;

void* ( *tiff_calloc )( size_t, size_t, void* ) = NULL;

void ( *tiff_free )( void*, void* ) = NULL;

int ( *tiff_fseek )( void*, size_t, int, int, void* ) = NULL;

int ( *tiff_ftell )( void*, size_t*, void* ) = NULL;

void ( *tiff_print )( void*, const char*, ... ) = NULL;


int TIFF_SEEK_SET;
int TIFF_SEEK_CUR;
int TIFF_SEEK_END;


#define TIFF_VERSION_NUMBER  42

enum
	{
	TIFF_TYPE_BYTE     = 1,
	TIFF_TYPE_ASCII    = 2,
	TIFF_TYPE_USHORT   = 3,
	TIFF_TYPE_ULONG    = 4,
	TIFF_TYPE_RATIONAL = 5
	};

const char* tiff_type_to_string( int type )
	{
	static const char* _tiff_type_strings[] =
		{
		NULL,
		"BYTE",
		"ASCII",
		"USHORT",
		"ULONG",
		"RATIONAL"
		};

	return ( 1 <= type && type <= 5 ) ? _tiff_type_strings[ type ] : NULL;
	}


enum
	{
	TIFF_LITTLE_ENDIAN = 0x4949,
	TIFF_BIG_ENDIAN    = 0x4D4D
	};


typedef enum
	{
	TIFF_TAG_IMAGE_WIDTH                = 256,
	TIFF_TAG_IMAGE_HEIGHT               = 257, 
	TIFF_TAG_BITS_PER_SAMPLE            = 258,
	TIFF_TAG_COMPRESSION_TYPE           = 259,
	TIFF_TAG_PHOTOMETRIC_INTERPRETATION = 262,
	TIFF_TAG_DOCUMENT_NAME              = 269,
	TIFF_TAG_IMAGE_DESCRIPTION          = 270,
	TIFF_TAG_MAKE                       = 271,
	TIFF_TAG_MODEL                      = 272,
	TIFF_TAG_STRIP_FILE_OFFSETS         = 273,
	TIFF_TAG_STRIP_BYTE_COUNTS          = 279,
	TIFF_TAG_PLANAR_CONFIGURATION       = 284,
	TIFF_TAG_PAGE_NAME                  = 285,
	TIFF_TAG_SOFTWARE                   = 305,
	TIFF_TAG_DATE_TIME                  = 306,
	TIFF_TAG_ARTIST                     = 315,
	TIFF_TAG_HOST_COMPUTER              = 316,
	TIFF_TAG_CZ_LSM_INFO                = 34412
	}
	TiffTagType;


enum
	{
	TIFF_BIT_IMAGE_WIDTH                = 0x00000001,
	TIFF_BIT_IMAGE_HEIGHT               = 0x00000002,
	TIFF_BIT_BITS_PER_SAMPLE            = 0x00000004,
	TIFF_BIT_COMPRESSION_TYPE           = 0x00000008,
	TIFF_BIT_PHOTOMETRIC_INTERPRETATION = 0x00000010,
	TIFF_BIT_STRIP_FILE_OFFSETS         = 0x00000020,
	TIFF_BIT_STRIP_BYTE_COUNTS          = 0x00000040
	};

#define TIFF_BLACK_0_WHITE_MAX  1

#define TIFF_UNCOMPRESSED  1


#define tiff_new( type, user_data ) ( ( type* )tiff_calloc( 1, sizeof( type ), (user_data) ) )


static int tiff_seek_from_beginning( void *stream, size_t offset, void *user_data )
	{  return ( 0 == tiff_fseek( stream, offset, 1, TIFF_SEEK_SET, user_data ) ) ? 1 : 0;  }


static int tiff_get_machine_endian( void )
   { 
   int k = 1;
   return ( ( 1 == *( ( char* )&k ) ) ? TIFF_LITTLE_ENDIAN : TIFF_BIG_ENDIAN );
   }


#define tiff_swap( type, value1, value2 )\
	{\
	type temp;\
	temp = ( value1 );\
	( value1 ) = ( value2 );\
	( value2 ) = temp;\
	}


static tiffint32 tiff_reverse_endian_i32( const tiffint32 in_value )
   {
	tiffint32   out_value;
	tiffuint8  *byte0;
	tiffuint8  *byte1;
	tiffuint8  *byte2;
	tiffuint8  *byte3;


	out_value = in_value;

	byte0 = ( ( tiffuint8* )&out_value ) + 0;
	byte1 = ( ( tiffuint8* )&out_value ) + 1;
	byte2 = ( ( tiffuint8* )&out_value ) + 2;
	byte3 = ( ( tiffuint8* )&out_value ) + 3;

	tiff_swap( tiffuint8, *byte0, *byte3 );
	tiff_swap( tiffuint8, *byte1, *byte2 );

	return out_value;
   }


static tiffint16 tiff_reverse_endian_i16( const tiffint16 in_value )
	{
	tiffint16   out_value;
	tiffuint8  *byte0;
	tiffuint8  *byte1;


	out_value = in_value;

	byte0 = ( ( tiffuint8* )&out_value ) + 0;
	byte1 = ( ( tiffuint8* )&out_value ) + 1;

	tiff_swap( tiffuint8, *byte0, *byte1 );

	return out_value;
	}


static double tiff_reverse_endian_f64( const double in_value )
   {
	double      out_value;
	tiffuint8  *byte0;
	tiffuint8  *byte1;
	tiffuint8  *byte2;
	tiffuint8  *byte3;
	tiffuint8  *byte4;
	tiffuint8  *byte5;
	tiffuint8  *byte6;
	tiffuint8  *byte7;

	out_value = in_value;

	byte0 = ( ( tiffuint8* )&out_value ) + 0;
	byte1 = ( ( tiffuint8* )&out_value ) + 1;
	byte2 = ( ( tiffuint8* )&out_value ) + 2;
	byte3 = ( ( tiffuint8* )&out_value ) + 3;
	byte4 = ( ( tiffuint8* )&out_value ) + 4;
	byte5 = ( ( tiffuint8* )&out_value ) + 5;
	byte6 = ( ( tiffuint8* )&out_value ) + 6;
	byte7 = ( ( tiffuint8* )&out_value ) + 7;

	tiff_swap( tiffuint8, *byte0, *byte7 );
	tiff_swap( tiffuint8, *byte1, *byte6 );
	tiff_swap( tiffuint8, *byte2, *byte5 );
	tiff_swap( tiffuint8, *byte3, *byte4 );

	return out_value;
   }


static TiffPixelInfo tiff_pixel_info_lum8 = 
	{
	TIFF_PIXEL_LUM8,
	TIFF_PIXEL_LUM_NUM_CHANNELS,
	TIFF_PIXEL_LUM8_NUM_BITS,
	TIFF_PIXEL_LUM8_NUM_BITS,
	0,
	0,
	0,
	0
	};

static TiffPixelInfo tiff_pixel_info_lum12 = 
	{
	TIFF_PIXEL_LUM12,
	TIFF_PIXEL_LUM_NUM_CHANNELS,
	TIFF_PIXEL_LUM16_NUM_BITS, /* IMPORTANT: Assuming 12 bits are packed into 16 bits. */
	TIFF_PIXEL_LUM12_NUM_BITS,
	0,
	0,
	0,
	0
	};

static TiffPixelInfo tiff_pixel_info_lum16 = 
	{
	TIFF_PIXEL_LUM16,
	TIFF_PIXEL_LUM_NUM_CHANNELS,
	TIFF_PIXEL_LUM16_NUM_BITS,
	TIFF_PIXEL_LUM16_NUM_BITS,
	0,
	0,
	0,
	0
	};


static int tiff_pixel_info( TiffPixelInfo *pixel_info, TiffPixelType pixel_type )
	{
	int supported = 1;

	switch( pixel_type )
		{
		case TIFF_PIXEL_LUM8:
			*pixel_info = tiff_pixel_info_lum8;
			break;
		case TIFF_PIXEL_LUM12:
			*pixel_info = tiff_pixel_info_lum12;
			break;
		case TIFF_PIXEL_LUM16:
			*pixel_info = tiff_pixel_info_lum16;
			break;
		default:
			supported = 0;
		}

	return supported;
	}


typedef struct _TiffHeader
   {
   tiffuint16  byte_order;
   tiffuint16  version_number;
   tiffuint32  first_directory_offset;
   }
   TiffHeader;

#define TIFF_HEADER_SIZE  8


typedef struct _TiffField
   {
   tiffuint16  tag;
   tiffuint16  type;
   tiffuint32  count;
   tiffuint32  value_offset;
   }
	TiffField;

#define TIFF_FIELD_SIZE  12


#define tiff_field( field, _tag, _type, _count, _value_offset )\
	{\
	( field )->tag          = ( _tag );\
	( field )->type         = ( _type );\
	( field )->count        = ( _count );\
	( field )->value_offset = ( _value_offset );\
	}


TiffField* tiff_field_new
	(
	tiffuint16   tag,
	tiffuint16   type,
	tiffuint32   count,
	tiffuint32   value_offset,
	void        *user_data
	)
	{
	TiffField *field;

   if( NULL == ( field = tiff_new( TiffField, user_data ) ) )
		return NULL;

	field->tag          = tag;
	field->type         = type;
	field->count        = count;
	field->value_offset = value_offset;

	return field;
	}


void tiff_field_free( TiffField *field, void *user_data )
	{
	assert( NULL != field );
	tiff_free( field, user_data );
	}


typedef struct _TiffDirectory
	{
	tiffuint16              num_fields;
	TiffField              *fields;
	tiffuint32              next_directory_offset;
	tiffuint32              size;
	struct _TiffDirectory  *next;
	}
	TiffDirectory;


static TiffDirectory* tiff_directory_new( tiffuint16 num_fields, void *user_data )
	{
	TiffDirectory *directory;

	if( NULL == ( directory = tiff_new( TiffDirectory, user_data ) ) )
		return NULL;

	if( NULL == ( directory->fields =
		 tiff_malloc( sizeof( TiffField ) * num_fields, user_data ) ) )
		{
		tiff_free( directory, user_data );
		return NULL;
		}

	directory->num_fields = num_fields;

	/* This is the size of the directory as written to file. */

	directory->size = sizeof( directory->num_fields ) +
							TIFF_FIELD_SIZE * directory->num_fields +
							sizeof( directory->next_directory_offset );

	return directory;
	}


static void tiff_directory_free( TiffDirectory *directory, void *user_data )
	{
	assert( NULL != directory );

	tiff_free( directory->fields, user_data );
	tiff_free( directory, user_data );
	}


/* NOTE: The 'field' is copied, not stored by pointer. */
static void tiff_directory_set_field
	(
	TiffDirectory    *directory,
	const TiffField  *field,
	tiffuint16        index
	)
	{
	assert( NULL != directory );
	assert( NULL != directory->fields );
	assert( index < directory->num_fields );

	directory->fields[ index ] = *field;
	}


static TiffField* tiff_directory_get_field( TiffDirectory *directory, tiffuint16 index )
	{
	assert( NULL != directory );
	assert( NULL != directory->fields );
	assert( index < directory->num_fields );

	return directory->fields + index;
	}


typedef struct _TiffStripsInfo
	{
	tiffuint16   type;
	tiffuint32   count;
	tiffuint32   bytes_per_value;
	tiffuint32   total_values_bytes;
	void        *values;
	}
	TiffStripsInfo;


static TiffStripsInfo* tiff_strips_info_new
	(
	tiffuint16   type,
	tiffuint32   count,
	void        *user_data
	)
	{
	TiffStripsInfo *strips_info;

	if( NULL == ( strips_info = tiff_new( TiffStripsInfo, user_data ) ) )
		return NULL;

	strips_info->type  = type;
	strips_info->count = count;

	switch( strips_info->type )
		{
		case TIFF_TYPE_USHORT:
			strips_info->bytes_per_value = TIFF_TYPE_USHORT_SIZE;
			break;

		case TIFF_TYPE_ULONG:
			strips_info->bytes_per_value = TIFF_TYPE_ULONG_SIZE;
			break;

		default:
			assert( 0 );;
		}

	strips_info->total_values_bytes = strips_info->bytes_per_value * strips_info->count;

	if( NULL == ( strips_info->values =
		 tiff_malloc( strips_info->total_values_bytes, user_data ) ) )
		{
		tiff_free( strips_info, user_data );
		return NULL;
		}

	return strips_info;
	}


static void tiff_strips_info_free( TiffStripsInfo *strips_info, void *user_data )
	{
	assert( NULL != strips_info );

	tiff_free( strips_info->values, user_data );
	tiff_free( strips_info, user_data );
	}


static tiffuint16 tiff_strip_info_value_i16( TiffStripsInfo *strips_info, tiffuint32 index )
	{
	assert( TIFF_TYPE_USHORT == strips_info->type );
	return ( ( tiffuint16* )strips_info->values )[ index ];
	}


static tiffuint32 tiff_strip_info_value_i32( TiffStripsInfo *strips_info, tiffuint32 index )
	{
	assert( TIFF_TYPE_ULONG == strips_info->type );
	return ( ( tiffuint32* )strips_info->values )[ index ];
	}


static tiffuint32 tiff_strip_info_value( TiffStripsInfo *strips_info, tiffuint32 index )
	{
	tiffuint32 value = 0;

	assert( NULL != strips_info );
	assert( NULL != strips_info->values );
	assert( index < strips_info->count );

	switch( strips_info->type )
		{
		case TIFF_TYPE_USHORT:
			value = tiff_strip_info_value_i16( strips_info, index );
			break;

		case TIFF_TYPE_ULONG:
			value = tiff_strip_info_value_i32( strips_info, index );
			break;

		default:
			assert( 0 );;
		}

	return value;
	}


typedef struct _TiffImage
   {
   tiffuint32          width;
   tiffuint32          height;
   tiffuint16          compression_type;
   tiffuint32          channels_per_pixel;
   tiffuint16          channel_bits[4];
   tiffuint32          num_strips;
	TiffPixelType       pixel_type;
	TiffStripsInfo     *strips_file_offsets;
	TiffStripsInfo     *strips_byte_counts;
	tiffuint32          total_strips_bytes;
	tiffuint32          flags;
   struct _TiffImage  *next;
   }
   TiffImage;


static TiffImage* tiff_image_new( void *user_data )
	{  return tiff_new( TiffImage, user_data );  }


static void tiff_image_free( TiffImage *image, void *user_data )
	{
	assert( NULL != image );

	if( NULL != image->strips_file_offsets )
		tiff_strips_info_free( image->strips_file_offsets, user_data );

	if( NULL != image->strips_byte_counts )
		tiff_strips_info_free( image->strips_byte_counts, user_data );

	tiff_free( image, user_data );
	}


static TiffErrorType tiff_image_interpret_pixel_type( TiffImage *image )
	{
	TiffErrorType error = TIFF_NO_ERROR;

	switch( image->channels_per_pixel )
		{
		case TIFF_PIXEL_LUM_NUM_CHANNELS:
			switch( image->channel_bits[0] )
				{
				case TIFF_PIXEL_LUM8_NUM_BITS:
					image->pixel_type = TIFF_PIXEL_LUM8;
					break;

				case TIFF_PIXEL_LUM12_NUM_BITS:
					image->pixel_type = TIFF_PIXEL_LUM12;
					break;

				case TIFF_PIXEL_LUM16_NUM_BITS:
					image->pixel_type = TIFF_PIXEL_LUM16;
					break;

				default:
					error = TIFF_ERROR_UNSUPPORTED;
				}
			break;

		default:
			error = TIFF_ERROR_UNSUPPORTED;
		}

	return error;
	}


#ifdef TIFF_VERBOSE

static void tiff_image_print( const TiffImage *image, void *user_data )
	{
	tiffuint32 i;

	tiff_print( user_data, "Image Width        : %u\n", image->width );
	tiff_print( user_data, "Image Height       : %u\n", image->height );

	tiff_print( user_data, "Compression        : " );
	switch( image->compression_type )
		{
		case TIFF_UNCOMPRESSED:
			tiff_print( user_data, "None\n" );
			break;
		default:
			tiff_print( user_data, "!ERROR!\n" );
		}

   tiff_print( user_data, "Channels Per Pixel : %u\n", image->channels_per_pixel );

	for( i = 0; i < image->channels_per_pixel; ++i )
		tiff_print( user_data, "Channel %u Bits     : %u\n", i, ( tiffuint32 )image->channel_bits[i] );

	tiff_print( user_data, "Pixel Type         : " );

	switch( image->pixel_type )
		{
		case TIFF_PIXEL_LUM8:
			tiff_print( user_data, "8-bit greyscale\n" );
			break;
		case TIFF_PIXEL_LUM12:
			tiff_print( user_data, "12-bit greyscale\n" );
			break;
		case TIFF_PIXEL_LUM16:
			tiff_print( user_data, "16-bit greyscale\n" );
			break;
		default:
			tiff_print( user_data, "Unsupported\n" );
		}

	tiff_print( user_data, "Total Strips       : %u\n", image->num_strips );

	if( image->flags & TIFF_BIT_IMAGE_WIDTH )
		tiff_print( user_data, "%u : Image Width\n", TIFF_TAG_IMAGE_WIDTH );
	if( image->flags & TIFF_BIT_IMAGE_HEIGHT )
		tiff_print( user_data, "%u : Image Height\n", TIFF_TAG_IMAGE_HEIGHT );
	if( image->flags & TIFF_BIT_BITS_PER_SAMPLE )
		tiff_print( user_data, "%u : Bits Per Sample\n", TIFF_TAG_BITS_PER_SAMPLE );
	if( image->flags & TIFF_BIT_COMPRESSION_TYPE )
		tiff_print( user_data, "%u : Compression Type\n", TIFF_TAG_COMPRESSION_TYPE );
	if( image->flags & TIFF_BIT_PHOTOMETRIC_INTERPRETATION )
		tiff_print( user_data, "%u : Photometric Interpretation\n", TIFF_TAG_PHOTOMETRIC_INTERPRETATION );
	if( image->flags & TIFF_BIT_STRIP_FILE_OFFSETS )
		tiff_print( user_data, "%u : Strip File Offsets\n", TIFF_TAG_STRIP_FILE_OFFSETS );	
	if( image->flags & TIFF_BIT_STRIP_BYTE_COUNTS )
		tiff_print( user_data, "%u : Strip Byte Counts\n", TIFF_TAG_STRIP_BYTE_COUNTS );	
	}

#endif/* TIFF_VERBOSE */


tiffuint32 tiff_image_strips_total_bytes_i16( TiffImage *image )
	{
	tiffuint32   total_bytes = 0;
	tiffuint16  *value;
	tiffuint32   i;


	assert( NULL != image->strips_byte_counts );
	assert( NULL != image->strips_byte_counts->values );
	assert( image->strips_byte_counts->total_values_bytes ==
					 image->strips_byte_counts->count * TIFF_TYPE_USHORT_SIZE );

	value = ( tiffuint16* )image->strips_byte_counts->values;

	for( i = 0; i < image->strips_byte_counts->count; ++i )
		{
		total_bytes += *value;
		++value;
		}

	return total_bytes;
	}


tiffuint32 tiff_image_strips_total_bytes_i32( TiffImage *image )
	{
	tiffuint32   total_bytes = 0;
	tiffuint32  *value;
	tiffuint32   i;


	assert( NULL != image->strips_byte_counts );
	assert( NULL != image->strips_byte_counts->values );
	assert( image->strips_byte_counts->total_values_bytes ==
					 image->strips_byte_counts->count * TIFF_TYPE_ULONG_SIZE );

	value = ( tiffuint32* )image->strips_byte_counts->values;

	for( i = 0; i < image->strips_byte_counts->count; ++i )
		{
		total_bytes += *value;
		++value;
		}

	return total_bytes;
	}


tiffuint32 tiff_image_strips_total_bytes( TiffImage *image )
	{
	tiffuint32 total_bytes = 0;

	assert( NULL != image->strips_byte_counts );

	switch( image->strips_byte_counts->type )
		{
		case TIFF_TYPE_USHORT:
			total_bytes = tiff_image_strips_total_bytes_i16( image );
			break;

		case TIFF_TYPE_ULONG:
			total_bytes = tiff_image_strips_total_bytes_i32( image );
			break;

		default:
			assert( 0 );;
		}

	return total_bytes;
	}


typedef struct _TiffReader
   {
   void           *stream;
   TiffHeader      header;
	TiffDirectory  *directories;
	TiffDirectory  *directories_tail;
	TiffImage      *images;
	TiffImage		*images_tail;
   void           *pixels;
	void           *end_pixels;
	size_t          pixels_size;
	size_t          max_intensity;
	int             allocate_pixels;
   int             machine_endian;
	char           *document_name;
	char           *image_description;
	char           *make;
	char           *model;
	char           *page_name;
	char           *software;
	char           *date_time;
	char           *artist;
	char           *host_computer;
	tiffint8        is_lsm;
	TiffLsmInfo    *lsm_info;
   }
   TiffReader;


static TiffReader* tiff_reader_new
	(
	const char     *file_name,
	TiffErrorType  *error,
	tiffint8        is_lsm,
	void           *user_data
	)
   {
	TiffReader *reader;

   if( NULL == ( reader = tiff_new( TiffReader, user_data ) ) )
		{
		*error = TIFF_ERROR_OUT_OF_MEMORY;
		return NULL;
		}

   if( NULL == ( reader->stream = tiff_fopen( file_name, "rb", user_data ) ) )
		{
		tiff_free( reader, user_data );
		*error = TIFF_ERROR_FILE_OPEN;
		return NULL;
		}

	reader->max_intensity   = 0;
	reader->allocate_pixels = 1;
   reader->machine_endian  = tiff_get_machine_endian();

	reader->document_name     = NULL;
	reader->image_description = NULL;
	reader->make              = NULL;
	reader->model             = NULL;
	reader->page_name         = NULL;
	reader->software          = NULL;
	reader->date_time         = NULL;
	reader->artist            = NULL;
	reader->host_computer     = NULL;

	reader->is_lsm   = is_lsm;
	reader->lsm_info = NULL;

   return reader;
   }


static TiffErrorType tiff_reader_delete
	(
	TiffReader     *reader,
	TiffErrorType   error,
	void           *user_data
	)
	{
	TiffDirectory  *directory;
	TiffDirectory  *temp_directory;
	TiffImage      *image;
	TiffImage      *temp_image;


   if( NULL != reader->stream )
		tiff_fclose( reader->stream, user_data );

	for( directory = reader->directories; NULL != directory; )
		{
		temp_directory = directory;
		directory      = directory->next;

		tiff_directory_free( temp_directory, user_data );
		}

	for( image = reader->images; NULL != image; )
		{
		temp_image = image;
		image      = image->next;

		tiff_image_free( temp_image, user_data );
		}

	if( reader->allocate_pixels )
		tiff_free( reader->pixels, user_data );

	tiff_free( reader->document_name, user_data );
	tiff_free( reader->image_description, user_data );
	tiff_free( reader->make, user_data );
	tiff_free( reader->model, user_data );
	tiff_free( reader->page_name, user_data );
	tiff_free( reader->software, user_data );
	tiff_free( reader->date_time, user_data );
	tiff_free( reader->artist, user_data );
	tiff_free( reader->host_computer, user_data );

	tiff_free( reader->lsm_info, user_data );

	tiff_free( reader, user_data );

   return error;
	}


static TiffErrorType tiff_reader_allocate_pixels
	(
	TiffReader  *reader,
	tiffuint32   bytes,
	void        *user_data
	)
	{
	assert( 0 < bytes );

	if( ! reader->allocate_pixels && reader->pixels_size < bytes )
		return TIFF_ERROR_NOT_ENOUGH_SPACE;

	if( reader->allocate_pixels )
		{
		assert( NULL == reader->pixels );

		if( NULL == ( reader->pixels = tiff_malloc( bytes, user_data ) ) )
			return TIFF_ERROR_OUT_OF_MEMORY;
		}

	reader->end_pixels = ( ( tiffuint8* )reader->pixels ) + bytes;

	return TIFF_NO_ERROR;
	}


static void* tiff_reader_release_pixels( TiffReader *reader )
	{
	/* Make the readers pixel pointer NULL so it doesnt free it. */
	void *pixels = reader->pixels;
	reader->pixels = NULL;

	return pixels;
	}


static char* _tiff_reader_release_string( TiffReader *reader, char **src_string )
	{
	char *dest_string = *src_string;
	*src_string = NULL;

	TIFF_USE_VARIABLE( reader );

	return dest_string;
	}

#define tiff_reader_release_string( reader, which )\
	_tiff_reader_release_string( (reader), &(reader)->which )


#define tiff_reader_swap_bytes_i16( reader, i16 )\
   if( ( reader )->machine_endian != ( reader )->header.byte_order )\
		{\
		assert( TIFF_TYPE_USHORT_SIZE == sizeof( ( i16 ) ) );\
		( i16 ) = tiff_reverse_endian_i16( ( i16 ) );\
		}


#define tiff_reader_swap_bytes_i32( reader, i32 )\
   if( ( reader )->machine_endian != ( reader )->header.byte_order )\
		{\
		assert( TIFF_TYPE_ULONG_SIZE == sizeof( ( i32 ) ) );\
      ( i32 ) = tiff_reverse_endian_i32( ( i32 ) );\
		}


#define tiff_reader_swap_bytes_f64( reader, f64 )\
   if( ( reader )->machine_endian != ( reader )->header.byte_order )\
		{\
		assert( sizeof( double ) == sizeof( ( f64 ) ) );\
      ( f64 ) = tiff_reverse_endian_f64( ( f64 ) );\
		}


/* NOTE: The TIFF 5.0 spec says that when a 16-bit value is packed into
	a 32-bit value that it is always stored in the low-order word. */

static tiffint16 tiff_unpack_i16_from_i32
	( 
	TiffReader        *reader,
	const tiffuint32   i32,
	int                allow_swap
	)
	{
	tiffuint16 i16 = *( ( ( tiffuint16* )&i32 ) + 0 );

	if( allow_swap )
		tiff_reader_swap_bytes_i16( reader, i16 );

	return i16;
	}


static void tiff_unpack_i16_x2_from_i32
	( 
	TiffReader        *reader,
	const tiffuint32   i32,
	int                allow_swap,
	tiffuint16        *i16_low,
	tiffuint16			*i16_high
	)
	{
   *i16_low  = *( ( ( tiffuint16* )&i32 ) + 0 );
	*i16_high = *( ( ( tiffuint16* )&i32 ) + 1 );

	if( allow_swap )
		{
		tiff_reader_swap_bytes_i16( reader, *i16_low );
		tiff_reader_swap_bytes_i16( reader, *i16_high );
		}
	}


static int _tiff_reader_read_i8
	(
	TiffReader  *reader,
	tiffint8    *i8,
	void        *user_data
	)
	{  return 1 == tiff_fread( i8, sizeof( tiffint8 ), 1, reader->stream, user_data );  }


static int _tiff_reader_read_i16
	(
	TiffReader  *reader,
	tiffint16   *i16,
	tiffuint32   bytes,
	int          allow_swap,
	void        *user_data
	)
	{
	assert( TIFF_TYPE_USHORT_SIZE == bytes );

	if( 1 != tiff_fread( i16, sizeof( tiffint16 ), 1, reader->stream, user_data ) )
		return 0;

	if( allow_swap )
		tiff_reader_swap_bytes_i16( reader, *i16 );

   return 1;
	}


static int _tiff_reader_read_i32
	(
	TiffReader  *reader,
	tiffint32   *i32,
	tiffuint32   bytes,
	int          allow_swap,
	void        *user_data
	)
	{
	assert( TIFF_TYPE_ULONG_SIZE == bytes );

	if( 1 != tiff_fread( i32, sizeof( tiffint32 ), 1, reader->stream, user_data ) )
		return 0;

	if( allow_swap )
		tiff_reader_swap_bytes_i32( reader, *i32 );

   return 1;
	}


static int _tiff_reader_read_f64
	(
	TiffReader  *reader,
	double      *f64,
	tiffuint32   bytes,
	int          allow_swap,
	void        *user_data
	)
	{
	assert( sizeof( double ) == bytes );

	if( 1 != tiff_fread( f64, sizeof( double ), 1, reader->stream, user_data ) )
		return 0;

	if( allow_swap )
		tiff_reader_swap_bytes_f64( reader, *f64 );

   return 1;
	}


#define tiff_reader_read_i16( reader, i16, allow_swap, user_data )\
	_tiff_reader_read_i16(\
		(reader),\
		( tiffint16* )(i16),\
		sizeof( *(i16) ),\
		(allow_swap),\
		(user_data)\
		)


#define tiff_reader_read_i32( reader, i32, allow_swap, user_data )\
	_tiff_reader_read_i32(\
		(reader),\
		( tiffint32* )(i32),\
		sizeof( *(i32) ),\
		(allow_swap),\
		(user_data)\
		)


#define tiff_reader_read_f64( reader, f64, allow_swap, user_data )\
	_tiff_reader_read_f64(\
		(reader),\
		( double* )(f64),\
		sizeof( *(f64) ),\
		(allow_swap),\
		(user_data)\
		)


static TiffErrorType tiff_reader_read_header( TiffReader *reader, void *user_data )
	{
	if( ! tiff_reader_read_i16( reader, &reader->header.byte_order, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

   if( reader->header.byte_order != TIFF_LITTLE_ENDIAN &&
		 reader->header.byte_order != TIFF_BIG_ENDIAN )
      return TIFF_ERROR_FILE_INVALID;

   if( ! tiff_reader_read_i16( reader, &reader->header.version_number, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

   if( TIFF_VERSION_NUMBER != reader->header.version_number )
      return TIFF_ERROR_FILE_INVALID;

	if( ! tiff_reader_read_i32( reader, &reader->header.first_directory_offset, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;


	#ifdef TIFF_VERBOSE

	tiff_print(
		user_data,
		"TIFF: byte_order = %s\n",
		TIFF_LITTLE_ENDIAN == reader->header.byte_order ? "TIFF_LITTLE_ENDIAN" : "TIFF_BIG_ENDIAN"
		);

	tiff_print(
		user_data,
		"TIFF: version_number = %u\n",
		( tiffuint32 )reader->header.version_number
		);

	tiff_print(
		user_data,
		"TIFF: first_directory_offset = %u\n",
		( tiffuint32 )reader->header.first_directory_offset
		);

	#endif/* TIFF_VERBOSE */

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_field
	(
	TiffReader  *reader,
	TiffField   *field,
	void        *user_data
	)
	{
	if( ! tiff_reader_read_i16( reader, &field->tag, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

	if( ! tiff_reader_read_i16( reader, &field->type, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

	if( ! tiff_reader_read_i32( reader, &field->count, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

	/* IMPORTANT: Dont swap the bytes on 'value_offset' yet! */

	if( ! tiff_reader_read_i32( reader, &field->value_offset, 0, user_data ) )
		return TIFF_ERROR_FILE_READ;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_directories( TiffReader *reader, void *user_data )
	{
	TiffDirectory  *directory;
	TiffField       field;
	tiffuint16      num_fields;
	tiffuint16      i;
	TiffErrorType   error;


   if( ! tiff_seek_from_beginning(
				reader->stream,
				reader->header.first_directory_offset,
				user_data
				) )
		return TIFF_ERROR_FILE_SEEK;

   for( ; ; )
      {
      if( ! tiff_reader_read_i16( reader, &num_fields, 1, user_data ) )
			return TIFF_ERROR_FILE_READ;

		if( NULL == ( directory = tiff_directory_new( num_fields, user_data ) ) )
			return TIFF_ERROR_OUT_OF_MEMORY;

		if( NULL == reader->directories )
			reader->directories = directory;
		else
			reader->directories_tail->next = directory;

	   reader->directories_tail = directory;

		for( i = 0; i < directory->num_fields; ++i )
			{
			if( TIFF_NO_ERROR != ( error = tiff_reader_read_field( reader, &field, user_data ) ) )
				return error;

			tiff_directory_set_field( directory, &field, i );
			}

      if( ! tiff_reader_read_i32( reader, &directory->next_directory_offset, 1, user_data ) )
			return TIFF_ERROR_FILE_READ;

      /* If the offset to the next directory is zero then no more images. */

      if( 0 == directory->next_directory_offset )
         break;

      if( ! tiff_seek_from_beginning(
					reader->stream,
					directory->next_directory_offset,
					user_data
					) )
		   return TIFF_ERROR_FILE_SEEK;
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_string
	(
	TiffReader   *reader,
	TiffField    *field,
	char        **string,
	void         *user_data
	)
	{
	char        *s, *p;
	tiffuint32   i;
	size_t       save_fpos;
	tiffuint32   offset;
	tiffint8     c;


	/* NOTE: Only read the string once. */
	if( NULL != *string )
		return TIFF_NO_ERROR;

	assert( 0 < field->count );

	if( NULL == ( *string = tiff_malloc( field->count, user_data ) ) )
		return TIFF_ERROR_OUT_OF_MEMORY;

	s = *string;

	if( field->count <= TIFF_TYPE_ULONG_SIZE )
		{
		p = ( tiffint8* )( &field->value_offset );

		for( i = 0; i < field->count; ++i )
			s[i] = ( char )( *p++ );
		}
	else
		{
		offset = field->value_offset;
		tiff_reader_swap_bytes_i32( reader, offset );

		if( -1 == tiff_ftell( reader->stream, &save_fpos, user_data ) ||
			 ! tiff_seek_from_beginning( reader->stream, offset, user_data ) )
			return TIFF_ERROR_FILE_SEEK;

		for( i = 0; i < field->count; ++i )
			{
			if( ! _tiff_reader_read_i8( reader, &c, user_data ) )
				return TIFF_ERROR_FILE_READ;

			s[i] = ( char )c;
			}

		if( ! tiff_seek_from_beginning( reader->stream, save_fpos, user_data ) )
			return TIFF_ERROR_FILE_SEEK;
		}

	/* Just make sure the string is terminated. */
	s[ field->count - 1 ] = 0;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_rational
	(
	TiffReader    *reader,
	TiffField     *field,
	TiffRational  *rational,
	void          *user_data
	)
	{
	size_t      save_fpos;
	tiffuint32  offset;


	/* NOTE: Only read the rational once. */
	if( 0 != rational->denominator )
		return TIFF_NO_ERROR;

	assert( 1 == field->count );

	offset = field->value_offset;
	tiff_reader_swap_bytes_i32( reader, offset );

	if( -1 == tiff_ftell( reader->stream, &save_fpos, user_data ) ||
		 ! tiff_seek_from_beginning( reader->stream, offset, user_data ) )
		return TIFF_ERROR_FILE_SEEK;

	if( ! tiff_reader_read_i32( reader, &rational->numerator, 1, user_data ) ||
		 ! tiff_reader_read_i32( reader, &rational->denominator, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

	if( 0 == rational->denominator )
		return TIFF_ERROR_PARAMETER;

	if( ! tiff_seek_from_beginning( reader->stream, save_fpos, user_data ) )
		return TIFF_ERROR_FILE_SEEK;

	return TIFF_NO_ERROR;
	}


typedef TiffErrorType ( *TiffReadTagFunc )( TiffReader*, TiffField*, TiffImage*, void* );


static TiffErrorType tiff_read_tag_image_width
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( user_data );

	if( TIFF_TYPE_USHORT == field->type )
		image->width = tiff_unpack_i16_from_i32( reader, field->value_offset, 1 );
	else if( TIFF_TYPE_ULONG == field->type )
		{
		image->width = field->value_offset;
		tiff_reader_swap_bytes_i32( reader, image->width );
		}
	else
		return TIFF_ERROR_FILE_INVALID;

   if( 0 == image->width )
       return TIFF_ERROR_FILE_INVALID;

   image->flags |= TIFF_BIT_IMAGE_WIDTH;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_image_height
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( user_data );

	if( TIFF_TYPE_USHORT == field->type )
		image->height = tiff_unpack_i16_from_i32( reader, field->value_offset, 1 );
	else if( TIFF_TYPE_ULONG == field->type )
		{
		image->height = field->value_offset;
		tiff_reader_swap_bytes_i32( reader, image->height );
		}
	else
		return TIFF_ERROR_FILE_INVALID;

   if( 0 == image->height )
       return TIFF_ERROR_FILE_INVALID;

   image->flags |= TIFF_BIT_IMAGE_HEIGHT;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_bits_per_sample
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( user_data );

	if( TIFF_TYPE_USHORT != field->type )
		return TIFF_ERROR_FILE_INVALID;
   
	image->channels_per_pixel = field->count;
   
	if( 0 == image->channels_per_pixel )
		return TIFF_ERROR_FILE_INVALID;
   
   /* NOTE: Currently can only read grayscale images, i.e. 1 channel,
		therefore we know that 'value_offset' contains the actual
		value, not the offset to the value. */

   if( 1 != image->channels_per_pixel )
		return TIFF_ERROR_UNSUPPORTED;

	image->channel_bits[0] = tiff_unpack_i16_from_i32( reader, field->value_offset, 1 );
             
   if( 0 == image->channel_bits[0] )
		return TIFF_ERROR_FILE_INVALID;
   
   image->flags |= TIFF_BIT_BITS_PER_SAMPLE;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_compression_type
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( user_data );

	if( TIFF_TYPE_USHORT != field->type )
		return TIFF_ERROR_FILE_INVALID;

	image->compression_type = tiff_unpack_i16_from_i32( reader, field->value_offset, 1 );

	/* NOTE: Currently cant handle any compression. */

   if( TIFF_UNCOMPRESSED != image->compression_type )
      return TIFF_ERROR_COMPRESSED;
   
	image->flags |= TIFF_BIT_COMPRESSION_TYPE;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_photometric_interpretation
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( reader );
	TIFF_USE_VARIABLE( field );
	TIFF_USE_VARIABLE( image );
	TIFF_USE_VARIABLE( user_data );

   /* NOTE: Currently just ignoring this field. */

	image->flags |= TIFF_BIT_PHOTOMETRIC_INTERPRETATION;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_document_name
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->document_name, user_data );
	}


static TiffErrorType tiff_read_tag_image_description
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->image_description, user_data );
	}


static TiffErrorType tiff_read_tag_make
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->make, user_data );
	}


static TiffErrorType tiff_read_tag_model
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->model, user_data );
	}


static TiffErrorType tiff_read_strips_info_by_value
	(
	TiffReader      *reader,
	tiffuint32       value,
	TiffStripsInfo  *strips_info
	)
	{
   if( TIFF_TYPE_ULONG == strips_info->type )
      {
		assert( 1 == strips_info->count );
		assert( TIFF_TYPE_ULONG_SIZE == strips_info->total_values_bytes );

		tiff_reader_swap_bytes_i32( reader, value );
		( ( tiffuint32* )strips_info->values )[0] = value;
      }
   else
      {
      assert( 1 == strips_info->count || 2 == strips_info->count );

		if( 1 == strips_info->count )
			{
         assert( TIFF_TYPE_USHORT_SIZE == strips_info->total_values_bytes );
			( ( tiffuint16* )strips_info->values )[0] = tiff_unpack_i16_from_i32( reader, value, 1 );
			}
		else
			{
         assert( TIFF_TYPE_USHORT_SIZE * 2 == strips_info->total_values_bytes );

			tiff_unpack_i16_x2_from_i32(
				reader,
				value,
				1,
				( ( tiffuint16* )strips_info->values ) + 0,
				( ( tiffuint16* )strips_info->values ) + 1
				);
			}
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_strips_info_by_offset_i16
	(
	TiffReader      *reader,
	TiffStripsInfo  *strips_info,
	void            *user_data
	)
	{
	tiffuint16  *value;
	tiffuint32   i;


	assert( TIFF_TYPE_USHORT_SIZE * strips_info->count == strips_info->total_values_bytes );

   value = ( tiffuint16* )strips_info->values;

	for( i = 0; i < strips_info->count; ++i )
		{
		if( ! tiff_reader_read_i16( reader, value, 1, user_data ) )
			return TIFF_ERROR_FILE_READ;

		++value;
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_strips_info_by_offset_i32
	(
	TiffReader      *reader,
	TiffStripsInfo  *strips_info,
	void            *user_data
	)
	{
	tiffuint32  *value;
	tiffuint32   i;


	assert( TIFF_TYPE_ULONG_SIZE * strips_info->count == strips_info->total_values_bytes );

   value = ( tiffuint32* )strips_info->values;

	for( i = 0; i < strips_info->count; ++i )
		{
		if( ! tiff_reader_read_i32( reader, value, 1, user_data ) )
			return TIFF_ERROR_FILE_READ;

		++value;
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_strips_info
	(
	TiffReader      *reader,
	TiffField       *field,
	TiffStripsInfo  *strips_info,
	void            *user_data
	)
	{
	TiffErrorType error = TIFF_NO_ERROR;

   /* This is tricky! If the number of bytes required to hold the values
      can fit in the fields 'value_offset' variable, then 'value_offset' contains
      the actual value(s), not the file offset to the value(s). See TIFF 5.0 spec. */

	if( strips_info->total_values_bytes <= TIFF_TYPE_ULONG_SIZE )
		error = tiff_read_strips_info_by_value( reader, field->value_offset, strips_info );
	else
		{
		tiffuint32  offset;
		size_t      save_fpos;


		offset = field->value_offset;
		tiff_reader_swap_bytes_i32( reader, offset );

		/* Seek to the location where the actual values are located in the file,
			but remember to save the current file position. */

		if( -1 == tiff_ftell( reader->stream, &save_fpos, user_data ) ||
		    ! tiff_seek_from_beginning( reader->stream, offset, user_data ) )
			return TIFF_ERROR_FILE_SEEK;

		switch( strips_info->type )
			{
			case TIFF_TYPE_USHORT:
				error = tiff_read_strips_info_by_offset_i16( reader, strips_info, user_data );
				break;

			case TIFF_TYPE_ULONG:
				error = tiff_read_strips_info_by_offset_i32( reader, strips_info, user_data );
				break;

			default:
				assert( 0 );;
			}

		if( TIFF_NO_ERROR != error )
			return error;

		if( ! tiff_seek_from_beginning( reader->stream, save_fpos, user_data ) )
			return TIFF_ERROR_FILE_SEEK;
		}

	return error;
	}


static TiffErrorType tiff_read_strip_file_offsets
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	if( NULL == ( image->strips_file_offsets =
		 tiff_strips_info_new( field->type, field->count, user_data ) ) )
		return TIFF_ERROR_OUT_OF_MEMORY;

	return tiff_read_strips_info( reader, field, image->strips_file_offsets, user_data );
	}


static TiffErrorType tiff_read_strip_byte_counts
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	if( NULL == ( image->strips_byte_counts =
		 tiff_strips_info_new( field->type, field->count, user_data ) ) )
		return TIFF_ERROR_OUT_OF_MEMORY;

	return tiff_read_strips_info( reader, field, image->strips_byte_counts, user_data );
	}


static TiffErrorType tiff_read_tag_strip_file_offsets
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TiffErrorType error;

	if( TIFF_TYPE_USHORT != field->type && TIFF_TYPE_ULONG != field->type )
		return TIFF_ERROR_FILE_INVALID;

	if( 0 == field->count )
		return TIFF_ERROR_FILE_INVALID;

	if( TIFF_NO_ERROR != ( error =
		 tiff_read_strip_file_offsets( reader, field, image, user_data ) ) )
		return error;

	image->flags |= TIFF_BIT_STRIP_FILE_OFFSETS;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_strip_byte_counts
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TiffErrorType error;

	if( TIFF_TYPE_USHORT != field->type && TIFF_TYPE_ULONG != field->type )
		return TIFF_ERROR_FILE_INVALID;

	if( 0 == field->count )
		return TIFF_ERROR_FILE_INVALID;

	if( TIFF_NO_ERROR != ( error =
		 tiff_read_strip_byte_counts( reader, field, image, user_data ) ) )
		return error;

	image->flags |= TIFF_BIT_STRIP_BYTE_COUNTS;
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_planar_configuration
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	tiffuint16 planar_configuration;

	TIFF_USE_VARIABLE( image );
	TIFF_USE_VARIABLE( user_data );

	if( TIFF_TYPE_USHORT != field->type )
		return TIFF_ERROR_FILE_INVALID;

	if( 0 == field->count )
		return TIFF_ERROR_FILE_INVALID;

	planar_configuration = tiff_unpack_i16_from_i32( reader, field->value_offset, 1 );

	/* For LSM files, the planar configuration seems to always be set to 2 which means
		that for a multi-channel image, the pixel channels are not saved interleaved.

		For example and RGB image saved with interleaved channels is:
		RGBRGBRGBRGB...

		With the planar configuration set to 2 it would be stored as:
		RRRRR...
		GGGGG...
		BBBBB...

		Note that for a single channel image this value is not meaningful and should
		either be omitted or set to 1. Guess this is a "bug" in some LSM file writers.
	*/
	if( ! reader->is_lsm )
		if( 1 != planar_configuration )
			return TIFF_ERROR_UNSUPPORTED;
	
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_page_name
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->page_name, user_data );
	}


static TiffErrorType tiff_read_tag_software
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->software, user_data );
	}


static TiffErrorType tiff_read_tag_date_time
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->date_time, user_data );
	}


static TiffErrorType tiff_read_tag_artist
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->artist, user_data );
	}


static TiffErrorType tiff_read_tag_host_computer
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	TIFF_USE_VARIABLE( image );

	if( TIFF_TYPE_ASCII != field->type )
		return TIFF_ERROR_FILE_INVALID;

	return tiff_reader_read_string( reader, field, &reader->host_computer, user_data );
	}


static TiffErrorType tiff_read_lsm_info
	(
	TiffReader   *reader,
	TiffLsmInfo  *lsm_info,
	void         *user_data
	)
	{
	if( ! tiff_reader_read_i32( reader, &lsm_info->magic_number, 1, user_data ) )
		return TIFF_ERROR_FILE_READ;

	/* See LSM spec. 5.0 or higher for this value. */
	if( lsm_info->magic_number != 0x400494C )
		return TIFF_ERROR_VERSION;

	if( ! tiff_reader_read_i32( reader, &lsm_info->structure_size, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->dimension_x, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->dimension_y, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->dimension_z, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->dimension_channels, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->dimension_time, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->s_data_type, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->thumbnail_x, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_i32( reader, &lsm_info->thumbnail_y, 1, user_data ) )return TIFF_ERROR_FILE_READ;

	if( ! tiff_reader_read_f64( reader, &lsm_info->voxel_size_x, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_f64( reader, &lsm_info->voxel_size_y, 1, user_data ) )return TIFF_ERROR_FILE_READ;
	if( ! tiff_reader_read_f64( reader, &lsm_info->voxel_size_z, 1, user_data ) )return TIFF_ERROR_FILE_READ;

	/* NOTE: Multiply by a million to convert from meters to micro-meters. */
	lsm_info->voxel_size_x *= 1000000.0;
	lsm_info->voxel_size_y *= 1000000.0;
	lsm_info->voxel_size_z *= 1000000.0;

	#ifdef TIFF_VERBOSE
	tiff_print(
		user_data,
		"Voxel Size: %f %f %f\n",
		lsm_info->voxel_size_x,
		lsm_info->voxel_size_y,
		lsm_info->voxel_size_z
		);
	#endif

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_read_tag_cz_lsm_info
	(
	TiffReader  *reader,
	TiffField   *field,
	TiffImage   *image,
	void        *user_data
	)
	{
	size_t         save_fpos;
	tiffuint32     offset;
	TiffErrorType  error;


	if( ! reader->is_lsm )
		return TIFF_NO_ERROR;

	TIFF_USE_VARIABLE( image );

	offset = field->value_offset;
	tiff_reader_swap_bytes_i32( reader, offset );

	if( -1 == tiff_ftell( reader->stream, &save_fpos, user_data ) ||
		 ! tiff_seek_from_beginning( reader->stream, offset, user_data ) )
		return TIFF_ERROR_FILE_SEEK;

	if( NULL == ( reader->lsm_info = tiff_malloc( sizeof( TiffLsmInfo ), user_data ) ) )
		return TIFF_ERROR_OUT_OF_MEMORY;

	if( TIFF_NO_ERROR != ( error = tiff_read_lsm_info( reader, reader->lsm_info, user_data ) ) )
		return error;

	if( ! tiff_seek_from_beginning( reader->stream, save_fpos, user_data ) )
		return TIFF_ERROR_FILE_SEEK;

	return TIFF_NO_ERROR;
	}


typedef struct _TiffTagFuncSlot
	{
	tiffuint16                tag;
	void                      ( *tag_func )( void );
	struct _TiffTagFuncSlot  *next;
	}
	TiffTagFuncSlot;


static TiffTagFuncSlot tiff_read_tag_image_width_slot =
	{ TIFF_TAG_IMAGE_WIDTH, ( void ( * )( void ) )tiff_read_tag_image_width, NULL };

static TiffTagFuncSlot tiff_read_tag_image_height_slot =
	{ TIFF_TAG_IMAGE_HEIGHT, ( void ( * )( void ) )tiff_read_tag_image_height, NULL };

static TiffTagFuncSlot tiff_read_tag_bits_per_sample_slot =
	{ TIFF_TAG_BITS_PER_SAMPLE, ( void ( * )( void ) )tiff_read_tag_bits_per_sample, NULL };

static TiffTagFuncSlot tiff_read_tag_compression_type_slot =
	{ TIFF_TAG_COMPRESSION_TYPE, ( void ( * )( void ) )tiff_read_tag_compression_type, NULL };

static TiffTagFuncSlot tiff_read_tag_photometric_interpretation_slot =
	{ TIFF_TAG_PHOTOMETRIC_INTERPRETATION, ( void ( * )( void ) )tiff_read_tag_photometric_interpretation, NULL };

static TiffTagFuncSlot tiff_read_tag_document_name_slot =
	{ TIFF_TAG_DOCUMENT_NAME, ( void ( * )( void ) )tiff_read_tag_document_name, NULL };

static TiffTagFuncSlot tiff_read_tag_image_description_slot =
	{ TIFF_TAG_IMAGE_DESCRIPTION, ( void ( * )( void ) )tiff_read_tag_image_description, NULL };

static TiffTagFuncSlot tiff_read_tag_make_slot =
	{ TIFF_TAG_MAKE, ( void ( * )( void ) )tiff_read_tag_make, NULL };

static TiffTagFuncSlot tiff_read_tag_model_slot =
	{ TIFF_TAG_MODEL, ( void ( * )( void ) )tiff_read_tag_model, NULL };

static TiffTagFuncSlot tiff_read_tag_strip_file_offsets_slot =
	{ TIFF_TAG_STRIP_FILE_OFFSETS, ( void ( * )( void ) )tiff_read_tag_strip_file_offsets, NULL };

static TiffTagFuncSlot tiff_read_tag_strip_byte_counts_slot =
	{ TIFF_TAG_STRIP_BYTE_COUNTS, ( void ( * )( void ) )tiff_read_tag_strip_byte_counts, NULL };

static TiffTagFuncSlot tiff_read_tag_planar_configuration_slot =
	{ TIFF_TAG_PLANAR_CONFIGURATION, ( void ( * )( void ) )tiff_read_tag_planar_configuration, NULL };

static TiffTagFuncSlot tiff_read_tag_page_name_slot =
	{ TIFF_TAG_PAGE_NAME, ( void ( * )( void ) )tiff_read_tag_page_name, NULL };

static TiffTagFuncSlot tiff_read_tag_software_slot =
	{ TIFF_TAG_SOFTWARE, ( void ( * )( void ) )tiff_read_tag_software, NULL };

static TiffTagFuncSlot tiff_read_tag_date_time_slot =
	{ TIFF_TAG_DATE_TIME, ( void ( * )( void ) )tiff_read_tag_date_time, NULL };

static TiffTagFuncSlot tiff_read_tag_artist_slot =
	{ TIFF_TAG_ARTIST, ( void ( * )( void ) )tiff_read_tag_artist, NULL };

static TiffTagFuncSlot tiff_read_tag_host_computer_slot =
	{ TIFF_TAG_HOST_COMPUTER, ( void ( * )( void ) )tiff_read_tag_host_computer, NULL };

static TiffTagFuncSlot tiff_read_tag_cz_lsm_info_slot =
	{ TIFF_TAG_CZ_LSM_INFO, ( void ( * )( void ) )tiff_read_tag_cz_lsm_info, NULL };


/* NOTE: Number of buckers is a prime number. */
#define TIFF_TAG_FUNC_TABLE_NUM_BUCKETS  41

typedef struct _TiffTagFuncTable
	{
	TiffTagFuncSlot* buckets[ TIFF_TAG_FUNC_TABLE_NUM_BUCKETS ];
	}
	TiffTagFuncTable;


static tiffuint32 tiff_hash_tag( tiffuint16 tag )
	{  return ( ( tiffuint32 )tag ) % TIFF_TAG_FUNC_TABLE_NUM_BUCKETS;  }


static void tiff_tag_func_table_add_slot( TiffTagFuncTable *table, TiffTagFuncSlot *slot )
	{
	tiffuint32 bucket = tiff_hash_tag( slot->tag );

	assert( NULL == slot->next );

	if( NULL == table->buckets[ bucket ] )
		table->buckets[ bucket ] = slot;
	else
		{
		/* Push to front of bucket. */

		slot->next = table->buckets[ bucket ];
		table->buckets[ bucket ] = slot;
		}
	}


static TiffTagFuncTable  tiff_read_tag_func_table;
static int               tiff_read_tag_func_table_initialized = 0;


static TiffReadTagFunc tiff_get_read_tag_func( const tiffuint16 tag )
	{
	TiffTagFuncSlot *slot;

	if( ! tiff_read_tag_func_table_initialized )
		{
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_image_width_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_image_height_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_bits_per_sample_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_compression_type_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_photometric_interpretation_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_document_name_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_image_description_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_make_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_model_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_strip_file_offsets_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_strip_byte_counts_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_planar_configuration_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_page_name_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_software_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_date_time_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_artist_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_host_computer_slot );
		tiff_tag_func_table_add_slot( &tiff_read_tag_func_table, &tiff_read_tag_cz_lsm_info_slot );

		tiff_read_tag_func_table_initialized = 1;
		}

	slot = tiff_read_tag_func_table.buckets[ tiff_hash_tag( tag ) ];

   for( ; NULL != slot; slot = slot->next )
		if( tag == slot->tag )
			return ( TiffReadTagFunc )slot->tag_func;

   return NULL;
	}


static TiffErrorType tiff_reader_read_images( TiffReader *reader, void *user_data )
	{
	TiffDirectory    *directory;
	TiffField        *field;
	tiffuint16        i;
	TiffReadTagFunc   read_tag_func;
	TiffErrorType     error;
	TiffImage		  *image;


   for( directory = reader->directories; NULL != directory; directory = directory->next )
		{
		if( NULL == ( image = tiff_image_new( user_data ) ) )
			return TIFF_ERROR_OUT_OF_MEMORY;

		if( NULL == reader->images )
			reader->images = image;
		else
			reader->images_tail->next = image;

	   reader->images_tail = image;

		for( i = 0; i < directory->num_fields; ++i )
			{
			field         = &directory->fields[ i ];
			read_tag_func = tiff_get_read_tag_func( field->tag );

			#ifdef TIFF_VERBOSE
			tiff_print(
				user_data,
				"directory %p : field %u : tag = %u type = %s\n",
				directory,
				( tiffuint32 )i,
				( tiffuint32 )field->tag,
				tiff_type_to_string( ( int )field->type )
				);
			#endif

         if( NULL != read_tag_func &&
				 TIFF_NO_ERROR != ( error = ( read_tag_func )( reader, field, image, user_data ) ) )
				{
				/* Splice this (thumbnail) image out of the list if LSM file and the error
					was that the pixel format is not supported. Otherwise its a "real" error. */
				if( reader->is_lsm && ( TIFF_ERROR_UNSUPPORTED == error ) )
					{
					if( reader->images == image )
						reader->images = NULL;

					reader->images_tail = reader->images;

					while( NULL != reader->images_tail && image != reader->images_tail->next )
						reader->images_tail = reader->images_tail->next;

					if( NULL != reader->images_tail )
						reader->images_tail->next = NULL;

					tiff_image_free( image, user_data );

					#ifdef TIFF_VERBOSE
					tiff_print( user_data, "Error %d occurred. Skipping this image.\n", error );
					#endif

					goto _TIFF_READER_READ_IMAGES_NEXT;
					}
				else
					return error;
				}
			}

		/* Check that certain essential fields have been read. */

		if( ! ( image->flags & TIFF_BIT_IMAGE_WIDTH )        ||
			 ! ( image->flags & TIFF_BIT_IMAGE_HEIGHT )       ||
			 ! ( image->flags & TIFF_BIT_BITS_PER_SAMPLE )    ||
			 ! ( image->flags & TIFF_BIT_STRIP_FILE_OFFSETS ) ||
			 ! ( image->flags & TIFF_BIT_STRIP_BYTE_COUNTS )     )
			return TIFF_ERROR_FILE_INVALID;

		/* Assume uncompressed image if not indicated in file. */

		if( ! ( image->flags & TIFF_BIT_COMPRESSION_TYPE ) )
			image->compression_type = TIFF_UNCOMPRESSED;

		assert( NULL != image->strips_file_offsets );
		assert( NULL != image->strips_byte_counts );

		/* The number of strips must be recorded consistently in the file. */

		if( image->strips_file_offsets->count != image->strips_byte_counts->count )
			return TIFF_ERROR_FILE_INVALID;

		image->num_strips = image->strips_file_offsets->count;

		if( TIFF_NO_ERROR != ( error = tiff_image_interpret_pixel_type( image ) ) )
			return error;

		#ifdef TIFF_VERBOSE
		tiff_image_print( image, user_data );
		#endif

		_TIFF_READER_READ_IMAGES_NEXT:;
		}

	return TIFF_NO_ERROR;
	}


static tiffuint32 tiff_reader_num_images( TiffReader *reader )
	{
	TiffImage   *image;
	tiffuint32   num_images = 0;

   for( image = reader->images; NULL != image; image = image->next )
		++num_images;
	
	return num_images;
	}


static TiffErrorType tiff_reader_interpret_file( TiffReader *reader, TiffFile *tiff_file )
	{
	TiffImage      *image;
	tiffuint32      width;
	tiffuint32      height;
	tiffuint32      length;
	TiffPixelType   pixel_type;
	TiffPixelInfo   pixel_info;

	
	if( 0 == ( length = tiff_reader_num_images( reader ) ) )
		return TIFF_ERROR_FILE_INVALID;

	/* Make sure that all the images are the same type and size.
      This library currently doesnt varying 2D slices. */

	image = reader->images;
	assert( NULL != image );

	/* Get stats from first image. */

	width      = image->width;
	height     = image->height;
	pixel_type = image->pixel_type;

	/* Compare against the other images. */

	image = image->next;

	for( ; NULL != image; image = image->next )
		if( image->width      != width      ||
			 image->height     != height     ||
			 image->pixel_type != pixel_type   )
			return TIFF_ERROR_UNSUPPORTED;

	tiff_file->width  = width;
	tiff_file->height = height;
	tiff_file->length = length;

	assert( tiff_pixel_info( &pixel_info, pixel_type ) );

	tiff_file->pixel_info.pixel_type         = pixel_info.pixel_type;
	tiff_file->pixel_info.channels_per_pixel = pixel_info.channels_per_pixel;
	tiff_file->pixel_info.bits_per_pixel     = pixel_info.bits_per_pixel;
	tiff_file->pixel_info.luminance_bits     = pixel_info.luminance_bits;
	tiff_file->pixel_info.red_bits           = pixel_info.red_bits;
	tiff_file->pixel_info.green_bits         = pixel_info.green_bits;
	tiff_file->pixel_info.blue_bits          = pixel_info.blue_bits;
	tiff_file->pixel_info.alpha_bits         = pixel_info.alpha_bits;

	return TIFF_NO_ERROR;
	}


static void tiff_pixels_lum16_reverse_endian( void* pixels, tiffuint32 byte_count )
	{
	tiffuint32   num_pixels;
	tiffuint32   i;
	tiffuint16  *pixel;


	num_pixels = byte_count / sizeof( tiffuint16 );
	pixel      = ( tiffuint16* )pixels;

	for( i = 0; i < num_pixels; ++i )
		{
		*pixel = tiff_reverse_endian_i16( *pixel );
		++pixel;
		}
	}


static void _tiff_pixels_reverse_endian
	( 
	void           *pixels,
	TiffPixelType   pixel_type,
	tiffuint32      byte_count
	)
	{
	switch( pixel_type )
		{
		/* NOTE: Again assuming 12 bits packed into 16. */
		case TIFF_PIXEL_LUM12:
			tiff_pixels_lum16_reverse_endian( pixels, byte_count );
			break;

		case TIFF_PIXEL_LUM16:
			tiff_pixels_lum16_reverse_endian( pixels, byte_count );
			break;
		}
	}


#define tiff_reader_reverse_pixels_endian( reader, pixels, pixel_type, byte_count )\
   if( ( reader )->machine_endian != ( reader )->header.byte_order )\
		_tiff_pixels_reverse_endian( ( pixels ), ( pixel_type ), ( byte_count ) )


static void tiff_pixels_lum16_max_intensity( TiffReader *reader, void *pixels, tiffuint32 byte_count )
	{
	tiffuint32   num_pixels;
	tiffuint32   i;
	tiffuint16  *pixel;


	num_pixels = byte_count / sizeof( tiffuint16 );
	pixel      = ( tiffuint16* )pixels;

	for( i = 0; i < num_pixels; ++i )
		{
		if( ( size_t )*pixel > reader->max_intensity )
			reader->max_intensity = ( size_t )*pixel;

		++pixel;
		}
	}


static TiffErrorType tiff_reader_read_image_strips
	( 
	TiffReader  *reader,
	TiffImage   *image,
	void        *pixels,
	void        *user_data
	)
	{
	tiffuint32  file_offset;
	tiffuint32  byte_count;
	tiffuint32  i;


	assert( image->num_strips == image->strips_byte_counts->count );
	assert( image->num_strips == image->strips_file_offsets->count );

	for( i = 0; i < image->num_strips; ++i )
		{
		file_offset = tiff_strip_info_value( image->strips_file_offsets, i );
		byte_count  = tiff_strip_info_value( image->strips_byte_counts, i );

		if( ! tiff_seek_from_beginning( reader->stream, file_offset, user_data ) )
			return TIFF_ERROR_FILE_SEEK;

		assert( pixels < reader->end_pixels );

		if( byte_count != tiff_fread( pixels, 1, byte_count, reader->stream, user_data ) )
			return TIFF_ERROR_FILE_READ;

		tiff_reader_reverse_pixels_endian( reader, pixels, image->pixel_type, byte_count );

		if( TIFF_PIXEL_LUM16 == image->pixel_type )
			tiff_pixels_lum16_max_intensity( reader, pixels, byte_count );

		pixels = ( ( tiffuint8* )pixels ) + byte_count;
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_pixels
	(
	TiffReader  *reader,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{
	TiffImage      *image;
	tiffuint32      total_images_bytes = 0;
	TiffErrorType   error;
	void           *pixels;
	tiffuint32      i, n;


	n = 0;
	for( image = reader->images; NULL != image; image = image->next )
		++n;

	for( image = reader->images; NULL != image; image = image->next )
		{
		if( 0 == ( image->total_strips_bytes = tiff_image_strips_total_bytes( image ) ) )
			return TIFF_ERROR_FILE_INVALID;

		total_images_bytes += image->total_strips_bytes;
		}

	if( 0 == total_images_bytes )
		return TIFF_ERROR_FILE_INVALID;

	if( TIFF_NO_ERROR != ( error =
		 tiff_reader_allocate_pixels( reader, total_images_bytes, user_data ) ) )
		return error;

	pixels = reader->pixels;

	i = 0;
	for( image = reader->images; NULL != image; image = image->next )
		{
		if( NULL != cancelled )
			if( ( cancelled )( user_data ) )
				return TIFF_NO_ERROR;

		if( TIFF_NO_ERROR != ( error =
			 tiff_reader_read_image_strips( reader, image, pixels, user_data ) ) )
			return error;

		pixels = ( ( tiffuint8* )pixels ) + image->total_strips_bytes;

		if( NULL != update )
			{
			( update )( ( float )i / ( float )n * 100.0f, user_data );
			++i;
			}
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_reader_read_pixels_at
	(
	TiffReader  *reader,
	size_t       z,
	void        *user_data
	)
	{
	TiffImage      *image;
	tiffuint32      i;
	TiffErrorType   error;


	/* Traverse to the right image in the list. */

	image = reader->images;

	for( i = 0; i < z; ++i )
		if( NULL != image )
			image = image->next;

	/* NOTE: Just assuming z is a valid index for now! */
	assert( NULL != image );

	if( 0 == ( image->total_strips_bytes = tiff_image_strips_total_bytes( image ) ) )
		return TIFF_ERROR_FILE_INVALID;

	if( TIFF_NO_ERROR != ( error =
		 tiff_reader_allocate_pixels( reader, image->total_strips_bytes, user_data ) ) )
		return error;

	return tiff_reader_read_image_strips( reader, image, reader->pixels, user_data );
	}


TiffFile* tiff_file_new( void *user_data )
   {
	TiffFile *tiff_file;

	if( NULL == ( tiff_file = tiff_new( TiffFile, user_data ) ) )
		return NULL;

	tiff_file_construct( tiff_file, user_data );
	return tiff_file;
	}


static void tiff_pixel_info_init( TiffPixelInfo *info )
	{
	info->pixel_type         = 0;
	info->channels_per_pixel = 0;
	info->bits_per_pixel     = 0;
	info->luminance_bits     = 0;
	info->red_bits           = 0;
	info->green_bits         = 0;
	info->blue_bits          = 0;
	info->alpha_bits         = 0;
	}


static void tiff_file_init( TiffFile *tiff_file, tiffint8 is_lsm )
	{
	tiff_pixel_info_init( &tiff_file->pixel_info );
	tiff_pixel_info_init( &tiff_file->conv_pixel_info );

	tiff_file->width              = 0;
	tiff_file->height             = 0;
	tiff_file->length             = 0;
	tiff_file->pixels             = NULL;
	tiff_file->max_intensity      = 0;
	tiff_file->document_name      = NULL;
	tiff_file->image_description  = NULL;
	tiff_file->make               = NULL;
	tiff_file->model              = NULL;
	tiff_file->page_name          = NULL;
	tiff_file->software           = NULL;
	tiff_file->date_time          = NULL;
	tiff_file->artist             = NULL;
	tiff_file->host_computer      = NULL;
	tiff_file->is_lsm             = is_lsm;
	tiff_file->lsm_info           = NULL;
	}


void tiff_file_construct( TiffFile *tiff_file, void *user_data )
	{
	TIFF_USE_VARIABLE( user_data );

	assert( NULL != tiff_file );
	tiff_file_init( tiff_file, 0 );
	}


void tiff_file_construct_lsm( TiffFile *tiff_file, void *user_data )
	{
	TIFF_USE_VARIABLE( user_data );

	assert( NULL != tiff_file );
	tiff_file_init( tiff_file, 1 );
	}


void tiff_file_delete( TiffFile *tiff_file, void *user_data )
   {
   assert( NULL != tiff_file );

	tiff_file_destruct( tiff_file, user_data );
   tiff_free( tiff_file, user_data );
   }


void tiff_file_destruct( TiffFile *tiff_file, void *user_data )
	{
   assert( NULL != tiff_file );

   tiff_free( tiff_file->pixels, user_data );

	tiff_free( ( char* )tiff_file->document_name, user_data );
	tiff_free( ( char* )tiff_file->image_description, user_data );
	tiff_free( ( char* )tiff_file->make, user_data );
	tiff_free( ( char* )tiff_file->model, user_data );
	tiff_free( ( char* )tiff_file->page_name, user_data );
	tiff_free( ( char* )tiff_file->software, user_data );
	tiff_free( ( char* )tiff_file->date_time, user_data );
	tiff_free( ( char* )tiff_file->artist, user_data );
	tiff_free( ( char* )tiff_file->host_computer, user_data );
	tiff_free( tiff_file->lsm_info, user_data );

	tiff_file_init( tiff_file, 0 );
	}


static TiffErrorType tiff_reader_read_info( TiffReader *reader, void *user_data )
	{
	TiffErrorType error;

   if( TIFF_NO_ERROR != ( error = tiff_reader_read_header( reader, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_directories( reader, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_images( reader, user_data ) ) )
		return error;

	return TIFF_NO_ERROR;
	}


static TiffLsmInfo* tiff_reader_release_lsm_info( TiffReader *reader )
	{
	TiffLsmInfo *lsm_info = reader->lsm_info;
	reader->lsm_info      = NULL;

	return lsm_info;
	}


static void tiff_reader_release_additional_info( TiffReader *reader, TiffFile *tiff_file )
	{
	tiff_file->document_name     = tiff_reader_release_string( reader, document_name );
	tiff_file->image_description = tiff_reader_release_string( reader, image_description );
	tiff_file->make              = tiff_reader_release_string( reader, make );
	tiff_file->model             = tiff_reader_release_string( reader, model );
	tiff_file->page_name         = tiff_reader_release_string( reader, page_name );
	tiff_file->software          = tiff_reader_release_string( reader, software );
	tiff_file->date_time         = tiff_reader_release_string( reader, date_time );
	tiff_file->artist            = tiff_reader_release_string( reader, artist );
	tiff_file->host_computer     = tiff_reader_release_string( reader, host_computer );
	tiff_file->lsm_info          = tiff_reader_release_lsm_info( reader );
	}


TiffErrorType tiff_file_read_info
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	void        *user_data
	)
	{
	TiffReader     *reader;
   TiffErrorType   error;


	assert( NULL != tiff_file );
	assert( NULL != file_name );

   if( NULL == ( reader = tiff_reader_new( file_name, &error, tiff_file->is_lsm, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_info( reader, user_data ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_reader_interpret_file( reader, tiff_file ) ) )
		return tiff_reader_delete( reader, error, user_data );

	tiff_reader_release_additional_info( reader, tiff_file );

   return tiff_reader_delete( reader, TIFF_NO_ERROR, user_data );
	}


static TiffErrorType tiff_file_do_read
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	void        *pixels,
	size_t       pixels_size,
	int          allocate_pixels,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{
	TiffReader     *reader;
   TiffErrorType   error;


	assert( NULL != tiff_file );
	assert( NULL != file_name );

   if( NULL == ( reader = tiff_reader_new( file_name, &error, tiff_file->is_lsm, user_data ) ) )
		return error;

	reader->pixels          = pixels;
	reader->pixels_size     = pixels_size;
	reader->allocate_pixels = allocate_pixels;

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_info( reader, user_data ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_reader_interpret_file( reader, tiff_file ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_pixels( reader, cancelled, update, user_data ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( NULL != cancelled )
		if( ( cancelled )( user_data ) )
			return tiff_reader_delete( reader, TIFF_NO_ERROR, user_data );

	tiff_file->pixels = allocate_pixels ? tiff_reader_release_pixels( reader ) : NULL;
	tiff_reader_release_additional_info( reader, tiff_file );

	tiff_file->max_intensity = reader->max_intensity;

   return tiff_reader_delete( reader, TIFF_NO_ERROR, user_data );
	}


TiffErrorType tiff_file_read
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void         *user_data
	)
	{
	return tiff_file_do_read(
					tiff_file,
					file_name,
					NULL,
					0,
					1,
					cancelled,
					update,
					user_data
					);
	}


TiffErrorType tiff_file_read_ex
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	void        *pixels,
	size_t       pixels_size,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void         *user_data
	)
	{
	return tiff_file_do_read(
					tiff_file,
					file_name,
					pixels,
					pixels_size,
					0,
					cancelled,
					update,
					user_data
					);
	}


TiffErrorType tiff_file_read_at
	(
	TiffFile    *tiff_file,
	const char  *file_name,
	size_t       z,
	void        *user_data
	)
	{
	TiffReader     *reader;
   TiffErrorType   error;


	assert( NULL != tiff_file );
	assert( NULL != file_name );

   if( NULL == ( reader = tiff_reader_new( file_name, &error, tiff_file->is_lsm, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_info( reader, user_data ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_reader_interpret_file( reader, tiff_file ) ) )
		return tiff_reader_delete( reader, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_reader_read_pixels_at( reader, z, user_data ) ) )
		return tiff_reader_delete( reader, error, user_data );

	tiff_file->pixels = tiff_reader_release_pixels( reader );
	tiff_reader_release_additional_info( reader, tiff_file );

	tiff_file->max_intensity = reader->max_intensity;

   return tiff_reader_delete( reader, TIFF_NO_ERROR, user_data );
	}


/******************************************************************************/


static tiffint32 tiff_pack_i16_into_i32( tiffint16 i16 )
	{
	tiffint32 i32 = 0;

	/* Pack it in the low bytes! */

	*( ( ( tiffint16* )&i32 ) + 0 ) = i16;

	return i32;
	}


static tiffint32 tiff_pack_i16_x2_into_i32( tiffint16 i16_0, tiffint16 i16_1 )
	{
	tiffint32 i32 = 0;

	*( ( ( tiffint16* )&i32 ) + 0 ) = i16_0;
	*( ( ( tiffint16* )&i32 ) + 1 ) = i16_1;

	return i32;
	}


struct _TiffWriter;
typedef struct _TiffWriter TiffWriter;

struct _TiffWriter
	{
	const char     *file_name;
	void           *stream;
	tiffuint32      full_width;
	tiffuint32      full_height;
	tiffuint32      full_length;
	TiffRegion      region;
	tiffuint32      num_pixels;
	TiffPixelInfo   pixel_info;
	TiffPixelInfo   conv_pixel_info;
	void            ( *conv_func )( const void*, void*, TiffWriter* );
	void           *conv_buffer;
	tiffuint32      conv_buffer_size;
	const void     *pixels;
	int             machine_endian;
	tiffuint32      bytes_written;
	};


static TiffWriter* tiff_writer_new
	(
	const char     *file_name,
	TiffErrorType  *error,
	void           *user_data
	)
	{
	TiffWriter *writer;

   if( NULL == ( writer = tiff_new( TiffWriter, user_data ) ) )
		{
		*error = TIFF_ERROR_OUT_OF_MEMORY;
		return NULL;
		}

	writer->file_name = file_name;

   if( NULL == ( writer->stream = tiff_fopen( file_name, "wb", user_data ) ) )
		{
		tiff_free( writer, user_data );
		*error = TIFF_ERROR_FILE_OPEN;
		return NULL;
		}

	writer->conv_func        = NULL;
	writer->conv_buffer      = NULL;
	writer->conv_buffer_size = 0;

   writer->machine_endian = tiff_get_machine_endian();

   return writer;
   }


static TiffErrorType tiff_writer_delete
	(
	TiffWriter     *writer,
	TiffErrorType   error,
	void           *user_data
	)
	{
   if( NULL != writer->stream )
		tiff_fclose( writer->stream, user_data );

   if( TIFF_NO_ERROR != error )
		tiff_remove( writer->file_name, user_data );

	tiff_free( writer->conv_buffer, user_data );

	tiff_free( writer, user_data );

   return error;
	}


static int _tiff_writer_write_i16
	(
	TiffWriter       *writer,
	const tiffint16  *i16,
	tiffuint32        bytes,
	void             *user_data
	)
	{
	assert( TIFF_TYPE_USHORT_SIZE == bytes );

	if( 1 != tiff_fwrite( i16, sizeof( tiffint16 ), 1, writer->stream, user_data ) )
		return 0;

	writer->bytes_written += sizeof( tiffint16 );

	return 1;
	}


static int _tiff_writer_write_i32
	(
	TiffWriter       *writer,
	const tiffint32  *i32,
	tiffuint32        bytes,
	void             *user_data
	)
	{
	assert( TIFF_TYPE_ULONG_SIZE == bytes );

	if( 1 != tiff_fwrite( i32, sizeof( tiffint32 ), 1, writer->stream, user_data ) )
		return 0;

	writer->bytes_written += sizeof( tiffint32 );

	return 1;
	}


#define tiff_writer_write_i16( writer, i16, user_data )\
	_tiff_writer_write_i16(\
		(writer),\
		( tiffint16* )(i16),\
		sizeof( *(i16) ),\
		(user_data)\
		)


#define tiff_writer_write_i32( writer, i32, user_data )\
	_tiff_writer_write_i32(\
		(writer),\
		( tiffint32* )(i32),\
		sizeof( *(i32) ),\
		(user_data)\
		)


static TiffErrorType tiff_writer_write_header( TiffWriter *writer, void *user_data )
	{
	TiffHeader header;

	header.byte_order             = ( tiffuint16 )writer->machine_endian;
	header.version_number         = TIFF_VERSION_NUMBER;
	header.first_directory_offset = TIFF_HEADER_SIZE;

	/* NOTE: The offset to the first directory is always immediately
		after the header! */

	if( ! tiff_writer_write_i16( writer, &header.byte_order, user_data )            ||
       ! tiff_writer_write_i16( writer, &header.version_number, user_data )        ||
       ! tiff_writer_write_i32( writer, &header.first_directory_offset, user_data )  )
		return TIFF_ERROR_FILE_WRITE;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_field
	(
	TiffWriter  *writer,
	TiffField   *field,
	void        *user_data
	)
	{
   if( ! tiff_writer_write_i16( writer, &field->tag, user_data )         ||
		 ! tiff_writer_write_i16( writer, &field->type, user_data )        ||
		 ! tiff_writer_write_i32( writer, &field->count, user_data )       ||
       ! tiff_writer_write_i32( writer, &field->value_offset, user_data )  ) 
		return TIFF_ERROR_FILE_WRITE;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_directory
	(
	TiffWriter     *writer,
	TiffDirectory  *directory,
	void           *user_data
	)
	{
	TiffErrorType   error;
	tiffuint16      i;
	TiffField      *field;


	if( ! tiff_writer_write_i16( writer, &directory->num_fields, user_data ) )
		return TIFF_ERROR_FILE_WRITE;

	for( i = 0; i < directory->num_fields; ++i )
		{
		field = tiff_directory_get_field( directory, i );

		if( TIFF_NO_ERROR != ( error =
			 tiff_writer_write_field( writer, field, user_data ) ) )
			return error;
		}

	if( ! tiff_writer_write_i32( writer, &directory->next_directory_offset, user_data ) )
		return TIFF_ERROR_FILE_WRITE;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_directories_lum
	(
	TiffWriter           *writer,
	const TiffPixelInfo  *pixel_info,
	void                 *user_data
	)
	{
	TiffDirectory   *directory;
	TiffField        field;
	tiffuint32       bytes_per_image;
	tiffuint32       i;
	TiffErrorType    error;


	if( NULL == ( directory = tiff_directory_new( 7, user_data ) ) )
		return TIFF_ERROR_OUT_OF_MEMORY;

	/* Set the fields that wont change for each image directory. */

	tiff_field( &field, TIFF_TAG_IMAGE_WIDTH, TIFF_TYPE_ULONG, 1, ( tiffuint32 )writer->region.width );
	tiff_directory_set_field( directory, &field, 0 );

	tiff_field( &field, TIFF_TAG_IMAGE_HEIGHT, TIFF_TYPE_ULONG, 1, ( tiffuint32 )writer->region.height );
	tiff_directory_set_field( directory, &field, 1 );

	tiff_field( &field, TIFF_TAG_BITS_PER_SAMPLE, TIFF_TYPE_USHORT, pixel_info->channels_per_pixel,
               tiff_pack_i16_into_i32( ( tiffint16 )pixel_info->luminance_bits ) );
	tiff_directory_set_field( directory, &field, 2 );

	tiff_field( &field, TIFF_TAG_COMPRESSION_TYPE, TIFF_TYPE_USHORT, 1,
               tiff_pack_i16_into_i32( TIFF_UNCOMPRESSED ) );
	tiff_directory_set_field( directory, &field, 3 );

	tiff_field( &field, TIFF_TAG_PHOTOMETRIC_INTERPRETATION, TIFF_TYPE_USHORT, 1,
               tiff_pack_i16_into_i32( TIFF_BLACK_0_WHITE_MAX ) );
	tiff_directory_set_field( directory, &field, 4 );

	/* Currently each 2D slice of the image is just saved as one big strip. */

	bytes_per_image = ( tiffuint32 )( writer->region.width * writer->region.height * ( pixel_info->bits_per_pixel / 8 ) );

	tiff_field( &field, TIFF_TAG_STRIP_BYTE_COUNTS, TIFF_TYPE_ULONG, 1, bytes_per_image );
	tiff_directory_set_field( directory, &field, 6 );

	for( i = 0; i < writer->region.length; ++i )
		{
		/* The strips begin right after these directories in the file. */

		tiff_field(
			&field,
			TIFF_TAG_STRIP_FILE_OFFSETS,
			TIFF_TYPE_ULONG, 1,
			( tiffuint32 )( TIFF_HEADER_SIZE + directory->size * writer->region.length + bytes_per_image * i )
			);

		tiff_directory_set_field( directory, &field, 5 );

		/* If last directory the offset to the next must be zero. */

		directory->next_directory_offset = ( i == writer->region.length - 1 )
													  ? 0 : TIFF_HEADER_SIZE + directory->size * ( i + 1 );

		if( TIFF_NO_ERROR != ( error = tiff_writer_write_directory( writer, directory, user_data ) ) )
			{
			tiff_directory_free( directory, user_data );
			return error;
			}
		}

	tiff_directory_free( directory, user_data );
	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_directories_lum8
	(
	TiffWriter           *writer,
	const TiffPixelInfo  *pixel_info,
	void                 *user_data
	)
	{  return tiff_writer_write_directories_lum( writer, pixel_info, user_data );  }


static TiffErrorType tiff_writer_write_directories_lum12
	(
	TiffWriter           *writer,
	const TiffPixelInfo  *pixel_info,
	void                 *user_data
	)
	{  return tiff_writer_write_directories_lum( writer, pixel_info, user_data );  }


static TiffErrorType tiff_writer_write_directories_lum16
	(
	TiffWriter           *writer,
	const TiffPixelInfo  *pixel_info,
	void                 *user_data
	)
	{  return tiff_writer_write_directories_lum( writer, pixel_info, user_data );  }


static TiffErrorType tiff_writer_write_directories( TiffWriter *writer, void *user_data )
	{
	TiffErrorType         error;
	const TiffPixelInfo  *pixel_info;
	


	error      = TIFF_NO_ERROR;
	pixel_info = writer->conv_pixel_info.pixel_type ? &writer->conv_pixel_info : &writer->pixel_info;

	switch( pixel_info->pixel_type )
		{
		case TIFF_PIXEL_LUM8:
			error = tiff_writer_write_directories_lum8( writer, pixel_info, user_data );
			break;

		case TIFF_PIXEL_LUM12:
			error = tiff_writer_write_directories_lum12( writer, pixel_info, user_data );
			break;

		case TIFF_PIXEL_LUM16:
			error = tiff_writer_write_directories_lum16( writer, pixel_info, user_data );
			break;

		default:
			assert( 0 );;
		}

	return error;
	}


static TiffErrorType tiff_writer_write_info( TiffWriter *writer, void *user_data )
	{
	TiffErrorType error;

   if( TIFF_NO_ERROR != ( error = tiff_writer_write_header( writer, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_writer_write_directories( writer, user_data ) ) )
   	return error;

	return TIFF_NO_ERROR;
	}


#define _TIFF_X_BIT_LUM_TO_X_BIT_LUM( func )\
	tiffuint32 x, y, width, height, first, offset;\
	\
	width  = writer->region.width;\
	height = writer->region.height;\
	first  = writer->region.x;\
	offset = writer->full_width;\
	\
	for( y = 0; y < height; ++y )\
		{\
		for( x = 0; x < width; ++x )\
			func;\
		\
		src += offset;\
		}


static void tiff_16_bit_lum_to_12_bit_lum( const tiffuint16 *src, tiffuint16 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( *( src + first + x ) >> 4 ) );  }


static void tiff_16_bit_lum_to_8_bit_lum( const tiffuint16 *src, tiffuint8 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( tiffuint8 )( *( src + first + x ) >> 8 ) );  }


static void tiff_12_bit_lum_to_8_bit_lum( const tiffuint16 *src, tiffuint8 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( tiffuint8 )( *( src + first + x ) >> 4 ) );  }


static void tiff_12_bit_lum_to_16_bit_lum( const tiffuint16 *src, tiffuint16 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( *( src + first + x ) << 4 ) );  }


static void tiff_8_bit_lum_to_12_bit_lum( const tiffuint8 *src, tiffuint16 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( tiffuint16 )( *( src + first + x ) << 4 ) );  }


static void tiff_8_bit_lum_to_16_bit_lum( const tiffuint8 *src, tiffuint16 *dest, TiffWriter *writer )
	{  _TIFF_X_BIT_LUM_TO_X_BIT_LUM( *dest++ = ( tiffuint16 )( *( src + first + x ) << 8 ) );  }


static TiffErrorType tiff_writer_interpret_file
	(
	TiffWriter        *writer,
	const TiffFile    *tiff_file,
	const TiffRegion  *tiff_region,
	void              *user_data
	)
	{	
	if( 0 == tiff_file->width || 0 == tiff_file->height || 0 == tiff_file->length )
		return TIFF_ERROR_PARAMETER;

	writer->full_width  = ( tiffuint32 )tiff_file->width;
	writer->full_height = ( tiffuint32 )tiff_file->height;
	writer->full_length = ( tiffuint32 )tiff_file->length;

	if( NULL == tiff_region )
		{
		writer->region.x      = 0;
		writer->region.y      = 0;
		writer->region.z      = 0;
		writer->region.width  = writer->full_width;
		writer->region.height = writer->full_height;
		writer->region.length = writer->full_length;
		}
	else
		{
		if( 0 == tiff_region->width || 0 == tiff_region->height || 0 == tiff_region->length )
			return TIFF_ERROR_PARAMETER;

		if( tiff_file->width  <= tiff_region->x ||
			 tiff_file->height <= tiff_region->y ||
			 tiff_file->length <= tiff_region->z   )
			return TIFF_ERROR_PARAMETER;

		if( tiff_file->width  < tiff_region->x + tiff_region->width  ||
			 tiff_file->height < tiff_region->y + tiff_region->height ||
			 tiff_file->length < tiff_region->z + tiff_region->length   )
			return TIFF_ERROR_PARAMETER;

		writer->region.x      = ( tiffuint32 )tiff_region->x;
		writer->region.y      = ( tiffuint32 )tiff_region->y;
		writer->region.z      = ( tiffuint32 )tiff_region->z;
		writer->region.width  = ( tiffuint32 )tiff_region->width;
		writer->region.height = ( tiffuint32 )tiff_region->height;
		writer->region.length = ( tiffuint32 )tiff_region->length;
		}

	writer->num_pixels = ( tiffuint32 )( writer->region.width * writer->region.height * writer->region.length );

	if( ! tiff_pixel_info( &writer->pixel_info, tiff_file->pixel_info.pixel_type ) )
		return TIFF_ERROR_UNSUPPORTED;

	if( writer->pixel_info.channels_per_pixel != tiff_file->pixel_info.channels_per_pixel ||
		 writer->pixel_info.bits_per_pixel     != tiff_file->pixel_info.bits_per_pixel     ||
		 writer->pixel_info.luminance_bits     != tiff_file->pixel_info.luminance_bits     ||
		 writer->pixel_info.red_bits           != tiff_file->pixel_info.red_bits           ||
		 writer->pixel_info.green_bits         != tiff_file->pixel_info.green_bits         ||
		 writer->pixel_info.blue_bits          != tiff_file->pixel_info.blue_bits          ||
		 writer->pixel_info.alpha_bits         != tiff_file->pixel_info.alpha_bits            )
		return TIFF_ERROR_UNSUPPORTED;

	if( tiff_file->conv_pixel_info.pixel_type )
		{
		if( ! tiff_pixel_info( &writer->conv_pixel_info, tiff_file->conv_pixel_info.pixel_type ) )
			return TIFF_ERROR_UNSUPPORTED;

		if( writer->conv_pixel_info.pixel_type == writer->pixel_info.pixel_type )
			return TIFF_ERROR_PARAMETER;

		if( writer->conv_pixel_info.channels_per_pixel != tiff_file->conv_pixel_info.channels_per_pixel ||
			 writer->conv_pixel_info.bits_per_pixel     != tiff_file->conv_pixel_info.bits_per_pixel     ||
			 writer->conv_pixel_info.luminance_bits     != tiff_file->conv_pixel_info.luminance_bits     ||
			 writer->conv_pixel_info.red_bits           != tiff_file->conv_pixel_info.red_bits           ||
			 writer->conv_pixel_info.green_bits         != tiff_file->conv_pixel_info.green_bits         ||
			 writer->conv_pixel_info.blue_bits          != tiff_file->conv_pixel_info.blue_bits          ||
			 writer->conv_pixel_info.alpha_bits         != tiff_file->conv_pixel_info.alpha_bits            )
			return TIFF_ERROR_UNSUPPORTED;

		assert( NULL == writer->conv_buffer );
		assert( 0 == writer->conv_buffer_size );

		writer->conv_buffer_size =
			( writer->conv_pixel_info.bits_per_pixel / 8 ) *
			writer->region.width *
			writer->region.height;

		if( NULL == ( writer->conv_buffer = tiff_malloc( writer->conv_buffer_size, user_data ) ) )
			return TIFF_ERROR_OUT_OF_MEMORY;

		assert( NULL == writer->conv_func );

		switch( writer->pixel_info.pixel_type )
			{
			case TIFF_PIXEL_LUM8:
				switch( writer->conv_pixel_info.pixel_type )
					{
					case TIFF_PIXEL_LUM12:
						writer->conv_func = tiff_8_bit_lum_to_12_bit_lum;
						break;

					case TIFF_PIXEL_LUM16:
						writer->conv_func = tiff_8_bit_lum_to_16_bit_lum;
						break;

					default:
						return TIFF_ERROR_PARAMETER;
					}
				break;

			case TIFF_PIXEL_LUM12:
				switch( writer->conv_pixel_info.pixel_type )
					{
					case TIFF_PIXEL_LUM8:
						writer->conv_func = tiff_12_bit_lum_to_8_bit_lum;
						break;

					case TIFF_PIXEL_LUM16:
						writer->conv_func = tiff_12_bit_lum_to_16_bit_lum;
						break;

					default:
						return TIFF_ERROR_PARAMETER;
					}
				break;

			case TIFF_PIXEL_LUM16:
				switch( writer->conv_pixel_info.pixel_type )
					{
					case TIFF_PIXEL_LUM8:
						writer->conv_func = tiff_16_bit_lum_to_8_bit_lum;
						break;

					case TIFF_PIXEL_LUM12:
						writer->conv_func = tiff_16_bit_lum_to_12_bit_lum;
						break;

					default:
						return TIFF_ERROR_PARAMETER;
					}
				break;

			default:
				return TIFF_ERROR_PARAMETER;
			}
		}

	if( NULL == tiff_file->pixels )
		return TIFF_ERROR_PARAMETER;

	writer->pixels = tiff_file->pixels;

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_buffer
	( 
	TiffWriter       *writer,
	const tiffuint8  *buffer,
	int               ( *cancelled )( void* ),
	void              ( *update )( float, void* ),
	void             *user_data
	)
	{
	tiffuint32        z, y;
	const tiffuint8  *slice, *row;
	tiffuint32        bytes_per_pixel;
	tiffuint32        bytes_per_slice;
	tiffuint32        full_bytes_per_row;
	tiffuint32        region_bytes_per_row;


	bytes_per_pixel      = writer->pixel_info.bits_per_pixel / 8;
	bytes_per_slice      = writer->full_width * writer->full_height * bytes_per_pixel;
	full_bytes_per_row   = writer->full_width * bytes_per_pixel;
	region_bytes_per_row = ( tiffuint32 )( writer->region.width * bytes_per_pixel );

	slice = buffer + ( writer->region.z * bytes_per_slice );

	for( z = 0; z < writer->region.length; ++z )
		{
		if( NULL != cancelled )
			if( ( cancelled )( user_data ) )
				return TIFF_NO_ERROR;

		#ifdef TIFF_VERBOSE
		tiff_print( user_data, "writing tiff slice %u out of length %u\n", z + 1, writer->region.length );
		#endif

		if( writer->conv_pixel_info.pixel_type )
			{
			( writer->conv_func )( slice, writer->conv_buffer, writer );

			if( writer->conv_buffer_size != tiff_fwrite( writer->conv_buffer, 1, writer->conv_buffer_size, writer->stream, user_data ) )
				return TIFF_ERROR_FILE_WRITE;

			writer->bytes_written += writer->conv_buffer_size;
			}
		else
			{
			row = slice + ( writer->region.y * full_bytes_per_row ) + ( writer->region.x * bytes_per_pixel );

			for( y = 0; y < writer->region.height; ++y )
				{
				if( region_bytes_per_row != tiff_fwrite( row, 1, region_bytes_per_row, writer->stream, user_data ) )
					return TIFF_ERROR_FILE_WRITE;

				writer->bytes_written += region_bytes_per_row;

				row += full_bytes_per_row;
				}
			}

		slice += bytes_per_slice;

		if( NULL != update )
			( update )( ( float )z / ( float )writer->region.length * 100.0f, user_data );
		}

	return TIFF_NO_ERROR;
	}


static TiffErrorType tiff_writer_write_pixels_lum8
	(
	TiffWriter  *writer,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{  return tiff_writer_write_buffer( writer, writer->pixels, cancelled, update, user_data );  }
 

static TiffErrorType tiff_writer_write_pixels_lum16
	(
	TiffWriter  *writer,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{  return tiff_writer_write_buffer( writer, writer->pixels, cancelled, update, user_data );  }


/* NOTE: Again assuming 12 bits packed into 16! */
static TiffErrorType tiff_writer_write_pixels_lum12
	(
	TiffWriter  *writer,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{  return tiff_writer_write_pixels_lum16( writer, cancelled, update, user_data );  }


static TiffErrorType tiff_writer_write_pixels
	(
	TiffWriter  *writer,
	int          ( *cancelled )( void* ),
	void         ( *update )( float, void* ),
	void        *user_data
	)
	{
	TiffErrorType error = TIFF_NO_ERROR;

	switch( writer->pixel_info.pixel_type )
		{
		case TIFF_PIXEL_LUM8:
			error = tiff_writer_write_pixels_lum8( writer, cancelled, update, user_data );
			break;

		case TIFF_PIXEL_LUM12:
			error = tiff_writer_write_pixels_lum12( writer, cancelled, update, user_data );
			break;

		case TIFF_PIXEL_LUM16:
			error = tiff_writer_write_pixels_lum16( writer, cancelled, update, user_data );
			break;

		default:
			assert( 0 );;
		}

	return error;
	}


static TiffErrorType tiff_file_do_write
	(
	const TiffFile    *tiff_file,
	const TiffRegion  *tiff_region,
	const char        *file_name,
	int                ( *cancelled )( void* ),
	void               ( *update )( float, void* ),
	void              *user_data
	)
	{
	TiffWriter     *writer;
   TiffErrorType   error;


   if( NULL == ( writer = tiff_writer_new( file_name, &error, user_data ) ) )
		return error;

	if( TIFF_NO_ERROR != ( error = tiff_writer_interpret_file( writer, tiff_file, tiff_region, user_data ) ) )
		return tiff_writer_delete( writer, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_writer_write_info( writer, user_data ) ) )
		return tiff_writer_delete( writer, error, user_data );

	if( TIFF_NO_ERROR != ( error = tiff_writer_write_pixels( writer, cancelled, update, user_data ) ) )
		return tiff_writer_delete( writer, error, user_data );

   return tiff_writer_delete( writer, TIFF_NO_ERROR, user_data );
	}


TiffErrorType tiff_file_write
	(
	const TiffFile  *tiff_file,
	const char      *file_name,
	int              ( *cancelled )( void* ),
	void             ( *update )( float, void* ),
	void            *user_data
	)
	{
	assert( NULL != tiff_file );
	assert( NULL != file_name );

	return tiff_file_do_write( tiff_file, NULL, file_name, cancelled, update, user_data );
	}


TiffErrorType tiff_file_write_ex
	(
	const TiffFile    *tiff_file,
	const TiffRegion  *tiff_region,
	const char        *file_name,
	int                ( *cancelled )( void* ),
	void               ( *update )( float, void* ),
	void              *user_data
	)
	{
	assert( NULL != tiff_file );
	assert( NULL != tiff_region );
	assert( NULL != file_name );

	return tiff_file_do_write( tiff_file, tiff_region, file_name, cancelled, update, user_data );
	}
