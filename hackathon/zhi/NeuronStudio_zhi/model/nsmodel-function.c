#include "nsmodel-function.h"


NS_PRIVATE void _ns_model_function_selected_do_vertex( nsmodelvertex curr_vertex, NsModelFunctionType func_type )
	{
	/* NOTE: Origin vertices are always set to "soma", and cannot be unset. */
	if( ! ns_model_vertex_is_origin( curr_vertex ) )
		ns_model_vertex_set_function_type( curr_vertex, func_type );
	}


NS_PRIVATE void _ns_model_function_selected_do_edges( nsmodelvertex curr_vertex, NsModelFunctionType func_type )
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge, end_edges;


	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		if( NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( curr_vertex ) )
			{
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			/* NOTE: Only those edges that connect 2 "soma" vertices are labeled as "soma". */
			if( NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( dest_vertex ) )
				ns_model_edge_set_function_type( curr_edge, NS_MODEL_FUNCTION_SOMA, NS_TRUE );
			}
		else
			ns_model_edge_set_function_type( curr_edge, func_type, NS_TRUE );
	}


void ns_model_function_edges_from_vertices( NsModel *model )
	{
	nsmodelvertex curr, end;

	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		_ns_model_function_selected_do_edges( curr, ns_model_vertex_get_function_type( curr ) );
	}


#define __GET_IN_PATH( V )\
	NS_POINTER_TO_INT( ns_model_vertex_get_data( (V), NS_MODEL_VERTEX_DATA_SLOT0 ) )

#define __SET_IN_PATH( V, v )\
	ns_model_vertex_set_data( (V), NS_MODEL_VERTEX_DATA_SLOT0, NS_INT_TO_POINTER( (v) ) )


#define __GET_FUNC_TYPE( V )\
	NS_POINTER_TO_INT( ns_model_vertex_get_data( (V), NS_MODEL_VERTEX_DATA_SLOT1 ) )

#define __SET_FUNC_TYPE( V, v )\
	ns_model_vertex_set_data( (V), NS_MODEL_VERTEX_DATA_SLOT1,  NS_INT_TO_POINTER( (v) ) )


NS_PRIVATE void _ns_model_function_selected_do_restore( NsList *list )
	{
	nslistiter     curr;
	nsmodelvertex  V;


	NS_LIST_FOREACH( list, curr )
		{
		V = ns_list_iter_get_object( curr ); 
		_ns_model_function_selected_do_vertex( V, __GET_FUNC_TYPE( V ) );
		}

	NS_LIST_FOREACH( list, curr )
		{
		V = ns_list_iter_get_object( curr ); 
		_ns_model_function_selected_do_edges( V, __GET_FUNC_TYPE( V ) );
		}
	}


NS_PRIVATE nsint _ns_model_function_selected_check_for_soma_not_connected
	(
	NsModel              *model,
	NsList               *list,
	NsModelFunctionType   func_type
	)
	{
	nslistiter     curr;
	nssize         conn_comp;
	nsmodelorigin  O;
	nsmodelvertex  V;
	nsmodelvertex  U;


	if( NS_MODEL_FUNCTION_SOMA != func_type )
		return NS_MODEL_FUNCTION_SELECTED_OK;

	conn_comp = 0;

	/* The first test is to see if all the vertices in the list are in the same component. If they arent
		then obviously this isnt a valid soma. */

	if( ! ns_list_is_empty( list ) )
		conn_comp = ns_model_vertex_get_conn_comp( ns_list_iter_get_object( ns_list_begin( list ) ) );

	NS_LIST_FOREACH( list, curr )
		if( conn_comp != ns_model_vertex_get_conn_comp( ns_list_iter_get_object( curr ) ) )
			return NS_MODEL_FUNCTION_SELECTED_SOMA_NOT_CONNECTED;

	O = ns_model_find_origin_by_conn_comp( model, conn_comp );
	ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, O ) );

	U = ns_model_origin_vertex( O );
	ns_assert( NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( U ) );

	/* Test for a path from the origin vertex to every other vertex that was set as soma. Note that
		we can do an optimization here and mark vertices on paths as we go. In other words if a path from
		the origin O to a vertex V1 exists and goes through some vertex V2, then we know V2 has a path and
		we dont need to check for it again. */
	NS_LIST_FOREACH( list, curr )
		{
		V = ns_list_iter_get_object( curr );

		if( ! __GET_IN_PATH( V ) )
			if( ! ns_model_vertex_function_path_exists( U, V, NS_MODEL_FUNCTION_SOMA ) )
				{
				_ns_model_function_selected_do_restore( list );
				return NS_MODEL_FUNCTION_SELECTED_SOMA_NOT_CONNECTED;
				}
		}

	return NS_MODEL_FUNCTION_SELECTED_OK;
	}


NS_PRIVATE nsint _ns_model_function_selected_check_for_soma_disconnected
	(
	NsModel              *model,
	NsList               *list,
	NsModelFunctionType   func_type
	)
	{
	nslistiter     curr;
	nssize         conn_comp;
	nsmodelorigin  O;
	nsmodelvertex  V;
	nsmodelvertex  U;
	nsmodelvertex  S;


	if( NS_MODEL_FUNCTION_SOMA == func_type )
		return NS_MODEL_FUNCTION_SELECTED_OK;

	/* Test for a path from the origin vertex to every other vertex for those
		connected components where a soma vertex was unset. i.e. that the somas
		for those components are still weakly connected. */

	NS_LIST_FOREACH( list, curr )
		{
		V = ns_list_iter_get_object( curr );

		/* Was the old function type of the selected vertex soma? */
		if( NS_MODEL_FUNCTION_SOMA == __GET_FUNC_TYPE( V ) )
			{
			conn_comp = ns_model_vertex_get_conn_comp( V );

			O = ns_model_find_origin_by_conn_comp( model, conn_comp );
			ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, O ) );

			U = ns_model_origin_vertex( O );
			ns_assert( NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( U ) );

			NS_MODEL_VERTEX_FOREACH( model, S )
				if( conn_comp == ns_model_vertex_get_conn_comp( S ) &&
					 NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( S ) )
					if( ! __GET_IN_PATH( S ) )
						if( ! ns_model_vertex_function_path_exists( U, S, NS_MODEL_FUNCTION_SOMA ) )
							{
							_ns_model_function_selected_do_restore( list );
							return NS_MODEL_FUNCTION_SELECTED_SOMA_DISCONNECTED;
							}
			}
		}

	return NS_MODEL_FUNCTION_SELECTED_OK;
	}


NS_PRIVATE void _ns_model_function_selected_enable_restore( NsModel *model, NsList *list )
	{
	nslistiter     curr;
	nsmodelvertex  V;


	NS_MODEL_VERTEX_FOREACH( model, V )
		__SET_IN_PATH( V, NS_FALSE );

	NS_LIST_FOREACH( list, curr )
		{
		V = ns_list_iter_get_object( curr ); 
		__SET_FUNC_TYPE( V, ns_model_vertex_get_function_type( V ) );
		}
	}


NS_PRIVATE void _ns_model_function_selected_do_list( NsList *list, NsModelFunctionType func_type )
	{
	nslistiter curr;

	NS_LIST_FOREACH( list, curr )
		_ns_model_function_selected_do_vertex( ns_list_iter_get_object( curr ), func_type );

	NS_LIST_FOREACH( list, curr )
		_ns_model_function_selected_do_edges( ns_list_iter_get_object( curr ), func_type );
	}


NsError ns_model_function_selected_objects( NsModel *model, NsModelFunctionType func_type, nsint *result )
	{
	NsList   list;
	NsError  error;


	ns_assert( NULL != model );
	ns_assert( NULL != result );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	_ns_model_function_selected_enable_restore( model, &list );
	_ns_model_function_selected_do_list( &list, func_type );

	*result = NS_MODEL_FUNCTION_SELECTED_OK;

	if( NS_MODEL_FUNCTION_SELECTED_OK == *result )
		*result = _ns_model_function_selected_check_for_soma_not_connected( model, &list, func_type );

	if( NS_MODEL_FUNCTION_SELECTED_OK == *result )
		*result = _ns_model_function_selected_check_for_soma_disconnected( model, &list, func_type );

	ns_list_destruct( &list );
	return ns_no_error();
	}


/* NOTE: The path boolean is set as the recursion unwinds. */
NS_PRIVATE nsboolean _ns_model_vertex_do_function_path_exists
	(
	const nsmodelvertex  parent_vertex,
	const nsmodelvertex  curr_vertex,
	const nsmodelvertex  target_vertex,
	NsModelFunctionType  func_type
	)
	{
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge, end_edges;


	if( func_type != ns_model_vertex_get_function_type( curr_vertex ) )
		return NS_FALSE;

	if( ns_model_vertex_equal( curr_vertex, target_vertex ) )
		{
		__SET_IN_PATH( curr_vertex, NS_TRUE );
		return NS_TRUE;
		}

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      if( func_type == ns_model_edge_get_function_type( curr_edge ) )
			{
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
				if( _ns_model_vertex_do_function_path_exists( curr_vertex, dest_vertex, target_vertex, func_type ) )
					{
					__SET_IN_PATH( curr_vertex, NS_TRUE );
					return NS_TRUE;
					}
			}

	return NS_FALSE;
	}
 

/* NOTE: The path boolean is set as the recursion unwinds. */
nsboolean ns_model_vertex_function_path_exists
	(
	const nsmodelvertex  curr_vertex,
	const nsmodelvertex  target_vertex,
	NsModelFunctionType  func_type
	)
	{
   nsmodeledge curr_edge, end_edges;

	if( func_type != ns_model_vertex_get_function_type( curr_vertex ) )
		return NS_FALSE;

	if( ns_model_vertex_equal( curr_vertex, target_vertex ) )
		{
		__SET_IN_PATH( curr_vertex, NS_TRUE );
		return NS_TRUE;
		}

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		if( func_type == ns_model_edge_get_function_type( curr_edge ) )
         if( _ns_model_vertex_do_function_path_exists(
					curr_vertex,
					ns_model_edge_dest_vertex( curr_edge ),
					target_vertex,
					func_type
					) )
				{
				__SET_IN_PATH( curr_vertex, NS_TRUE );
				return NS_TRUE;
				}

	return NS_FALSE;
	}
