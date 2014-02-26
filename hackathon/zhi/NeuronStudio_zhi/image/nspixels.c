#include "nspixels.h"


NS_PRIVATE NsPixelConvertFunc _ns_pixel_convert_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_TYPES ];
NS_PRIVATE NsPixelConvertFunc _ns_pixel_convert_for_intensity_mask_palette_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_TYPES ];

NS_PRIVATE NsPixelRowGetFunc _ns_pixel_row_get_funcs[ NS_PIXEL_NUM_TYPES ];
NS_PRIVATE NsPixelRowSetFunc _ns_pixel_row_set_funcs[ NS_PIXEL_NUM_TYPES ];

NS_PRIVATE NsPixelGetUintChannelFunc _ns_pixel_get_uint_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];
NS_PRIVATE NsPixelGetFloatChannelFunc _ns_pixel_get_float_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];

NS_PRIVATE NsPixelSetUintChannelFunc _ns_pixel_set_uint_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];
NS_PRIVATE NsPixelSetFloatChannelFunc _ns_pixel_set_float_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];

NS_PRIVATE NsPixelRowGetUintChannelFunc _ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];
NS_PRIVATE NsPixelRowGetFloatChannelFunc _ns_pixel_row_get_float_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];

NS_PRIVATE NsPixelRowSetUintChannelFunc _ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];
NS_PRIVATE NsPixelRowSetFloatChannelFunc _ns_pixel_row_set_float_channel_funcs[ NS_PIXEL_NUM_TYPES ][ NS_PIXEL_NUM_CHANNELS ];

NS_PRIVATE NsPixelInfo _ns_pixel_infos[ NS_PIXEL_NUM_TYPES ];


const nschar* ns_pixel_format_type_to_string( NsPixelFormatType type )
	{
	NS_PRIVATE const nschar* _ns_pixel_format_type_strings[ NS_PIXEL_FORMAT_NUM_TYPES ] =
		{
		NULL,
		"luminance",
		"luminance alpha",
		"red green blue",
		"red green blue alpha",
		"blue green red",
		"blue green red alpha",
		"hue saturation value",
		"cyan magenta yellow black",
		"Commission Internationale de l'Eclairage XYZ"
		};

	ns_assert( type < NS_PIXEL_FORMAT_NUM_TYPES );
	return _ns_pixel_format_type_strings[ type ];
	}


const nschar* ns_pixel_channel_type_to_string( NsPixelFormatType type )
	{
	NS_PRIVATE const nschar* _ns_pixel_channel_type_strings[ NS_PIXEL_CHANNEL_NUM_TYPES ] =
		{
		NULL,
		"unsigned int",
		"float"
		};

	ns_assert( type < NS_PIXEL_CHANNEL_NUM_TYPES );
	return _ns_pixel_channel_type_strings[ type ];
	}


const nschar* ns_pixel_type_to_string( NsPixelType type )
	{
	NS_PRIVATE const nschar* _ns_pixel_type_strings[ NS_PIXEL_NUM_TYPES ] =
		{
		NULL,
		"1-bit luminance " NS_STRINGIZE( NS_PIXEL_LUM_U1 ),
		"8-bit luminance " NS_STRINGIZE( NS_PIXEL_LUM_U8 ),
		"12-bit luminance " NS_STRINGIZE( NS_PIXEL_LUM_U12 ),
		"16-bit luminance " NS_STRINGIZE( NS_PIXEL_LUM_U16 ),
		"32-bit luminance " NS_STRINGIZE( NS_PIXEL_LUM_F32 ),
		"16-bit luminance-alpha " NS_STRINGIZE( NS_PIXEL_LUM_ALPHA_U8_U8 ),
		"24-bit RGB " NS_STRINGIZE( NS_PIXEL_RGB_U8_U8_U8 ),
		"32-bit RGBA " NS_STRINGIZE( NS_PIXEL_RGBA_U8_U8_U8_U8 ),
		"24-bit BGR " NS_STRINGIZE( NS_PIXEL_BGR_U8_U8_U8 ),
		"32-bit BGRA " NS_STRINGIZE( NS_PIXEL_BGRA_U8_U8_U8_U8 )
		};

	ns_assert( type < NS_PIXEL_NUM_TYPES );
	return _ns_pixel_type_strings[ type ];
	}


NsPixelLumU1* ns_pixel_lum_u1
	(
	NsPixelLumU1  *pixel,
	nsuint8        luminance
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance ? 1 : 0;
	return pixel;
	}


NsPixelLumU8* ns_pixel_lum_u8
	(
	NsPixelLumU8  *pixel,
	nsuint8        luminance
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance;
	return pixel;
	}


NsPixelLumU12* ns_pixel_lum_u12
	(
	NsPixelLumU12  *pixel,
	nsuint16        luminance
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance;
	return pixel;
	}


NsPixelLumU16* ns_pixel_lum_u16
	(
	NsPixelLumU16  *pixel,
	nsuint16        luminance
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance;
	return pixel;
	}


NsPixelLumF32* ns_pixel_lum_f32
	(
	NsPixelLumF32  *pixel,
	nsfloat32       luminance
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance;
	return pixel;
	}


NsPixelLumAlphaU8U8* ns_pixel_lum_alpha_u8_u8
	(
	NsPixelLumAlphaU8U8  *pixel,
	nsuint8               luminance,
	nsuint8               alpha
	)
	{
	ns_assert( NULL != pixel );

	pixel->luminance = luminance;
	pixel->alpha     = alpha;

	return pixel;
	}


NsPixelRgbU8U8U8* ns_pixel_rgb_u8_u8_u8
	(
	NsPixelRgbU8U8U8  *pixel,
	nsuint8            red,
	nsuint8            green,
	nsuint8            blue
	)
	{
	ns_assert( NULL != pixel );

	pixel->red   = red;
	pixel->green = green;
	pixel->blue  = blue;

	return pixel;
	}


NsPixelRgbaU8U8U8U8* ns_pixel_rgba_u8_u8_u8_u8
	(
	NsPixelRgbaU8U8U8U8  *pixel,
	nsuint8               red,
	nsuint8               green,
	nsuint8               blue,
	nsuint8               alpha
	)
	{
	ns_assert( NULL != pixel );

	pixel->red   = red;
	pixel->green = green;
	pixel->blue  = blue;
	pixel->alpha = alpha;

	return pixel;
	}


NsPixelBgrU8U8U8* ns_pixel_bgr_u8_u8_u8
	(
	NsPixelBgrU8U8U8  *pixel,
	nsuint8            blue,
	nsuint8            green,
	nsuint8            red
	)
	{
	ns_assert( NULL != pixel );

	pixel->blue  = blue;
	pixel->green = green;
	pixel->red   = red;

	return pixel;
	}


NsPixelBgraU8U8U8U8* ns_pixel_bgra_u8_u8_u8_u8
	(
	NsPixelBgraU8U8U8U8  *pixel,
	nsuint8               blue,
	nsuint8               green,
	nsuint8               red,
	nsuint8               alpha
	)
	{
	ns_assert( NULL != pixel );

	pixel->blue  = blue;
	pixel->green = green;
	pixel->red   = red;
	pixel->alpha = alpha;

	return pixel;
	}


NsPixel* ns_pixel_init( NsPixel *pixel, NsPixelType type )
	{
	ns_assert( NULL != pixel );
	ns_assert( type < NS_PIXEL_NUM_TYPES );

	pixel->type = type;
	ns_pixel_reset( pixel );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_u1
	(
	NsPixel  *pixel,
	nsuint8   luminance
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_U1 );
	ns_pixel_lum_u1( &pixel->value.lum_u1, luminance );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_u8
	(
	NsPixel  *pixel,
	nsuint8   luminance
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_U8 );
	ns_pixel_lum_u8( &pixel->value.lum_u8, luminance );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_u12
	(
	NsPixel   *pixel,
	nsuint16   luminance
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_U12 );
	ns_pixel_lum_u12( &pixel->value.lum_u12, luminance );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_u16
	(
	NsPixel   *pixel,
	nsuint16   luminance
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_U16 );
	ns_pixel_lum_u16( &pixel->value.lum_u16, luminance );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_f32
	(
	NsPixel    *pixel,
	nsfloat32   luminance
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_F32 );
	ns_pixel_lum_f32( &pixel->value.lum_f32, luminance );

	return pixel;
	}


NsPixel* ns_pixel_init_lum_alpha_u8_u8
	(
	NsPixel  *pixel,
	nsuint8   luminance,
	nsuint8   alpha
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_LUM_ALPHA_U8_U8 );
	ns_pixel_lum_alpha_u8_u8( &pixel->value.lum_alpha_u8_u8, luminance, alpha );

	return pixel;
	}


NsPixel* ns_pixel_init_rgb_u8_u8_u8
	(
	NsPixel  *pixel,
	nsuint8   red,
	nsuint8   green,
	nsuint8   blue
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_RGB_U8_U8_U8 );
	ns_pixel_rgb_u8_u8_u8( &pixel->value.rgb_u8_u8_u8, red, green, blue );

	return pixel;
	}


NsPixel* ns_pixel_init_rgba_u8_u8_u8_u8
	(
	NsPixel  *pixel,
	nsuint8   red,
	nsuint8   green,
	nsuint8   blue,
	nsuint8   alpha
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_RGBA_U8_U8_U8_U8 );
	ns_pixel_rgba_u8_u8_u8_u8( &pixel->value.rgba_u8_u8_u8_u8, red, green, blue, alpha );

	return pixel;
	}


NsPixel* ns_pixel_init_bgr_u8_u8_u8
	(
	NsPixel  *pixel,
	nsuint8   blue,
	nsuint8   green,
	nsuint8   red
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_BGR_U8_U8_U8 );
	ns_pixel_bgr_u8_u8_u8( &pixel->value.bgr_u8_u8_u8, blue, green, red );

	return pixel;
	}


NsPixel* ns_pixel_init_bgra_u8_u8_u8_u8
	(
	NsPixel  *pixel,
	nsuint8   blue,
	nsuint8   green,
	nsuint8   red,
	nsuint8   alpha
	)
	{
	ns_assert( NULL != pixel );

	ns_pixel_init( pixel, NS_PIXEL_BGRA_U8_U8_U8_U8 );
	ns_pixel_bgra_u8_u8_u8_u8( &pixel->value.bgra_u8_u8_u8_u8, blue, green, red, alpha );

	return pixel;
	}


/* NOTE: Assuming all bits zero! */
NS_PRIVATE NsPixelValue _ns_pixel_value_null;

NsPixel* ns_pixel_reset( NsPixel *pixel )
	{
	ns_assert( NULL != pixel );

	pixel->value = _ns_pixel_value_null;
	return pixel;
	}


NsPixel* ns_pixel_convert( const NsPixel *src_pixel, NsPixel *dest_pixel )
	{
	NsPixelConvertFunc func;

    //ns_assert( NULL != src_pixel );
//	ns_assert( NULL != dest_pixel );

	if( src_pixel->type != dest_pixel->type )
		{
		func = ns_pixel_convert_func( src_pixel->type, dest_pixel->type );
    //	ns_assert( NULL != func );

		( func )( src_pixel, dest_pixel );
		}
	else
		*dest_pixel = *src_pixel;

	return dest_pixel;
	}


#define _NS_PIXEL_GET( stype, ptype, pixel, value )\
	NsPixelConvertFunc func;\
	\
	ns_assert( NULL != (pixel) );\
	ns_assert( NULL != (value) );\
	\
	if( ptype != (pixel)->type )\
		{\
		func = ns_pixel_convert_func( (pixel)->type, ptype );\
		ns_assert( NULL != func );\
		\
		( func )( (pixel), (value) );\
		}\
	else\
		*(value) = *( ( const stype* )(pixel) );\
	\
	return (value)


NsPixelLumU1* ns_pixel_get_lum_u1( const NsPixel *pixel, NsPixelLumU1 *value )
	{  _NS_PIXEL_GET( NsPixelLumU1, NS_PIXEL_LUM_U1, pixel, value );  }

NsPixelLumU8* ns_pixel_get_lum_u8( const NsPixel *pixel, NsPixelLumU8 *value )
	{  _NS_PIXEL_GET( NsPixelLumU8, NS_PIXEL_LUM_U8, pixel, value );  }

NsPixelLumU12* ns_pixel_get_lum_u12( const NsPixel *pixel, NsPixelLumU12 *value )
	{  _NS_PIXEL_GET( NsPixelLumU12, NS_PIXEL_LUM_U12, pixel, value );  }

NsPixelLumU16* ns_pixel_get_lum_u16( const NsPixel *pixel, NsPixelLumU16 *value )
	{  _NS_PIXEL_GET( NsPixelLumU16, NS_PIXEL_LUM_U16, pixel, value );  }

NsPixelLumF32* ns_pixel_get_lum_f32( const NsPixel *pixel, NsPixelLumF32 *value )
	{  _NS_PIXEL_GET( NsPixelLumF32, NS_PIXEL_LUM_F32, pixel, value );  }

NsPixelLumAlphaU8U8* ns_pixel_get_lum_alpha_u8_u8( const NsPixel *pixel, NsPixelLumAlphaU8U8 *value )
	{  _NS_PIXEL_GET( NsPixelLumAlphaU8U8, NS_PIXEL_LUM_ALPHA_U8_U8, pixel, value );  }

NsPixelRgbU8U8U8* ns_pixel_get_rgb_u8_u8_u8( const NsPixel *pixel, NsPixelRgbU8U8U8 *value )
	{  _NS_PIXEL_GET( NsPixelRgbU8U8U8, NS_PIXEL_RGB_U8_U8_U8, pixel, value );  }

NsPixelRgbaU8U8U8U8* ns_pixel_get_rgba_u8_u8_u8_u8( const NsPixel *pixel, NsPixelRgbaU8U8U8U8 *value )
	{  _NS_PIXEL_GET( NsPixelRgbaU8U8U8U8, NS_PIXEL_RGBA_U8_U8_U8_U8, pixel, value );  }

NsPixelBgrU8U8U8* ns_pixel_get_bgr_u8_u8_u8( const NsPixel *pixel, NsPixelBgrU8U8U8 *value )
	{  _NS_PIXEL_GET( NsPixelBgrU8U8U8, NS_PIXEL_BGR_U8_U8_U8, pixel, value );  }

NsPixelBgraU8U8U8U8* ns_pixel_get_bgra_u8_u8_u8_u8( const NsPixel *pixel, NsPixelBgraU8U8U8U8 *value )
	{  _NS_PIXEL_GET( NsPixelBgraU8U8U8U8, NS_PIXEL_BGRA_U8_U8_U8_U8, pixel, value );  }


#define _NS_PIXEL_SET( stype, ptype, pixel, value )\
	NsPixelConvertFunc func;\
	\
	ns_assert( NULL != (pixel) );\
	ns_assert( NULL != (value) );\
	\
	if( ptype != (pixel)->type )\
		{\
		func = ns_pixel_convert_func( ptype, (pixel)->type );\
		ns_assert( NULL != func );\
		\
		ns_pixel_reset( (pixel) );\
		( func )( (value), (pixel) );\
		}\
	else\
		*( ( stype* )(pixel) ) = *(value);\
	\
	return (pixel)


NsPixel* ns_pixel_set_lum_u1( NsPixel *pixel, const NsPixelLumU1 *value )
	{  _NS_PIXEL_SET( NsPixelLumU1, NS_PIXEL_LUM_U1, pixel, value );  }

NsPixel* ns_pixel_set_lum_u8( NsPixel *pixel, const NsPixelLumU8 *value )
	{  _NS_PIXEL_SET( NsPixelLumU8, NS_PIXEL_LUM_U8, pixel, value );  }

NsPixel* ns_pixel_set_lum_u12( NsPixel *pixel, const NsPixelLumU12 *value )
	{  _NS_PIXEL_SET( NsPixelLumU12, NS_PIXEL_LUM_U12, pixel, value );  }

NsPixel* ns_pixel_set_lum_u16( NsPixel *pixel, const NsPixelLumU16 *value )
	{  _NS_PIXEL_SET( NsPixelLumU16, NS_PIXEL_LUM_U16, pixel, value );  }

NsPixel* ns_pixel_set_lum_f32( NsPixel *pixel, const NsPixelLumF32 *value )
	{  _NS_PIXEL_SET( NsPixelLumF32, NS_PIXEL_LUM_F32, pixel, value );  }

NsPixel* ns_pixel_set_lum_alpha_u8_u8( NsPixel *pixel, const NsPixelLumAlphaU8U8 *value )
	{  _NS_PIXEL_SET( NsPixelLumAlphaU8U8, NS_PIXEL_LUM_ALPHA_U8_U8, pixel, value );  }

NsPixel* ns_pixel_set_rgb_u8_u8_u8( NsPixel *pixel, const NsPixelRgbU8U8U8 *value )
	{  _NS_PIXEL_SET( NsPixelRgbU8U8U8, NS_PIXEL_RGB_U8_U8_U8, pixel, value );  }

NsPixel* ns_pixel_set_rgba_u8_u8_u8_u8( NsPixel *pixel, const NsPixelRgbaU8U8U8U8 *value )
	{  _NS_PIXEL_SET( NsPixelRgbaU8U8U8U8, NS_PIXEL_RGBA_U8_U8_U8_U8, pixel, value );  }

NsPixel* ns_pixel_set_bgr_u8_u8_u8( NsPixel *pixel, const NsPixelBgrU8U8U8 *value )
	{  _NS_PIXEL_SET( NsPixelBgrU8U8U8, NS_PIXEL_BGR_U8_U8_U8, pixel, value );  }

NsPixel* ns_pixel_set_bgra_u8_u8_u8_u8( NsPixel *pixel, const NsPixelBgraU8U8U8U8 *value )
	{  _NS_PIXEL_SET( NsPixelBgraU8U8U8U8, NS_PIXEL_BGRA_U8_U8_U8_U8, pixel, value );  }


NsPixel* ns_pixel_row_get( NsPixel *pixel, NsPixelType type, nsconstpointer row, nssize x )
	{
	NsPixel            conv;
	NsPixelRowGetFunc  func;


	ns_assert( NULL != pixel );

	func = ns_pixel_row_get_func( type );
	ns_assert( NULL != func );
	
	if( pixel->type != type )
		{
		ns_pixel_init( &conv, type );
		( func )( &conv, row, x );
		ns_pixel_convert( &conv, pixel );
		}
	else
		( func )( pixel, row, x );

	return pixel;
	}


void ns_pixel_row_set( const NsPixel *pixel, NsPixelType type, nspointer row, nssize x )
	{
	NsPixel            conv;
	NsPixelRowSetFunc  func;


    ns_assert( NULL != pixel );
	func = ns_pixel_row_set_func( type );
    //ns_assert( NULL != func );

    if( pixel->type != type )
		{
		ns_pixel_init( &conv, type );
		ns_pixel_convert( pixel, &conv );
		( func )( &conv, row, x );
		}
    else
		( func )( pixel, row, x );
	}


nsuint ns_pixel_get_uint_channel( const NsPixel *pixel, nssize channel )
	{
	NsPixelGetUintChannelFunc func;

	ns_assert( NULL != pixel );

	func = ns_pixel_get_uint_channel_func( pixel->type, channel );
	ns_assert( NULL != func );

	return ( func )( pixel );
	}


nsfloat ns_pixel_get_float_channel( const NsPixel *pixel, nssize channel )
	{
	NsPixelGetFloatChannelFunc func;

	ns_assert( NULL != pixel );

	func = ns_pixel_get_float_channel_func( pixel->type, channel );
	ns_assert( NULL != func );

	return ( func )( pixel );
	}


void ns_pixel_set_uint_channel( NsPixel *pixel, nssize channel, nsuint value )
	{
	NsPixelSetUintChannelFunc func;

	ns_assert( NULL != pixel );

	func = ns_pixel_set_uint_channel_func( pixel->type, channel );
	ns_assert( NULL != func );

	( func )( pixel, value );
	}


void ns_pixel_set_float_channel( NsPixel *pixel, nssize channel, nsfloat value )
	{
	NsPixelSetFloatChannelFunc func;

	ns_assert( NULL != pixel );

	func = ns_pixel_set_float_channel_func( pixel->type, channel );
	ns_assert( NULL != func );

	( func )( pixel, value );
	}


void ns_pixel_info( NsPixelInfo *info, NsPixelType type )
	{
	ns_assert( NULL != info );
	ns_assert( type < NS_PIXEL_NUM_TYPES );

	*info = _ns_pixel_infos[ type ];
	}


nssize ns_pixel_num_channels( NsPixelType type )
	{
	NsPixelInfo info;

	ns_assert( type < NS_PIXEL_NUM_TYPES );

	ns_pixel_info( &info, type );
	return info.num_channels;
	}


nssize ns_pixel_bits( NsPixelType type )
	{
	NsPixelInfo            info;
	const NsPixelChannel  *channel;
	nssize                 bits;


	ns_pixel_info( &info, type );

	bits = 0;

	for( channel = info.channel; NULL != channel; channel = channel->next )
		bits += channel->bits;

	return bits;
	}


nssize ns_pixel_used_bits( NsPixelType type )
	{
	NsPixelInfo            info;
	const NsPixelChannel  *channel;
	nssize                 bits;


	ns_pixel_info( &info, type );

	bits = 0;

	for( channel = info.channel; NULL != channel; channel = channel->next )
		bits += channel->ubits;

	return bits;
	}


nsboolean ns_pixel_byte_bounded( NsPixelType type )
	{  return 0 == ( ns_pixel_bits( type ) % 8 );  }


nssize ns_pixel_buffer_size
	(
	NsPixelType  type, 
	nssize       width,
	nssize       height,
	nssize       length,
	nssize       row_align
	)
	{
	nsuintmax bytes;

	/* NOTE: Using a 'euintmax' in case the image is large. */
	bytes  = ( nsuintmax )ns_pixel_buffer_slice_size( type, width, height, row_align );
	bytes *= ( nsuintmax )( length );

	return ( nssize )bytes;
	}


/* Assuming the number of bytes per slice is less than the maximum
	value for the nssize data type. */
nssize ns_pixel_buffer_slice_size
	(
	NsPixelType  type,
	nssize       width,
	nssize       height,
	nssize       row_align
	)
	{  return ns_pixel_buffer_row_size( type, width, row_align ) * height;  }


/* Assuming the number of bytes per row can is less than the maximum
	value for the nssize data type. */
nssize ns_pixel_buffer_row_size
	(
	NsPixelType  type,
	nssize       width,
	nssize       row_align
	)
	{
	nssize bits, bytes;

	bits  = ns_pixel_bits( type ) * width;
	bytes = NS_BITS_TO_BYTES( bits );

	ns_assert(
		1 == row_align ||
		2 == row_align ||
		4 == row_align ||
		8 == row_align
		);

	return NS_ALIGN( bytes, row_align );
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_pixel_type, NsPixelType );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pixel_type, NsPixelType );
NS_VALUE_ARG_IMPLEMENT( ns_value_arg_pixel_type, NsPixelType );

NsError ns_value_register_pixel_type( void )
	{
	return ns_value_register(
				NS_VALUE_PIXEL_TYPE,
				sizeof( NsPixelType ),
				NULL,
				NULL,
				ns_value_arg_pixel_type
				);
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_pixel, NsPixel* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pixel, NsPixel* );
NS_VALUE_ARG_IMPLEMENT( ns_value_arg_pixel, NsPixel* );

NsError ns_value_register_pixel( void )
	{
	return ns_value_register(
				NS_VALUE_PIXEL,
				sizeof( NsPixel* ),
				NULL,
				NULL,
				ns_value_arg_pixel
				);
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_pixel_region, NsPixelRegion* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pixel_region, NsPixelRegion* );
NS_VALUE_ARG_IMPLEMENT( ns_value_arg_pixel_region, NsPixelRegion* );

NsError ns_value_register_pixel_region( void )
	{
	return ns_value_register(
				NS_VALUE_PIXEL_REGION,
				sizeof( NsPixelRegion* ),
				NULL,
				NULL,
				ns_value_arg_pixel_region
				);
	}


#include "nspixels-getpixel.h"
#include "nspixels-setpixel.h"
#include "nspixels-convert.h"
#include "nspixels-convert-imp.h"
#include "nspixels-copy.h"
#include "nspixels-blur.h"
#include "nspixels-brightnesscontrast.h"
#include "nspixels-orthoproject.h"
#include "nspixels-dynamicrange.h"
#include "nspixels-mpfilter.h"
#include "nspixels-resize.h"
#include "nspixels-gammacorrect.h"
#include "nspixels-noise.h"
#include "nspixels-flip.h"
#include "nspixels-subsample.h"


NsError ns_pixel_register_value_types( void )
	{
	NsError error;

	if( NS_FAILURE( ns_value_register_pixel_type(), error )   ||
		 NS_FAILURE( ns_value_register_pixel(), error )        ||
		 NS_FAILURE( ns_value_register_pixel_region(), error ) ||
		 NS_FAILURE( ns_value_register_progress(), error )     ||
		 NS_FAILURE( ns_value_register_file(), error )           )
		return error;

	return ns_no_error();
	}


NsError ns_pixel_proc_db_register_std( NsProcDb *db )
	{
	NsError error;

	ns_assert( NULL != db );

	if( NS_FAILURE( ns_pixel_register_value_types(), error ) )
		return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_GET_PIXEL ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_get_pixel() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_SET_PIXEL ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_set_pixel() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_CONVERT ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_convert() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_CONVERT_FOR_INTENSITY_MASK_PALETTE ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_convert_for_intensity_mask_palette() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_COPY ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_copy() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_BLUR ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_blur() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_BRIGHTNESS_CONTRAST ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_brightness_contrast() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_ORTHO_PROJECT ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_ortho_project() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_DYNAMIC_RANGE ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_dynamic_range() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_MP_FILTER ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_mp_filter() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_RESIZE ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_resize() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_GAMMA_CORRECT ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_gamma_correct() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_NOISE ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_noise() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_FLIP ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_flip() ), error ) )
			return error;

	if( ! ns_proc_db_is_registered( db, NS_PIXEL_PROC_SUBSAMPLE ) )
		if( NS_FAILURE( ns_proc_db_register( db, ns_pixel_proc_subsample() ), error ) )
			return error;

	return ns_no_error();	
	}


NsPixelConvertFunc ns_pixel_convert_func( NsPixelType src_type, NsPixelType dest_type )
	{
	ns_assert( src_type < NS_PIXEL_NUM_TYPES );
	ns_assert( dest_type < NS_PIXEL_NUM_TYPES );

	return _ns_pixel_convert_funcs[ src_type ][ dest_type ];
	}


NsPixelConvertFunc ns_pixel_convert_for_intensity_mask_palette_func( NsPixelType src_type, NsPixelType dest_type )
	{
	ns_assert( src_type < NS_PIXEL_NUM_TYPES );
	ns_assert( dest_type < NS_PIXEL_NUM_TYPES );

	return _ns_pixel_convert_for_intensity_mask_palette_funcs[ src_type ][ dest_type ];
	}


NsPixelRowGetFunc ns_pixel_row_get_func( NsPixelType type )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	return _ns_pixel_row_get_funcs[ type ];
	}


NsPixelRowSetFunc ns_pixel_row_set_func( NsPixelType type )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	return _ns_pixel_row_set_funcs[ type ];
	}


NsPixelGetUintChannelFunc ns_pixel_get_uint_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_get_uint_channel_funcs[ type ][ channel ];
	}


NsPixelGetFloatChannelFunc ns_pixel_get_float_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_get_float_channel_funcs[ type ][ channel ];
	}


NsPixelSetUintChannelFunc ns_pixel_set_uint_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_set_uint_channel_funcs[ type ][ channel ];
	}


NsPixelSetFloatChannelFunc ns_pixel_set_float_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_set_float_channel_funcs[ type ][ channel ];
	}


NsPixelRowGetUintChannelFunc ns_pixel_row_get_uint_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_row_get_uint_channel_funcs[ type ][ channel ];
	}


NsPixelRowGetFloatChannelFunc ns_pixel_row_get_float_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_row_get_float_channel_funcs[ type ][ channel ];
	}


NsPixelRowSetUintChannelFunc ns_pixel_row_set_uint_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_row_set_uint_channel_funcs[ type ][ channel ];
	}


NsPixelRowSetFloatChannelFunc ns_pixel_row_set_float_channel_func( NsPixelType type, nssize channel )
	{
	ns_assert( type < NS_PIXEL_NUM_TYPES );
	ns_assert( channel < NS_PIXEL_NUM_CHANNELS );

	return _ns_pixel_row_set_float_channel_funcs[ type ][ channel ];
	}


#include "nspixels-convert.inl"
#include "nspixels-row.inl"
#include "nspixels-channel-uint.inl"
#include "nspixels-channel-float.inl"
#include "nspixels-channel-uint-row.inl"
#include "nspixels-channel-float-row.inl"
#include "nspixels-info.inl"


void _ns_pixels_init( void )
	{
	_ns_pixel_convert_funcs_init();
	_ns_pixel_convert_for_intensity_mask_palette_funcs_init();

	_ns_pixel_row_get_funcs_init();
	_ns_pixel_row_set_funcs_init();

	_ns_pixel_get_uint_channel_funcs_init();
	_ns_pixel_get_float_channel_funcs_init();

	_ns_pixel_set_uint_channel_funcs_init();
	_ns_pixel_set_float_channel_funcs_init();

	_ns_pixel_row_get_uint_channel_funcs_init();
	_ns_pixel_row_get_float_channel_funcs_init();

	_ns_pixel_row_set_uint_channel_funcs_init();
	_ns_pixel_row_set_float_channel_funcs_init();

	_ns_pixel_infos_init();
	}


NsPixelRegion* ns_pixel_region
	(
	NsPixelRegion   *region,
	nssize           x,
	nssize           y,
	nssize           z,
	nssize           width,
	nssize           height,
	nssize           length
	)
	{
	ns_assert( NULL != region );

	region->x      = x;
	region->y      = y;
	region->z      = z;
	region->width  = width;
	region->height = height;
	region->length = length;

	return region;
	}


void _ns_pixel_region_verify
	(
	const NsPixelRegion  *region,
	nssize                width,
	nssize                height,
	nssize                length,
	const nschar         *details
	)
	{
	ns_assert( NULL != region );

	ns_verify_with_details( region->x < width, details );
	ns_verify_with_details( region->y < height, details );
	ns_verify_with_details( region->z < length, details );
	ns_verify_with_details( region->x + region->width  <= width, details );
	ns_verify_with_details( region->y + region->height <= height, details );
	ns_verify_with_details( region->z + region->length <= length, details );
	}


#ifndef NS_DEBUG

void ns_pixel_region_verify
	(
	const NsPixelRegion  *region,
	nssize                width,
	nssize                height,
	nssize                length
	)
	{
	ns_assert( NULL != region );
	_ns_pixel_region_verify( region, width, height, length, "" );
	}

#endif/* NS_DEBUG */


const NsPixelRegion* ns_pixel_region_extract
	(
	const NsPixelRegion  *sub_region,
	NsPixelRegion        *full_region,
	nssize                width,
	nssize                height,
	nssize                length
	)
	{
	const NsPixelRegion *region;

	if( NULL != sub_region )
		region = sub_region;
	else
		{
		ns_assert( NULL != full_region );

		region = ns_pixel_region(
						full_region,
						0,
						0,
						0,
						width,
						height,
						length
						);
		}

	return region;
	}


NsError ns_pixel_lut_init( NsPixelLut *lut, const NsPixelChannel *channel )
	{
	ns_assert( NULL != lut );
	ns_assert( NULL != channel );
	ns_assert( NS_PIXEL_CHANNEL_UINT == channel->type );
	ns_assert( channel->bits <= 16 );

	/* We need to use the actual number of bits (ubits) used in the channel as the table size. */
	lut->size   = 1 << channel->ubits;
	lut->values = ns_malloc( lut->size * sizeof( nsuint ) );
	
	return NULL != lut->values ?
			 ns_no_error() : ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


void ns_pixel_lut_finalize( NsPixelLut *lut )
	{
	ns_assert( NULL != lut );
	ns_free( lut->values );
	}
