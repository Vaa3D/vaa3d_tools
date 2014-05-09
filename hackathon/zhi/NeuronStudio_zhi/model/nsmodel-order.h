#ifndef __NS_MODEL_MODEL_ORDER_H__
#define __NS_MODEL_MODEL_ORDER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_MODEL_ORDER_NONE,
   NS_MODEL_ORDER_CENTRIFUGAL,
   NS_MODEL_ORDER_CENTRIPETAL,

	NS_MODEL_ORDER_NUM_TYPES  /* Dont use! */
	}
	NsModelOrderType;


NS_IMPEXP NsError ns_model_order
	(
	NsModel           *model,
	NsModelOrderType   type,
	NsProgress        *progress
	);

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_ORDER_H__ */
