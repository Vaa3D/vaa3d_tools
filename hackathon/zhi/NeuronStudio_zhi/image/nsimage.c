#include "nsimage.h"


#define _NS_IMAGE_BIT_RELEASE_ONLY  0x001
#define _NS_IMAGE_BIT_GROW_ONLY     0x002


void ns_image_construct( NsImage *image )
	{
	NsError error;

	ns_assert( NULL != image );

	ns_verify( NS_SUCCESS( ns_string_construct_init( &image->name, "" ), error ) );

	/* NOTE: Assure successful construction of the internal byte array by
		initially passing 0 as the size of array. */
	ns_verify(
		NS_SUCCESS(
			ns_byte_array_construct(
				&image->pixels,
				0
				),
			error
			)
		);

	image->pixel_proc_db = NULL;
	image->io_proc_db    = NULL;

	image->flags = 0;
	}


void ns_image_destruct( NsImage *image )
   {
	ns_assert( NULL != image );

	ns_string_destruct( &image->name );

   ns_image_clear( image );
	ns_byte_array_destruct( &image->pixels );

	ns_image_set_pixel_proc_db( image, NULL );
	ns_image_set_io_proc_db( image, NULL );
	}


const nschar* ns_image_get_name( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ns_string_get( &image->name );
	}


NsError ns_image_set_name( NsImage *image, const nschar *name )
	{
	ns_assert( NULL != image );
	return ns_string_set( &image->name, name );
	}


void ns_image_clear( NsImage *image )
	{
	ns_assert( NULL != image );

	if( ( nsboolean )( image->flags & _NS_IMAGE_BIT_RELEASE_ONLY ) )
		ns_byte_array_release( &image->pixels );
	else if( ! ( nsboolean )( image->flags & _NS_IMAGE_BIT_GROW_ONLY ) )
		ns_byte_array_clear( &image->pixels );

	image->flags &= ~_NS_IMAGE_BIT_RELEASE_ONLY;

	ns_memzero( &image->image_info, sizeof( NsImageInfo ) );
	ns_memzero( &image->pixel_info, sizeof( NsPixelInfo ) );
	}


NsError ns_image_create
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align
	)
	{
	NsError  error;
	nssize   bytes;


	ns_assert( NULL != image );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

    ns_image_clear( image );


	bytes = ns_pixel_buffer_size( pixel_type, width, height, length, row_align );

	if( ! ( nsboolean )( image->flags & _NS_IMAGE_BIT_GROW_ONLY ) ||
		 ns_byte_array_size( &image->pixels ) < bytes )
		{

/*TEMP!!!!!!!!!!!!!!!!!!!!*/
if( ( nsboolean )( image->flags & _NS_IMAGE_BIT_GROW_ONLY ) )
ns_println( "Resizing growable-only image from " NS_FMT_ULONG " to " NS_FMT_ULONG,
ns_byte_array_size( &image->pixels ), bytes );

		if( NS_FAILURE( ns_byte_array_resize( &image->pixels, bytes ), error ) )
			return error;
		}

	ns_pixel_info( &image->pixel_info, pixel_type );

	image->image_info.width     = width;
	image->image_info.height    = height;
	image->image_info.length    = length;
	image->image_info.row_align = row_align;

	image->bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	image->bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

	return ns_no_error();
	}


void ns_image_buffer_and_free
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nspointer     pixels
	)
	{
	nssize bytes;

	ns_assert( NULL != image );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	ns_image_clear( image );

	bytes = ns_pixel_buffer_size( pixel_type, width, height, length, row_align );

	ns_byte_array_buffer( &image->pixels, pixels, bytes );

	ns_pixel_info( &image->pixel_info, pixel_type );

	image->image_info.width     = width;
	image->image_info.height    = height;
	image->image_info.length    = length;
	image->image_info.row_align = row_align;

	image->bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
	image->bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );
	}


void ns_image_buffer_and_release
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nspointer     pixels
	)
	{
	ns_assert( NULL != image );
	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	ns_image_buffer_and_free( image, pixel_type, width, height, length, row_align, pixels );

	image->flags |= _NS_IMAGE_BIT_RELEASE_ONLY;
	}


void ns_image_set_buffer_type( NsImage *image, nsint type )
	{
	switch( type )
		{
		case NS_IMAGE_BUFFER_RESIZABLE:
			image->flags &= ~_NS_IMAGE_BIT_GROW_ONLY;
			break;

		case NS_IMAGE_BUFFER_GROW_ONLY:
			image->flags |= _NS_IMAGE_BIT_GROW_ONLY;
			break;

		default:
			ns_assert_not_reached();
		}
	}


void ns_image_release( NsImage *image )
	{
	ns_assert( NULL != image );

	ns_byte_array_release( &image->pixels );
	ns_image_clear( image );
	}


NsProcDb* ns_image_get_pixel_proc_db( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ( NsProcDb* )image->pixel_proc_db;
	}


void ns_image_set_pixel_proc_db( NsImage *image, NsProcDb *pixel_proc_db )
	{
	ns_assert( NULL != image );
	image->pixel_proc_db = pixel_proc_db;
	}


nsboolean ns_image_has_pixel_proc_db( const NsImage *image )
	{
	ns_assert( NULL != image );
	return NULL != image->pixel_proc_db;
	}


NsProcDb* ns_image_get_io_proc_db( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ( NsProcDb* )image->io_proc_db;
	}


void ns_image_set_io_proc_db( NsImage *image, NsProcDb *io_proc_db )
	{
	ns_assert( NULL != image );
	image->io_proc_db = io_proc_db;
	}


nsboolean ns_image_has_io_proc_db( const NsImage *image )
	{
	ns_assert( NULL != image );
	return NULL != image->io_proc_db;
	}


nssize ns_image_width( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->image_info.width;
	}


nssize ns_image_height( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->image_info.height;
	}


nssize ns_image_length( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->image_info.length;
	}


nssize ns_image_row_align( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->image_info.row_align;
	}


NsPixelType ns_image_pixel_type( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->pixel_info.pixel_type;
	}


NsPixelFormatType ns_image_pixel_format_type( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->pixel_info.format_type;
	}


nssize ns_image_pixel_num_channels( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->pixel_info.num_channels;
	}


NsPixelChannelType ns_image_pixel_channel_type( const NsImage *image, nssize channel )
	{
	ns_assert( NULL != image );
	ns_assert( channel < image->pixel_info.num_channels );

	return image->pixel_info.channels[ channel ].type;
	}


nssize ns_image_pixel_channel_bits( const NsImage *image, nssize channel )
	{
	ns_assert( NULL != image );
	ns_assert( channel < image->pixel_info.num_channels );

	return image->pixel_info.channels[ channel ].bits;
	}


nspointer ns_image_pixels( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ns_byte_array_begin( &image->pixels );
	}


nssize ns_image_size( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ns_byte_array_size( &image->pixels );
	}


nspointer ns_image_pixels_slice( const NsImage *image, nssize z )
	{
	ns_assert( NULL != image );
	ns_assert( z < ns_image_length( image ) );

	return ( ( nsuint8* )ns_image_pixels( image ) ) + ( z * image->bytes_per_slice );
	}


nspointer ns_image_pixels_row( const NsImage *image, nssize y, nssize z )
	{
	ns_assert( NULL != image );
	ns_assert( y < ns_image_height( image ) );
	ns_assert( z < ns_image_length( image ) );

	return ( ( nsuint8* )ns_image_pixels_slice( image, z ) ) + ( y * image->bytes_per_row );
	}


nspointer ns_image_get_user_data( const NsImage *image )
	{
	ns_assert( NULL != image );
	return image->user_data;
	}


void ns_image_set_user_data( NsImage *image, nspointer user_data )
	{
	ns_assert( NULL != image );
	image->user_data = user_data;
	}


NsError ns_image_get_pixel
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z,
	NsPixel        *pixel
	)
	{
	ns_assert( NULL != image );
	ns_assert( NULL != pixel );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_GET_PIXEL ) );

	ns_pixel_reset( pixel );

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_GET_PIXEL,
				NULL,
				NULL,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				x,
				y,
				z,
				pixel
				);
	}


NsError ns_image_set_pixel
	(
	NsImage        *image,
	nssize          x,
	nssize          y,
	nssize          z,
	const NsPixel  *pixel
	)
	{
	ns_assert( NULL != image );
	ns_assert( NULL != pixel );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_SET_PIXEL ) );

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_SET_PIXEL,
				NULL,
				NULL,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				x,
				y,
				z,
				pixel
				);	
	}


void ns_image_get_pixel_ex
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z,
	NsPixel        *pixel
	)
	{
	nsconstpointer slice, row;

	ns_assert( NULL != image );
	ns_assert( NULL != pixel );
	ns_assert( x < image->image_info.width );
	ns_assert( y < image->image_info.height );
	ns_assert( z < image->image_info.length );

	slice = NS_OFFSET_POINTER( const void, ns_image_pixels( image ), image->bytes_per_slice * z );
	row   = NS_OFFSET_POINTER( const void, slice, image->bytes_per_row * y );

	ns_pixel_row_get( pixel, image->pixel_info.pixel_type, row, x );
	}


void ns_image_set_pixel_ex
	(
	NsImage        *image,
	nssize          x,
	nssize          y,
	nssize          z,
	const NsPixel  *pixel
	)
	{
	nspointer slice, row;

	ns_assert( NULL != image );
	ns_assert( NULL != pixel );
	ns_assert( x < image->image_info.width );
	ns_assert( y < image->image_info.height );
    ns_assert( z < image->image_info.length );

    slice = NS_OFFSET_POINTER( void, ns_image_pixels( image ), image->bytes_per_slice * z );
    row   = NS_OFFSET_POINTER( void, slice, image->bytes_per_row * y );

    ns_pixel_row_set( pixel, image->pixel_info.pixel_type, row, x );
	}


NsError ns_image_convert
	(
	const NsImage  *src,
	NsPixelType     dest_pixel_type,
	nssize          dest_row_align,
	NsImage        *dest,
	NsProgress     *progress
	)
	{
	NsError error;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( src != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_CONVERT ) );

	if( src->pixel_info.pixel_type != dest_pixel_type ||
		 src->image_info.row_align != dest_row_align     )
		{
		if( NS_FAILURE( ns_image_create(
								dest,
								dest_pixel_type,
								src->image_info.width,
								src->image_info.height,
								src->image_info.length,
								dest_row_align
								),
								error ) )
			return error;

		return ns_proc_db_run(
					src->pixel_proc_db,
					NS_PIXEL_PROC_CONVERT,
					NULL,
					NULL,
					/* begin params */
					src->pixel_info.pixel_type,
					ns_image_pixels( src ),
					src->image_info.width,
					src->image_info.height,
					src->image_info.length,
					src->image_info.row_align,
					dest_pixel_type,
					ns_image_pixels( dest ),
					dest_row_align,
					progress
					);
		}
	else
		return ns_image_assign( dest, src, NULL, progress );
	}


NsError ns_image_convert_for_intensity_mask_palette
	(
	const NsImage  *src,
	NsPixelType     dest_pixel_type,
	nssize          dest_row_align,
	NsImage        *dest,
	NsProgress     *progress
	)
	{
	NsError error;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( src != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_CONVERT_FOR_INTENSITY_MASK_PALETTE ) );

	if( src->pixel_info.pixel_type != dest_pixel_type ||
		 src->image_info.row_align != dest_row_align     )
		{
		if( NS_FAILURE( ns_image_create(
								dest,
								dest_pixel_type,
								src->image_info.width,
								src->image_info.height,
								src->image_info.length,
								dest_row_align
								),
								error ) )
			return error;

		return ns_proc_db_run(
					src->pixel_proc_db,
					NS_PIXEL_PROC_CONVERT_FOR_INTENSITY_MASK_PALETTE,
					NULL,
					NULL,
					/* begin params */
					src->pixel_info.pixel_type,
					ns_image_pixels( src ),
					src->image_info.width,
					src->image_info.height,
					src->image_info.length,
					src->image_info.row_align,
					dest_pixel_type,
					ns_image_pixels( dest ),
					dest_row_align,
					progress
					);
		}
	else
		return ns_image_assign( dest, src, NULL, progress );
	}


#define _NS_IMAGE_REGION_SIZE( image, region, _width, _height, _length )\
	if( NULL != (region) )\
		{\
		(_width)  = (region)->width;\
		(_height) = (region)->height;\
		(_length) = (region)->length;\
		}\
	else\
		{\
		(_width)  = ns_image_width( (image) );\
		(_height) = ns_image_height( (image) );\
		(_length) = ns_image_length( (image) );\
		}


NsError ns_image_assign
	(
	NsImage              *dest,
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	nssize   width;
	nssize   height;
	nssize   length;
	NsError  error;


	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( src != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_COPY ) );

	_NS_IMAGE_REGION_SIZE( src, region, width, height, length );

	if( NS_FAILURE( ns_image_create(
							dest,
							src->pixel_info.pixel_type,
							width,
							height,
							length,
							src->image_info.row_align
							),
							error ) )
		return error;

	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_COPY,
				NULL,
				NULL,
				/* begin params */
				src->pixel_info.pixel_type,
				ns_image_pixels( src ),
				src->image_info.width,
				src->image_info.height,
				src->image_info.length,
				src->image_info.row_align,
				ns_image_pixels( dest ),
				region,
				progress
				);
	}


NsError ns_image_copy
	(
	NsImage              *dest,
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsProgress           *progress
	)
	{
	nssize  width;
	nssize  height;
	nssize  length;


	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( src != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_COPY ) );

	ns_assert( ns_image_pixel_type( dest ) == ns_image_pixel_type( src ) );
	ns_assert( ns_image_row_align( dest )  == ns_image_row_align( src ) );

	_NS_IMAGE_REGION_SIZE( src, region, width, height, length );

	ns_assert( ns_image_width( dest )  == width );
	ns_assert( ns_image_height( dest ) == height );
	ns_assert( ns_image_length( dest ) == length );

	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_COPY,
				NULL,
				NULL,
				/* begin params */
				src->pixel_info.pixel_type,
				ns_image_pixels( src ),
				src->image_info.width,
				src->image_info.height,
				src->image_info.length,
				src->image_info.row_align,
				ns_image_pixels( dest ),
				region,
				progress
				);
	}


void ns_image2d_copy_ex( NsImage *dest, const NsImage *src )
	{
	nssize           src_bytes_per_row, dest_bytes_per_row;
	nssize           y, height;
	const nsuint8   *src_row;
	nsuint8         *dest_row;


	ns_assert( NULL != dest );
	ns_assert( NULL != src );

	ns_assert( 1 == ns_image_length( dest ) );
	ns_assert( 1 == ns_image_length( src ) );

	src_bytes_per_row =
		ns_pixel_buffer_row_size(
			ns_image_pixel_type( src ),
			ns_image_width( src ),
			ns_image_row_align( src )
			);

	dest_bytes_per_row =
		ns_pixel_buffer_row_size(
			ns_image_pixel_type( dest ),
			ns_image_width( dest ),
			ns_image_row_align( dest )
			);

	ns_assert( src_bytes_per_row <= dest_bytes_per_row );
	ns_assert( ns_image_height( src ) == ns_image_height( dest ) );

	height   = ns_image_height( src );
	src_row  = ns_image_pixels( src );
	dest_row = ns_image_pixels( dest );

	for( y = 0; y < height; ++y )
		{
		ns_memcpy( dest_row, src_row, src_bytes_per_row );

		src_row  += src_bytes_per_row;
		dest_row += dest_bytes_per_row;
		}
	}


NsError ns_image_blur
	(
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsImage              *dest,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	NsError error;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_BLUR ) );

	if( src != dest )
		{
		/* NOTE: Small optimization. If the 'region' is NULL then
			there's no need to copy the original pixels. */
		if( NULL == region )
			{
			if( NS_FAILURE( ns_image_create(
									dest,
									src->pixel_info.pixel_type,
									src->image_info.width,
									src->image_info.height,
									src->image_info.length,
									src->image_info.row_align
									),
									error ) )
				return error;
			}
		else
			{
			if( NS_FAILURE( ns_image_assign( dest, src, NULL, NULL ), error ) )
				return error;
			}
		}

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( src=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", dest=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( src ),
		region,
		ns_image_get_name( dest ),
		progress,
		record
		);

	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_BLUR,
				NULL,
				record,
				src->pixel_info.pixel_type,
				ns_image_pixels( src ),
				src->image_info.width,
				src->image_info.height,
				src->image_info.length,
				src->image_info.row_align,
				ns_image_pixels( dest ),
				region,
				progress
				);
	}


NsError ns_image_brightness_contrast
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              brightness,
	nsdouble              contrast,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_BRIGHTNESS_CONTRAST ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", brightness=" NS_FMT_DOUBLE
		", contrast=" NS_FMT_DOUBLE
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		region,
		brightness,
		contrast,
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_BRIGHTNESS_CONTRAST,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				brightness,
				contrast,
				region,
				progress
				);	
	}


NsError ns_image_noise
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              level,
	nssize                iterations,
	nsdouble              average_intensity,
	nsdouble             *signal_to_noise_ratio,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_NOISE ) );
	ns_assert( NULL != signal_to_noise_ratio );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", level=" NS_FMT_DOUBLE
		", iterations=" NS_FMT_ULONG
		", average_intensity=" NS_FMT_DOUBLE
		", signal_to_noise_ratio=" NS_FMT_POINTER
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		region,
		level,
		iterations,
		average_intensity,
		signal_to_noise_ratio,
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_NOISE,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				level,
				iterations,
				average_intensity,
				signal_to_noise_ratio,
				region,
				progress
				);	
	}


NsError ns_image_gamma_correct
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              gamma,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_GAMMA_CORRECT ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", gamma=" NS_FMT_DOUBLE
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		region,
		gamma,
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_GAMMA_CORRECT,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				gamma,
				region,
				progress
				);	
	}


NS_PRIVATE NsPixelType _ns_image_ortho_project_pixel_type
	(
	const NsImage  *src,
	nsint           project_type
	)
	{
	NsPixelType pixel_type = NS_PIXEL_UNSUPPORTED;

	switch( project_type )
		{
		case NS_PIXEL_PROC_ORTHO_PROJECT_MAX:
			pixel_type = ns_image_pixel_type( src );
			break;

		case NS_PIXEL_PROC_ORTHO_PROJECT_MIN:
			pixel_type = ns_image_pixel_type( src );
			break;

		case NS_PIXEL_PROC_ORTHO_PROJECT_SUM:
			pixel_type = NS_PIXEL_LUM_F32;
			break;

		default:
			ns_assert_not_reached();
		}

	return pixel_type;
	}


NsError ns_image_ortho_project_create
	(
	const NsImage        *src,
	const NsPixelRegion  *region,
	nssize                dest_xy_row_align,
	NsImage              *dest_xy,
	nssize                dest_zy_row_align,
	NsImage              *dest_zy,
	nssize                dest_xz_row_align,
	NsImage              *dest_xz,
	nsenum                project_type,
	nsboolean             use_src_size
	)
	{
	nssize       width;
	nssize       height;
	nssize       length;
	NsPixelType  dest_pixel_type;
	NsError      error;


	ns_assert( NULL != src );
	ns_assert( NULL != dest_xy );
	ns_assert( NULL != dest_zy );
	ns_assert( NULL != dest_xz );

	if( use_src_size )
		{
		width  = ns_image_width( src );
		height = ns_image_height( src );
		length = ns_image_length( src );
		}
	else
		{
		_NS_IMAGE_REGION_SIZE( src, region, width, height, length );
		}

	dest_pixel_type = _ns_image_ortho_project_pixel_type( src, project_type );

	if( NS_FAILURE( ns_image_create(
							dest_xy,
							dest_pixel_type,
							width,
							height,
							1,
							dest_xy_row_align
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_image_create(
							dest_zy,
							dest_pixel_type,
							length,
							height,
							1,
							dest_zy_row_align
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_image_create(
							dest_xz,
							dest_pixel_type,
							width,
							length,
							1,
							dest_xz_row_align
							),
							error ) )
		return error;

	return ns_no_error();
	}


void ns_image_ortho_project_init
	(
	NsImage  *dest_xy,
	NsImage  *dest_zy,
	NsImage  *dest_xz,
	nsenum    project_type
	)
	{
	ns_assert( NULL != dest_xy );
	ns_assert( NULL != dest_zy );
	ns_assert( NULL != dest_xz );

	if( NS_PIXEL_PROC_ORTHO_PROJECT_MIN == project_type )
		{
		ns_image_memset( dest_xy, NS_UINT8_MAX );
		ns_image_memset( dest_zy, NS_UINT8_MAX );
		ns_image_memset( dest_xz, NS_UINT8_MAX );
		}
	else
		{
		ns_image_zero( dest_xy );
		ns_image_zero( dest_zy );
		ns_image_zero( dest_xz );
		}
	}


NsError ns_image_ortho_project
	(
	const NsImage        *src,
	const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
	nssize                dest_xy_row_align,
	NsImage              *dest_xy,
	nssize                dest_zy_row_align,
	NsImage              *dest_zy,
	nssize                dest_xz_row_align,
	NsImage              *dest_xz,
	nsenum                project_type,
	nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
	NsProgress           *progress
	)
	{
	ns_assert( NULL != src );
	ns_assert( NULL != dest_xy );
	ns_assert( NULL != dest_zy );
	ns_assert( NULL != dest_xz );
	ns_assert( NULL != average_intensity );
	ns_assert( NULL != num_pixels );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_ORTHO_PROJECT ) );

	if( NULL == region )
		{
		ns_assert( ns_image_width( dest_xy )  == ns_image_width( src ) );
		ns_assert( ns_image_height( dest_xy ) == ns_image_height( src ) );
		ns_assert( ns_image_width( dest_zy )  == ns_image_length( src ) );
		ns_assert( ns_image_height( dest_zy ) == ns_image_height( src ) );
		ns_assert( ns_image_width( dest_xz )  == ns_image_width( src ) );
		ns_assert( ns_image_height( dest_xz ) == ns_image_length( src ) );
		}

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( src=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", z_indices=" NS_FMT_POINTER
		", num_z_indices=" NS_FMT_ULONG
		", dest_xy_row_align=" NS_FMT_ULONG
		", dest_xy=" NS_FMT_STRING_DOUBLE_QUOTED
		", dest_zy_row_align=" NS_FMT_ULONG
		", dest_zy=" NS_FMT_STRING_DOUBLE_QUOTED
		", dest_xz_row_align=" NS_FMT_ULONG
		", dest_xz=" NS_FMT_STRING_DOUBLE_QUOTED
		", project_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", average_intensity=" NS_FMT_POINTER
		", num_pixels=" NS_FMT_POINTER
		", use_src_size=" NS_FMT_INT
		", progress=" NS_FMT_POINTER
		" )",
		ns_image_get_name( src ),
		region,
		z_indices,
		num_z_indices,
		dest_xy_row_align,
		ns_image_get_name( dest_xy ),
		dest_zy_row_align,
		ns_image_get_name( dest_zy ),
		dest_xz_row_align,
		ns_image_get_name( dest_xz ),
		ns_pixel_proc_ortho_project_type_to_string( project_type ),
		average_intensity,
		num_pixels,
		use_src_size,
		progress
		);

	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_ORTHO_PROJECT,
				NULL,
				NULL,
				/* begin params */
				ns_image_pixel_type( src ),
				ns_image_pixels( src ),
				ns_image_width( src ),
				ns_image_height( src ),
				ns_image_length( src ),
				ns_image_row_align( src ),
				project_type,
				_ns_image_ortho_project_pixel_type( src, project_type ),
				ns_image_pixels( dest_xy ),
				ns_image_row_align( dest_xy ),
				ns_image_pixels( dest_zy ),
				ns_image_row_align( dest_zy ),
				ns_image_pixels( dest_xz ),
				ns_image_row_align( dest_xz ),
				region,
				z_indices,
				num_z_indices,
				average_intensity,
				num_pixels,
				use_src_size,
				progress
				);
	}


NsError ns_image_dynamic_range
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsfloat               channel_min,
	nsfloat               channel_max,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_DYNAMIC_RANGE ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", channel_min=" NS_FMT_DOUBLE
		", channel_max=" NS_FMT_DOUBLE
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		region,
		channel_min,
		channel_max,
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_DYNAMIC_RANGE,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				channel_min,
				channel_max,
				region,
				progress
				);	
	}


NsError ns_image_mp_filter
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsint                 type,
	nssize                num_cpu,
	NsProgress           *progress,
	NsClosureRecord      *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_MP_FILTER ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", region=" NS_FMT_POINTER
		", type=" NS_FMT_STRING_DOUBLE_QUOTED
		", num_cpu=" NS_FMT_ULONG
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		region,
		ns_pixel_proc_mp_filter_type_to_string( type ),
		num_cpu,
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_MP_FILTER,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				type,
				num_cpu,
				region,
				progress
				);	
	}


NsError ns_image_resize
	(
	const NsImage          *src,
	NsPixelProcResizeType   interp_type,
	nssize                  width,
	nssize                  height,
	nssize                  length,
	nssize                  row_align,
	NsImage                *dest,
	NsProgress             *progress,
	NsClosureRecord        *record
	)
	{
	NsError error;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( src != dest );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_RESIZE ) );

	ns_assert( 0 < width );
	ns_assert( 0 < height );
	ns_assert( 0 < length );

	if( NS_FAILURE( ns_image_create(
							dest,
							src->pixel_info.pixel_type,
							width,
							height,
							length,
							row_align
							),
							error ) )
		return error;

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( src=" NS_FMT_STRING_DOUBLE_QUOTED
		", interp_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", row_align=" NS_FMT_ULONG
		", dest=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( src ),
		ns_pixel_proc_resize_type_to_string( interp_type ),
		width,
		height,
		length,
		row_align,
		ns_image_get_name( dest ),
		progress,
		record
		);
							
	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_RESIZE,
				NULL,
				record,
				src->pixel_info.pixel_type,
				ns_image_pixels( src ),
				src->image_info.width,
				src->image_info.height,
				src->image_info.length,
				src->image_info.row_align,
				interp_type,
				ns_image_pixels( dest ),
				dest->image_info.width,
				dest->image_info.height,
				dest->image_info.length,
				dest->image_info.row_align,
				progress
				);
	}


NsError ns_image_flip_vertical
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_FLIP ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_FLIP,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				NS_PIXEL_PROC_FLIP_VERTICAL,
				progress
				);
	}


NsError ns_image_flip_horizontal
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_FLIP ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_FLIP,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				NS_PIXEL_PROC_FLIP_HORIZONTAL,
				progress
				);
	}


NsError ns_image_flip_optical_axis
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, NS_PIXEL_PROC_FLIP ) );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( image=" NS_FMT_STRING_DOUBLE_QUOTED
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( image ),
		progress,
		record
		);

	return ns_proc_db_run(
				image->pixel_proc_db,
				NS_PIXEL_PROC_FLIP,
				NULL,
				record,
				/* begin params */
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				NS_PIXEL_PROC_FLIP_OPTICAL_AXIS,
				progress
				);
	}


#include <ext/subsample.h>

NsError ns_image_subsample
	(
	const NsImage    *src,
	NsImage          *dest,
	nssize            dest_row_align,
	nsdouble          scale_x,
	nsdouble          scale_y,
	nsdouble          scale_z,
	NsProgress       *progress,
	NsClosureRecord  *record
	)
	{
	nsint    dest_width, dest_height, dest_length;
	NsError  error;


	ns_assert( NULL != src );
	ns_assert( ns_image_has_pixel_proc_db( src ) );
	ns_assert( ns_proc_db_is_registered( src->pixel_proc_db, NS_PIXEL_PROC_SUBSAMPLE ) );
	ns_assert( NULL != dest );
	ns_assert( 0.0 <= scale_x && scale_x <= 100.0 );
	ns_assert( 0.0 <= scale_y && scale_y <= 100.0 );
	ns_assert( 0.0 <= scale_z && scale_z <= 100.0 );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( src=" NS_FMT_STRING_DOUBLE_QUOTED
		", dest=" NS_FMT_STRING_DOUBLE_QUOTED
		", scale={" NS_FMT_DOUBLE "," NS_FMT_DOUBLE "," NS_FMT_DOUBLE "}"
		", progress=" NS_FMT_POINTER
		", record=" NS_FMT_POINTER
		" )",
		ns_image_get_name( src ),
		ns_image_get_name( dest ),
		scale_x, scale_y, scale_z,
		progress,
		record
		);

	SubsamplerSizes(
		( nsint )ns_image_width( src ),
		( nsint )ns_image_height( src ),
		( nsint )ns_image_length( src ),
		scale_x / 100.0,
		scale_y / 100.0,
		scale_z / 100.0,
		&dest_width,
		&dest_height,
		&dest_length
		);

	if( NS_FAILURE(
			ns_image_create(
				dest,
				ns_image_pixel_type( src ),
				( nssize )dest_width,
				( nssize )dest_height,
				( nssize )dest_length,
				dest_row_align
				),
			error ) )
		return error;
				
	return ns_proc_db_run(
				src->pixel_proc_db,
				NS_PIXEL_PROC_SUBSAMPLE,
				NULL,
				record,
				/* begin params */
				src->pixel_info.pixel_type,
				ns_image_pixels( src ),
				src->image_info.width,
				src->image_info.height,
				src->image_info.length,
				src->image_info.row_align,
				ns_image_pixels( dest ),
				dest->image_info.width,
				dest->image_info.height,
				dest->image_info.length,
				dest->image_info.row_align,
				scale_x,
				scale_y,
				scale_z,
				progress
				);
	}


void ns_image_zero( NsImage *image )
	{
	ns_assert( NULL != image );
	ns_byte_array_memzero( &image->pixels );
	}


void ns_image_memset( NsImage *image, nsuint8 value )
	{
	ns_assert( NULL != image );
	ns_byte_array_memset( &image->pixels, value );
	}


NsError ns_image_pixel_proc_invoke
	(
	NsImage       *image,
	const nschar  *name,
	NsValue       *params,
	nssize         num_params,
	NsValue       *ret_value
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, name ) );

	return ns_proc_db_invoke( image->pixel_proc_db, name, params, num_params, ret_value, NULL );
	}


NsError ns_image_pixel_proc_run
	(
	NsImage       *image,
	const nschar  *name,
	NsValue       *ret_value,
	...
	)
	{
	ns_va_list  args;
	NsError     error;


	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, name ) );

	ns_va_start( args, ret_value );
	error = ns_proc_db_vrun( image->pixel_proc_db, name, args, ret_value, NULL );
	ns_va_end( args );

	return error;
	}


NsError ns_image_pixel_proc_vrun
	(
	NsImage       *image,
	const nschar  *name,
	ns_va_list     args,
	NsValue       *ret_value
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_pixel_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->pixel_proc_db, name ) );

	return ns_proc_db_vrun( image->pixel_proc_db, name, args, ret_value, NULL );
	}




NsError ns_image_add_string( NsList *strings, const nschar *src_string )
	{
	nschar   *dest_string;
	NsError   error;


	ns_assert( NULL != strings );
	ns_assert( NULL != src_string );

	if( NULL == ( dest_string = ns_ascii_strdup( src_string ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_list_push_back( strings, dest_string ), error ) )
		{
		ns_free( dest_string );
		return error;
		}

	return ns_no_error();
	}


void ns_image_clear_strings( NsList *strings )
	{
	ns_assert( NULL != strings );
	ns_list_clear( strings );
	}


void ns_image_remove_string( NsList *strings, nslistiter I )
	{
	ns_assert( NULL != strings );
	ns_list_erase( strings, I );
	}


nslistiter ns_image_begin_strings( const NsList *strings )
	{
	ns_assert( NULL != strings );
	return ns_list_begin( strings );
	}


nslistiter ns_image_end_strings( const NsList *strings )
	{
	ns_assert( NULL != strings );
	return ns_list_end( strings );
	}


nsboolean ns_image_has_string( const NsList *strings, const nschar *string )
	{
	nslistiter curr, end;

	ns_assert( NULL != strings );
	ns_assert( NULL != string );

	curr = ns_image_begin_strings( strings );
	end  = ns_image_end_strings( strings );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		if( ns_ascii_streq( ns_list_iter_get_object( curr ), string ) )
			return NS_TRUE;

	return NS_FALSE;
	}




#include "nsio-writeraw.h"
#include "nsio-writetiff.h"
#include "nsio-writejpeg.h"


NsError ns_image_io_proc_db_register_std( NsProcDb *db )
	{
	NsError error;

	ns_assert( NULL != db );

	if( NS_FAILURE( ns_pixel_register_value_types(), error ) )
		return error;

	if( ! ns_proc_db_is_registered( db, NS_IO_PROC_WRITE_RAW ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_io_proc_write_raw() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_IO_PROC_WRITE_TIFF ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_io_proc_write_tiff() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_IO_PROC_WRITE_JPEG ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_io_proc_write_jpeg() ), error ) )
			return error;

	return ns_no_error();	
	}


NsError ns_image_write_raw
	(
	const NsImage        *image,
	const NsPixelRegion  *region,
	const nschar         *file_name,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_io_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->io_proc_db, NS_IO_PROC_WRITE_RAW ) );
	ns_assert( NULL != file_name );

	return ns_proc_db_run(
				image->io_proc_db,
				NS_IO_PROC_WRITE_RAW,
				NULL,
				NULL,
				/* begin params */
				file_name,
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				region,
				conv_pixel_type,
				progress
				);
	}


NsError ns_image_write_tiff
	(
	const NsImage        *image,
	const NsPixelRegion  *region,
	const nschar         *file_name,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_io_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->io_proc_db, NS_IO_PROC_WRITE_TIFF ) );
	ns_assert( NULL != file_name );

	return ns_proc_db_run(
				image->io_proc_db,
				NS_IO_PROC_WRITE_TIFF,
				NULL,
				NULL,
				/* begin params */
				file_name,
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				region,
				conv_pixel_type,
				progress
				);
	}


NsError ns_image_write_jpeg
	(
	const NsImage  *image,
	const nschar   *file_name,
	nsint           quality,
	NsProgress     *progress
	)
	{
	ns_assert( NULL != image );
	ns_assert( ns_image_has_io_proc_db( image ) );
	ns_assert( ns_proc_db_is_registered( image->io_proc_db, NS_IO_PROC_WRITE_JPEG ) );
	ns_assert( NULL != file_name );

	if( 1 != ns_image_length( image ) )
		return ns_error_noimp( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	ns_assert( 0 <= quality && quality <= 100 );

	return ns_proc_db_run(
				image->io_proc_db,
				NS_IO_PROC_WRITE_JPEG,
				NULL,
				NULL,
				/* begin params */
				file_name,
				image->pixel_info.pixel_type,
				ns_image_pixels( image ),
				image->image_info.width,
				image->image_info.height,
				image->image_info.length,
				image->image_info.row_align,
				quality,
				progress
				);
	}




NS_PRIVATE nsimageiter _ns_image_iter( const NsImage *image )
	{
	nsimageiter I;

	ns_assert( 0 < image->image_info.width );
	ns_assert( 0 < image->image_info.height );
	ns_assert( 0 < image->image_info.length );

	I.image = ( NsImage* )image;
	return I;
	}


nsimageiter ns_image_begin( const NsImage *image )
	{
	nsimageiter I;

	ns_assert( NULL != image );
	I = _ns_image_iter( image );

	I.x = 0;
	I.y = 0;
	I.z = 0;

	return I;
	}


nsimageiter ns_image_end( const NsImage *image )
	{
	nsimageiter I;

	ns_assert( NULL != image );
	I = _ns_image_iter( image );

	I.x = 0;
	I.y = 0;
	I.z = image->image_info.length;

	return I;
	}


nsimagereviter ns_image_rev_begin( const NsImage *image )
	{
	nsimagereviter I;

	ns_assert( NULL != image );
	I = _ns_image_iter( image );

	I.x = image->image_info.width  - 1;
	I.y = image->image_info.height - 1;
	I.z = image->image_info.length - 1;

	return I;
	}


nsimagereviter ns_image_rev_end( const NsImage *image )
	{
	ns_assert( NULL != image );
	return ns_image_end( image );
	}


nsimageiter ns_image_at
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z
	)
	{
	nsimageiter I;

	ns_assert( NULL != image );
	I = _ns_image_iter( image );

	ns_assert( x < image->image_info.width );
	ns_assert( y < image->image_info.height );
	ns_assert( z < image->image_info.length );

	I.x = x;
	I.y = y;
	I.z = z;

	return I;
	}


NsPixel* ns_image_iter_get_pixel( const nsimageiter I, NsPixel *pixel )
	{
	ns_assert( NULL != pixel );
	
	ns_image_get_pixel( I.image, I.x, I.y, I.z, pixel );
	return pixel;
	}


void ns_image_iter_set_pixel( nsimageiter I, const NsPixel *pixel )
	{
	ns_assert( NULL != pixel );
	ns_image_set_pixel( I.image, I.x, I.y, I.z, pixel );
	}


nsimageiter ns_image_iter_next( nsimageiter I )
	{
	ns_assert( I.x < I.image->image_info.width );
	ns_assert( I.y < I.image->image_info.height );
	ns_assert( I.z < I.image->image_info.length );

	++I.x;

	if( I.image->image_info.width <= I.x )
		{
		I.x = 0;
		++I.y;
		}

	if( I.image->image_info.height <= I.y )
		{
		I.y = 0;
		++I.z;
		}

	return I;
	}


nsimageiter ns_image_iter_prev( nsimageiter I )
	{
	if( 0 == I.x )
		{
		I.x = I.image->image_info.width - 1;

		if( 0 == I.y )
			{
			I.y = I.image->image_info.height - 1;

			if( 0 == I.z )
				I = ns_image_end( I.image );
			else
				--I.z;
			}
		else
			--I.y;
		}
	else
		--I.x;

	return I;
	}


nsimageiter ns_image_iter_offset( nsimageiter I, nslong signed_offset )
	{
	nssize offset, width, height, dx, dy, dz;

	width  = I.image->image_info.width;
	height = I.image->image_info.height;

	if( 0 <= signed_offset )
		offset = ( nssize )signed_offset;
	else
		offset = ( nssize )( -signed_offset );

	/* NOTE: Integer division! */
	dz      = offset / ( width * height );
	offset -= dz * ( width * height );
	dy      = offset / width;
	offset -= dy * width;
	dx      = offset;

	if( 0 <= signed_offset )
		{
		ns_assert( I.x + dx < width );
		ns_assert( I.y + dy < height );
		ns_assert( I.z + dz < I.image->image_info.length );		

		I.x += dx;
		I.y += dy;
		I.z += dz;
		}
	else
		{
		ns_assert( dx <= I.x );
		ns_assert( dy <= I.y );
		ns_assert( dz <= I.z );

		I.x -= dx;
		I.y -= dy;
		I.z -= dz;
		}

	return I;
	}


nsboolean ns_image_iter_less( const nsimageiter I1, const nsimageiter I2 )
	{
	ns_assert( I1.image == I2.image );

	if( I1.z < I2.z )
		return NS_TRUE;
	else if( I1.z == I2.z )
		{
		if( I1.y < I2.y )
			return NS_TRUE;
		else if( I1.y == I2.y )
			{
			if( I1.x < I2.x )
				return NS_TRUE;
			}
		}

	return NS_FALSE;
	}


nsimageiter ns_image_iter_next_x( nsimageiter I )
	{
	ns_assert( I.x < I.image->image_info.width );

	++I.x;
	return I;
	}


nsimageiter ns_image_iter_prev_x( nsimageiter I )
	{
	ns_assert( 0 < I.x );

	--I.x;
	return I;
	}


nsimageiter ns_image_iter_next_y( nsimageiter I )
	{
	ns_assert( I.y < I.image->image_info.height );

	++I.y;
	return I;
	}


nsimageiter ns_image_iter_prev_y( nsimageiter I )
	{
	ns_assert( 0 < I.y );

	--I.y;
	return I;
	}


nsimageiter ns_image_iter_next_z( nsimageiter I )
	{
	ns_assert( I.z < I.image->image_info.length );

	++I.z;
	return I;
	}


nsimageiter ns_image_iter_prev_z( nsimageiter I )
	{
	ns_assert( 0 < I.z );

	--I.z;
	return I;
	}


nsimageiter ns_image_iter_set_x( nsimageiter I, nssize x )
	{
	ns_assert( x < I.image->image_info.width );

	I.x = x;
	return I;
	}


nsimageiter ns_image_iter_set_y( nsimageiter I, nssize y )
	{
	ns_assert( y < I.image->image_info.height );

	I.y = y;
	return I;
	}


nsimageiter ns_image_iter_set_z( nsimageiter I, nssize z )
	{
	ns_assert( z < I.image->image_info.length );

	I.z = z;
	return I;
	}


nsimageiter ns_image_iter_set_xyz( nsimageiter I, nssize x, nssize y, nssize z )
	{
	ns_assert( x < I.image->image_info.width );
	ns_assert( y < I.image->image_info.height );
	ns_assert( z < I.image->image_info.length );

	I.x = x;
	I.y = y;
	I.z = z;

	return I;
	}
