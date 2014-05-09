#include "nsgraph.h"


NsGraphVertex* _ns_graph_vertex( const nsgraphvertexiter V )
	{
	NsGraphVertex *vertex;

	ns_assert( NULL != V );
	vertex = ( NsGraphVertex* )ns_list_iter_get_object( V );

	ns_assert( NULL != vertex );
	return vertex;
	}


NsGraphEdge* _ns_graph_edge( const nsgraphedgeiter E )
	{
	NsGraphEdge *edge;

	ns_assert( NULL != E );
	edge = ( NsGraphEdge* )ns_list_iter_get_object( E );

	ns_assert( NULL != edge );
	return edge;
	}


NS_PRIVATE void _ns_graph_vertex_finalize( nspointer object )
	{
	NsGraphVertex *vertex = ( NsGraphVertex* )object;

	ns_assert( NULL != vertex );
	ns_assert( NULL != vertex->graph );

	/* NOTE: If the graph is undirected and there are vertices with edges pointing
		to themselves, then the _ns_graph_edge_finalize() callback function defined below
		is unsafe when ns_list_destruct() is called. See implementation of ns_list_erase_all().
		This function sets a next pointer before erasing the current node but the next node
		will be removed if the vertex has an edge pointing to itself, so the pointer becomes
		invalid. We can avoid this by simply popping edges until the list is empty. */

	if( ! ns_graph_is_directed( vertex->graph ) )
		while( ! ns_list_is_empty( &vertex->edges ) )
			ns_list_pop_front( &vertex->edges );

	ns_list_destruct( &vertex->edges );

	if( NULL != vertex->graph->vertex_finalize_func )
		( vertex->graph->vertex_finalize_func )( vertex->object );

	ns_delete( vertex );
	}


NS_PRIVATE void _ns_graph_edge_finalize( nspointer object )
	{
	NsGraphEdge  *edge;
	NsGraph      *graph;


	edge = ( NsGraphEdge* )object;
	ns_assert( NULL != edge );

	if( NULL != edge->mirror_edge )
		{
		/* NOTE: Set it to NULL to avoid infinite recursion. */
		_ns_graph_edge( edge->mirror_edge )->mirror_edge = NULL;
		ns_graph_vertex_remove_edge( edge->dest_vertex, edge->mirror_edge );
		}

	graph = _ns_graph_vertex( edge->src_vertex )->graph;
	ns_assert( NULL != graph );

	if( NULL != graph->edge_finalize_func )
		( graph->edge_finalize_func )( edge->object );

	ns_delete( edge );
	}


NS_IMPEXP void ns_graph_construct
	( 
	NsGraph         *graph,
	nsboolean        is_directed,
	NsFinalizeFunc   vertex_finalize_func,
	NsFinalizeFunc   edge_finalize_func
   )
	{
	ns_assert( NULL != graph );

	graph->is_directed          = is_directed;
	graph->vertex_finalize_func = vertex_finalize_func;
	graph->edge_finalize_func   = edge_finalize_func;

	ns_list_construct( &graph->vertices, _ns_graph_vertex_finalize );
	}


void ns_graph_destruct( NsGraph *graph )
	{
	ns_assert( NULL != graph );

	ns_graph_clear( graph );
	ns_list_destruct( &graph->vertices );
	}


nsboolean ns_graph_is_directed( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return graph->is_directed;
	}


nssize ns_graph_num_vertices( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return ns_list_size( &graph->vertices );
	}


NsList* ns_graph_vertex_list( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return ( NsList* )&graph->vertices;
	}


nssize ns_graph_num_edges( const NsGraph *graph )
	{
	nsgraphvertexiter  curr;
	nsgraphvertexiter  end;
	nssize             count;


	ns_assert( NULL != graph );

	count = 0;
	curr  = ns_graph_begin_vertices( graph );
	end   = ns_graph_end_vertices( graph );

	for( ; ns_graph_vertex_iter_not_equal( curr, end ); curr = ns_graph_vertex_iter_next( curr ) )
		count += ns_graph_vertex_num_edges( curr );

	return ns_graph_is_directed( graph ) ? count : count / 2;
	}


void ns_graph_clear( NsGraph *graph )
	{
	ns_assert( NULL != graph );
	ns_list_clear( &graph->vertices );
	}


NS_PRIVATE nsuint _ns_graph_pointer_hash_func( nsconstpointer object )
	{  return ( nsuint )( nsulong )object;  }


NS_PRIVATE nsboolean _ns_graph_pointer_equal_func( nsconstpointer l, nsconstpointer r )
	{  return l == r;  }


NS_PRIVATE NsError _ns_graph_do_assign
	(
	NsGraph	      *lhs,
	const NsGraph  *rhs,
	NsAssignFunc 	 vertex_assign_func,
	NsAssignFunc    edge_assign_func
	)
	{
	NsHashTable        ht;
	nsgraphvertexiter  rhs_curr_vertex;
	nsgraphvertexiter  lhs_curr_vertex;
	nsgraphvertexiter  rhs_end_vertices;
	nsgraphvertexiter  rhs_dest_vertex;
	nsgraphvertexiter  lhs_dest_vertex;
	nsgraphedgeiter    rhs_curr_edge;
	nsgraphedgeiter    rhs_end_edges;
	nsgraphedgeiter    rhs_mirror_edge;
	nspointer          rhs_vertex_object;
	nspointer          lhs_vertex_object;
	nspointer          rhs_from_to_edge_object;
	nspointer          lhs_from_to_edge_object;
	nspointer          rhs_to_from_edge_object;
	nspointer          lhs_to_from_edge_object;
	NsError            error;


	if( NS_FAILURE( ns_hash_table_construct(
							&ht,
							_ns_graph_pointer_hash_func,
							_ns_graph_pointer_equal_func,
							NULL,
							NULL
							),
							error ) )
		return error;

	/* Add all vertices to the 'lhs' graph and create a hash from
		the vertices of the 'rhs' graph to the 'lhs' graph. */
	rhs_curr_vertex  = ns_graph_begin_vertices( rhs );
	rhs_end_vertices = ns_graph_end_vertices( rhs );

	for( ; ns_graph_vertex_iter_not_equal( rhs_curr_vertex, rhs_end_vertices );
			 rhs_curr_vertex = ns_graph_vertex_iter_next( rhs_curr_vertex ) )
		{
		/* Shallow copy! */
		lhs_vertex_object = rhs_vertex_object = ns_graph_vertex_iter_get_object( rhs_curr_vertex );

		if( NULL != vertex_assign_func )
			if( NS_FAILURE( ( ( vertex_assign_func )( &lhs_vertex_object, rhs_vertex_object ) ), error ) )
				goto _NS_GRAPH_ASSIGN_EXIT;

		if( NS_FAILURE( ns_graph_add_vertex( lhs, lhs_vertex_object, &lhs_curr_vertex ), error ) )
			{
			if( NULL != lhs->vertex_finalize_func )
				( lhs->vertex_finalize_func )( lhs_vertex_object );

			goto _NS_GRAPH_ASSIGN_EXIT;
			}

		if( NS_FAILURE( ns_hash_table_add( &ht, rhs_curr_vertex, lhs_curr_vertex ), error ) )
			goto _NS_GRAPH_ASSIGN_EXIT;
		}

	/* Now create all the edges of the 'lhs' graph. */
	rhs_curr_vertex  = ns_graph_begin_vertices( rhs );
	rhs_end_vertices = ns_graph_end_vertices( rhs );

	for( ; ns_graph_vertex_iter_not_equal( rhs_curr_vertex, rhs_end_vertices );
			 rhs_curr_vertex = ns_graph_vertex_iter_next( rhs_curr_vertex ) )
		{
		ns_verify( ns_hash_table_lookup( &ht, rhs_curr_vertex, &lhs_curr_vertex ) );

		rhs_curr_edge = ns_graph_vertex_begin_edges( rhs_curr_vertex );
		rhs_end_edges = ns_graph_vertex_end_edges( rhs_curr_vertex );

		for( ; ns_graph_edge_iter_not_equal( rhs_curr_edge, rhs_end_edges );
				 rhs_curr_edge = ns_graph_edge_iter_next( rhs_curr_edge ) )
			{
			rhs_dest_vertex = ns_graph_edge_dest_vertex( rhs_curr_edge );

			/* See NOTE below why this could fail, for undirected graphs that is! */
			if( ! ns_hash_table_lookup( &ht, rhs_dest_vertex, &lhs_dest_vertex ) )
				continue;

			/* In undirected graphs, for self-loops the destination vertex is the
				current so it cannot have been removed from the hash yet. Therefore
				store the mirror in the hash as a "seen" edge so that these self-loops
				are not duplicated twice. */
			if( ! rhs->is_directed && ns_graph_vertex_iter_equal( rhs_curr_vertex, rhs_dest_vertex ) )
				{
				if( ns_hash_table_exists( &ht, rhs_curr_edge ) )
					continue;

				rhs_mirror_edge = ns_graph_undir_edge_mirror( rhs_curr_edge );

				if( NS_FAILURE( ns_hash_table_add( &ht, rhs_mirror_edge, rhs_mirror_edge ), error ) )
					goto _NS_GRAPH_ASSIGN_EXIT;
				}

			lhs_from_to_edge_object = rhs_from_to_edge_object = ns_graph_edge_iter_get_object( rhs_curr_edge );

			if( NULL != edge_assign_func )
				if( NS_FAILURE( ( ( edge_assign_func )( &lhs_from_to_edge_object, rhs_from_to_edge_object ) ), error ) )
					goto _NS_GRAPH_ASSIGN_EXIT;

			if( rhs->is_directed )
				{
				if( NS_FAILURE( ns_graph_vertex_add_dir_edge(
										lhs_curr_vertex,
										lhs_dest_vertex,
										lhs_from_to_edge_object,
										NULL
										),
										error ) )
					{
					if( NULL != lhs->edge_finalize_func )
						( lhs->edge_finalize_func )( lhs_from_to_edge_object );

					goto _NS_GRAPH_ASSIGN_EXIT;
					}
				}
			else
				{
				rhs_mirror_edge = ns_graph_undir_edge_mirror( rhs_curr_edge );
				lhs_to_from_edge_object = rhs_to_from_edge_object = ns_graph_edge_iter_get_object( rhs_mirror_edge );

				if( NULL != edge_assign_func )
					if( NS_FAILURE( ( ( edge_assign_func )( &lhs_to_from_edge_object, rhs_to_from_edge_object ) ), error ) )
						{
						if( NULL != lhs->edge_finalize_func )
							( lhs->edge_finalize_func )( lhs_from_to_edge_object );

						goto _NS_GRAPH_ASSIGN_EXIT;
						}

				if( NS_FAILURE( ns_graph_vertex_add_undir_edge(
										lhs_curr_vertex,
										lhs_dest_vertex,
										lhs_from_to_edge_object,
										lhs_to_from_edge_object,
										NULL,
										NULL
										),
										error ) )
					{
					if( NULL != lhs->edge_finalize_func )
						{
						( lhs->edge_finalize_func )( lhs_from_to_edge_object );
						( lhs->edge_finalize_func )( lhs_to_from_edge_object );
						}

					goto _NS_GRAPH_ASSIGN_EXIT;
					}
				}
			}

		/* NOTE: For undirected graphs, we remove a vertex from the hash after
			copying its edges, otherwise all edges will be duplicated twice by
			the ...add_undir_edge() function. */
		if( ! rhs->is_directed )
			ns_verify( ns_hash_table_remove( &ht, rhs_curr_vertex ) );
		}

	_NS_GRAPH_ASSIGN_EXIT:

	ns_hash_table_destruct( &ht );
	return error;
	}


NsError ns_graph_assign
	(
	NsGraph	      *lhs,
	const NsGraph  *rhs,
	NsAssignFunc 	 vertex_assign_func,
	NsAssignFunc    edge_assign_func
	)
	{
	ns_assert( NULL != lhs );
	ns_assert( NULL != rhs );
	ns_assert( lhs != rhs );

	ns_graph_clear( lhs );

	lhs->is_directed          = rhs->is_directed;
	lhs->vertex_finalize_func = rhs->vertex_finalize_func;
	lhs->edge_finalize_func   = rhs->edge_finalize_func;

	return _ns_graph_do_assign( lhs, rhs, vertex_assign_func, edge_assign_func );
	}


NsError ns_graph_add
	(
	NsGraph	      *lhs,
	const NsGraph  *rhs,
	NsAssignFunc 	 vertex_assign_func,
	NsAssignFunc    edge_assign_func
	)
	{
	ns_assert( NULL != lhs );
	ns_assert( NULL != rhs );
	ns_assert( lhs != rhs );
	ns_assert( lhs->is_directed == rhs->is_directed );

	if( lhs->vertex_finalize_func != rhs->vertex_finalize_func )
		ns_warning(
			NS_WARNING_LEVEL_CAUTION
			NS_MODULE
			" The destination graphs vertex finalize function"
			" may not be valid for the source graphs vertices."
			);

	if( lhs->edge_finalize_func != rhs->edge_finalize_func )
		ns_warning(
			NS_WARNING_LEVEL_CAUTION
			NS_MODULE
			" The destination graphs edge finalize function"
			" may not be valid for the source graphs edges."
			);

	/* NOTE: The destination graph 'lhs' is not cleared. */
	return _ns_graph_do_assign( lhs, rhs, vertex_assign_func, edge_assign_func );
	}


NsFinalizeFunc ns_graph_get_vertex_finalize_func( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return graph->vertex_finalize_func;
	}


void ns_graph_set_vertex_finalize_func( NsGraph *graph, NsFinalizeFunc vertex_finalize_func )
	{
	ns_assert( NULL != graph );
	graph->vertex_finalize_func = vertex_finalize_func;
	}


NsFinalizeFunc ns_graph_get_edge_finalize_func( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return graph->edge_finalize_func;
	}


void ns_graph_set_edge_finalize_func( NsGraph *graph, NsFinalizeFunc edge_finalize_func )
	{
	ns_assert( NULL != graph );
	graph->edge_finalize_func = edge_finalize_func;
	}


NsError ns_graph_add_vertex( NsGraph *graph, nspointer object, nsgraphvertexiter *V )
	{
	NsGraphVertex  *vertex;
	NsError         error;


	ns_assert( NULL != graph );

	if( NULL == ( vertex = ns_new( NsGraphVertex ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_list_construct( &vertex->edges, _ns_graph_edge_finalize );

	vertex->graph  = graph;
	vertex->object = object;

	if( NS_FAILURE( ns_list_push_back( &graph->vertices, vertex ), error ) )
		{
		ns_list_destruct( &vertex->edges );
		ns_delete( vertex );

		return error;
		}

	if( NULL != V )
		*V = ns_list_rev_begin( &graph->vertices );

	return ns_no_error();
	}


void ns_graph_remove_vertex( NsGraph *graph, nsgraphvertexiter V, nsboolean clean )
	{
	NsGraphVertex      *vertex;
	nsgraphvertexiter   curr_vertex;
	nsgraphvertexiter   end_vertices;
	nsgraphedgeiter     curr_edge;
	nsgraphedgeiter     end_edges;
	nsgraphedgeiter     del_edge;


	ns_assert( NULL != graph );
	ns_assert( NULL != V );

	vertex = _ns_graph_vertex( V );
	ns_assert( graph == vertex->graph );

	/* Remove all edges pointing to V. */
	if( ns_graph_is_directed( graph ) && clean )
		{
		curr_vertex  = ns_graph_begin_vertices( graph );
		end_vertices = ns_graph_end_vertices( graph );

		for( ; ns_graph_vertex_iter_not_equal( curr_vertex, end_vertices );
				 curr_vertex = ns_graph_vertex_iter_next( curr_vertex ) )
			{
			curr_edge = ns_graph_vertex_begin_edges( curr_vertex );
			end_edges = ns_graph_vertex_end_edges( curr_vertex );

			while( ns_graph_edge_iter_not_equal( curr_edge, end_edges ) )
				if( ns_graph_vertex_iter_equal( V, _ns_graph_edge( curr_edge )->dest_vertex ) )
					{
					del_edge  = curr_edge;
					curr_edge = ns_graph_edge_iter_next( curr_edge );

					ns_graph_vertex_remove_edge( curr_vertex, del_edge );
					}
				else
					curr_edge = ns_graph_edge_iter_next( curr_edge );
			}
		}
	
	ns_list_erase( &graph->vertices, V );
	}


nsgraphvertexiter ns_graph_rev_begin_vertices( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return ns_list_rev_begin( &graph->vertices );
	}


nsgraphvertexiter ns_graph_rev_end_vertices( const NsGraph *graph )
	{
	ns_assert( NULL != graph );
	return ns_list_rev_end( &graph->vertices );
	}


nssize ns_graph_vertex_num_edges( const nsgraphvertexiter V )
	{
	ns_assert( NULL != V );
	return ns_list_size( &( _ns_graph_vertex( V )->edges ) );
	}


nslong ns_graph_vertex_get_color( const nsgraphvertexiter V )
	{
	ns_assert( NULL != V );
	return _ns_graph_vertex( V )->color;
	}


void ns_graph_vertex_set_color( nsgraphvertexiter V, nslong color )
	{
	ns_assert( NULL != V );
	_ns_graph_vertex( V )->color = color;
	}


void ns_graph_color_vertices( NsGraph *graph, nslong color )
	{
	nsgraphvertexiter  curr;
	nsgraphvertexiter  end;

	ns_assert( NULL != graph );

	curr = ns_graph_begin_vertices( graph );
	end  = ns_graph_end_vertices( graph );

	for( ; ns_graph_vertex_iter_not_equal( curr, end ); curr = ns_graph_vertex_iter_next( curr ) )
		ns_graph_vertex_set_color( curr, color );
	}


void ns_graph_color_vertices_recursive( nsgraphvertexiter curr_vertex, nslong color )
	{
	nsgraphedgeiter curr, end;

	ns_assert( NULL != curr_vertex );

	if( ns_graph_vertex_get_color( curr_vertex ) != color )
		{
		ns_graph_vertex_set_color( curr_vertex, color );

		curr = ns_graph_vertex_begin_edges( curr_vertex );
		end  = ns_graph_vertex_end_edges( curr_vertex );

		for( ; ns_graph_edge_iter_not_equal( curr, end ); curr = ns_graph_edge_iter_next( curr ) )
			ns_graph_color_vertices_recursive( ns_graph_edge_dest_vertex( curr ), color );
		}
	}


void ns_graph_color_edges( NsGraph *graph, nslong color )
	{
	nsgraphvertexiter  curr_vertex;
	nsgraphvertexiter  end_vertices;
	nsgraphedgeiter    curr_edge;
	nsgraphedgeiter    end_edges;


	curr_vertex  = ns_graph_begin_vertices( graph );
	end_vertices = ns_graph_end_vertices( graph );

	for( ; ns_graph_vertex_iter_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_graph_vertex_iter_next( curr_vertex ) )
		{
		curr_edge = ns_graph_vertex_begin_edges( curr_vertex );
		end_edges = ns_graph_vertex_end_edges( curr_vertex );

		for( ; ns_graph_edge_iter_not_equal( curr_edge, end_edges );
				 curr_edge = ns_graph_edge_iter_next( curr_edge ) )
			ns_graph_edge_set_color( curr_edge, color );
		}
	}


NsError ns_graph_vertex_add_dir_edge
	(
	nsgraphvertexiter  from,
	nsgraphvertexiter  to,
	nspointer          object,
	nsgraphedgeiter   *E
	)
	{
	NsGraphVertex  *vertex;
	NsGraphEdge    *edge;
	NsError         error;


	ns_assert( NULL != from );
	ns_assert( NULL != to );

	vertex = _ns_graph_vertex( from );

	ns_assert( vertex->graph == _ns_graph_vertex( to )->graph );
	ns_assert( ns_graph_is_directed( vertex->graph ) );

	if( NULL == ( edge = ns_new( NsGraphEdge ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	edge->src_vertex  = from;
	edge->dest_vertex = to;
	edge->mirror_edge = NULL;
	edge->object      = object;

	if( NS_FAILURE( ns_list_push_back( &vertex->edges, edge ), error ) )
		{
		ns_delete( edge );
		return error;
		}

	if( NULL != E )
		*E = ns_list_rev_begin( &vertex->edges );

	return ns_no_error();
	}


NsError ns_graph_vertex_add_undir_edge
	(
	nsgraphvertexiter   from,
	nsgraphvertexiter   to,
	nspointer           from_to_object,
	nspointer           to_from_object,
	nsgraphedgeiter    *FT,
	nsgraphedgeiter    *TF
	)
	{
	NsGraphVertex    *from_vertex;
	NsGraphVertex    *to_vertex;
	NsGraphEdge      *from_to_edge;
	NsGraphEdge      *to_from_edge;
	nsgraphedgeiter   from_to;
	nsgraphedgeiter   to_from;
	NsError           error;


	ns_assert( NULL != from );
	ns_assert( NULL != to );

	from_vertex = _ns_graph_vertex( from );
	to_vertex   = _ns_graph_vertex( to );

	ns_assert( from_vertex->graph == to_vertex->graph );
	ns_assert( ! ns_graph_is_directed( from_vertex->graph ) );

	from_to_edge = NULL;
	to_from_edge = NULL;

	if( NULL == ( from_to_edge = ns_new( NsGraphEdge ) ) ||
		 NULL == ( to_from_edge = ns_new( NsGraphEdge ) )   )
		{
		ns_delete( from_to_edge );
		ns_delete( to_from_edge );

		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	if( NS_FAILURE( ns_list_push_back( &from_vertex->edges, from_to_edge ), error ) )
		{
		ns_delete( from_to_edge );
		ns_delete( to_from_edge );

		return error;
		}

	if( NS_FAILURE( ns_list_push_back( &to_vertex->edges, to_from_edge ), error ) )
		{
		/* Temporarily set the vertices edge list destructor to NULL. */
		NsFinalizeFunc finalize_func = ns_list_get_finalize_func( &from_vertex->edges );

		ns_list_set_finalize_func( &from_vertex->edges, NULL );
		ns_list_pop_back( &from_vertex->edges );
		ns_list_set_finalize_func( &from_vertex->edges, finalize_func );

		ns_delete( from_to_edge );
		ns_delete( to_from_edge );

		return error;
		}

	from_to = ns_list_rev_begin( &from_vertex->edges );
	to_from = ns_list_rev_begin( &to_vertex->edges );

	from_to_edge->src_vertex  = from;
	from_to_edge->dest_vertex = to;
	from_to_edge->mirror_edge = to_from;
	from_to_edge->object      = from_to_object;

	to_from_edge->src_vertex  = to;
	to_from_edge->dest_vertex = from;
	to_from_edge->mirror_edge = from_to;
	to_from_edge->object      = to_from_object;

	if( NULL != FT )
		*FT = from_to;

	if( NULL != TF )
		*TF = to_from;

	return ns_no_error();
	}


void ns_graph_vertex_remove_edge( nsgraphvertexiter V, nsgraphedgeiter E )
	{
	ns_assert( NULL != V );
	ns_assert( NULL != E );
	ns_assert( ns_graph_vertex_iter_equal( V, _ns_graph_edge( E )->src_vertex ) );

	ns_list_erase( &( _ns_graph_vertex( V )->edges ), E );
	}


nspointer ns_graph_vertex_iter_get_object( const nsgraphvertexiter V )
	{
	ns_assert( NULL != V );
	return _ns_graph_vertex( V )->object;
	}


void ns_graph_vertex_iter_set_object( nsgraphvertexiter V, nspointer object )
	{
	ns_assert( NULL != V );
	_ns_graph_vertex( V )->object = object;
	}


nsgraphvertexiter ns_graph_edge_src_vertex( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	return _ns_graph_edge( E )->src_vertex;
	}


nsgraphvertexiter ns_graph_edge_dest_vertex( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	return _ns_graph_edge( E )->dest_vertex;
	}


nslong ns_graph_edge_get_color( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	return _ns_graph_edge( E )->color;
	}


void ns_graph_edge_set_color( nsgraphedgeiter E, nslong color )
	{
	ns_assert( NULL != E );
	_ns_graph_edge( E )->color = color;
	}


nsgraphedgeiter ns_graph_undir_edge_mirror( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	ns_assert( ! ns_graph_is_directed( _ns_graph_vertex( _ns_graph_edge( E )->src_vertex )->graph ) );

	return _ns_graph_edge( E )->mirror_edge;
	}


nspointer ns_graph_edge_iter_get_object( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	return _ns_graph_edge( E )->object;
	}


void ns_graph_edge_iter_set_object( nsgraphedgeiter E, nspointer object )
	{
	ns_assert( NULL != E );
	_ns_graph_edge( E )->object = object;
	}


NsGraph* ns_graph_vertex_graph( const nsgraphvertexiter V )
	{
	ns_assert( NULL != V );
	return _ns_graph_vertex( V )->graph;
	}


NsGraph* ns_graph_edgns_graph( const nsgraphedgeiter E )
	{
	ns_assert( NULL != E );
	return _ns_graph_vertex( _ns_graph_edge( E )->src_vertex )->graph;
	}
