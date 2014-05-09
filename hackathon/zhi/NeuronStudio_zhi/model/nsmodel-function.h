#ifndef __NS_MODEL_MODEL_FUNCTION_H__
#define __NS_MODEL_MODEL_FUNCTION_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

/* A number of restrictions are imposed on labeling objects with a function type...

1) An origin vertex is always set to "soma" by default, and cannot be set to anything else. (DONE)

2) Objects to be added to the "soma" must form a path (i.e. weakly connected) to any existing objects labeled as "soma".
	i.e. the "soma" objects themselves must always from a weakly connected sub-graph.

3) The "soma" objects cannot be split (disconnected) by re-labeling one or more of the existing objects. Again the "soma"
	must exist as a weakly connected sub-graph.

4) Resetting an origin vertex clears function labeling, unless it is reset to a vertex that is also part of the "soma". (DONE)

5) Select subtree stops at soma type. (DONE)
*/


enum
	{
	NS_MODEL_FUNCTION_SELECTED_OK,
	NS_MODEL_FUNCTION_SELECTED_SOMA_NOT_CONNECTED,
	NS_MODEL_FUNCTION_SELECTED_SOMA_DISCONNECTED
	};

/* 'result' is set to one of the above return codes. */
NS_IMPEXP NsError ns_model_function_selected_objects( NsModel *model, NsModelFunctionType func_type, nsint *result );


/* Returns true if there exists a series of vertices and edges with the passed
	'func_type' that ends up at 'V2'.

	'V1' must have its function type set to 'func_type', otherwise an assertion
	will be thrown. 'V2' does not have to have its function type set to 'func_type'
	for the function to return true.

	i.e. the function looks for a set of vertices of the form [V1, ..., V2) where
	'[' means included and ')' means not included.
*/
NS_IMPEXP nsboolean ns_model_vertex_function_path_exists
	(
	const nsmodelvertex  V1,
	const nsmodelvertex  V2,
	NsModelFunctionType  func_type
	);


/* Given that the vertices have had their function types set, set the edges of the model to
	appropriate function types. This function is useful e.g. when reading a SWC file where the
	vertex types are stored, but not the edges. */
NS_IMPEXP void ns_model_function_edges_from_vertices( NsModel *model );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_FUNCTION_H__ */
