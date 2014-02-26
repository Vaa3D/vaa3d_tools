#include "nsvoxel-buffer.h"
#include "nsvoxel-buffer.inl"


/* IMPORTANT: This macro will only work for pixels that
	have >= 8 bits and dont cross byte boundaries. */
#define __VOXEL( type, voxel_buffer, x, y, z )\
	NS_OFFSET_POINTER(\
		type,\
		(voxel_buffer)->mem_block,\
		((nssize)(z))*(voxel_buffer)->bytes_per_slice +\
		((nssize)(y))*(voxel_buffer)->bytes_per_row +\
		((nssize)(x))*sizeof(type)\
		)


#ifdef NS_DEBUG_EXTREME

NS_PRIVATE void ns_voxel_buffer_assert_xyz
	(
	const NsVoxelBuffer  *voxel_buffer,
	nsint                 x,
	nsint                 y,
	nsint                 z,
	const nschar         *file,
	nsint                 line
	)
	{
	ns_assert_at_file_line( 0 <= x, file, line );
	ns_assert_at_file_line( 0 <= y, file, line );
	ns_assert_at_file_line( 0 <= z, file, line );
	ns_assert_at_file_line( x < voxel_buffer->width, file, line );
	ns_assert_at_file_line( y < voxel_buffer->height, file, line );
	ns_assert_at_file_line( z < voxel_buffer->length, file, line );
	}

#endif/* NS_DEBUG_EXTREME */


NS_PRIVATE nsulong ns_voxel_buffer_get_lum_u8
	(
	const NsVoxelBuffer  *voxel_buffer,
	nsint                 x,
	nsint                 y,
	nsint                 z
	#ifdef NS_DEBUG_EXTREME
	,
	const nschar         *file,
	nsint                 line
	#endif
	)
	{
	#ifdef NS_DEBUG_EXTREME
	ns_voxel_buffer_assert_xyz( voxel_buffer, x, y, z, file, line );
	#endif

	return *( __VOXEL( const nsuint8, voxel_buffer, x, y, z ) );
	}


NS_PRIVATE nsulong ns_voxel_buffer_get_lum_u12_or_u16
	(
	const NsVoxelBuffer  *voxel_buffer,
	nsint                 x,
	nsint                 y,
	nsint                 z
	#ifdef NS_DEBUG_EXTREME
	,
	const nschar         *file,
	nsint                 line
	#endif
	)
	{
	#ifdef NS_DEBUG_EXTREME
	ns_voxel_buffer_assert_xyz( voxel_buffer, x, y, z, file, line );
	#endif

	return *( __VOXEL( const nsuint16, voxel_buffer, x, y, z ) );
	}


NS_PRIVATE void ns_voxel_buffer_set_lum_u8
	(
	NsVoxelBuffer  *voxel_buffer,
	nsint           x,
	nsint           y,
	nsint           z,
	nsulong         value
	#ifdef NS_DEBUG_EXTREME
	,
	const nschar   *file,
	nsint           line
	#endif
	)
	{
	#ifdef NS_DEBUG_EXTREME
	ns_voxel_buffer_assert_xyz( voxel_buffer, x, y, z, file, line );
	#endif

	*( __VOXEL( nsuint8, voxel_buffer, x, y, z ) ) = ( nsuint8 )value;
	}


NS_PRIVATE void ns_voxel_buffer_set_lum_u12_or_u16
	(
	NsVoxelBuffer  *voxel_buffer,
	nsint           x,
	nsint           y,
	nsint           z,
	nsulong         value
	#ifdef NS_DEBUG_EXTREME
	,
	const nschar   *file,
	nsint           line
	#endif
	)
	{
	#ifdef NS_DEBUG_EXTREME
	ns_voxel_buffer_assert_xyz( voxel_buffer, x, y, z, file, line );
	#endif

	*( __VOXEL( nsuint16, voxel_buffer, x, y, z ) ) = ( nsuint16 )value;
	}


#define _NS_VOXEL_BUFFER_FOR_ALL_FUNC\
	( callback )( &V, ( nsulong )*pixel, user_data )


NS_PRIVATE void ns_voxel_buffer_forall_xy_lum_u8
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_XY_TEMPLATE( nsuint8, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }


NS_PRIVATE void ns_voxel_buffer_forall_xy_lum_u12_or_u16
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_XY_TEMPLATE( nsuint16, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }

/*
NS_PRIVATE void ns_voxel_buffer_forall_zy_lum_u8
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_ZY_TEMPLATE( nsuint8, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }


NS_PRIVATE void ns_voxel_buffer_forall_zy_lum_u12_or_u16
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_ZY_TEMPLATE( nsuint16, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }


NS_PRIVATE void ns_voxel_buffer_forall_xz_lum_u8
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_XZ_TEMPLATE( nsuint8, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }


NS_PRIVATE void ns_voxel_buffer_forall_xz_lum_u12_or_u16
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	void                  ( *callback )( const NsVector3i*, nsulong, nspointer ),
	nspointer             user_data,
	NsProgress           *progress
	)
	{  _NS_VOXEL_BUFFER_FORALL_XZ_TEMPLATE( nsuint16, _NS_VOXEL_BUFFER_FOR_ALL_FUNC );  }
*/

nsulong ns_voxel_buffer_convert_to_lum_u8( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU8 pixel;

	ns_pixel_set_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM, ( nsuint )intensity );
	return ( nsulong )( ns_pixel_get_lum_u8( &voxel_buffer->pixel, &pixel )->luminance );
	}


nsulong ns_voxel_buffer_convert_to_lum_u12( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU12 pixel;

	ns_pixel_set_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM, ( nsuint )intensity );
	return ( nsulong )( ns_pixel_get_lum_u12( &voxel_buffer->pixel, &pixel )->luminance );
	}


nsulong ns_voxel_buffer_convert_to_lum_u16( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU16 pixel;

	ns_pixel_set_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM, ( nsuint )intensity );
	return ( nsulong )( ns_pixel_get_lum_u16( &voxel_buffer->pixel, &pixel )->luminance );
	}


nsulong ns_voxel_buffer_convert_from_lum_u8( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU8 pixel;

	ns_pixel_set_lum_u8( &voxel_buffer->pixel, ns_pixel_lum_u8( &pixel, ( nsuint8 )intensity ) );
	return ( nsulong )ns_pixel_get_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM );
	}


nsulong ns_voxel_buffer_convert_from_lum_u12( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU12 pixel;

	ns_pixel_set_lum_u12( &voxel_buffer->pixel, ns_pixel_lum_u12( &pixel, ( nsuint16 )intensity ) );
	return ( nsulong )ns_pixel_get_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM );
	}


nsulong ns_voxel_buffer_convert_from_lum_u16( NsVoxelBuffer *voxel_buffer, nsulong intensity )
	{
	NsPixelLumU16 pixel;

	ns_pixel_set_lum_u16( &voxel_buffer->pixel, ns_pixel_lum_u16( &pixel, ( nsuint16 )intensity ) );
	return ( nsulong )ns_pixel_get_uint_channel( &voxel_buffer->pixel, NS_PIXEL_CHANNEL_LUM );
	}


const nsuint8* ns_voxel_buffer_lut_lum_u8( void )
	{
	NS_PRIVATE const nsuint8 ____ns_voxel_buffer_lut_lum_u8[ NS_UINT8_MAX + 1 ] =
		{
		 32,  32,  32,  32,  32,  32,  32,  32,  33,  33,  33,  33,  33,  33,  34,  34,
		 34,  34,  35,  35,  35,  36,  36,  36,  37,  37,  38,  38,  39,  39,  40,  40,
		 41,  41,  42,  42,  43,  43,  44,  45,  45,  46,  47,  47,  48,  49,  49,  50,
		 51,  52,  52,  53,  54,  55,  56,  57,  58,  58,  59,  60,  61,  62,  63,  64,
		 65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  79,  80,  81,
		 82,  83,  84,  85,  87,  88,  89,  90,  91,  93,  94,  95,  96,  98,  99, 100,
		101, 103, 104, 105, 106, 108, 109, 110, 112, 113, 114, 116, 117, 118, 120, 121,
		122, 124, 125, 126, 128, 129, 130, 132, 133, 135, 136, 137, 139, 140, 141, 143,
		144, 146, 147, 148, 150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 163, 165,
		166, 167, 169, 170, 171, 173, 174, 175, 177, 178, 179, 181, 182, 183, 184, 186,
		187, 188, 189, 191, 192, 193, 194, 196, 197, 198, 199, 200, 202, 203, 204, 205,
		206, 207, 208, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222,
		223, 224, 225, 226, 227, 228, 229, 229, 230, 231, 232, 233, 234, 235, 235, 236,
		237, 238, 238, 239, 240, 240, 241, 242, 242, 243, 244, 244, 245, 245, 246, 246,
		247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253,
		253, 253, 254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255
		};

	return ____ns_voxel_buffer_lut_lum_u8;
	}


void ns_voxel_buffer_init( NsVoxelBuffer *voxel_buffer, const NsImage *image )
	{
	NsPixelType  pixel_type;
	nssize       row_align;


	ns_assert( NULL != voxel_buffer );
	ns_assert( NULL != image );

	ns_assert( ns_image_width( image )  <= ( nssize )NS_INT_MAX );
	ns_assert( ns_image_height( image ) <= ( nssize )NS_INT_MAX );
	ns_assert( ns_image_length( image ) <= ( nssize )NS_INT_MAX );

	voxel_buffer->width     = ( nsint )ns_image_width( image );
	voxel_buffer->height    = ( nsint )ns_image_height( image );
	voxel_buffer->length    = ( nsint )ns_image_length( image );
	voxel_buffer->mem_block = ns_image_pixels( image );

	ns_assert( 0 < voxel_buffer->width );
	ns_assert( 0 < voxel_buffer->height );
	ns_assert( 0 < voxel_buffer->length );
	ns_assert( NULL != voxel_buffer->mem_block );

	pixel_type = ns_image_pixel_type( image );
	row_align  = ns_image_row_align( image );

	voxel_buffer->bytes_per_row =
		ns_pixel_buffer_row_size(
			pixel_type,
			( nssize )voxel_buffer->width,
			row_align
			);

	voxel_buffer->bytes_per_slice =
		ns_pixel_buffer_slice_size(
			pixel_type,
			( nssize )voxel_buffer->width,
			( nssize )voxel_buffer->height,
			row_align
			);

	ns_pixel_init( &voxel_buffer->pixel, pixel_type );

	voxel_buffer->pixel_type = pixel_type;

	switch( pixel_type )
		{
		case NS_PIXEL_LUM_U8:
			voxel_buffer->get           = ns_voxel_buffer_get_lum_u8;
			voxel_buffer->set           = ns_voxel_buffer_set_lum_u8;
			voxel_buffer->max_intensity = NS_UINT8_MAX;
			voxel_buffer->dynamic_range = ( nsdouble )( voxel_buffer->max_intensity + 1 );

			voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XY ] = ns_voxel_buffer_forall_xy_lum_u8;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_ZY ] = ns_voxel_buffer_forall_zy_lum_u8;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XZ ] = ns_voxel_buffer_forall_xz_lum_u8;
			break;

		case NS_PIXEL_LUM_U12:
			voxel_buffer->get           = ns_voxel_buffer_get_lum_u12_or_u16;
			voxel_buffer->set           = ns_voxel_buffer_set_lum_u12_or_u16;
			voxel_buffer->max_intensity = 4095;
			voxel_buffer->dynamic_range = ( nsdouble )( voxel_buffer->max_intensity + 1 );

			voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XY ] = ns_voxel_buffer_forall_xy_lum_u12_or_u16;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_ZY ] = ns_voxel_buffer_forall_zy_lum_u12_or_u16;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XZ ] = ns_voxel_buffer_forall_xz_lum_u12_or_u16;
			break;

		case NS_PIXEL_LUM_U16:
			voxel_buffer->get           = ns_voxel_buffer_get_lum_u12_or_u16;
			voxel_buffer->set           = ns_voxel_buffer_set_lum_u12_or_u16;
			voxel_buffer->max_intensity = NS_UINT16_MAX;
			voxel_buffer->dynamic_range = ( nsdouble )( voxel_buffer->max_intensity + 1 );

			voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XY ] = ns_voxel_buffer_forall_xy_lum_u12_or_u16;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_ZY ] = ns_voxel_buffer_forall_zy_lum_u12_or_u16;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XZ ] = ns_voxel_buffer_forall_xz_lum_u12_or_u16;
			break;

		case NS_PIXEL_LUM_F32:
			voxel_buffer->get           = NULL;
			voxel_buffer->set           = NULL;
			voxel_buffer->max_intensity = 0;
			voxel_buffer->dynamic_range = 0.0;

			voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XY ] = NULL;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_ZY ] = NULL;
			//voxel_buffer->forall[ NS_VOXEL_BUFFER_FORALL_XZ ] = NULL;
			break;

		default:
			ns_assert_not_reached();
		}
	}


nsulong ns_voxel_get_ex( const NsVoxelBuffer *voxel_buffer, nsint x, nsint y, nsint z )
	{
	return ( 0 <= x && x < voxel_buffer->width &&
				0 <= y && y < voxel_buffer->height &&
				0 <= z && z < voxel_buffer->length )
				?
			 ns_voxel_get( voxel_buffer, x, y, z ) : 0;
	}


void ns_voxel_set_ex( NsVoxelBuffer *voxel_buffer, nsint x, nsint y, nsint z, nsulong intensity )
	{
	if( 0 <= x && x < voxel_buffer->width &&
		 0 <= y && y < voxel_buffer->height &&
		 0 <= z && z < voxel_buffer->length )
		ns_voxel_set( voxel_buffer, x, y, z, intensity );
	}


nsfloat ns_voxel_intensity_convert( nsfloat intensity, NsPixelType from, NsPixelType to )
	{
	switch( from )
		{
		case NS_PIXEL_LUM_U8:
			switch( to )
				{
				case NS_PIXEL_LUM_U8:
					break;

				case NS_PIXEL_LUM_U12:
					intensity *= 16.0f;
					break;

				case NS_PIXEL_LUM_U16:
					intensity *= 256.0f;
					break;

				default:
					ns_assert_not_reached();
				}
			break;

		case NS_PIXEL_LUM_U12:
			switch( to )
				{
				case NS_PIXEL_LUM_U8:
					intensity /= 16.0f;
					break;

				case NS_PIXEL_LUM_U12:
					break;

				case NS_PIXEL_LUM_U16:
					intensity *= 16.0f;
					break;

				default:
					ns_assert_not_reached();
				}
			break;

		case NS_PIXEL_LUM_U16:
			switch( to )
				{
				case NS_PIXEL_LUM_U8:
					intensity /= 256.0f;
					break;

				case NS_PIXEL_LUM_U12:
					intensity /= 16.0f;
					break;

				case NS_PIXEL_LUM_U16:
					break;

				default:
					ns_assert_not_reached();
				}
			break;

		default:
			ns_assert_not_reached();
		}

	return intensity;
	}
