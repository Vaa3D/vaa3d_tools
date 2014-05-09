#ifndef __NS_IMAGE_SAMPLER_H__
#define __NS_IMAGE_SAMPLER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsprogress.h>
#include <std/nsfile.h>
#include <std/nslog.h>
#include <std/nsbytearray.h>
#include <std/nshistogram.h>
#include <std/nsprogress.h>
#include <std/nsmutex.h>
#include <math/nsvector.h>
#include <math/nstriangle.h>
#include <math/nspoint.h>
#include <math/nssphere.h>
#include <math/nscircle.h>
#include <math/nscube.h>
#include <math/nsdragrect.h>
#include <math/nsprojector.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nscolor.h>
#include <image/nsseed.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>
#include <model/nsmodel-origs.h>
#include <model/nsrayburst.h>
#include <ext/threshold.h>
#include <ext/rotate.h>
#include <image/nssampler-decl.h>
#include <render/nsrender.h>


NS_DECLS_BEGIN

struct _NsSampler
	{
	NsList              samples;
	NsVoxelBuffer       src;
	NsVoxelBuffer       src_xy;
	NsVoxelBuffer       src_zy;
	NsVoxelBuffer       src_xz;
	const nsuint8      *lut_lum_u8;
	const NsVoxelInfo  *voxel_info;
	nsboolean           is_running;
	NsMutex             mutex;
	};


NS_IMPEXP void ns_sampler_construct( NsSampler *sampler );
NS_IMPEXP void ns_sampler_destruct( NsSampler *sampler );

NS_IMPEXP void ns_sampler_lock( const NsSampler *sampler );
NS_IMPEXP nsboolean ns_sampler_try_lock( const NsSampler *sampler );
NS_IMPEXP void ns_sampler_unlock( const NsSampler *sampler );

NS_IMPEXP void ns_sampler_init
	(
	NsSampler          *sampler,
	const NsVoxelInfo  *voxel_info,
	const NsImage      *image,
	const NsImage      *proj_xy,
	const NsImage      *proj_zy,
	const NsImage      *proj_xz
	);

NS_IMPEXP nssize ns_sampler_size( const NsSampler *sampler );

#define ns_sampler_is_empty( sampler )\
	( 0 == ns_sampler_size( (sampler) ) )

NS_IMPEXP NS_SYNCHRONIZED void ns_sampler_clear( NsSampler *sampler );

NS_IMPEXP NS_SYNCHRONIZED NsError ns_sampler_add( NsSampler *sampler, nspointer *sample );

NS_IMPEXP NS_SYNCHRONIZED void ns_sampler_remove( NsSampler *sampler, nspointer sample );
NS_IMPEXP NS_SYNCHRONIZED void ns_sampler_remove_selected( NsSampler *sampler );


NS_IMPEXP void ns_sampler_select( NsSampler *sampler, nsboolean selected );

NS_IMPEXP NsError ns_sampler_select_by_aabbox( NsSampler *sampler, const NsAABBox3d *B );

NS_IMPEXP NsError ns_sampler_select_by_projection_onto_drag_rect
	(
	NsSampler          *sampler,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj
	);


NS_IMPEXP nspointer ns_sampler_first( const NsSampler *sampler );
NS_IMPEXP nspointer ns_sampler_last( const NsSampler *sampler );

NS_IMPEXP nsfloat ns_sample_get_threshold( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_threshold( nspointer handle, nsfloat threshold );

NS_IMPEXP nsfloat ns_sample_get_contrast( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_contrast( nspointer handle, nsfloat contrast );


NS_IMPEXP nsfloat ns_sample_get_radius( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_radius( nspointer handle, nsfloat radius );

NS_IMPEXP void ns_sample_change_radius( nspointer handle, nsfloat percent, nsfloat min_radius );


NS_IMPEXP nsfloat ns_sample_get_length( nsconstpointer handle );

NS_IMPEXP void ns_sample_get_unit_direction( nsconstpointer handle, NsVector3f *D );

NS_IMPEXP void ns_sample_get_origin( nsconstpointer handle, NsVector3f *O );
NS_IMPEXP void ns_sample_set_origin( nspointer handle, const NsVector3f *O );

NS_IMPEXP void ns_sample_get_center( nsconstpointer handle, NsVector3f *C );
NS_IMPEXP void ns_sample_set_center( nspointer handle, const NsVector3f *C );

NS_IMPEXP nsfloat ns_sample_get_angle( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_angle( nspointer handle, nsfloat angle );

NS_IMPEXP nsdouble ns_sample_get_volume( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_volume( nspointer handle, nsdouble volume );

NS_IMPEXP nsdouble ns_sample_get_surface_area( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_surface_area( nspointer handle, nsdouble surface_area );

NS_IMPEXP nssize ns_sample_get_num_steps( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_num_steps( nspointer handle, nssize num_steps );

NS_IMPEXP nssize ns_sample_get_curr_step( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_curr_step( nspointer handle, nssize curr_step );

NS_IMPEXP void ns_sample_get_endpoints( nsconstpointer handle, NsVector3f *P1, NsVector3f *P2 );
NS_IMPEXP void ns_sample_set_endpoints( nspointer handle, const NsVector3f *P1, const NsVector3f *P2 );

NS_IMPEXP void ns_sample_get_rays( nsconstpointer handle, NsVector3f *R1, NsVector3f *R2 );
NS_IMPEXP void ns_sample_set_rays( nspointer handle, const NsVector3f *R1, const NsVector3f *R2 );

NS_IMPEXP void ns_sample_get_corners( nsconstpointer handle, NsVector3i *C1, NsVector3i *C2 );
NS_IMPEXP void ns_sample_set_corners( nspointer handle, const NsVector3i *C1, const NsVector3i *C2 );

NS_IMPEXP void ns_sample_get_distances( nsconstpointer handle, const nsfloat **distances, nssize *num_distances );
NS_IMPEXP void ns_sample_set_distances( nspointer handle, const nsfloat *distances, nssize num_distances );

NS_IMPEXP NsRayburstKernelType ns_sample_get_rayburst_kernel_type( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_rayburst_kernel_type( nspointer handle, NsRayburstKernelType kernel_type );

NS_IMPEXP nsint ns_sample_get_user_type( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_user_type( nspointer handle, nsint user_type );


NS_IMPEXP NsSpheref* ns_sample_to_sphere( const NsSampler *sampler, nspointer handle, NsSpheref *S );
NS_IMPEXP NsSphered* ns_sample_to_sphere_ex( const NsSampler *sampler, nspointer handle, NsSphered *S );


NS_IMPEXP nsboolean ns_sample_get_is_selected( nsconstpointer handle );
NS_IMPEXP void ns_sample_set_is_selected( nspointer handle, nsboolean is_selected );


NS_IMPEXP NS_SYNCHRONIZED NsError ns_sample_make_images( NsSampler *sampler, nspointer handle );


/* Applies the 'callback' function to every sample. */
NS_IMPEXP void ns_sampler_foreach
	(
	NsSampler *sampler,
	void ( *callback )( NsSampler *sampler, nspointer sample, nspointer user_data ),
	nspointer user_data
	);


NS_IMPEXP nspointer ns_sampler_begin( const NsSampler *sampler );
NS_IMPEXP nspointer ns_sampler_end( const NsSampler *sampler );

#define NS_SAMPLE_NIL  NULL

#define ns_sample_not_equal( handle1, handle2 )\
	ns_list_iter_not_equal( (handle1), (handle2) )

#define ns_sample_next( handle )\
	ns_list_iter_next( ( nslistiter )(handle) )

#define ns_sample_prev( handle )\
	ns_list_iter_prev( ( nslistiter )(handle) )


NS_IMPEXP void ns_sample_rotate_right( nspointer handle );
NS_IMPEXP void ns_sample_rotate_left( nspointer handle );
NS_IMPEXP void ns_sample_translate_up( nspointer handle );
NS_IMPEXP void ns_sample_translate_down( nspointer handle );


NS_IMPEXP void ns_sampler_render_borders
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_lines
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_triangles
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_spheres
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_ellipses
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_planes
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_images
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_text
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_selected
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);

NS_IMPEXP void ns_sampler_render_last
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	);


/* Write the samples to file. */
NS_IMPEXP NsError ns_sampler_output( const NsSampler *sampler, const nschar *name );


NS_IMPEXP NS_SYNCHRONIZED NsError ns_sampler_to_model( const NsSampler *sampler, NsModel *model );


/* #define NS_SAMPLER_RUN_ENABLE_3D */


NS_IMPEXP NsError ns_sampler_run
	(
	NsSampler             *sampler,
	const NsImage         *image,
	const NsPoint3f       *P1,
	const NsPoint3f       *P2,
	nssize                 num_steps,
	nsboolean              do_3d_radii,
	NsRayburstKernelType   kernel_type,
	NsRayburstInterpType   interp_type,
	const NsVoxelInfo     *voxel_info,
	nsdouble               average_intensity,
	const NsVector3i      *jitter,
	nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	nsboolean              use_fixed_threshold,
	nsfloat                threshold_value,
	nsfloat               *radius,
	nsfloat               *length,
	nsboolean             *did_create_sample
	);

NS_IMPEXP NsError ns_sampler_run_2d
	(
	NsSampler             *sampler,
	const NsImage         *image,
	const NsPoint2f       *P1,
	const NsPoint2f       *P2,
	nsfloat                z,
	nssize                 num_steps,
	NsRayburstInterpType   interp_type,
	const NsVoxelInfo     *voxel_info,
	nsdouble               average_intensity,
	const NsVector2i      *jitter,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	nsboolean              use_fixed_threshold,
	nsfloat                threshold_value,
	nsfloat               *radius,
	nsfloat               *length,
	nsboolean             *did_create_sample
	);


NS_IMPEXP NsError ns_sample_run
	(
	nspointer           sample,
	const NsImage      *image,
	const NsVoxelInfo  *voxel_info,
	nsdouble            average_intensity,
	const NsVector3i   *jitter,
	nsboolean           use_fixed_threshold,
	nsfloat             threshold_value,
	nsfloat            *ret_radius
	);


NS_IMPEXP nsboolean ns_sampler_is_running( const NsSampler *sampler );


NS_IMPEXP NsError ns_sampler_run_ex
	(
	NsSampler             *sampler,
	nspointer              sample,
	const NsVector3f      *V1,
	const NsVector3f      *V2,
	const NsModel         *model,
	const NsImage         *image,
	const NsCubei         *roi,
	nsboolean              do_3d_radii,
	NsRayburstKernelType   kernel_type,
	NsRayburstInterpType   interp_type,
	const NsVoxelInfo     *voxel_info,
	nsdouble               average_intensity,
	const NsVector3i      *jitter,
	nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	nsboolean              use_fixed_threshold,
	nsfloat                threshold_value,
	nssize                *num_created_samples,
	void                   ( *render )( nspointer user_data ),
	nspointer              user_data,
	NsProgress            *progress
	);

NS_DECLS_END

#endif/* __NS_IMAGE_SAMPLER_H__ */
