
NS_PRIVATE nsfloat _ns_pixel_lum_u1_get_float_channel_lum( const NsPixelLumU1 *pixel )
	{  return ( nsfloat )pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_u8_get_float_channel_lum( const NsPixelLumU8 *pixel )
	{  return ( nsfloat )pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_u12_get_float_channel_lum( const NsPixelLumU12 *pixel )
	{  return ( nsfloat )pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_u16_get_float_channel_lum( const NsPixelLumU16 *pixel )
	{  return ( nsfloat )pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_f32_get_float_channel_lum( const NsPixelLumF32 *pixel )
	{  return pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_alpha_u8_u8_get_float_channel_lum( const NsPixelLumAlphaU8U8 *pixel )
	{  return ( nsfloat )pixel->luminance;  }

NS_PRIVATE nsfloat _ns_pixel_lum_alpha_u8_u8_get_float_channel_alpha( const NsPixelLumAlphaU8U8 *pixel )
	{  return ( nsfloat )pixel->alpha;  }

NS_PRIVATE nsfloat _ns_pixel_rgb_u8_u8_u8_get_float_channel_red( const NsPixelRgbU8U8U8 *pixel )
	{  return ( nsfloat )pixel->red;  }

NS_PRIVATE nsfloat _ns_pixel_rgb_u8_u8_u8_get_float_channel_green( const NsPixelRgbU8U8U8 *pixel )
	{  return ( nsfloat )pixel->green;  }

NS_PRIVATE nsfloat _ns_pixel_rgb_u8_u8_u8_get_float_channel_blue( const NsPixelRgbU8U8U8 *pixel )
	{  return ( nsfloat )pixel->blue;  }

NS_PRIVATE nsfloat _ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_red( const NsPixelRgbaU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->red;  }

NS_PRIVATE nsfloat _ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_green( const NsPixelRgbaU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->green;  }

NS_PRIVATE nsfloat _ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_blue( const NsPixelRgbaU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->blue;  }

NS_PRIVATE nsfloat _ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_alpha( const NsPixelRgbaU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->alpha;  }

NS_PRIVATE nsfloat _ns_pixel_bgr_u8_u8_u8_get_float_channel_blue( const NsPixelBgrU8U8U8 *pixel )
	{  return ( nsfloat )pixel->blue;  }

NS_PRIVATE nsfloat _ns_pixel_bgr_u8_u8_u8_get_float_channel_green( const NsPixelBgrU8U8U8 *pixel )
	{  return ( nsfloat )pixel->green;  }

NS_PRIVATE nsfloat _ns_pixel_bgr_u8_u8_u8_get_float_channel_red( const NsPixelBgrU8U8U8 *pixel )
	{  return ( nsfloat )pixel->red;  }

NS_PRIVATE nsfloat _ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_blue( const NsPixelBgraU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->blue;  }

NS_PRIVATE nsfloat _ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_green( const NsPixelBgraU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->green;  }

NS_PRIVATE nsfloat _ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_red( const NsPixelBgraU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->red;  }

NS_PRIVATE nsfloat _ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_alpha( const NsPixelBgraU8U8U8U8 *pixel )
	{  return ( nsfloat )pixel->alpha;  }


NS_PRIVATE void _ns_pixel_lum_u1_set_float_channel_lum( NsPixelLumU1 *pixel, nsfloat value )
	{  pixel->luminance = 0.0f < value ? 1 : 0;  }

NS_PRIVATE void _ns_pixel_lum_u8_set_float_channel_lum( NsPixelLumU8 *pixel, nsfloat value )
	{  pixel->luminance = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_lum_u12_set_float_channel_lum( NsPixelLumU12 *pixel, nsfloat value )
	{  pixel->luminance = ( nsuint16 )value;  }

NS_PRIVATE void _ns_pixel_lum_u16_set_float_channel_lum( NsPixelLumU16 *pixel, nsfloat value )
	{  pixel->luminance = ( nsuint16 )value;  }

NS_PRIVATE void _ns_pixel_lum_f32_set_float_channel_lum( NsPixelLumF32 *pixel, nsfloat value )
	{  pixel->luminance = value;  }

NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_set_float_channel_lum( NsPixelLumAlphaU8U8 *pixel, nsfloat value )
	{  pixel->luminance = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_set_float_channel_alpha( NsPixelLumAlphaU8U8 *pixel, nsfloat value )
	{  pixel->alpha = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_set_float_channel_red( NsPixelRgbU8U8U8 *pixel, nsfloat value )
	{  pixel->red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_set_float_channel_green( NsPixelRgbU8U8U8 *pixel, nsfloat value )
	{  pixel->green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_set_float_channel_blue( NsPixelRgbU8U8U8 *pixel, nsfloat value )
	{  pixel->blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_red( NsPixelRgbaU8U8U8U8 *pixel, nsfloat value )
	{  pixel->red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_green( NsPixelRgbaU8U8U8U8 *pixel, nsfloat value )
	{  pixel->green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_blue( NsPixelRgbaU8U8U8U8 *pixel, nsfloat value )
	{  pixel->blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_alpha( NsPixelRgbaU8U8U8U8 *pixel, nsfloat value )
	{  pixel->alpha = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_set_float_channel_blue( NsPixelBgrU8U8U8 *pixel, nsfloat value )
	{  pixel->blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_set_float_channel_green( NsPixelBgrU8U8U8 *pixel, nsfloat value )
	{  pixel->green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_set_float_channel_red( NsPixelBgrU8U8U8 *pixel, nsfloat value )
	{  pixel->red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_blue( NsPixelBgraU8U8U8U8 *pixel, nsfloat value )
	{  pixel->blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_green( NsPixelBgraU8U8U8U8 *pixel, nsfloat value )
	{  pixel->green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_red( NsPixelBgraU8U8U8U8 *pixel, nsfloat value )
	{  pixel->red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_alpha( NsPixelBgraU8U8U8U8 *pixel, nsfloat value )
	{  pixel->alpha = ( nsuint8 )value;  }


NS_PRIVATE void _ns_pixel_get_float_channel_funcs_init( void )
	{
	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u1_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u8_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u12_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u16_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_f32_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_alpha_u8_u8_get_float_channel_lum;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_lum_alpha_u8_u8_get_float_channel_alpha;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgb_u8_u8_u8_get_float_channel_red;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgb_u8_u8_u8_get_float_channel_green;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgb_u8_u8_u8_get_float_channel_blue;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_red;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_green;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_blue;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_rgba_u8_u8_u8_u8_get_float_channel_alpha;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgr_u8_u8_u8_get_float_channel_blue;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgr_u8_u8_u8_get_float_channel_green;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgr_u8_u8_u8_get_float_channel_red;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_blue;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_green;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_red;

	_ns_pixel_get_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_bgra_u8_u8_u8_u8_get_float_channel_alpha;
	}


NS_PRIVATE void _ns_pixel_set_float_channel_funcs_init( void )
	{
	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u1_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u8_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u12_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u16_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_f32_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_alpha_u8_u8_set_float_channel_lum;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_lum_alpha_u8_u8_set_float_channel_alpha;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgb_u8_u8_u8_set_float_channel_red;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgb_u8_u8_u8_set_float_channel_green;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgb_u8_u8_u8_set_float_channel_blue;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_red;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_green;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_blue;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_rgba_u8_u8_u8_u8_set_float_channel_alpha;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgr_u8_u8_u8_set_float_channel_blue;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgr_u8_u8_u8_set_float_channel_green;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgr_u8_u8_u8_set_float_channel_red;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_blue;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_green;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_red;

	_ns_pixel_set_float_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_bgra_u8_u8_u8_u8_set_float_channel_alpha;
	}
