#ifndef __NS_MODEL_MODEL_MASK_H__
#define __NS_MODEL_MODEL_MASK_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <math/nsvector.h>
#include <image/nscolor.h>
#include <image/nsimage.h>
#include <image/nsvoxel-buffer.h>
#include <model/nsmodel.h>
#include <render/nsrender.h>

NS_DECLS_BEGIN

typedef struct _NsModelMask
	{
	NsImage        image_xy;
	NsImage        image_zy;
	NsImage        image_xz;
	NsVoxelBuffer  buffer_xy;
	NsVoxelBuffer  buffer_zy;
	NsVoxelBuffer  buffer_xz;
	}
	NsModelMask;


NS_IMPEXP void ns_model_mask_construct( NsModelMask *mask );
NS_IMPEXP void ns_model_mask_destruct( NsModelMask *mask );

NS_IMPEXP NsError ns_model_mask_create
	(
	NsModelMask       *mask,
	const NsModel     *model,
	const NsImage     *volume,
	const NsSettings  *settings
	);

NS_IMPEXP nsboolean ns_model_mask_at( const NsModelMask *mask, const NsVector3i *V );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_MASK_H__ */
