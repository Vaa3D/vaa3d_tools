#ifndef __NS_GUI_SEED3D_H__
#define __NS_GUI_SEED3D_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsprogress.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nssplats.h>
#include <math/nsvector.h>
#include <math/nscube.h>
#include <model/nsrayburst.h>
#include <ns-gl.h>

NS_DECLS_BEGIN


NS_IMPEXP void ns_seed_find_get_ray
	(
	nsint            window_x,
	nsint            window_y,
	const GLdouble   modelview_matrix[16],
	const GLdouble   projection_matrix[16],
	NsRay3d         *R
	);

NS_IMPEXP void ns_seed_find_get_ray_and_line
	(
	nsint            window_x,
	nsint            window_y,
	const GLdouble   modelview_matrix[16],
	const GLdouble   projection_matrix[16],
	NsRay3d         *R,
	NsLine3d        *L
	);

/* Sets the 'seed' and 'found' after calling. */
NS_IMPEXP NsError ns_seed_find_3d
	(
	const NsVoxelInfo  *voxel_info,
	const NsImage      *image,
	const NsCubei      *roi,
	const NsSplats     *splats,
	NsRay3d            *R,
	NsVector3i         *seed,
	nsboolean          *found,
	NsProgress         *progress
	);

NS_DECLS_END

#endif/* __NS_GUI_SEED3D_H__ */
