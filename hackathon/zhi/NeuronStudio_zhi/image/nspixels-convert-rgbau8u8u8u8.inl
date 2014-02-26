
NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_u1( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumU1 *dest )
	{
	/* Use conversion formula to 8-bit luminance value ignoring alpha channel.
		Then scale to a 1-bit value. */
	nsuint8 luminance = _NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue );
	dest->luminance = luminance >> 7;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_u8( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumU8 *dest )
	{
	/* Use conversion formula to 8-bit luminance value ignoring alpha channel. */
	dest->luminance = _NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue );
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_u12( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumU12 *dest )
	{
	/* Use conversion formula to 8-bit luminance value ignoring alpha channel. Then scale the
		8-bit value to a 12-bit value. */
	dest->luminance = ( nsuint16 )_NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue ) << 4;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_u16( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumU16 *dest )
	{
	/* Use conversion formula to 8-bit luminance value ignoring alpha channel. Then scale the
		8-bit value to a 16-bit value. */
	dest->luminance = ( nsuint16 )_NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue ) << 8;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_f32( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumF32 *dest )
	{
	/* Use conversion formula to 8-bit luminance value ignoring alpha channel. Then convert to a
		floating point number. */
	dest->luminance = ( nsfloat32 )_NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue );
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_lum_alpha_u8_u8( const NsPixelRgbaU8U8U8U8 *src, NsPixelLumAlphaU8U8 *dest )
	{
	/* Use conversion formula to 8-bit luminance value. Set alpha channel. */
	dest->luminance = _NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( src->red, src->green, src->blue );
	dest->alpha     = src->alpha;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_rgb_u8_u8_u8( const NsPixelRgbaU8U8U8U8 *src, NsPixelRgbU8U8U8 *dest )
	{
	/* Set all the channels ignoring alpha channel. */
	dest->red   = src->red;
	dest->green = src->green;
	dest->blue  = src->blue;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_bgr_u8_u8_u8( const NsPixelRgbaU8U8U8U8 *src, NsPixelBgrU8U8U8 *dest )
	{
	/* Set all the channels ignoring alpha channel. */
	dest->blue  = src->blue;
	dest->green = src->green;
	dest->red   = src->red;
	}


NS_PRIVATE void _ns_pixel_rgba_u8_u8_u8_u8_to_bgra_u8_u8_u8_u8( const NsPixelRgbaU8U8U8U8 *src, NsPixelBgraU8U8U8U8 *dest )
	{
	/* Set all the channels. */
	dest->blue  = src->blue;
	dest->green = src->green;
	dest->red   = src->red;
	dest->alpha = src->alpha;
	}
