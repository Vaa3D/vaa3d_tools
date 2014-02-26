#include "nsmodel-vertex.h"


NsError ns_model_change_selected_vertices_radii
	(
	NsModel  *model,
	nsfloat   percent,
	nsfloat   min_radius,
	NsList   *list
	)
	{
	NsList         vertices;
	nslistiter     curr;
	nsmodelvertex  V;
	nsfloat        radius;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != list );

	ns_list_construct( &vertices, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &vertices ), error ) )
		{
		ns_list_destruct( &vertices );
		return error;
		}

	NS_LIST_FOREACH( &vertices, curr )
		{
		V = ns_list_iter_get_object( curr );

		if( ! ns_model_vertex_is_manually_traced( V ) )
			ns_model_vertex_mark_manually_traced_recursive( V, NS_TRUE );

		/* Increase or decrease radius by a percentage. */
		radius = ns_model_vertex_get_radius( V ) * percent / 100.0f;

		if( radius < min_radius )
			radius = min_radius;

		ns_model_vertex_set_radius( V, radius );

		if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V ), error ) )
			{
			ns_list_destruct( &vertices );
			return error;
			}
		}

	ns_list_destruct( &vertices );
	return ns_no_error();
	}


NsError ns_model_selected_vertex_to_origin( NsModel *model, NsList *list )
	{
	nsmodelvertex  V, U;
	nsmodelorigin  O;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != list );

	error = ns_no_error();
	ns_model_lock( model );

	ns_model_needs_edge_centers( model );

	if( ! ns_model_buffer_selected_vertices( model, &V, 1 ) )
		goto _NS_MODEL_SELECTED_VERTEX_TO_ORIGIN_EXIT;

	/* If its an origin alreday then do nothing. */
	if( ns_model_vertex_is_origin( V ) )
		goto _NS_MODEL_SELECTED_VERTEX_TO_ORIGIN_EXIT;

	if( ! ns_model_vertex_is_manually_traced( V ) )
		ns_model_vertex_mark_manually_traced_recursive( V, NS_TRUE );

	O = ns_model_find_origin_by_conn_comp( model, ns_model_vertex_get_conn_comp( V ) );
	ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, O ) );

	/* See also nsmodel-function.h for explanation of why the function labels
		are possibly cleared. */
	if( NS_MODEL_FUNCTION_SOMA != ns_model_vertex_get_function_type( V ) )
		{
		ns_model_set_function_type_recursive( ns_model_origin_vertex( O ), NS_MODEL_FUNCTION_UNKNOWN );

		/* Set it back to "soma" just to be consistent. */
		ns_model_vertex_set_function_type( ns_model_origin_vertex( O ), NS_MODEL_FUNCTION_SOMA );
		}

	/* NOTE: The following function resets the 'O' iterator, so we need to store a reference to
		the vertex it points to prior to the call. */
	U = ns_model_origin_vertex( O );
	ns_model_reset_origin( model, O, V );

	ns_assert( NS_MODEL_FUNCTION_SOMA    == ns_model_vertex_get_function_type( V ) );
	ns_assert( NS_MODEL_FUNCTION_UNKNOWN == ns_model_vertex_get_function_type( U ) );

	/* This is a little tricky... the old origin 'U' now has its function type set to "unknown". So we need
		to see if the old origin is part of the "weakly connected" sub-graph that is labeled as "soma", that
		we know now consists of the vertex 'V'. If it is then we reset its function type back to "soma". See also
		nsmodel-function.h. */
	if( ns_model_vertex_function_path_exists( V, U, NS_MODEL_FUNCTION_SOMA ) )
		ns_model_vertex_set_function_type( U, NS_MODEL_FUNCTION_SOMA );

	if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V ), error ) )
		goto _NS_MODEL_SELECTED_VERTEX_TO_ORIGIN_EXIT;

	_NS_MODEL_SELECTED_VERTEX_TO_ORIGIN_EXIT:

	ns_model_unlock( model );
	return error;
	}


NS_PRIVATE void _ns_model_do_select_conn_comp_of_selected_vertex( nsmodelvertex V )
	{
	nsmodeledge curr, end;

	if( ! ns_model_vertex_is_selected( V ) )
		{
		ns_model_vertex_mark_selected( V, NS_TRUE );

		/* NOTE: In this case it doesnt if we see the parent vertex while
			traversing the edges since its guaranteed to be selected already
			and we wont go down that path. */

		curr = ns_model_vertex_begin_edges( V );
		end  = ns_model_vertex_end_edges( V );

		for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
			_ns_model_do_select_conn_comp_of_selected_vertex( ns_model_edge_dest_vertex( curr ) );
		}
	}


NS_PRIVATE void _ns_model_select_conn_comp_of_selected_vertex( nsmodelvertex V )
	{
	nsmodeledge curr, end;

	ns_assert( ns_model_vertex_is_selected( V ) );

	curr = ns_model_vertex_begin_edges( V );
	end  = ns_model_vertex_end_edges( V );

	for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
		_ns_model_do_select_conn_comp_of_selected_vertex( ns_model_edge_dest_vertex( curr ) );
	}


NsError ns_model_select_conn_comps_of_selected_vertices( NsModel *model )
	{
	NsList      list;
	nslistiter  curr;
	NsError     error;


	ns_assert( NULL != model );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	NS_LIST_FOREACH( &list, curr )
		_ns_model_select_conn_comp_of_selected_vertex( ns_list_iter_get_object( curr ) );

	ns_list_destruct( &list );

	/* NOTE: Hidden vertices may have become selected in this procedure. */
	ns_model_deselect_all_hidden_vertices( model );

	return ns_no_error();
	}


NS_PRIVATE void _ns_model_do_select_sub_tree_of_selected_vertex( nsmodelvertex V )
	{
	nsmodeledge curr, end;

	if( ! ns_model_vertex_is_selected( V ) && NS_MODEL_FUNCTION_SOMA != ns_model_vertex_get_function_type( V ) )
		{
		ns_model_vertex_mark_selected( V, NS_TRUE );

		/* NOTE: In this case it doesnt matter if we see the parent vertex while
			traversing the edges since its guaranteed to be selected already
			and we wont go down that path. */

		curr = ns_model_vertex_begin_edges( V );
		end  = ns_model_vertex_end_edges( V );

		for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
			_ns_model_do_select_sub_tree_of_selected_vertex( ns_model_edge_dest_vertex( curr ) );
		}
	}


NS_PRIVATE void _ns_model_select_sub_tree_of_selected_vertex( nsmodelvertex V )
	{
	nsmodeledge curr, end;

	ns_assert( ns_model_vertex_is_selected( V ) );

	if( NS_MODEL_FUNCTION_SOMA != ns_model_vertex_get_function_type( V ) )
		{
		curr = ns_model_vertex_begin_edges( V );
		end  = ns_model_vertex_end_edges( V );

		for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
			_ns_model_do_select_sub_tree_of_selected_vertex( ns_model_edge_dest_vertex( curr ) );
		}
	}


NsError ns_model_select_sub_trees_of_selected_vertices( NsModel *model )
	{
	NsList      list;
	nslistiter  curr;
	NsError     error;


	ns_assert( NULL != model );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	NS_LIST_FOREACH( &list, curr )
		_ns_model_select_sub_tree_of_selected_vertex( ns_list_iter_get_object( curr ) );

	ns_list_destruct( &list );

	/* NOTE: Hidden vertices may have become selected in this procedure. */
	ns_model_deselect_all_hidden_vertices( model );

	return ns_no_error();
	}


NS_PRIVATE nsboolean _ns_model_do_select_paths_of_selected_vertex( nsmodelvertex parent_vertex, nsmodelvertex curr_vertex )
	{
	nsmodeledge    curr_edge, end_edges;
	nsmodelvertex  dest_vertex;
	nsboolean      ret_value;


	ret_value = NS_FALSE;

	/* If hit a selected vertex, then we're done. */
	if( ns_model_vertex_is_selected( curr_vertex ) )
		ret_value = NS_TRUE;
	else
		{
		curr_edge = ns_model_vertex_begin_edges( curr_vertex );
		end_edges = ns_model_vertex_end_edges( curr_vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			{
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			/* See if any children hit a selected vertex, if so then select the parent vertex.
				NOTE: Have to make sure we dont go back to the parent. */
			if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
				ret_value = ret_value || _ns_model_do_select_paths_of_selected_vertex( curr_vertex, dest_vertex );
			}
		}

	if( ret_value )
		ns_model_vertex_mark_selected( parent_vertex, NS_TRUE );

	return ret_value;
	}


NS_PRIVATE void _ns_model_select_paths_of_selected_vertex( nsmodelvertex V )
	{
	nsmodeledge curr, end;

	ns_assert( ns_model_vertex_is_selected( V ) );

	curr = ns_model_vertex_begin_edges( V );
	end  = ns_model_vertex_end_edges( V );

	for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
		_ns_model_do_select_paths_of_selected_vertex( V, ns_model_edge_dest_vertex( curr ) );
	}


NsError ns_model_select_paths_of_selected_vertices( NsModel *model )
	{
	NsList      list;
	nslistiter  curr;
	NsError     error;


	ns_assert( NULL != model );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	NS_LIST_FOREACH( &list, curr )
		_ns_model_select_paths_of_selected_vertex( ns_list_iter_get_object( curr ) );

	ns_list_destruct( &list );

	/* NOTE: Hidden vertices may have become selected in this procedure. */
	ns_model_deselect_all_hidden_vertices( model );

	return ns_no_error();
	}


NsError ns_model_get_order_of_selected_vertices( NsModel *model, nsulong *max_order )
	{
	NsList         list;
	nslistiter     iter;
	nsmodelvertex  vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nsulong        curr_order;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != max_order );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	*max_order = 0;

	NS_LIST_FOREACH( &list, iter )
		{
		vertex = ns_list_iter_get_object( iter );

		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			if( ns_model_vertex_is_selected( ns_model_edge_dest_vertex( curr_edge ) ) )
				{
				curr_order = ns_model_edge_get_order( curr_edge );

				if( curr_order > *max_order )
					*max_order = curr_order;
				}
		}

	ns_list_destruct( &list );
	return ns_no_error();
	}


NsError ns_model_set_order_of_selected_vertices( NsModel *model, nsulong order )
	{
	NsList         list;
	nslistiter     iter;
	nsmodelvertex  vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	NsError        error;


	ns_assert( NULL != model );

	ns_list_construct( &list, NULL );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &list ), error ) )
		{
		ns_list_destruct( &list );
		return error;
		}

	/* Since we are only setting the order of the vertices we dont
		need the section centers to be regenerated. */
	ns_model_needs_edge_order_centers( model );

	NS_LIST_FOREACH( &list, iter )
		{
		vertex = ns_list_iter_get_object( iter );

		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			if( ns_model_vertex_is_selected( ns_model_edge_dest_vertex( curr_edge ) ) )
				ns_model_edge_set_order( curr_edge, ( nsushort )order, NS_TRUE );
		}

	ns_list_destruct( &list );
	return ns_no_error();
	}


NS_PRIVATE void _ns_model_do_transpose_child_edges_of_selected_vertices
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex
	)
	{
	nsmodeledge    curr_edge, end_edges;
	nsmodelvertex  dest_vertex;


	if( ns_model_vertex_is_selected( curr_vertex ) &&
		 ns_model_vertex_not_equal( parent_vertex, NS_MODEL_VERTEX_NIL ) )
		ns_model_vertex_transpose_edges( curr_vertex );

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_do_transpose_child_edges_of_selected_vertices(
				curr_vertex,
				dest_vertex
				);
		}
	}


void ns_model_transpose_child_edges_of_selected_vertices( NsModel *model )
	{
	nsmodelorigin curr, end;

	ns_assert( NULL != model );

	curr = ns_model_begin_origins( model );
	end  = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
		_ns_model_do_transpose_child_edges_of_selected_vertices(
			NS_MODEL_VERTEX_NIL,
			ns_model_origin_vertex( curr )
			);
	}
