#include "nsmodel-remove.h"


NsError ns_model_remove_selected_vertices( NsModel *model, NsList *list )
	{
	nsmodelvertex  curr, next, end;
	nssize         conn_comp, num_conn_comps;
	NsList         vertices;
	nslistiter     iter;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != list );

	error = ns_no_error();

	ns_model_needs_edge_centers( model );

	ns_list_construct( &vertices, NULL );

	ns_model_lock( model );

	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

	while( ns_model_vertex_not_equal( curr, end ) )
		{
		next = ns_model_vertex_next( curr );

		if( ns_model_vertex_is_selected( curr ) )
			{
			if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, curr ), error ) )
				goto _NS_MODEL_REMOVE_SELECTED_VERTICES_EXIT;

			/* Before we delete the vertex we recursively mark
				all connected vertices of it as manually edited. */
			if( ! ns_model_vertex_is_manually_traced( curr ) )
				ns_model_vertex_mark_manually_traced_recursive( curr, NS_TRUE );

			if( ns_model_vertex_is_origin( curr ) )
				ns_model_remove_origin_by_vertex( model, curr );

			ns_model_remove_vertex( model, curr );
			}

		curr = next;
		}

	conn_comp = 0;

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr ) )
			{
			/* Set the number for all vertices in this component. */
         ns_model_vertex_set_conn_comp_number_recursive(
				curr,
				conn_comp
				);

			if( NS_FAILURE( ns_list_push_back( &vertices, curr ), error ) )
				goto _NS_MODEL_REMOVE_SELECTED_VERTICES_EXIT;

			++conn_comp;
			}

	num_conn_comps = conn_comp;

	/* If theres no origin for a component, then just pick it
		as the vertex with the biggest radius. */
	NS_LIST_FOREACH( &vertices, iter )
		if( NS_FAILURE(
				ns_model_origin_create_if_none(
					model,
					ns_list_iter_get_object( iter )
					),
				error ) )
			goto _NS_MODEL_REMOVE_SELECTED_VERTICES_EXIT;

	ns_assert( num_conn_comps == ns_model_num_origins( model ) );

	_NS_MODEL_REMOVE_SELECTED_VERTICES_EXIT:

	ns_list_destruct( &vertices );

	ns_model_unlock( model );
	return error;
	}
