#ifndef __NS_MODEL_MODEL_GRAFT_H__
#define __NS_MODEL_MODEL_GRAFT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nslist.h>
#include <std/nsbytearray.h>
#include <std/nsprogress.h>
#include <std/nsmath.h>
#include <std/nsconfigdb.h>
#include <std/nsthread.h>
#include <math/nsvector.h>
#include <math/nssphere.h>
#include <math/nsfrustum.h>
#include <math/nscube.h>
#include <math/nsrandom.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nsvoxel-table.h>
#include <image/nscolor.h>
#include <image/nssampler.h>
#include <model/nssettings.h>
#include <model/nsmodel.h>
#include <model/nsrayburst.h>
#include <render/nsrender.h>
#include <ext/naas.h>

NS_DECLS_BEGIN


NS_IMPEXP nsboolean ns_grafting_do_filter( void );

/* 'use_2d_sampling', 'aabbox_scalar', and 'min_window' are as
	described in ns_grafting_calc_aabbox().  */
NS_IMPEXP NsError ns_grafting_read_params
	(
	const nschar  *file_name,
	nsboolean     *use_2d_sampling,
	nsfloat       *aabbox_scalar,
	nsint         *min_window
	);


NS_IMPEXP NsError ns_model_new_grafter( NsModel *model );
NS_IMPEXP void ns_model_delete_grafter( NsModel *model );

NS_IMPEXP nsboolean ns_model_grafter_is_running( const NsModel *model );


NS_IMPEXP void ns_model_grafter_clear_voxels( NsModel *model );


/* Creates BOTH the vertices AND the edges of the RAW model.

	'use_2d_sampling', 'aabbox_scalar', and 'min_window' are as
	described in ns_grafting_calc_aabbox().

	'did_run' is set to false if the grafting was seeded within
	the existing 'filtered_model'.
*/
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_graft
	(
	NsModel               *raw_model,
	NsModel               *filtered_model,
	const NsSettings      *settings,
	const NsImage         *image,
	const NsCubei         *roi,
	nsboolean              use_proj_xy_as_image,
	const NsImage         *proj_xy,
	const NsImage         *proj_zy,
	const NsImage         *proj_xz,
	nsfloat                first_threshold,
	nsfloat                first_contrast,
	NsRayburstInterpType   interp_type,
	nsdouble               average_intensity,
	nsfloat                min_confidence,
	nsfloat                update_rate,
	nsboolean              make_images,
	nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	NsProgress            *progress,
	void                   ( *render_func )( void ),
	NsLabelingType         labeling_type,
	NsResidualSmearType    residual_smear_type,
	nsboolean             *did_run
	);


NS_IMPEXP nsmodelvertex ns_model_grafter_begin_vertices( const NsModel *model );


NS_IMPEXP void ns_model_render_grafter
	(
	const NsModel  *model,
	NsRenderState  *renderer
	);


/* C1s, and C2s receive the corners of the
	AABBOX in image space.

	IMPORTANT: The 'min_window' MUST be odd!
*/
NS_IMPEXP void ns_grafting_calc_aabbox
	(
	const NsVoxelInfo  *voxel_info,
	nsfloat             max_distance, /* The size of the "radius" of the AABBOX in voxel space. */
	const NsPoint3f    *pivot, /* Center of the AABBOX in voxel space. */
	const NsVector3i   *max, /* Maximum image indices, i.e. width - 1, height - 1, length - 1. */
	nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar, /* Applied to the passed 'max_distance' parameter. */
	nsint               min_window, /* The minimum size in pixels of the calculate AABBOX. */
	NsVector3i         *C1,
	NsVector3i         *C2
	);

NS_IMPEXP void ns_grafting_calc_aabbox_ex
	(
	const NsVoxelInfo  *voxel_info,
	nsfloat             radius_x,
	nsfloat             radius_y,
	nsfloat             radius_z,
	const NsPoint3f    *pivot,
	const NsVector3i   *max,
	nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar, /* Applied to each of the passed radii. */
	nsint               min_window,
	NsVector3i         *C1,
	NsVector3i         *C2
	);



/* 'use_2d_sampling', 'aabbox_scalar', and 'min_window' are as
	described in ns_grafting_calc_aabbox().  */
NS_IMPEXP NsError ns_grafting_establish_thresholds_and_contrasts
	(
	const NsSettings  *settings,
	const NsImage     *image,
	const NsCubei     *roi,
	nsboolean          use_2d_sampling,
	nsfloat            aabbox_scalar,
	nsint              min_window,
	NsModel           *model,
	NsProgress        *progress
	);

NS_IMPEXP void ns_grafting_sample_values
	(
	NsVoxelBuffer     *voxel_buffer,
	nsfloat           *values,
	nssize             num_values,
	const NsVector3i  *C1,
	const NsVector3i  *C2
	);

NS_IMPEXP NsError ns_grafting_output_sample_values
	(
	const nsfloat  *values,
	nssize          num_values,
	const nschar   *file_name
	);

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_GRAFT_H__ */
