
NS_PRIVATE void _ns_pixel_lum_u8_to_lum_u1( const NsPixelLumU8 *src, NsPixelLumU1 *dest )
	{
	/* 8-bit value to a 1-bit value. */
	dest->luminance = src->luminance >> 7;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_lum_u12( const NsPixelLumU8 *src, NsPixelLumU12 *dest )
	{
	/* 8-bit value to a 12-bit value. */
	dest->luminance = ( nsuint16 )src->luminance << 4;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_lum_u16( const NsPixelLumU8 *src, NsPixelLumU16 *dest )
	{
	/* 8-bit value to a 16-bit value. */
	dest->luminance = ( nsuint16 )src->luminance << 8;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_lum_f32( const NsPixelLumU8 *src, NsPixelLumF32 *dest )
	{
	/* Just convert to floating point number. */
	dest->luminance = ( nsfloat32 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_lum_alpha_u8_u8( const NsPixelLumU8 *src, NsPixelLumAlphaU8U8 *dest )
	{
	/* Set luminance value. Make alpha channel opaque. */
	dest->luminance = src->luminance;
	dest->alpha     = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_rgb_u8_u8_u8( const NsPixelLumU8 *src, NsPixelRgbU8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. */
	dest->red   = src->luminance;
	dest->green = src->luminance;
	dest->blue  = src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_rgba_u8_u8_u8_u8( const NsPixelLumU8 *src, NsPixelRgbaU8U8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Make alpha channel opaque. */
	dest->red   = src->luminance;
	dest->green = src->luminance;
	dest->blue  = src->luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_bgr_u8_u8_u8( const NsPixelLumU8 *src, NsPixelBgrU8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. */
	dest->blue  = src->luminance;
	dest->green = src->luminance;
	dest->red   = src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u8_to_bgra_u8_u8_u8_u8( const NsPixelLumU8 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	/* Set all the channels to the luminance value. Make alpha channel opaque. */
	dest->blue  = src->luminance;
	dest->green = src->luminance;
	dest->red   = src->luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}
