

#define _NS_PIXEL_RGB_U8_U8_U8_TO_LUM_U8( red, green, blue )\
	(\
	( nsuint8 )\
		(\
		( nsfloat )(red) * 0.3f\
			+\
		( nsfloat )(green) * 0.59f\
			+\
		( nsfloat )(blue) * 0.11f\
			+\
		0.5f\
		)\
	)


#include "nspixels-convert-lumu1.inl"
#include "nspixels-convert-lumu8.inl"
#include "nspixels-convert-lumu12.inl"
#include "nspixels-convert-lumu16.inl"
#include "nspixels-convert-lumf32.inl"
#include "nspixels-convert-lumalphau8u8.inl"
#include "nspixels-convert-rgbu8u8u8.inl"
#include "nspixels-convert-rgbau8u8u8u8.inl"
#include "nspixels-convert-bgru8u8u8.inl"
#include "nspixels-convert-bgrau8u8u8u8.inl"


NS_PRIVATE void _ns_pixel_convert_func_all( const NsPixel *src, NsPixel *dest )
	{  *dest = *src;  }


NS_PRIVATE void _ns_pixel_convert_funcs_init( void )
	{
	nsenum r, c;

	for( r = 0; r < NS_PIXEL_NUM_TYPES; ++r )
		for( c = 0; c < NS_PIXEL_NUM_TYPES; ++c )
			_ns_pixel_convert_funcs[r][c] = _ns_pixel_convert_func_all;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_u1_to_lum_u8; 

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_lum_u1_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_lum_u1_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_lum_u1_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_lum_u1_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_u1_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u1_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_u1_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U1 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u1_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_lum_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_lum_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_lum_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_lum_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_lum_u8_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_u8_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u8_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_u8_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U8 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u8_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_lum_u12_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_u12_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_lum_u12_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_lum_u12_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_lum_u12_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_u12_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u12_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_u12_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u12_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_lum_u16_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_u16_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_lum_u16_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_lum_u16_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_lum_u16_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_u16_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u16_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_u16_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_u16_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_lum_f32_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_f32_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_lum_f32_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_lum_f32_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_lum_f32_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_f32_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_f32_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_f32_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_F32 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_f32_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_lum_alpha_u8_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_alpha_u8_u8_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_lum_alpha_u8_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_lum_alpha_u8_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_lum_alpha_u8_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_lum_alpha_u8_u8_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_alpha_u8_u8_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_lum_alpha_u8_u8_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_LUM_ALPHA_U8_U8 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_lum_alpha_u8_u8_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_rgb_u8_u8_u8_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_rgb_u8_u8_u8_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_rgb_u8_u8_u8_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGB_U8_U8_U8 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_rgb_u8_u8_u8_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_bgr_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_RGBA_U8_U8_U8_U8 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_rgba_u8_u8_u8_u8_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_bgr_u8_u8_u8_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_bgr_u8_u8_u8_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_bgr_u8_u8_u8_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGR_U8_U8_U8 ][ NS_PIXEL_BGRA_U8_U8_U8_U8 ] =
		_ns_pixel_bgr_u8_u8_u8_to_bgra_u8_u8_u8_u8;


	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U1 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_u1;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U12 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_u12;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_U16 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_u16;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_F32 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_f32;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_LUM_ALPHA_U8_U8 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_lum_alpha_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_RGB_U8_U8_U8 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_rgb_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_RGBA_U8_U8_U8_U8 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_rgba_u8_u8_u8_u8;

	_ns_pixel_convert_funcs[ NS_PIXEL_BGRA_U8_U8_U8_U8 ][ NS_PIXEL_BGR_U8_U8_U8 ] =
		_ns_pixel_bgra_u8_u8_u8_u8_to_bgr_u8_u8_u8;
	}


NS_PRIVATE void _ns_pixel_lum_u12_to_lum_u8_for_intensity_mask_palette( const NsPixelLumU12 *src, NsPixelLumU8 *dest )
	{
	nsuint luminance = src->luminance;

	/* NOTE: 0 goes to 15, and 4095 goes to 4080. */
	if( luminance < 16 )
		luminance += 15;
	else if( luminance >= 4080 )
		luminance -= 15;

	/* 12-bit value to an 8-bit value. */
	dest->luminance = ( nsuint8 )( luminance >> 4 );
	}


NS_PRIVATE void _ns_pixel_lum_u16_to_lum_u8_for_intensity_mask_palette( const NsPixelLumU16 *src, NsPixelLumU8 *dest )
	{
	nsuint luminance = src->luminance;

	/* NOTE: 0 goes to 255, and 65535 goes to 65280. */
	if( luminance < 256 )
		luminance += 255;
	else if( luminance >= 65280 )
		luminance -= 255;

	/* 16-bit value to an 8-bit value. */
	dest->luminance = ( nsuint8 )( luminance >> 8 );
	}


NS_PRIVATE void _ns_pixel_convert_for_intensity_mask_palette_funcs_init( void )
	{
	nsenum r, c;

	for( r = 0; r < NS_PIXEL_NUM_TYPES; ++r )
		for( c = 0; c < NS_PIXEL_NUM_TYPES; ++c )
			_ns_pixel_convert_for_intensity_mask_palette_funcs[r][c] = _ns_pixel_convert_func_all;


	_ns_pixel_convert_for_intensity_mask_palette_funcs[ NS_PIXEL_LUM_U12 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_u12_to_lum_u8_for_intensity_mask_palette;

	_ns_pixel_convert_for_intensity_mask_palette_funcs[ NS_PIXEL_LUM_U16 ][ NS_PIXEL_LUM_U8 ] =
		_ns_pixel_lum_u16_to_lum_u8_for_intensity_mask_palette;
	}
