#ifndef __NS_IMAGE_SEED_H__
#define __NS_IMAGE_SEED_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsbytearray.h>
#include <std/nsprogress.h>
#include <image/nsimage.h>
#include <image/nsvoxel-buffer.h>
#include <math/nscube.h>

NS_DECLS_BEGIN


enum
	{
	NS_SEED_ZY_FIND_X,
	NS_SEED_XZ_FIND_Y,
	NS_SEED_XY_FIND_Z
	};


/* NOTE: 'a' and 'b' is the location of the seed in the
	other two dimensions.

	example: x = 24, y = 56, z = ?

	call as: ns_seed_find_2d( image, x, y, NS_SEED_XY_FIND_Z, &z )
*/

NS_IMPEXP NsError ns_seed_find_2d
	(
	const NsImage  *image,
	const NsCubei  *roi,
	nsint           a,
	nsint           b,
	nsenum          which,
	nsint          *index,
	NsProgress     *progress
	);


/* NOTE: Call ns_free() on 'indices' when not needed anymore.
	This should be called even if 'num_indices' is set to 0. */
NS_IMPEXP NsError ns_seed_find_2d_ex
	(
	const NsImage   *image,
	const NsCubei   *roi,
	nsint            a,
	nsint            b,
	nsenum           which,
	nsint          **indices,
	nsint           *num_indices,
	NsProgress      *progress
	);

NS_DECLS_END

#endif/* __NS_IMAGE_SEED_H__ */
