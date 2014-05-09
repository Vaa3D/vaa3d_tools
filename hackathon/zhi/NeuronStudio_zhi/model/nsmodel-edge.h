#ifndef __NS_MODEL_MODEL_EDGE_H__
#define __NS_MODEL_MODEL_EDGE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>
#include <model/nsmodel-origs.h>

NS_DECLS_BEGIN

/* 3 criteria must be met for an edge to be added.

	1) The number of selected vertices must be 2.
	2) The 2 vertices must be in different components.
	3) No edge currently exists between the 2 vertices.

	Sets the list 'list' as a list of vertex auto iteration numbers. */
NS_IMPEXP NsError ns_model_selected_vertices_add_edge( NsModel *model, NsList *list );

/* 3 criteria must be met for an edge to be removed.

	1) The number of selected vertices must be 2.
	2) The 2 vertices must be in the same component.
	3) The edge currently exists between the 2 vertices.

	Sets the list 'list' as a list of vertex auto iteration numbers. */
NS_IMPEXP NsError ns_model_selected_vertices_remove_edge( NsModel *model, NsList *list );

/* Tries to 'add' and edge. If that cant be done than 'remove' is called. */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_selected_vertices_add_or_remove_edge( NsModel *model, NsList *list );


NS_IMPEXP NS_SYNCHRONIZED void ns_model_adjust_origins_on_adding_edge( NsModel *model, nsmodelvertex V0, nsmodelvertex V1 );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_EDGE_H__ */
