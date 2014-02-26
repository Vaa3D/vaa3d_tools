
NS_PRIVATE void _ns_pixel_lum_u12_to_lum_u1( const NsPixelLumU12 *src, NsPixelLumU1 *dest )
	{
	/* 12-bit value to a 1-bit value. */
	dest->luminance = ( nsuint8 )( src->luminance >> 11 );
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_lum_u8( const NsPixelLumU12 *src, NsPixelLumU8 *dest )
	{
	/* 12-bit value to an 8-bit value. */
	dest->luminance = ( nsuint8 )( src->luminance >> 4 );
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_lum_u16( const NsPixelLumU12 *src, NsPixelLumU16 *dest )
	{
	/* 12-bit value to a 16-bit value. */
	dest->luminance = src->luminance << 4;
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_lum_f32( const NsPixelLumU12 *src, NsPixelLumF32 *dest )
	{
	/* Just convert to floating point number. */
	dest->luminance = ( nsfloat32 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_lum_alpha_u8_u8( const NsPixelLumU12 *src, NsPixelLumAlphaU8U8 *dest )
	{
	/* 12-bit luminance value to an 8-bit value. Make alpha channel opaque. */
	dest->luminance = ( nsuint8 )( src->luminance >> 4 );
	dest->alpha     = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_rgb_u8_u8_u8( const NsPixelLumU12 *src, NsPixelRgbU8U8U8 *dest )
	{
	/* Set all the channels to the 8-bit luminance value. */
	dest->red   = ( nsuint8 )( src->luminance >> 4 );
	dest->green = ( nsuint8 )( src->luminance >> 4 );
	dest->blue  = ( nsuint8 )( src->luminance >> 4 );
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_rgba_u8_u8_u8_u8( const NsPixelLumU12 *src, NsPixelRgbaU8U8U8U8 *dest )
	{
	/* Set all the channels to the 8-bit luminance value. Make alpha channel opaque. */
	dest->red   = ( nsuint8 )( src->luminance >> 4 );
	dest->green = ( nsuint8 )( src->luminance >> 4 );
	dest->blue  = ( nsuint8 )( src->luminance >> 4 );
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_bgr_u8_u8_u8( const NsPixelLumU12 *src, NsPixelBgrU8U8U8 *dest )
	{
	/* Set all the channels to the 8-bit luminance value. */
	dest->blue  = ( nsuint8 )( src->luminance >> 4 );
	dest->green = ( nsuint8 )( src->luminance >> 4 );
	dest->red   = ( nsuint8 )( src->luminance >> 4 );
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_bgra_u8_u8_u8_u8( const NsPixelLumU12 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	/* Set all the channels to the 8-bit luminance value. Make alpha channel opaque. */
	dest->blue  = ( nsuint8 )( src->luminance >> 4 );
	dest->green = ( nsuint8 )( src->luminance >> 4 );
	dest->red   = ( nsuint8 )( src->luminance >> 4 );
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}
