#include "nsmodel-edge.h"


void ns_model_adjust_origins_on_adding_edge( NsModel *model, nsmodelvertex V0, nsmodelvertex V1 )
	{
	nsmodelorigin O[2];

	/* Remove the larger of the two component origins. */
	O[0] = ns_model_find_origin_by_conn_comp( model, ns_model_vertex_get_conn_comp( V0 ) );
	O[1] = ns_model_find_origin_by_conn_comp( model, ns_model_vertex_get_conn_comp( V1 ) );

	ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, O[0] ) );
	ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, O[1] ) );

	if( ns_model_vertex_get_radius( ns_model_origin_vertex( O[0] ) )
			>
		 ns_model_vertex_get_radius( ns_model_origin_vertex( O[1] ) ) )
		ns_model_remove_origin( model, O[1] );
	else
		ns_model_remove_origin( model, O[0] );

	ns_model_set_conn_comp_numbers( model );
	}


NS_PRIVATE NsError _ns_model_do_selected_vertices_add_edge
	(
	NsModel        *model,
	NsList         *list,
	nsmodelvertex  *V,
	nsboolean      *result
	)
	{
	NsError error;

	/* Test 2 */
	if( ns_model_vertex_get_conn_comp( V[0] ) == ns_model_vertex_get_conn_comp( V[1] ) )
		return ns_no_error();

	/* Test 3 */
	if( ns_model_vertex_edge_exists( V[0], V[1] ) )
		return ns_no_error();

	if( ! ns_model_vertex_is_manually_traced( V[0] ) )
		ns_model_vertex_mark_manually_traced_recursive( V[0], NS_TRUE );

	if( ! ns_model_vertex_is_manually_traced( V[1] ) )
		ns_model_vertex_mark_manually_traced_recursive( V[1], NS_TRUE );

	if( NS_FAILURE( ns_model_vertex_add_edge( V[0], V[1], NULL, NULL ), error ) )
		return error;

	//ns_model_vertex_mark_selected( V[0], NS_FALSE );
	//ns_model_vertex_mark_selected( V[1], NS_FALSE );

	ns_model_adjust_origins_on_adding_edge( model, V[0], V[1] );

	if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V[0] ), error ) )
		return error;

	if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V[1] ), error ) )
		return error;

	if( NULL != result )
		*result = NS_TRUE;

	return ns_no_error();
	}


NsError ns_model_selected_vertices_add_edge( NsModel *model, NsList *list )
	{
	nsmodelvertex V[2];

	ns_assert( NULL != model );
	ns_assert( NULL != list );

	/* Test 1 */
	if( ! ns_model_buffer_selected_vertices( model, V, 2 ) )
		return ns_no_error();

	return _ns_model_do_selected_vertices_add_edge( model, list, V, NULL );
	}


NS_PRIVATE NsError _ns_model_do_selected_vertices_remove_edge
	(
	NsModel        *model,
	NsList         *list,
	nsmodelvertex  *V,
	nsboolean      *result
	)
	{
	NsError error;

	/* Test 2 */
	if( ns_model_vertex_get_conn_comp( V[0] ) != ns_model_vertex_get_conn_comp( V[1] ) )
		return ns_no_error();

	/* Test 3 */
	if( ! ns_model_vertex_edge_exists( V[0], V[1] ) )
		return ns_no_error();

	if( ! ns_model_vertex_is_manually_traced( V[0] ) )
		ns_model_vertex_mark_manually_traced_recursive( V[0], NS_TRUE );

	if( ! ns_model_vertex_is_manually_traced( V[1] ) )
		ns_model_vertex_mark_manually_traced_recursive( V[1], NS_TRUE );

	ns_model_vertex_remove_edge( V[0], ns_model_vertex_find_edge( V[0], V[1] ) );

	//ns_model_vertex_mark_selected( V[0], NS_FALSE );
	//ns_model_vertex_mark_selected( V[1], NS_FALSE );

	ns_model_set_conn_comp_numbers( model );

	if( NS_FAILURE( ns_model_origin_create_if_none( model, V[0] ), error ) )
		return error;

	if( NS_FAILURE( ns_model_origin_create_if_none( model, V[1] ), error ) )
		return error;

	if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V[0] ), error ) )
		return error;

	if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V[1] ), error ) )
		return error;

	if( NULL != result )
		*result = NS_TRUE;

	return ns_no_error();
	}


NsError ns_model_selected_vertices_remove_edge( NsModel *model, NsList *list )
	{
	nsmodelvertex V[2];

	ns_assert( NULL != model );
	ns_assert( NULL != list );

	/* Test 1 */
	if( ! ns_model_buffer_selected_vertices( model, V, 2 ) )
		return ns_no_error();

	return _ns_model_do_selected_vertices_remove_edge( model, list, V, NULL );
	}


NsError ns_model_selected_vertices_add_or_remove_edge( NsModel *model, NsList *list )
	{
	nsmodelvertex  V[2];
	nsboolean      result;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != list );

	error  = ns_no_error();
	result = NS_FALSE;

	ns_model_lock( model );

	ns_model_needs_edge_centers( model );

	/* Test 1 */
	if( ! ns_model_buffer_selected_vertices( model, V, 2 ) )
		goto _NS_MODEL_SELECTED_VERTICES_ADD_OR_REMOVE_EDGE_EXIT;

	if( NS_FAILURE( _ns_model_do_selected_vertices_add_edge( model, list, V, &result ), error ) )
		goto _NS_MODEL_SELECTED_VERTICES_ADD_OR_REMOVE_EDGE_EXIT;

	if( result )
		goto _NS_MODEL_SELECTED_VERTICES_ADD_OR_REMOVE_EDGE_EXIT;

	if( NS_FAILURE( _ns_model_do_selected_vertices_remove_edge( model, list, V, &result ), error ) )
		goto _NS_MODEL_SELECTED_VERTICES_ADD_OR_REMOVE_EDGE_EXIT;

	_NS_MODEL_SELECTED_VERTICES_ADD_OR_REMOVE_EDGE_EXIT:

	ns_model_unlock( model );
	return error;
	}
