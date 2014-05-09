#include "nsmodel-origs.h"


NS_PRIVATE void _ns_model_do_find_origins_as_largest
   (
	nsmodelvertex   vertex,
	nsmodelvertex  *origin
   )
   {
	nsmodeledge curr, end;

	if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) )
		{
		ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

		if( ns_model_vertex_get_radius( *origin ) < ns_model_vertex_get_radius( vertex ) )
			*origin = vertex;

		curr = ns_model_vertex_begin_edges( vertex );
		end  = ns_model_vertex_end_edges( vertex );

		for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
			_ns_model_do_find_origins_as_largest(
				ns_model_edge_dest_vertex( curr ),
				origin
				);
		}
   }


NsError ns_model_create_origins( NsModel *model, NsProgress *progress )
	{
   nsmodelvertex  curr, end, origin;
   NsError        error;


   ns_assert( NULL != model );

	if( NULL != progress )
		ns_progress_set_title( progress, "Creating Origins..." );

	ns_model_clear_origins( model );

	error = ns_no_error();
	ns_model_lock( model );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr ) )
         {
			origin = curr;

			_ns_model_do_find_origins_as_largest( curr, &origin );

         if( NS_FAILURE( ns_model_add_origin( model, origin, NULL ), error ) )
            goto _NS_MODEL_CREATE_ORIGINS_EXIT;
         }

	_NS_MODEL_CREATE_ORIGINS_EXIT:

	ns_model_unlock( model );
	return error;
	}


NS_PRIVATE void _ns_model_do_find_origins_as_largest_recursive
   (
	const nsmodelvertex   parent_vertex,
	const nsmodelvertex   curr_vertex,
	nsmodelvertex        *origin_vertex,
	nsdouble             *sum,
	nssize               *count
   )
   {
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge, end_edges;


	*sum += ( nsdouble )ns_model_vertex_get_radius( curr_vertex );
	++(*count);

	if( ns_model_vertex_get_radius( *origin_vertex ) <
		 ns_model_vertex_get_radius( curr_vertex ) )
		*origin_vertex = curr_vertex;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_do_find_origins_as_largest_recursive(
				curr_vertex,
				dest_vertex,
				origin_vertex,
				sum,
				count
				);
		}
   }


NS_PRIVATE void _ns_model_origins_do_find_largest_external_vertex
   (
	const nsmodelvertex   parent_vertex,
	const nsmodelvertex   curr_vertex,
	nsmodelvertex        *largest_external_vertex
   )
   {
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge, end_edges;


	if( ns_model_vertex_is_external( curr_vertex ) )
		if( ns_model_vertex_equal( NS_MODEL_VERTEX_NIL, *largest_external_vertex ) ||
				ns_model_vertex_get_radius( *largest_external_vertex ) <
				ns_model_vertex_get_radius( curr_vertex ) )
			*largest_external_vertex = curr_vertex;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_origins_do_find_largest_external_vertex(
				curr_vertex,
				dest_vertex,
				largest_external_vertex
				);
		}
   }


NS_PRIVATE NsError ns_model_find_origin_as_largest_recursive( NsModel *model, nsmodelvertex curr_vertex )
	{
	nsmodelvertex  origin_vertex;
	nsmodelorigin  O;
	nsdouble       sum;
	nsdouble       average;
	nssize         count;
	NsError        error;


   ns_assert( NULL != model );

	origin_vertex = curr_vertex;

	sum   = 0.0;
	count = 0;

	_ns_model_do_find_origins_as_largest_recursive(
		NS_MODEL_VERTEX_NIL,
		curr_vertex,
		&origin_vertex,
		&sum,
		&count
		);

	if( NS_FAILURE( ns_model_add_origin( model, origin_vertex, &O ), error ) )
		return error;

	/* NOTE: Dont include the current origin's value into the average. */
	ns_assert( ( nsdouble )ns_model_vertex_get_radius( origin_vertex ) <= sum );
	ns_assert( 0 < count );
	sum -= ( nsdouble )ns_model_vertex_get_radius( origin_vertex );
	--count;

	average = sum / ( nsdouble )count;

	if( ns_model_vertex_get_radius( origin_vertex ) < NS_MODEL_ORIGIN_SCALAR * average )
		{
		origin_vertex = NS_MODEL_VERTEX_NIL;

		_ns_model_origins_do_find_largest_external_vertex(
			NS_MODEL_VERTEX_NIL,
			curr_vertex,
			&origin_vertex
			);

		ns_assert( ns_model_vertex_not_equal( NS_MODEL_VERTEX_NIL, origin_vertex ) );
		ns_model_reset_origin( model, O, origin_vertex );
		}

	return ns_no_error();
	}


NsError ns_model_origin_create_if_none( NsModel *model, nsmodelvertex V )
	{
	nssize   conn_comp;
	NsError  error;


	ns_assert( NULL != model );

	conn_comp = ns_model_vertex_get_conn_comp( V );

	if( ns_model_origin_equal(
			NS_MODEL_ORIGIN_NIL,
			ns_model_find_origin_by_conn_comp( model, conn_comp ) ) )
		if( NS_FAILURE(
				ns_model_find_origin_as_largest_recursive(
					model,
					V
					),
				error ) )
			return error;

	return ns_no_error();
	}
