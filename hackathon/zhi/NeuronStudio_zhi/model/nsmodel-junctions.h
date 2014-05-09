#ifndef __NS_MODEL_MODEL_JUNCTIONS_H__
#define __NS_MODEL_MODEL_JUNCTIONS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <image/nssampler.h>
#include <model/nsmodel.h>
#include <model/nssettings.h>

NS_DECLS_BEGIN

NS_IMPEXP NsError ns_model_reposition_junction_vertices
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	);


NS_IMPEXP NsError ns_model_resize_junction_vertices
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	);

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_JUNCTIONS_H__ */
