#ifndef __NS_MODEL_MODEL_REMOVE_H__
#define __NS_MODEL_MODEL_REMOVE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>
#include <model/nsmodel-origs.h>

NS_DECLS_BEGIN

/* Deletes selected vertices. Origins are found as necessary in any
	components that dont have one. Also, any edited components are
	marked as manually traced.

	Sets the list 'list' as a list of vertex auto iteration numbers. */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_remove_selected_vertices( NsModel *model, NsList *ids );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_REMOVE_H__ */
