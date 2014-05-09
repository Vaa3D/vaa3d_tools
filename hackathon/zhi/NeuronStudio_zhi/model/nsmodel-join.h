#ifndef __NS_MODEL_MODEL_JOIN_H__
#define __NS_MODEL_MODEL_JOIN_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

/* Attempts to link the 'src' model vertices onto the 'dest' model. If it does
	link up and the 'src' model is manually traced, the component of the 'dest'
	model it attached to is also set to manually traced. If the 'src' model doesnt
	connect with any of the 'dest' model components it is just added to the
	'dest' model.

	IMPORTANT: The 'src' model must have only one connected component!

	Sets the list 'list' as a list of vertex auto iteration numbers. */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_join( NsModel *dest, const NsModel *src, NsList *list );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_JOIN_H__ */
