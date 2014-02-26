#ifndef __NS_MODEL_MODEL_VERTEX_H__
#define __NS_MODEL_MODEL_VERTEX_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>
#include <model/nsmodel-function.h>

NS_DECLS_BEGIN

/* Sets the list 'list' as a list of vertex auto iteration numbers. */
NS_IMPEXP NsError ns_model_change_selected_vertices_radii
	(
	NsModel  *model,
	nsfloat   percent,
	nsfloat   min_radius,
	NsList   *list
	);

NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_selected_vertex_to_origin( NsModel *model, NsList *list );

NS_IMPEXP NsError ns_model_select_conn_comps_of_selected_vertices( NsModel *model );

/* This function also sets the sub-tree id's for the vertices that become selected. */
NS_IMPEXP NsError ns_model_select_sub_trees_of_selected_vertices( NsModel *model );


NS_IMPEXP NsError ns_model_select_paths_of_selected_vertices( NsModel *model );


NS_IMPEXP NsError ns_model_get_order_of_selected_vertices( NsModel *model, nsulong *order );
NS_IMPEXP NsError ns_model_set_order_of_selected_vertices( NsModel *model, nsulong order );


NS_IMPEXP void ns_model_transpose_child_edges_of_selected_vertices( NsModel *model );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_VERTEX_H__ */
