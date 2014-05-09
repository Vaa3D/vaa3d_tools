#ifndef __NS_IMAGE_VOXEL_BUFFER_H__
#define __NS_IMAGE_VOXEL_BUFFER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsprogress.h>
#include <math/nsvector.h>
#include <math/nscube.h>
#include <image/nspixels.h>
#include <image/nsimage.h>

NS_DECLS_BEGIN

struct _NsVoxelBuffer;
typedef struct _NsVoxelBuffer NsVoxelBuffer;


typedef void ( *_NsVoxelBufferForAllFunc )
		(
		const NsVoxelBuffer*,
		const NsCubei*,
		void (*)( const NsVector3i*, nsulong, nspointer ),
		nspointer,
		NsProgress*
		);


enum
	{
	NS_VOXEL_BUFFER_FORALL_XY/*,
	NS_VOXEL_BUFFER_FORALL_ZY,
	NS_VOXEL_BUFFER_FORALL_XZ*/
	};


struct _NsVoxelBuffer
	{
	nspointer    mem_block;
	NsPixelType  pixel_type;
	nssize       bytes_per_row;
	nssize       bytes_per_slice;
	nsulong      max_intensity;
	nsdouble     dynamic_range;
	NsPixel      pixel;

	nsulong ( *get )
		(
		const NsVoxelBuffer*,
		nsint,
		nsint,
		nsint
		#ifdef NS_DEBUG
		, const nschar*, nsint
		#endif
		);

	void ( *set )
		(
		NsVoxelBuffer*,
		nsint,
		nsint,
		nsint,
		nsulong
		#ifdef NS_DEBUG
		, const nschar*, nsint
		#endif
		);

	_NsVoxelBufferForAllFunc forall[3];

	nsint  width;
	nsint  height;
	nsint  length;
	};


NS_IMPEXP void ns_voxel_buffer_init( NsVoxelBuffer *voxel_buffer, const NsImage *image );


NS_IMPEXP nsulong ns_voxel_buffer_convert_to_lum_u8( NsVoxelBuffer *voxel_buffer, nsulong intensity );
NS_IMPEXP nsulong ns_voxel_buffer_convert_to_lum_u12( NsVoxelBuffer *voxel_buffer, nsulong intensity );
NS_IMPEXP nsulong ns_voxel_buffer_convert_to_lum_u16( NsVoxelBuffer *voxel_buffer, nsulong intensity );

NS_IMPEXP nsulong ns_voxel_buffer_convert_from_lum_u8( NsVoxelBuffer *voxel_buffer, nsulong intensity );
NS_IMPEXP nsulong ns_voxel_buffer_convert_from_lum_u12( NsVoxelBuffer *voxel_buffer, nsulong intensity );
NS_IMPEXP nsulong ns_voxel_buffer_convert_from_lum_u16( NsVoxelBuffer *voxel_buffer, nsulong intensity );


NS_IMPEXP nsfloat ns_voxel_intensity_convert( nsfloat intensity, NsPixelType from, NsPixelType to );


NS_IMPEXP const nsuint8* ns_voxel_buffer_lut_lum_u8( void );


#ifdef NS_DEBUG

#define ns_voxel_get( voxel_buffer, x, y, z )\
	( ( (voxel_buffer)->get )( (voxel_buffer), (x), (y), (z), __FILE__, __LINE__ ) )

#define ns_voxel_set( voxel_buffer, x, y, z, value )\
	( ( (voxel_buffer)->set )( (voxel_buffer), (x), (y), (z), (value), __FILE__, __LINE__ ) )

#else

#define ns_voxel_get( voxel_buffer, x, y, z )\
	( ( (voxel_buffer)->get )( (voxel_buffer), (x), (y), (z) ) )

#define ns_voxel_set( voxel_buffer, x, y, z, value )\
	( ( (voxel_buffer)->set )( (voxel_buffer), (x), (y), (z), (value) ) )

#endif/* NS_DEBUG */


/* The x,y,z can be out of bounds for this one, in which case 0 is returned as the intensity. */
NS_IMPEXP nsulong ns_voxel_get_ex( const NsVoxelBuffer *voxel_buffer, nsint x, nsint y, nsint z );
NS_IMPEXP void ns_voxel_set_ex( NsVoxelBuffer *voxel_buffer, nsint x, nsint y, nsint z, nsulong intensity );


#define ns_voxel_buffer_forall( voxel_buffer, which, callback, user_data, progress )\
	( ( (voxel_buffer)->forall[ which ] )( (voxel_buffer), NULL, (callback), (user_data), (progress) ) )

#define ns_voxel_buffer_forall_ex( voxel_buffer, which, roi, callback, user_data, progress )\
	( ( (voxel_buffer)->forall[ which ] )( (voxel_buffer), (roi), (callback), (user_data), (progress) ) )


#define ns_voxel_buffer_mem_block( voxel_buffer )\
	( (voxel_buffer)->mem_block )

#define ns_voxel_buffer_pixel_type( voxel_buffer )\
	( (voxel_buffer)->pixel_type )

#define ns_voxel_buffer_bytes_per_row( voxel_buffer )\
	( (voxel_buffer)->bytes_per_row )

#define ns_voxel_buffer_bytes_per_slice( voxel_buffer )\
	( (voxel_buffer)->bytes_per_slice )

#define ns_voxel_buffer_max_intensity( voxel_buffer )\
	( (voxel_buffer)->max_intensity )

#define ns_voxel_buffer_dynamic_range( voxel_buffer )\
	( (voxel_buffer)->dynamic_range )

#define ns_voxel_buffer_width( voxel_buffer )\
	( (voxel_buffer)->width )

#define ns_voxel_buffer_height( voxel_buffer )\
	( (voxel_buffer)->height )

#define ns_voxel_buffer_length( voxel_buffer )\
	( (voxel_buffer)->length )

NS_DECLS_END

#endif/* __NS_IMAGE_VOXEL_BUFFER_H__ */
