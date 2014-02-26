
NS_PRIVATE void _ns_pixel_lum_u1_to_lum_u8( const NsPixelLumU1 *src, NsPixelLumU8 *dest )
	{
	/* 1-bit value to a monochrome 8-bit value. */
	dest->luminance = src->luminance ? NS_UINT8_MAX : 0;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_lum_u12( const NsPixelLumU1 *src, NsPixelLumU12 *dest )
	{
	/* 1-bit value to a monochrome 12-bit value. */
	dest->luminance = src->luminance ? 4095 : 0;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_lum_u16( const NsPixelLumU1 *src, NsPixelLumU16 *dest )
	{
	/* 1-bit value to a monochrome 16-bit value. */
	dest->luminance = src->luminance ? NS_UINT16_MAX : 0;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_lum_f32( const NsPixelLumU1 *src, NsPixelLumF32 *dest )
	{
	/* Just convert to floating point number. */
	dest->luminance = ( nsfloat32 )src->luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_lum_alpha_u8_u8( const NsPixelLumU1 *src, NsPixelLumAlphaU8U8 *dest )
	{
	/* Set luminance value. Make alpha channel opaque. */
	dest->luminance = src->luminance ? NS_UINT8_MAX : 0;
	dest->alpha     = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_rgb_u8_u8_u8( const NsPixelLumU1 *src, NsPixelRgbU8U8U8 *dest )
	{
	nsuint8 luminance = src->luminance ? NS_UINT8_MAX : 0; 
	
	/* Set all the channels to the luminance value. */
	dest->red   = luminance;
	dest->green = luminance;
	dest->blue  = luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_rgba_u8_u8_u8_u8( const NsPixelLumU1 *src, NsPixelRgbaU8U8U8U8 *dest )
	{
	nsuint8 luminance = src->luminance ? NS_UINT8_MAX : 0; 

	/* Set all the channels to the luminance value. Make alpha channel opaque. */
	dest->red   = luminance;
	dest->green = luminance;
	dest->blue  = luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_bgr_u8_u8_u8( const NsPixelLumU1 *src, NsPixelBgrU8U8U8 *dest )
	{
	nsuint8 luminance = src->luminance ? NS_UINT8_MAX : 0; 

	/* Set all the channels to the luminance value. */
	dest->blue  = luminance;
	dest->green = luminance;
	dest->red   = luminance;
	}


NS_PRIVATE void _ns_pixel_lum_u1_to_bgra_u8_u8_u8_u8( const NsPixelLumU1 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	nsuint8 luminance = src->luminance ? NS_UINT8_MAX : 0; 

	/* Set all the channels to the luminance value. Make alpha channel opaque. */
	dest->blue  = luminance;
	dest->green = luminance;
	dest->red   = luminance;
	dest->alpha = NS_PIXEL_ALPHA_OPAQUE_U8;
	}
