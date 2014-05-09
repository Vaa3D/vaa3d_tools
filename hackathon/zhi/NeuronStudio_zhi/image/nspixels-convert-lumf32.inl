
NS_PRIVATE void _ns_pixel_lum_f32_to_lum_u1( const NsPixelLumF32 *src, NsPixelLumU1 *dest )
	{
	/* To 8-bit luminance value, with safe cast. */
	dest->luminance = ( nsuint8 )src->luminance;

	/* 8-bit value to a 1-bit value. */
	dest->luminance >>= 7;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_lum_u8( const NsPixelLumF32 *src, NsPixelLumU8 *dest )
	{
	/* To 8-bit luminance value, with safe cast. */
	dest->luminance = ( nsuint8 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_lum_u12( const NsPixelLumF32 *src, NsPixelLumU12 *dest )
	{
	/* To 12-bit luminance value, with safe cast.*/
	dest->luminance = ( nsuint16 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_lum_u16( const NsPixelLumF32 *src, NsPixelLumU16 *dest )
	{
	/* To 16-bit luminance value, with safe cast. */
	dest->luminance = ( nsuint16 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_lum_alpha_u8_u8( const NsPixelLumF32 *src, NsPixelLumAlphaU8U8 *dest )
	{
	/* To 8-bit luminance value, with safe cast. Make alpha channel opaque. */
	dest->luminance = ( nsuint8 )src->luminance;
	dest->alpha     = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_rgb_u8_u8_u8( const NsPixelLumF32 *src, NsPixelRgbU8U8U8 *dest )
	{
	nsuint8 luminance;

	/* Set all the channels to the 8-bit luminance value, with safe cast. */
	luminance   = ( nsuint8 )src->luminance;
	dest->red   = luminance;
	dest->green = luminance;
	dest->blue  = luminance;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_rgba_u8_u8_u8_u8( const NsPixelLumF32 *src, NsPixelRgbaU8U8U8U8 *dest )
	{
	nsuint8 luminance;

	/* Set all the channels to the 8-bit luminance value, with safe cast. Make alpha channel opaque. */
	luminance   = ( nsuint8 )src->luminance;
	dest->red   = luminance;
	dest->green = luminance;
	dest->blue  = luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_bgr_u8_u8_u8( const NsPixelLumF32 *src, NsPixelBgrU8U8U8 *dest )
	{
	nsuint8 luminance;

	/* Set all the channels to the 8-bit luminance value, with safe cast. */
	luminance   = ( nsuint8 )src->luminance;
	dest->blue  = luminance;
	dest->green = luminance;
	dest->red   = luminance;
	}


NS_PRIVATE void _ns_pixel_lum_f32_to_bgra_u8_u8_u8_u8( const NsPixelLumF32 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	nsuint8 luminance;

	/* Set all the channels to the 8-bit luminance value, with safe cast. Make alpha channel opaque. */
	luminance   = ( nsuint8 )src->luminance;
	dest->blue  = luminance;
	dest->green = luminance;
	dest->red   = luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}
