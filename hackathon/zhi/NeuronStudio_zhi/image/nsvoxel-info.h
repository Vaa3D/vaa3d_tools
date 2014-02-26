#ifndef __NS_IMAGE_VOXEL_INFO_H__
#define __NS_IMAGE_VOXEL_INFO_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nslog.h>
#include <math/nsvector.h>

NS_DECLS_BEGIN

typedef struct _NsVoxelInfo
	{
	NsVector3f  size;
	NsVector3f  one_over_size;
	nsfloat     min_size;
	nsfloat     max_size;
	nsfloat     volume;
	nsfloat     surface_area;
	nsfloat     min_radius;
	nsfloat     max_radius;
	nsfloat     tolerance;
	nsfloat     max_size_xy;
	nsfloat     max_size_zy;
	nsfloat     max_size_xz;
	nsfloat     diag_length;
	}
	NsVoxelInfo;

NS_IMPEXP void ns_voxel_info
	(
	NsVoxelInfo  *voxel_info,
	nsfloat       size_x,
	nsfloat       size_y,
	nsfloat       size_z
	);

#define ns_voxel_info_min_size( voxel_info )\
	( (voxel_info)->min_size )

#define ns_voxel_info_max_size( voxel_info )\
	( (voxel_info)->max_size )

#define ns_voxel_info_min_radius( voxel_info )\
	( (voxel_info)->min_radius )

#define ns_voxel_info_max_radius( voxel_info )\
	( (voxel_info)->max_radius )

#define ns_voxel_info_volume( voxel_info )\
	( (voxel_info)->volume )

#define ns_voxel_info_surface_area( voxel_info )\
	( (voxel_info)->surface_area )

#define ns_voxel_info_size( voxel_info )\
	( &( (voxel_info)->size ) )

#define ns_voxel_info_one_over_size( voxel_info )\
	( &( (voxel_info)->one_over_size ) )

#define ns_voxel_info_size_x( voxel_info )\
	( (voxel_info)->size.x )

#define ns_voxel_info_size_y( voxel_info )\
	( (voxel_info)->size.y )

#define ns_voxel_info_size_z( voxel_info )\
	( (voxel_info)->size.z )

#define ns_voxel_info_one_over_size_x( voxel_info )\
	( (voxel_info)->one_over_size.x )

#define ns_voxel_info_one_over_size_y( voxel_info )\
	( (voxel_info)->one_over_size.y )

#define ns_voxel_info_one_over_size_z( voxel_info )\
	( (voxel_info)->one_over_size.z )

#define ns_voxel_info_tolerance( voxel_info )\
	( (voxel_info)->tolerance )


#define ns_voxel_info_max_size_xy( voxel_info )\
	( (voxel_info)->max_size_xy )

#define ns_voxel_info_max_size_zy( voxel_info )\
	( (voxel_info)->max_size_zy )

#define ns_voxel_info_max_size_xz( voxel_info )\
	( (voxel_info)->max_size_xz )


#define NS_TO_VOXEL_SPACE( in, voxel_size )\
	( ( nsfloat )(in) * (voxel_size) )

#define NS_TO_IMAGE_SPACE( in, one_over_voxel_size )\
	( ( nsfloat )(in) * (one_over_voxel_size) + .5f )


enum{ NS_COMPONENT_X, NS_COMPONENT_Y, NS_COMPONENT_Z };


NS_IMPEXP NS_INTERNAL void ____ns_to_voxel_space
	(
	const NsPoint3i    *in,
	NsPoint3f          *out,
	const NsVoxelInfo  *voxel_info,
	const nschar       *details
	);

NS_IMPEXP NS_INTERNAL void ____ns_to_image_space
	(
	const NsPoint3f    *in,
	NsPoint3i          *out,
	const NsVoxelInfo  *voxel_info,
	const nschar       *details
	);

NS_IMPEXP NS_INTERNAL nsfloat ____ns_to_voxel_space_component
	(
	nsint               in,
	const NsVoxelInfo  *voxel_info,
	nsint               which,
	const nschar       *details
	);

NS_IMPEXP NS_INTERNAL nsint ____ns_to_image_space_component
	(
	nsfloat             in,
	const NsVoxelInfo  *voxel_info,
	nsint               which,
	const nschar       *details
	);


#ifdef NS_DEBUG

#define ns_to_voxel_space( in, out, voxel_info )\
	____ns_to_voxel_space( (in), (out), (voxel_info), NS_MODULE )

#define ns_to_image_space( in, out, voxel_info )\
	____ns_to_image_space( (in), (out), (voxel_info), NS_MODULE )

#define ns_to_voxel_space_component( in, voxel_info, which )\
	____ns_to_voxel_space_component( (in), (voxel_info), (which), NS_MODULE )

#define ns_to_image_space_component( in, voxel_info, which )\
	____ns_to_image_space_component( (in), (voxel_info), (which), NS_MODULE )

#else

#define ns_to_voxel_space( in, out, voxel_info )\
	____ns_to_voxel_space( (in), (out), (voxel_info), NULL )

#define ns_to_image_space( in, out, voxel_info )\
	____ns_to_image_space( (in), (out), (voxel_info), NULL )

#define ns_to_voxel_space_component( in, voxel_info, which )\
	____ns_to_voxel_space_component( (in), (voxel_info), (which), NULL )

#define ns_to_image_space_component( in, voxel_info, which )\
	____ns_to_image_space_component( (in), (voxel_info), (which), NULL )

#endif/* NS_DEBUG */


/* NOTE: For the following functions, 'out' can equal 'in'
	if an "in-place" conversion is required. */

NS_IMPEXP void ns_to_voxel_space_ex
	(
	const NsPoint3f    *in,
	NsPoint3f          *out,
	const NsVoxelInfo  *voxel_info
	);

NS_IMPEXP void ns_to_image_space_ex
	(
	const NsPoint3f    *in,
	NsPoint3f          *out,
	const NsVoxelInfo  *voxel_info
	);

NS_IMPEXP nsfloat ns_to_voxel_space_component_ex
	(
	nsfloat             in,
	const NsVoxelInfo  *voxel_info,
	nsint               which
	);

NS_IMPEXP nsfloat ns_to_image_space_component_ex
	(
	nsfloat             in,
	const NsVoxelInfo  *voxel_info,
	nsint               which
	);


/* NOTE: When going from client to voxel space, one of the components
	will always be set to zero. For example if looking in XY (forward)
	direction, the z component of the 'out' vector will be zero. */
NS_IMPEXP void ns_client_to_voxel_space
	(
	const NsPoint2i    *in,
	NsPoint3f          *out,
	const NsVoxelInfo  *voxel_info,
	nsint               view, /* NS_XY, NS_ZY, or NS_XZ */
	const NsPoint2i    *corner,
	nsfloat             zoom
	);

NS_DECLS_END

#endif/* __NS_IMAGE_VOXEL_INFO_H__ */
