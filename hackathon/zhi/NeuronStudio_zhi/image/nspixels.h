#ifndef __NS_IMAGE_PIXELS_H__
#define __NS_IMAGE_PIXELS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsbits.h>
#include <std/nsmemory.h>
#include <std/nsfunc.h>
#include <std/nsascii.h>
#include <std/nsvalue.h>
#include <std/nshashtable.h>
#include <std/nsenumeration.h>
#include <std/nsprogress.h>
#include <std/nsprocdb.h>
#include <std/nslog.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef enum
   {
   NS_PIXEL_FORMAT_UNSUPPORTED,
   NS_PIXEL_FORMAT_LUM,  /* luminance, i.e. greyscale */
   NS_PIXEL_FORMAT_LUM_ALPHA,
   NS_PIXEL_FORMAT_RGB,
   NS_PIXEL_FORMAT_RGBA,
   NS_PIXEL_FORMAT_BGR,
   NS_PIXEL_FORMAT_BGRA,
   NS_PIXEL_FORMAT_HSV,
   NS_PIXEL_FORMAT_CMYK,
   NS_PIXEL_FORMAT_CIEXYZ,

   _NS_PIXEL_FORMAT_NUM_TYPES
   }
   NsPixelFormatType;

#define NS_PIXEL_FORMAT_NUM_TYPES  ( ( nsenum )_NS_PIXEL_FORMAT_NUM_TYPES )

NS_IMPEXP const nschar* ns_pixel_format_type_to_string( NsPixelFormatType type );


typedef enum
   {
   NS_PIXEL_CHANNEL_UNSUPPORTED,
   NS_PIXEL_CHANNEL_UINT,
   NS_PIXEL_CHANNEL_FLOAT,

   _NS_PIXEL_CHANNEL_NUM_TYPES
   }
   NsPixelChannelType;

#define NS_PIXEL_CHANNEL_NUM_TYPES  ( ( nsenum )_NS_PIXEL_CHANNEL_NUM_TYPES )

//NS_IMPEXP const nschar* ns_pixel_channel_type_to_string( NsPixelChannelType type );


typedef enum
   {
   NS_PIXEL_UNSUPPORTED,
   NS_PIXEL_LUM_U1,
   NS_PIXEL_LUM_U8,
   NS_PIXEL_LUM_U12, /* Packed into 16 bits! */
   NS_PIXEL_LUM_U16,
   NS_PIXEL_LUM_F32,
   NS_PIXEL_LUM_ALPHA_U8_U8,
   NS_PIXEL_RGB_U8_U8_U8,
   NS_PIXEL_RGBA_U8_U8_U8_U8,
   NS_PIXEL_BGR_U8_U8_U8,
   NS_PIXEL_BGRA_U8_U8_U8_U8,

   _NS_PIXEL_NUM_TYPES
   }
   NsPixelType;

#define NS_PIXEL_NUM_TYPES  ( ( nsenum )_NS_PIXEL_NUM_TYPES )

NS_IMPEXP const nschar* ns_pixel_type_to_string( NsPixelType type );


#ifdef NS_OS_WINDOWS
   #pragma pack( push, 1 )
#endif

typedef struct _NsPixelLumU1
   {
   nsuint8  luminance;
   }
   NsPixelLumU1;

typedef struct _NsPixelLumU8
   {
   nsuint8  luminance;
   }
   NsPixelLumU8;

typedef struct _NsPixelLumU12
   {
   nsuint16  luminance;
   }
   NsPixelLumU12;

typedef struct _NsPixelLumU16
   {
   nsuint16  luminance;
   }
   NsPixelLumU16;

typedef struct _NsPixelLumF32
   {
   nsfloat32  luminance;
   }
   NsPixelLumF32;

typedef struct _NsPixelLumAlphaU8U8
   {
   nsuint8  luminance;
   nsuint8  alpha;
   }
   NsPixelLumAlphaU8U8;

typedef struct _NsPixelRgbU8U8U8
   {
   nsuint8  red;
   nsuint8  green;
   nsuint8  blue;
   }
   NsPixelRgbU8U8U8;

typedef struct _NsPixelRgbaU8U8U8U8
   {
   nsuint8  red;
   nsuint8  green;
   nsuint8  blue;
   nsuint8  alpha;
   }
   NsPixelRgbaU8U8U8U8;

typedef struct _NsPixelBgrU8U8U8
   {
   nsuint8  blue;
   nsuint8  green;
   nsuint8  red;
   }
   NsPixelBgrU8U8U8;

typedef struct _NsPixelBgraU8U8U8U8
   {
   nsuint8  blue;
   nsuint8  green;
   nsuint8  red;
   nsuint8  alpha;
   }
   NsPixelBgraU8U8U8U8;

#ifdef NS_OS_WINDOWS
   #pragma pack( pop )
#endif

NS_COMPILE_TIME_SIZE_CHECK( NsPixelLumU8,        1*sizeof( nsuint8   ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelLumU12,       1*sizeof( nsuint16  ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelLumU16,       1*sizeof( nsuint16  ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelLumF32,       1*sizeof( nsfloat32 ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelLumAlphaU8U8, 2*sizeof( nsuint8   ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelRgbU8U8U8,    3*sizeof( nsuint8   ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelRgbaU8U8U8U8, 4*sizeof( nsuint8   ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelBgrU8U8U8,    3*sizeof( nsuint8   ) );
NS_COMPILE_TIME_SIZE_CHECK( NsPixelBgraU8U8U8U8, 4*sizeof( nsuint8   ) );


NS_IMPEXP NsPixelLumU1* ns_pixel_lum_u1
   (
   NsPixelLumU1  *pixel,
   nsuint8        luminance
   );

NS_IMPEXP NsPixelLumU8* ns_pixel_lum_u8
   (
   NsPixelLumU8  *pixel,
   nsuint8        luminance
   );

NS_IMPEXP NsPixelLumU12* ns_pixel_lum_u12
   (
   NsPixelLumU12  *pixel,
   nsuint16        luminance
   );

NS_IMPEXP NsPixelLumU16* ns_pixel_lum_u16
   (
   NsPixelLumU16  *pixel,
   nsuint16        luminance
   );

NS_IMPEXP NsPixelLumF32* ns_pixel_lum_f32
   (
   NsPixelLumF32  *pixel,
   nsfloat32       luminance
   );

NS_IMPEXP NsPixelLumAlphaU8U8* ns_pixel_lum_alpha_u8_u8
   (
   NsPixelLumAlphaU8U8  *pixel,
   nsuint8               luminance,
   nsuint8               alpha
   );

NS_IMPEXP NsPixelRgbU8U8U8* ns_pixel_rgb_u8_u8_u8
   (
   NsPixelRgbU8U8U8  *pixel,
   nsuint8            red,
   nsuint8            green,
   nsuint8            blue
   );

NS_IMPEXP NsPixelRgbaU8U8U8U8* ns_pixel_rgba_u8_u8_u8_u8
   (
   NsPixelRgbaU8U8U8U8  *pixel,
   nsuint8               red,
   nsuint8               green,
   nsuint8               blue,
   nsuint8               alpha
   );

NS_IMPEXP NsPixelBgrU8U8U8* ns_pixel_bgr_u8_u8_u8
   (
   NsPixelBgrU8U8U8  *pixel,
   nsuint8            blue,
   nsuint8            green,
   nsuint8            red
   );

NS_IMPEXP NsPixelBgraU8U8U8U8* ns_pixel_bgra_u8_u8_u8_u8
   (
   NsPixelBgraU8U8U8U8  *pixel,
   nsuint8               blue,
   nsuint8               green,
   nsuint8               red,
   nsuint8               alpha
   );


typedef union _NsPixelValue
   {
   NsPixelLumU1         lum_u1;
   NsPixelLumU8         lum_u8;
   NsPixelLumU12        lum_u12;
   NsPixelLumU16        lum_u16;
   NsPixelLumF32        lum_f32;
   NsPixelLumAlphaU8U8  lum_alpha_u8_u8;
   NsPixelRgbU8U8U8     rgb_u8_u8_u8;
   NsPixelRgbaU8U8U8U8  rgba_u8_u8_u8_u8;
   NsPixelBgrU8U8U8     bgr_u8_u8_u8;
   NsPixelBgraU8U8U8U8  bgra_u8_u8_u8_u8;
   }
   NsPixelValue;

/* NOTE: The NsPixelValue member must be first so an NsPixel*
   can be converted to any of the above pixel types. */
typedef struct _NsPixel
   {
   NsPixelValue  value;
   NsPixelType   type;
   }
   NsPixel;


/* Initializes the pixel to a certain type. */
NS_IMPEXP NsPixel* ns_pixel_init( NsPixel *pixel, NsPixelType type );

NS_IMPEXP NsPixel* ns_pixel_init_lum_u1
   (
   NsPixel  *pixel,
   nsuint8   luminance
   );

NS_IMPEXP NsPixel* ns_pixel_init_lum_u8
   (
   NsPixel  *pixel,
   nsuint8   luminance
   );

NS_IMPEXP NsPixel* ns_pixel_init_lum_u12
   (
   NsPixel   *pixel,
   nsuint16   luminance
   );

NS_IMPEXP NsPixel* ns_pixel_init_lum_u16
   (
   NsPixel   *pixel,
   nsuint16   luminance
   );

NS_IMPEXP NsPixel* ns_pixel_init_lum_f32
   (
   NsPixel    *pixel,
   nsfloat32   luminance
   );

NS_IMPEXP NsPixel* ns_pixel_init_lum_alpha_u8_u8
   (
   NsPixel  *pixel,
   nsuint8   luminance,
   nsuint8   alpha
   );

NS_IMPEXP NsPixel* ns_pixel_init_rgb_u8_u8_u8
   (
   NsPixel  *pixel,
   nsuint8   red,
   nsuint8   green,
   nsuint8   blue
   );

NS_IMPEXP NsPixel* ns_pixel_init_rgba_u8_u8_u8_u8
   (
   NsPixel  *pixel,
   nsuint8   red,
   nsuint8   green,
   nsuint8   blue,
   nsuint8   alpha
   );

NS_IMPEXP NsPixel* ns_pixel_init_bgr_u8_u8_u8
   (
   NsPixel  *pixel,
   nsuint8   blue,
   nsuint8   green,
   nsuint8   red
   );

NS_IMPEXP NsPixel* ns_pixel_init_bgra_u8_u8_u8_u8
   (
   NsPixel  *pixel,
   nsuint8   blue,
   nsuint8   green,
   nsuint8   red,
   nsuint8   alpha
   );


/* Sets the pixel->value to all bits zero.
   NOTE: The type does NOT change. */
NS_IMPEXP NsPixel* ns_pixel_reset( NsPixel *pixel );


/* Accepts pointers to any of the above pixel types including NsPixel. */
typedef void ( *NsPixelConvertFunc )( nsconstpointer src_pixel, nspointer dest_pixel );

NS_IMPEXP NsPixelConvertFunc ns_pixel_convert_func( NsPixelType src_type, NsPixelType dest_type );
NS_IMPEXP NsPixelConvertFunc ns_pixel_convert_for_intensity_mask_palette_func( NsPixelType src_type, NsPixelType dest_type );

NS_IMPEXP NsPixel* ns_pixel_convert( const NsPixel *src_pixel, NsPixel *dest_pixel );


/* The pixel->value is converted as necessary to the proper 'value' type. */
NS_IMPEXP NsPixelLumU1* ns_pixel_get_lum_u1( const NsPixel *pixel, NsPixelLumU1 *value );
NS_IMPEXP NsPixelLumU8* ns_pixel_get_lum_u8( const NsPixel *pixel, NsPixelLumU8 *value );
NS_IMPEXP NsPixelLumU12* ns_pixel_get_lum_u12( const NsPixel *pixel, NsPixelLumU12 *value );
NS_IMPEXP NsPixelLumU16* ns_pixel_get_lum_u16( const NsPixel *pixel, NsPixelLumU16 *value );
NS_IMPEXP NsPixelLumF32* ns_pixel_get_lum_f32( const NsPixel *pixel, NsPixelLumF32 *value );
NS_IMPEXP NsPixelLumAlphaU8U8* ns_pixel_get_lum_alpha_u8_u8( const NsPixel *pixel, NsPixelLumAlphaU8U8 *value );
NS_IMPEXP NsPixelRgbU8U8U8* ns_pixel_get_rgb_u8_u8_u8( const NsPixel *pixel, NsPixelRgbU8U8U8 *value );
NS_IMPEXP NsPixelRgbaU8U8U8U8* ns_pixel_get_rgba_u8_u8_u8_u8( const NsPixel *pixel, NsPixelRgbaU8U8U8U8 *value );
NS_IMPEXP NsPixelBgrU8U8U8* ns_pixel_get_bgr_u8_u8_u8( const NsPixel *pixel, NsPixelBgrU8U8U8 *value );
NS_IMPEXP NsPixelBgraU8U8U8U8* ns_pixel_get_bgra_u8_u8_u8_u8( const NsPixel *pixel, NsPixelBgraU8U8U8U8 *value );

/* The 'value' is converted as necessary. NOTE: The pixel->type does NOT change. */
NS_IMPEXP NsPixel* ns_pixel_set_lum_u1( NsPixel *pixel, const NsPixelLumU1 *value );
NS_IMPEXP NsPixel* ns_pixel_set_lum_u8( NsPixel *pixel, const NsPixelLumU8 *value );
NS_IMPEXP NsPixel* ns_pixel_set_lum_u12( NsPixel *pixel, const NsPixelLumU12 *value );
NS_IMPEXP NsPixel* ns_pixel_set_lum_u16( NsPixel *pixel, const NsPixelLumU16 *value );
NS_IMPEXP NsPixel* ns_pixel_set_lum_f32( NsPixel *pixel, const NsPixelLumF32 *value );
NS_IMPEXP NsPixel* ns_pixel_set_lum_alpha_u8_u8( NsPixel *pixel, const NsPixelLumAlphaU8U8 *value );
NS_IMPEXP NsPixel* ns_pixel_set_rgb_u8_u8_u8( NsPixel *pixel, const NsPixelRgbU8U8U8 *value );
NS_IMPEXP NsPixel* ns_pixel_set_rgba_u8_u8_u8_u8( NsPixel *pixel, const NsPixelRgbaU8U8U8U8 *value );
NS_IMPEXP NsPixel* ns_pixel_set_bgr_u8_u8_u8( NsPixel *pixel, const NsPixelBgrU8U8U8 *value );
NS_IMPEXP NsPixel* ns_pixel_set_bgra_u8_u8_u8_u8( NsPixel *pixel, const NsPixelBgraU8U8U8U8 *value );


/* NOTE: 'pixel' can be any of the pixel types including NsPixel. */
typedef void ( *NsPixelRowGetFunc )( nspointer pixel, nsconstpointer row, nssize x );
typedef void ( *NsPixelRowSetFunc )( nsconstpointer pixel, nspointer row, nssize x );

NS_IMPEXP NsPixelRowGetFunc ns_pixel_row_get_func( NsPixelType type );
NS_IMPEXP NsPixelRowSetFunc ns_pixel_row_set_func( NsPixelType type );

/* The 'type' is the type of pixels in the 'row'. The type of the 'pixel' is
   converted if necessary. */
NS_IMPEXP NsPixel* ns_pixel_row_get( NsPixel *pixel, NsPixelType type, nsconstpointer row, nssize x );
NS_IMPEXP void ns_pixel_row_set( const NsPixel *pixel, NsPixelType type, nspointer row, nssize x );


#define NS_PIXEL_NUM_CHANNELS  4

#define NS_PIXEL_CHANNEL_LUM  0

#define NS_PIXEL_CHANNEL_RED    0
#define NS_PIXEL_CHANNEL_GREEN  1
#define NS_PIXEL_CHANNEL_BLUE   2
#define NS_PIXEL_CHANNEL_ALPHA  3

#define NS_PIXEL_CHANNEL_HUE    0
#define NS_PIXEL_CHANNEL_SAT    1
#define NS_PIXEL_CHANNEL_VALUE  2

#define NS_PIXEL_CHANNEL_CYAN     0
#define NS_PIXEL_CHANNEL_MAGENTA  1
#define NS_PIXEL_CHANNEL_YELLOW   2
#define NS_PIXEL_CHANNEL_BLACK    3


/* get/set a specific channel of a pixel.
   'pixel' can be any of the pixel types including NsPixel.
   Conversion to the type is performed as necessary. */
typedef nsuint ( *NsPixelGetUintChannelFunc )( nsconstpointer pixel );
typedef nsfloat ( *NsPixelGetFloatChannelFunc )( nsconstpointer pixel );

typedef void ( *NsPixelSetUintChannelFunc )( nspointer pixel, nsuint value );
typedef void ( *NsPixelSetFloatChannelFunc )( nspointer pixel, nsfloat value );

NS_IMPEXP NsPixelGetUintChannelFunc ns_pixel_get_uint_channel_func( NsPixelType type, nssize channel );
NS_IMPEXP NsPixelGetFloatChannelFunc ns_pixel_get_float_channel_func( NsPixelType type, nssize channel );

NS_IMPEXP NsPixelSetUintChannelFunc ns_pixel_set_uint_channel_func( NsPixelType type, nssize channel );
NS_IMPEXP NsPixelSetFloatChannelFunc ns_pixel_set_float_channel_func( NsPixelType type, nssize channel );

NS_IMPEXP nsuint ns_pixel_get_uint_channel( const NsPixel *pixel, nssize channel );
NS_IMPEXP nsfloat ns_pixel_get_float_channel( const NsPixel *pixel, nssize channel );

NS_IMPEXP void ns_pixel_set_uint_channel( NsPixel *pixel, nssize channel, nsuint value );
NS_IMPEXP void ns_pixel_set_float_channel( NsPixel *pixel, nssize channel, nsfloat value );


/* get/set a specific channel of a specific pixel in a row.
   Faster than setting an NsPixel and then setting it in the row.
   Conversion to the type is performed as necessary. */
typedef nsuint ( *NsPixelRowGetUintChannelFunc )( nsconstpointer row, nssize x );
typedef nsfloat ( *NsPixelRowGetFloatChannelFunc )( nsconstpointer row, nssize x );

typedef void ( *NsPixelRowSetUintChannelFunc )( nspointer row, nssize x, nsuint value );
typedef void ( *NsPixelRowSetFloatChannelFunc )( nspointer row, nssize x, nsfloat value );

NS_IMPEXP NsPixelRowGetUintChannelFunc ns_pixel_row_get_uint_channel_func( NsPixelType type, nssize channel );
NS_IMPEXP NsPixelRowGetFloatChannelFunc ns_pixel_row_get_float_channel_func( NsPixelType type, nssize channel );

NS_IMPEXP NsPixelRowSetUintChannelFunc ns_pixel_row_set_uint_channel_func( NsPixelType type, nssize channel );
NS_IMPEXP NsPixelRowSetFloatChannelFunc ns_pixel_row_set_float_channel_func( NsPixelType type, nssize channel );


#define NS_PIXEL_ALPHA_OPAQUE_U8        255
#define NS_PIXEL_ALPHA_OPAQUE_F32       1.0f
#define NS_PIXEL_ALPHA_TRANSPARENT_U8   0
#define NS_PIXEL_ALPHA_TRANSPARENT_F32  0.0f


typedef struct _NsPixelChannel
   {
   NsPixelChannelType             type;
   nssize                         bits;
	nssize                         ubits;
   const nschar                  *name;
   nssize                         index;
   const struct _NsPixelChannel  *next;
   }
   NsPixelChannel;


/* Values for NsPixelChannel::name. */
#define NS_PIXEL_CHANNEL_LUM_NAME  "luminance"

#define NS_PIXEL_CHANNEL_RED_NAME    "red"
#define NS_PIXEL_CHANNEL_GREEN_NAME  "green"
#define NS_PIXEL_CHANNEL_BLUE_NAME   "blue"
#define NS_PIXEL_CHANNEL_ALPHA_NAME  "alpha"

#define NS_PIXEL_CHANNEL_HUE_NAME    "hue"
#define NS_PIXEL_CHANNEL_SAT_NAME    "saturation"
#define NS_PIXEL_CHANNEL_VALUE_NAME  "value"

#define NS_PIXEL_CHANNEL_CYAN_NAME     "cyan"
#define NS_PIXEL_CHANNEL_MAGENTA_NAME  "magenta"
#define NS_PIXEL_CHANNEL_YELLOW_NAME   "yellow"
#define NS_PIXEL_CHANNEL_BLACK_NAME    "black"

#define ns_pixel_channel_is( channel, _name )\
   ns_ascii_streq( (channel)->name, (_name) )


/* NOTE: The pixel channels can be accessed by array index or iteratively. */
typedef struct _NsPixelInfo 
   {
   NsPixelType            pixel_type;
   NsPixelFormatType      format_type;
   nssize                 num_channels;
   NsPixelChannel         channels[ NS_PIXEL_NUM_CHANNELS ];
   const NsPixelChannel  *channel;
   }
   NsPixelInfo;

NS_IMPEXP void ns_pixel_info( NsPixelInfo *info, NsPixelType type );

NS_IMPEXP nssize ns_pixel_num_channels( NsPixelType type );


/* The total number of bits per pixel. */
NS_IMPEXP nssize ns_pixel_bits( NsPixelType type );

/* The number of bits per pixel that are actually used. For example for 12-bit
	luminance this would return 12 while ns_pixel_bits() would return 16. */
NS_IMPEXP nssize ns_pixel_used_bits( NsPixelType type );


/* Returns true if the pixel type does not cross byte-boundaries.
   i.e. if bits_per_pixel % 8 == 0. Example: a 4-bit pixel or
   a 12-bit pixel would return FALSE, but an 8-bit or 16-bit pixel
   would return TRUE. */
NS_IMPEXP nsboolean ns_pixel_byte_bounded( NsPixelType type );


/* Get the minimum number of bytes required to hold the pixels.
   'row_align' must be a power of 2 and should be 1 if the 
   pixels rows are not padded to any byte boundary.

   Note also that if the pixels are less than 8 bits and/or cross
   byte boundaries then the number of bytes per pixel row will be
   calculated to be able to hold the bits.

   Example:
   The pixel size is 4 bits and the width is 3, then we would
   need at least 12 bits per row, i.e. 2 bytes.
*/
NS_IMPEXP nssize ns_pixel_buffer_size
   (
   NsPixelType  type, 
   nssize       width,
   nssize       height,
   nssize       length,
   nssize       row_align
   );

NS_IMPEXP nssize ns_pixel_buffer_slice_size
   (
   NsPixelType  type,
   nssize       width,
   nssize       height,
   nssize       row_align
   );

NS_IMPEXP nssize ns_pixel_buffer_row_size
   (
   NsPixelType  type,
   nssize       width,
   nssize       row_align
   );


typedef struct _NsPixelRegion
   {
   nssize  x;
   nssize  y;
   nssize  z;
   nssize  width;
   nssize  height;
   nssize  length;
   }
   NsPixelRegion;


NS_IMPEXP NsPixelRegion* ns_pixel_region
   (
   NsPixelRegion   *region,
   nssize           x,
   nssize           y,
   nssize           z,
   nssize           width,
   nssize           height,
   nssize           length
   );


#ifdef NS_DEBUG

#define ns_pixel_region_verify( region, width, height, length )\
   _ns_pixel_region_verify( (region), (width), (height), (length), NS_MODULE )

NS_IMPEXP void _ns_pixel_region_verify
   (
   const NsPixelRegion  *region,
   nssize                width,
   nssize                height,
   nssize                length,
   const nschar         *details
   );

#else

/* Verifies the 'region' is within the bounds of the 'image'. */
NS_IMPEXP void ns_pixel_region_verify
   (
   const NsPixelRegion  *region,
   nssize                width,
   nssize                height,
   nssize                length
   );

#endif/* NS_DEBUG */


/* If 'sub_region' is not NULL returns 'sub_region', else
   it sets 'full_region' to the passed dimensions and then
   returns 'full_region'. */
NS_IMPEXP const NsPixelRegion* ns_pixel_region_extract
   (
   const NsPixelRegion  *sub_region,
   NsPixelRegion        *full_region,
   nssize                width,
   nssize                height,
   nssize                length
   );


typedef struct _NsPixelLut
   {
   nsuint  *values;
   nssize   size;
   }
   NsPixelLut;


/* NOTE: 'init_func' can be NULL if not necessary. */
NS_IMPEXP NsError ns_pixel_lut_init( NsPixelLut *lut, const NsPixelChannel *channel );
NS_IMPEXP void ns_pixel_lut_finalize( NsPixelLut *lut );

#define ns_pixel_lut_get_value( lut, index )\
   ( (lut)->values[ (index) ] )

#define ns_pixel_lut_set_value( lut, index, value )\
   ( (lut)->values[ (index) ] = (value) )


/* Need to create some EValue types. */
#define NS_VALUE_PIXEL_TYPE  "NsPixelType"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pixel_type, NsPixelType );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pixel_type, NsPixelType );
NS_IMPEXP NS_VALUE_ARG_DECLARE( ns_value_arg_pixel_type, NsPixelType );
NS_IMPEXP NsError ns_value_register_pixel_type( void );

#define NS_VALUE_PIXEL  "NsPixel*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pixel, NsPixel* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pixel, NsPixel* );
NS_IMPEXP NS_VALUE_ARG_DECLARE( ns_value_arg_pixel, NsPixel* );
NS_IMPEXP NsError ns_value_register_pixel( void );

#define NS_VALUE_PIXEL_REGION  "NsPixelRegion*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pixel_region, NsPixelRegion* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pixel_region, NsPixelRegion* );
NS_IMPEXP NS_VALUE_ARG_DECLARE( ns_value_arg_pixel_region, NsPixelRegion* );
NS_IMPEXP NsError ns_value_register_pixel_region( void );


/* Standard procedures. See nspixels-procs.h as well.

   NOTE: All procedures should return ns_error_noimp()
   if the passed parameter values are not supported. */

#define NS_PIXEL_PROC_GET_PIXEL\
   "_ns_pixel_proc_get_pixel"

#define NS_PIXEL_PROC_SET_PIXEL\
   "_ns_pixel_proc_set_pixel"

#define NS_PIXEL_PROC_CONVERT\
   "_ns_pixel_proc_convert"

#define NS_PIXEL_PROC_CONVERT_FOR_INTENSITY_MASK_PALETTE\
   "_ns_pixel_proc_convert_for_intensity_mask_palette"

#define NS_PIXEL_PROC_COPY\
   "_ns_pixel_proc_copy"

#define NS_PIXEL_PROC_BLUR\
   "_ns_pixel_proc_blur"

#define NS_PIXEL_PROC_BRIGHTNESS_CONTRAST\
   "_ns_pixel_proc_brightness_contrast"

#define NS_PIXEL_PROC_ORTHO_PROJECT\
   "_ns_pixel_proc_ortho_project"

#define NS_PIXEL_PROC_DYNAMIC_RANGE\
   "_ns_pixel_proc_dynamic_range"

#define NS_PIXEL_PROC_SEGMENT\
   "_ns_pixel_proc_segment"

#define NS_PIXEL_PROC_MP_FILTER\
   "_ns_pixel_proc_mp_filter"

#define NS_PIXEL_PROC_RESIZE\
   "_ns_pixel_proc_resize"

#define NS_PIXEL_PROC_GAMMA_CORRECT\
   "_ns_pixel_proc_gamma_correct"

#define NS_PIXEL_PROC_NOISE\
   "_ns_pixel_proc_noise"

#define NS_PIXEL_PROC_FLIP\
   "_ns_pixel_proc_flip"

#define NS_PIXEL_PROC_SUBSAMPLE\
   "_ns_pixel_proc_subsample"


#define NS_IO_PROC_WRITE_RAW\
	"_ns_io_proc_write_raw"

#define NS_IO_PROC_WRITE_TIFF\
	"_ns_io_proc_write_tiff"

#define NS_IO_PROC_WRITE_JPEG\
	"_ns_io_proc_write_jpeg"


/* Registers necessary value types. Called internally by
   ns_pixel_proc_db_register_std(). NOTE: Its OK if this
   function is called more than once, but its not necessary.*/
NS_IMPEXP NsError ns_pixel_register_value_types( void );

/* Load up the above standard procedures. */
NS_IMPEXP NsError ns_pixel_proc_db_register_std( NsProcDb *db );


/* Internal. DO NOT USE! */
NS_IMPEXP NS_INTERNAL void _ns_pixels_init( void );

NS_DECLS_END

#endif/* __NS_IMAGE_PIXELS_H__ */


