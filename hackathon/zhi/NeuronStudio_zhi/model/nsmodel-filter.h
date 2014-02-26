#ifndef __NS_MODEL_MODEL_FILTER_H__
#define __NS_MODEL_MODEL_FILTER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsbytearray.h>
#include <std/nsprogress.h>
#include <math/nsvector.h>
#include <math/nsray.h>
#include <math/nspoint.h>
#include <math/nssphere.h>
#include <image/nsvoxel-info.h>
#include <model/nssettings.h>
#include <model/nsmodel.h>
#include <model/nsmodel-origs.h>
#include <model/nsmodel-junctions.h>

NS_DECLS_BEGIN

NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_filter
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	);


NS_IMPEXP void ns_model_filter_fix_small_radii
	(
	NsModel           *model,
	const NsSettings  *settings
	);

NS_IMPEXP void ns_model_filter_smooth
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   );

NS_IMPEXP void ns_model_filter_average_thresholds
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   );


NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_filter_discretize
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   );


/* Only call these on those models that are manually traced. */
NS_IMPEXP void ns_model_filter_adjust_shrinkage( NsModel *model, const NsVector3f *percent );
NS_IMPEXP void ns_model_filter_translate_vertices( NsModel *model, const NsVector3f *T );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_FILTER_H__ */
