
NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_lum_u1( const NsPixelLumAlphaU8U8 *src, NsPixelLumU1 *dest )
	{
	/* 8-bit value to a 1-bit value. Ignore alpha channel. */
	dest->luminance = src->luminance >> 7;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_lum_u8( const NsPixelLumAlphaU8U8 *src, NsPixelLumU8 *dest )
	{
	/* Set luminance value. Ignore alpha channel. */
	dest->luminance = src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_lum_u12( const NsPixelLumAlphaU8U8 *src, NsPixelLumU12 *dest )
	{
	/* 8-bit value to a 12-bit value. Ignore alpha channel. */
	dest->luminance = ( nsuint16 )src->luminance << 4;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_lum_u16( const NsPixelLumAlphaU8U8 *src, NsPixelLumU16 *dest )
	{
	/* 8-bit value to a 16-bit value. Ignore alpha channel. */
	dest->luminance = ( nsuint16 )src->luminance << 8;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_lum_f32( const NsPixelLumAlphaU8U8 *src, NsPixelLumF32 *dest )
	{
	/* Just convert to floating point number. Ignore alpha channel. */
	dest->luminance = ( nsfloat32 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_rgb_u8_u8_u8( const NsPixelLumAlphaU8U8 *src, NsPixelRgbU8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Ignore alpha channel. */
	dest->red   = src->luminance;
	dest->green = src->luminance;
	dest->blue  = src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_rgba_u8_u8_u8_u8( const NsPixelLumAlphaU8U8 *src, NsPixelRgbaU8U8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Set alpha channel. */
	dest->red   = src->luminance;
	dest->green = src->luminance;
	dest->blue  = src->luminance;
	dest->alpha = src->alpha;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_bgr_u8_u8_u8( const NsPixelLumAlphaU8U8 *src, NsPixelBgrU8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Ignore alpha channel. */
	dest->blue  = src->luminance;
	dest->green = src->luminance;
	dest->red   = src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_alpha_u8_u8_to_bgra_u8_u8_u8_u8( const NsPixelLumAlphaU8U8 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Set alpha channel. */
	dest->blue  = src->luminance;
	dest->green = src->luminance;
	dest->red   = src->luminance;
	dest->alpha = src->alpha;
	}
