
NS_PRIVATE void _ns_pixel_row_get_func_lum_u1( NsPixelLumU1 *pixel, const nsuint8 *row, nssize x )
	{  pixel->luminance = ns_bit_buf_is_set( row, x ) ? 1 : 0;  }

NS_PRIVATE void _ns_pixel_row_get_func_lum_u8( NsPixelLumU8 *pixel, const NsPixelLumU8 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_lum_u12( NsPixelLumU12 *pixel, const NsPixelLumU12 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_lum_u16( NsPixelLumU16 *pixel, const NsPixelLumU16 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_lum_f32( NsPixelLumF32 *pixel, const NsPixelLumF32 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_lum_alpha_u8_u8( NsPixelLumAlphaU8U8 *pixel, const NsPixelLumAlphaU8U8 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_rgb_u8_u8_u8( NsPixelRgbU8U8U8 *pixel, const NsPixelRgbU8U8U8 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_rgba_u8_u8_u8_u8( NsPixelRgbaU8U8U8U8 *pixel, const NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_bgr_u8_u8_u8( NsPixelBgrU8U8U8 *pixel, const NsPixelBgrU8U8U8 *row, nssize x )
	{  *pixel = *( row + x );  }

NS_PRIVATE void _ns_pixel_row_get_func_bgra_u8_u8_u8_u8( NsPixelBgraU8U8U8U8 *pixel, const NsPixelBgraU8U8U8U8 *row, nssize x )
	{  *pixel = *( row + x );  }


NS_PRIVATE void _ns_pixel_row_set_func_lum_u1( const NsPixelLumU1 *pixel, nsuint8 *row, nssize x )
	{
	if( pixel->luminance )
		ns_bit_buf_set( row, x );
	else
		ns_bit_buf_clear( row, x );
	}

NS_PRIVATE void _ns_pixel_row_set_func_lum_u8( const NsPixelLumU8 *pixel, NsPixelLumU8 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_lum_u12( const NsPixelLumU12 *pixel, NsPixelLumU12 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_lum_u16( const NsPixelLumU16 *pixel, NsPixelLumU16 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_lum_f32( const NsPixelLumF32 *pixel, NsPixelLumF32 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_lum_alpha_u8_u8( const NsPixelLumAlphaU8U8 *pixel, NsPixelLumAlphaU8U8 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_rgb_u8_u8_u8( const NsPixelRgbU8U8U8 *pixel, NsPixelRgbU8U8U8 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_rgba_u8_u8_u8_u8( const NsPixelRgbaU8U8U8U8 *pixel, NsPixelRgbaU8U8U8U8 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_bgr_u8_u8_u8( const NsPixelBgrU8U8U8 *pixel, NsPixelBgrU8U8U8 *row, nssize x )
	{  *( row + x ) = *pixel;  }

NS_PRIVATE void _ns_pixel_row_set_func_bgra_u8_u8_u8_u8( const NsPixelBgraU8U8U8U8 *pixel, NsPixelBgraU8U8U8U8 *row, nssize x )
	{  *( row + x ) = *pixel;  }


NS_PRIVATE void _ns_pixel_row_get_funcs_init( void )
	{
	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_row_get_func_lum_u1;

	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_row_get_func_lum_u8;

	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_row_get_func_lum_u12;

	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_row_get_func_lum_u16;

	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_row_get_func_lum_f32;

	_ns_pixel_row_get_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_row_get_func_lum_alpha_u8_u8;

	_ns_pixel_row_get_funcs[ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_row_get_func_rgb_u8_u8_u8;

	_ns_pixel_row_get_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_row_get_func_rgba_u8_u8_u8_u8;

	_ns_pixel_row_get_funcs[ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_row_get_func_bgr_u8_u8_u8;

	_ns_pixel_row_get_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_row_get_func_bgra_u8_u8_u8_u8;
	}


NS_PRIVATE void _ns_pixel_row_set_funcs_init( void )
	{
	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_row_set_func_lum_u1;

	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_row_set_func_lum_u8;

	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_row_set_func_lum_u12;

	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_row_set_func_lum_u16;

	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_row_set_func_lum_f32;

	_ns_pixel_row_set_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_row_set_func_lum_alpha_u8_u8;

	_ns_pixel_row_set_funcs[ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_row_set_func_rgb_u8_u8_u8;

	_ns_pixel_row_set_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_row_set_func_rgba_u8_u8_u8_u8;

	_ns_pixel_row_set_funcs[ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_row_set_func_bgr_u8_u8_u8;

	_ns_pixel_row_set_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_row_set_func_bgra_u8_u8_u8_u8;
	}
