#ifndef __NS_IMAGE_IMAGE_H__
#define __NS_IMAGE_IMAGE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsbytearray.h>
#include <std/nslist.h>
#include <std/nsascii.h>
#include <std/nsprogress.h>
#include <std/nsprocdb.h>
#include <std/nsstring.h>
#include <std/nsfile.h>
#include <std/nsutil.h>
#include <std/nslog.h>
#include <math/nspoint.h>
#include <image/nscolor.h>
#include <image/nspixels.h>
#include <image/nspixels-orthoproject.h>
#include <image/nspixels-mpfilter.h>
#include <image/nspixels-resize.h>
#include <image/nspixels-flip.h>
#include <image/nspixels-copy.h>
#include <image/nspixels-subsample.h>

NS_DECLS_BEGIN

typedef struct _NsImageInfo
	{
	nssize  width;
	nssize  height;
	nssize  length;         /* Should be 1 for 2D images */
	nssize  row_align;      /* Must be power of 2 */
	nsint   endian_type;
	nssize  header_length;  /* Only used for raw files */
	}
	NsImageInfo;


typedef struct _NsImage
	{
	NsString      name;
	NsImageInfo   image_info;
	NsPixelInfo   pixel_info;
	NsByteArray   pixels;
	NsProcDb     *pixel_proc_db;
	NsProcDb     *io_proc_db;
	nspointer     user_data;
	nssize        bytes_per_row;
	nssize        bytes_per_slice;
	nsulong       flags;
	}
	NsImage;


NS_IMPEXP void ns_image_construct( NsImage *image );
NS_IMPEXP void ns_image_destruct( NsImage *image );

NS_IMPEXP void ns_image_clear( NsImage *image );

NS_IMPEXP const nschar* ns_image_get_name( const NsImage *image );
NS_IMPEXP NsError ns_image_set_name( NsImage *image, const nschar *name );

NS_IMPEXP NsError ns_image_create
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align
	);


typedef enum
	{
	NS_IMAGE_BUFFER_RESIZABLE,
	NS_IMAGE_BUFFER_GROW_ONLY
	};

NS_IMPEXP void ns_image_set_buffer_type( NsImage *image, nsint type );


/* IMPORTANT: The 'image' will free the 'pixels' when it
	is cleared or destroyed! */
NS_IMPEXP void ns_image_buffer_and_free
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nspointer     pixels
	);

/* IMPORTANT: The 'image' will just release the 'pixels' when
	it is cleared or destroyed, i.e. no free'ing. */
NS_IMPEXP void ns_image_buffer_and_release
	(
	NsImage      *image,
	NsPixelType   pixel_type,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nspointer     pixels
	);


/* Clears the image but does NOT free the pixels. */
NS_IMPEXP void ns_image_release( NsImage *image );


NS_IMPEXP NsProcDb* ns_image_get_pixel_proc_db( const NsImage *image );

/* Can pass NULL for 'pixel_proc_db'. */
NS_IMPEXP void ns_image_set_pixel_proc_db( NsImage *image, NsProcDb *pixel_proc_db );

NS_IMPEXP nsboolean ns_image_has_pixel_proc_db( const NsImage *image );

NS_IMPEXP NsProcDb* ns_image_get_io_proc_db( const NsImage *image );

/* Can pass NULL for 'io_proc_db'. */
NS_IMPEXP void ns_image_set_io_proc_db( NsImage *image, NsProcDb *io_proc_db );

NS_IMPEXP nsboolean ns_image_has_io_proc_db( const NsImage *image );

NS_IMPEXP nssize ns_image_width( const NsImage *image );
NS_IMPEXP nssize ns_image_height( const NsImage *image );
NS_IMPEXP nssize ns_image_length( const NsImage *image );

NS_IMPEXP nssize ns_image_row_align( const NsImage *image );

NS_IMPEXP NsPixelType ns_image_pixel_type( const NsImage *image );

NS_IMPEXP NsPixelFormatType ns_image_pixel_format_type( const NsImage *image );

NS_IMPEXP nssize ns_image_pixel_num_channels( const NsImage *image );

NS_IMPEXP NsPixelChannelType ns_image_pixel_channel_type( const NsImage *image, nssize channel );
NS_IMPEXP nssize ns_image_pixel_channel_bits( const NsImage *image, nssize channel );

NS_IMPEXP nspointer ns_image_pixels( const NsImage *image );

NS_IMPEXP nspointer ns_image_pixels_slice( const NsImage *image, nssize z );
NS_IMPEXP nspointer ns_image_pixels_row( const NsImage *image, nssize y, nssize z );

/* Returns size in bytes! */
NS_IMPEXP nssize ns_image_size( const NsImage *image );

NS_IMPEXP nspointer ns_image_get_user_data( const NsImage *image );
NS_IMPEXP void ns_image_set_user_data( NsImage *image, nspointer user_data );

NS_IMPEXP NsError ns_image_get_pixel
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z,
	NsPixel        *pixel
	);

NS_IMPEXP NsError ns_image_set_pixel
	(
	NsImage        *image,
	nssize          x,
	nssize          y,
	nssize          z,
	const NsPixel  *pixel
	);


/* Faster versions of get/set pixel. Not as much
	error checking. */
NS_IMPEXP void ns_image_get_pixel_ex
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z,
	NsPixel        *pixel
	);

NS_IMPEXP void ns_image_set_pixel_ex
	(
	NsImage        *image,
	nssize          x,
	nssize          y,
	nssize          z,
	const NsPixel  *pixel
	);

/* dest = src converted to type, 'progress' can be NULL. */
NS_IMPEXP NsError ns_image_convert
	(
	const NsImage  *src,
	NsPixelType     dest_pixel_type,
	nssize          dest_row_align,
	NsImage        *dest,
	NsProgress     *progress
	);


/* dest = src converted to type, 'progress' can be NULL. This one
	is special in that it preserves the true and maximum intensity
	pixels.

	For example in 12-bit to 8-bit normal conversion the lower 16
	values in the 12-bit data all get mapped to 0 in the 8-bit image.
	In this routine only a 12-bit 0 gets mapped to an 8-bit 0. The 
	12-bit values 1 - 15 get mapped to an 8-bit 1.
*/
NS_IMPEXP NsError ns_image_convert_for_intensity_mask_palette
	(
	const NsImage  *src,
	NsPixelType     dest_pixel_type,
	nssize          dest_row_align,
	NsImage        *dest,
	NsProgress     *progress
	);


/* dest = src, 'progress' can be NULL.
	IMPORTANT: 'dest' is cleared before
	the assignment is made. */
NS_IMPEXP NsError ns_image_assign
	(
	NsImage              *dest,
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


/* dest = src, BUT the 'dest' image is NOT cleared beore the assignment.
	That means 'dest' must be properly set up to be equal to 'src'! */
NS_IMPEXP NsError ns_image_copy
	(
	NsImage              *dest,
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


/* TEMP? dest = src. Fast copy. No clearing of the destination image. The
	function goes row by row copying pixels. The only requirement is
	that the number of bytes per row of the source image is less than or
	equal to the number of bytes per row of the destination image, and that
	the heights are equal. */
NS_IMPEXP void ns_image2d_copy_ex( NsImage *dest, const NsImage *src );


/* If src == dest, and "in-place" blur will occur. */
NS_IMPEXP NsError ns_image_blur
	(
	const NsImage        *src,
	const NsPixelRegion  *region,
	NsImage              *dest,
	NsProgress           *progress,
	NsClosureRecord      *record
	);

NS_IMPEXP NsError ns_image_brightness_contrast
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              brightness,
	nsdouble              contrast,
	NsProgress           *progress,
	NsClosureRecord      *record
	);

NS_IMPEXP NsError ns_image_noise
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              level,
	nssize                iterations,
	nsdouble              average_intensity,
	nsdouble             *signal_to_noise_ratio,
	NsProgress           *progress,
	NsClosureRecord      *record
	);

NS_IMPEXP NsError ns_image_gamma_correct
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsdouble              gamma,
	NsProgress           *progress,
	NsClosureRecord      *record
	);


/* This creates the destination images in preparation
	for the projection. If 'use_src_size' is true then
	the size of the projection is determined from the
	'src' image not the 'region'. */
NS_IMPEXP NsError ns_image_ortho_project_create
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
	);

/* Sets the destination pixels to an appropriate
	value based on the projection type, e.g. all
	0 for a maximal projection. */
NS_IMPEXP void ns_image_ortho_project_init
	(
	NsImage  *dest_xy,
	NsImage  *dest_zy,
	NsImage  *dest_xz,
	nsenum    project_type
	);


/* NOTE: If 'z_indices' is not NULL then it is an array of
	z locations to run the projections instead of on the whole
	image. The indices MUST fall within the 'region', that is
	if there is one (i.e. not NULL). */
NS_IMPEXP NsError ns_image_ortho_project
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
	);


NS_IMPEXP NsError ns_image_dynamic_range
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsfloat               channel_min,
	nsfloat               channel_max,
	NsProgress           *progress,
	NsClosureRecord      *record
	);

NS_IMPEXP NsError ns_image_mp_filter
	(
	NsImage              *image,
	const NsPixelRegion  *region,
	nsint                 type,
	nssize                num_cpu,
	NsProgress           *progress,
	NsClosureRecord      *record
	);

NS_IMPEXP NsError ns_image_resize
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
	);

NS_IMPEXP NsError ns_image_flip_vertical
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	);

NS_IMPEXP NsError ns_image_flip_horizontal
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	);

NS_IMPEXP NsError ns_image_flip_optical_axis
	(
	NsImage          *image,
	NsProgress       *progress,
	NsClosureRecord  *record
	);

/* The scale factors must range from [0,100].
	i.e. percentage values. */
NS_IMPEXP NsError ns_image_subsample
	(
	const NsImage    *src,
	NsImage          *dest,
	nssize            dest_row_align,
	nsdouble          scale_x,
	nsdouble          scale_y,
	nsdouble          scale_z,
	NsProgress       *progress,
	NsClosureRecord  *record
	);


/* All pixel bits to zero. */
NS_IMPEXP void ns_image_zero( NsImage *image );

NS_IMPEXP void ns_image_memset( NsImage *image, nsuint8 value );

NS_IMPEXP NsError ns_image_pixel_proc_invoke
	(
	NsImage       *image,
	const nschar  *name,
	NsValue       *params,
	nssize         num_params,
	NsValue       *ret_value
	);

NS_IMPEXP NsError ns_image_pixel_proc_run
	(
	NsImage       *image,
	const nschar  *name,
	NsValue       *ret_value,
	...
	);

NS_IMPEXP NsError ns_image_pixel_proc_vrun
	(
	NsImage       *image,
	const nschar  *name,
	ns_va_list     args,
	NsValue       *ret_value
	);




NS_IMPEXP NsError ns_image_add_string( NsList *strings, const nschar *string );

NS_IMPEXP void ns_image_clear_strings( NsList *strings );

NS_IMPEXP void ns_image_remove_string( NsList *strings, nslistiter I );

/* NOTE: Convert ...get_object() call to a const nschar*. */
NS_IMPEXP nslistiter ns_image_begin_strings( const NsList *strings );
NS_IMPEXP nslistiter ns_image_end_strings( const NsList *strings );

NS_IMPEXP nsboolean ns_image_has_string( const NsList *strings, const nschar *string );




/* Standard procedures. See nsimage-io-procs.h as well. */

/* Load up the above standard procedures. */
NS_IMPEXP NsError ns_image_io_proc_db_register_std( NsProcDb *db );


/* Pass NS_PIXEL_UNSUPPORTED for 'conv_pixel_type' if no pixel format
	conversion required when writing. */
NS_IMPEXP NsError ns_image_write_raw
	(
	const NsImage        *image,
	const NsPixelRegion  *region,
	const nschar         *file_name,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	);

/* Pass NS_PIXEL_UNSUPPORTED for 'conv_pixel_type' if no pixel format
	conversion required when writing. */
NS_IMPEXP NsError ns_image_write_tiff
	(
	const NsImage        *image,
	const NsPixelRegion  *region,
	const nschar         *file_name,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	);

/* 'quality' ranges from 0 to 100. */
NS_IMPEXP NsError ns_image_write_jpeg
	(
	const NsImage  *image,
	const nschar   *file_name,
	nsint           quality,
	NsProgress     *progress
	);


typedef struct _nsimageiter
	{
	NsImage  *image;
	nssize    x;
	nssize    y;
	nssize    z;
	}
	nsimageiter;

typedef nsimageiter nsimagereviter;


NS_IMPEXP nsimageiter ns_image_begin( const NsImage *image );
NS_IMPEXP nsimageiter ns_image_end( const NsImage *image );

NS_IMPEXP nsimagereviter ns_image_rev_begin( const NsImage *image );
NS_IMPEXP nsimagereviter ns_image_rev_end( const NsImage *image );

NS_IMPEXP nsimageiter ns_image_at
	(
	const NsImage  *image,
	nssize          x,
	nssize          y,
	nssize          z
	);

#define ns_image_iter_equal( I1, I2 )\
	( (I1).x == (I2).x && (I1).y == (I2).y && (I1).z == (I2).z )

#define ns_image_iter_get_x( I )\
	(I).x

#define ns_image_iter_get_y( I )\
	(I).y

#define ns_image_iter_get_z( I )\
	(I).z

NS_IMPEXP NsPixel* ns_image_iter_get_pixel( const nsimageiter I, NsPixel *pixel );
NS_IMPEXP void ns_image_iter_set_pixel( nsimageiter I, const NsPixel *pixel );

NS_IMPEXP nsimageiter ns_image_iter_next( nsimageiter I ); /* ++ */
NS_IMPEXP nsimageiter ns_image_iter_prev( nsimageiter I ); /* -- */

NS_IMPEXP nsimageiter ns_image_iter_offset( nsimageiter I, nslong offset ); /* += */

NS_IMPEXP nsboolean ns_image_iter_less( const nsimageiter I1, const nsimageiter I2 ); /* < */

NS_IMPEXP nsimageiter ns_image_iter_next_x( nsimageiter I ); /* ++x */
NS_IMPEXP nsimageiter ns_image_iter_prev_x( nsimageiter I ); /* --x */

NS_IMPEXP nsimageiter ns_image_iter_next_y( nsimageiter I ); /* ++y */
NS_IMPEXP nsimageiter ns_image_iter_prev_y( nsimageiter I ); /* --y */

NS_IMPEXP nsimageiter ns_image_iter_next_z( nsimageiter I ); /* ++z */
NS_IMPEXP nsimageiter ns_image_iter_prev_z( nsimageiter I ); /* --z */

NS_IMPEXP nsimageiter ns_image_iter_set_x( nsimageiter I, nssize x );
NS_IMPEXP nsimageiter ns_image_iter_set_y( nsimageiter I, nssize y );
NS_IMPEXP nsimageiter ns_image_iter_set_z( nsimageiter I, nssize z );

NS_IMPEXP nsimageiter ns_image_iter_set_xyz( nsimageiter I, nssize x, nssize y, nssize z );

#define ns_image_iter_not_equal( I1, I2 )\
	( ! ns_image_iter_equal( (I1), (I2) ) )

#define ns_image_iter_less_equal( I1, I2 )\
   ( ns_image_iter_less( (I1), (I2) ) || ns_image_iter_equal( (I1), (I2) ) )

#define ns_image_iter_greater( I1, I2 )\
   ( ! ns_image_iter_less_equal( (I1), (I2) ) )

#define ns_image_iter_greater_equal( I1, I2 )\
	( ! ns_image_iter_less( (I1), (I2) ) )

#define ns_image_rev_iter_equal( I1, I2 )\
	ns_image_iter_equal( (I1), (I2) )

#define ns_image_rev_iter_not_equal( I1, I2 )\
	( ! ns_image_rev_iter_equal( (I1), (I2) ) )

#define ns_image_rev_iter_get_pixel( I, pixel )\
	ns_image_iter_get_pixel( (I), (pixel) )

#define ns_image_rev_iter_set_pixel( I, pixel )\
	ns_image_iter_set_pixel( (I), (pixel) )

#define ns_image_rev_iter_next( I )\
	ns_image_iter_prev( (I) )

#define ns_image_rev_iter_prev( I )\
	ns_image_iter_next( (I) )

#define ns_image_rev_iter_offset( I, offset )\
	ns_image_iter_offset( (I), -(offset) )

NS_DECLS_END

#endif/* __NS_IMAGE_IMAGE_H__ */
