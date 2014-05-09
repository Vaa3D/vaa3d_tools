#ifndef __NS_MODEL_MODEL_ORIGS_H__
#define __NS_MODEL_MODEL_ORIGS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

/* TEMP??? */
#define NS_MODEL_ORIGIN_SCALAR  4.0f

NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_create_origins( NsModel *model, NsProgress *progress );

/* Only adds an origin if there isnt one already there for the component that
	the vertex 'V' is in. */
NS_IMPEXP NsError ns_model_origin_create_if_none( NsModel *model, nsmodelvertex V );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_ORIGS_H__ */
