
NS_PRIVATE nsuint _ns_pixel_lum_u1_row_get_uint_channel_lum( const nsuint8 *row, nssize x )
	{  return ( nsuint )ns_bit_buf_is_set( row, x );  }

NS_PRIVATE nsuint _ns_pixel_lum_u8_row_get_uint_channel_lum( const NsPixelLumU8 *row, nssize x )
	{  return ( nsuint )row[x].luminance;  }

NS_PRIVATE nsuint _ns_pixel_lum_u12_row_get_uint_channel_lum( const NsPixelLumU12 *row, nssize x )
	{  return ( nsuint )row[x].luminance;  }

NS_PRIVATE nsuint _ns_pixel_lum_u16_row_get_uint_channel_lum( const NsPixelLumU16 *row, nssize x )
	{  return ( nsuint )row[x].luminance;  }

NS_PRIVATE nsuint _ns_pixel_lum_f32_row_get_uint_channel_lum( const NsPixelLumF32 *row, nssize x )
	{  return ( nsuint )row[x].luminance;  }

NS_PRIVATE nsuint _ns_pixel_lum_alpha_u8_u8_row_get_uint_channel_lum( const NsPixelLumAlphaU8U8 *row, nssize x )
	{  return ( nsuint )row[x].luminance;  }

NS_PRIVATE nsuint _ns_pixel_lum_alpha_u8_u8_row_get_uint_channel_alpha( const NsPixelLumAlphaU8U8 *row, nssize x )
	{  return ( nsuint )row[x].alpha;  }

NS_PRIVATE nsuint _ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_red( const NsPixelRgbU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].red;  }

NS_PRIVATE nsuint _ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_green( const NsPixelRgbU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].green;  }

NS_PRIVATE nsuint _ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_blue( const NsPixelRgbU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].blue;  }

NS_PRIVATE nsuint _ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_red( const NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].red;  }

NS_PRIVATE nsuint _ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_green( const NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].green;  }

NS_PRIVATE nsuint _ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_blue( const NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].blue;  }

NS_PRIVATE nsuint _ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_alpha( const NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].alpha;  }

NS_PRIVATE nsuint _ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_blue( const NsPixelBgrU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].blue;  }

NS_PRIVATE nsuint _ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_green( const NsPixelBgrU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].green;  }

NS_PRIVATE nsuint _ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_red( const NsPixelBgrU8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].red;  }

NS_PRIVATE nsuint _ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_blue( const NsPixelBgraU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].blue;  }

NS_PRIVATE nsuint _ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_green( const NsPixelBgraU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].green;  }

NS_PRIVATE nsuint _ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_red( const NsPixelBgraU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].red;  }

NS_PRIVATE nsuint _ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_alpha( const NsPixelBgraU8U8U8U8 *row, nssize x )
	{  return ( nsuint )row[x].alpha;  }


NS_PRIVATE void _ns_pixel_lum_u1_row_set_uint_channel_lum( nsuint8 *row, nssize x, nsuint value )
	{
	if( value )
		ns_bit_buf_set( row, x );
	else
		ns_bit_buf_clear( row, x );
	}

NS_PRIVATE void _ns_pixel_lum_u8_row_set_uint_channel_lum( NsPixelLumU8 *row, nssize x, nsuint value )
	{  row[x].luminance = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_lum_u12_row_set_uint_channel_lum( NsPixelLumU12 *row, nssize x, nsuint value )
	{  row[x].luminance = ( nsuint16 )value;  }

NS_PRIVATE void _ns_pixel_lum_u16_row_set_uint_channel_lum( NsPixelLumU16 *row, nssize x, nsuint value )
	{  row[x].luminance = ( nsuint16 )value;  }

NS_PRIVATE void _ns_pixel_lum_f32_row_set_uint_channel_lum( NsPixelLumF32 *row, nssize x, nsuint value )
	{  row[x].luminance = ( nsfloat )value;  }

NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_row_set_uint_channel_lum( NsPixelLumAlphaU8U8 *row, nssize x, nsuint value )
	{  row[x].luminance = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_row_set_uint_channel_alpha( NsPixelLumAlphaU8U8 *row, nssize x, nsuint value )
	{  row[x].alpha = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_red( NsPixelRgbU8U8U8 *row, nssize x, nsuint value )
	{  row[x].red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_green( NsPixelRgbU8U8U8 *row, nssize x, nsuint value )
	{  row[x].green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_blue( NsPixelRgbU8U8U8 *row, nssize x, nsuint value )
	{  row[x].blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_red( NsPixelRgbaU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_green( NsPixelRgbaU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_blue( NsPixelRgbaU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_alpha( NsPixelRgbaU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].alpha = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_blue( NsPixelBgrU8U8U8 *row, nssize x, nsuint value )
	{  row[x].blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_green( NsPixelBgrU8U8U8 *row, nssize x, nsuint value )
	{  row[x].green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_red( NsPixelBgrU8U8U8 *row, nssize x, nsuint value )
	{  row[x].red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_blue( NsPixelBgraU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].blue = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_green( NsPixelBgraU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].green = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_red( NsPixelBgraU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].red = ( nsuint8 )value;  }

NS_PRIVATE void _ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_alpha( NsPixelBgraU8U8U8U8 *row, nssize x, nsuint value )
	{  row[x].alpha = ( nsuint8 )value;  }


NS_PRIVATE void _ns_pixel_row_get_uint_channel_funcs_init( void )
	{
	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u1_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u8_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u12_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u16_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_f32_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_alpha_u8_u8_row_get_uint_channel_lum;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_lum_alpha_u8_u8_row_get_uint_channel_alpha;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_red;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_green;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgb_u8_u8_u8_row_get_uint_channel_blue;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_red;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_green;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_blue;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_get_uint_channel_alpha;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_blue;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_green;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgr_u8_u8_u8_row_get_uint_channel_red;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_blue;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_green;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_red;

	_ns_pixel_row_get_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_get_uint_channel_alpha;
	}


NS_PRIVATE void _ns_pixel_row_set_uint_channel_funcs_init( void )
	{
	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u1_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u8_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u12_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_u16_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_f32_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_LUM ] =
		_ns_pixel_lum_alpha_u8_u8_row_set_uint_channel_lum;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_lum_alpha_u8_u8_row_set_uint_channel_alpha;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_red;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_green;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgb_u8_u8_u8_row_set_uint_channel_blue;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_red;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_green;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_blue;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_rgba_u8_u8_u8_u8_row_set_uint_channel_alpha;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_blue;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_green;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgr_u8_u8_u8_row_set_uint_channel_red;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_BLUE ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_blue;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_GREEN ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_green;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_RED ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_red;

	_ns_pixel_row_set_uint_channel_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_CHANNEL_ALPHA ] =
		_ns_pixel_bgra_u8_u8_u8_u8_row_set_uint_channel_alpha;
	}
