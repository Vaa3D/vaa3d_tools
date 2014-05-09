#include "nsmodel-order.h"


NS_PRIVATE void _ns_model_do_order_centrifugal( nsmodeledge edge )
	{
	nsmodelvertex  vertex;
   nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nsushort       order;


	vertex = ns_model_edge_dest_vertex( edge );

	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
	ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

	/* If not a branching point, propagate current order further,
		else increment order and then propagate. */

	order = ns_model_edge_get_order( edge );

	if( ns_model_vertex_is_junction( vertex ) )
		++order;
		
	curr_edge = ns_model_vertex_begin_edges( vertex );
	end_edges = ns_model_vertex_end_edges( vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) ) 
		if( NS_MODEL_VERTEX_UNSEEN ==
			 ns_model_vertex_get_color( ns_model_edge_dest_vertex( curr_edge ) ) )
			{
			ns_model_edge_set_order( curr_edge, order, NS_TRUE );
			_ns_model_do_order_centrifugal( curr_edge );
			}
	}


/* Start at origin and recursively set edge orders by incrementing order
	number every time a branching point is reached. */
NS_PRIVATE void _ns_model_order_centrifugal( NsModel *model, NsProgress *progress )
	{
   nsmodeltree    curr;
   nsmodeltree    end;
	nsmodelvertex  parent;
	nsmodelvertex  root;
   nsmodeledge    edge;


   ns_assert( NULL != model );
   NS_USE_VARIABLE( progress );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_trees( model );
   end  = ns_model_end_trees( model );

   for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
      {
      parent = ns_model_tree_parent( curr );
		root   = ns_model_tree_root( curr );

      ns_model_vertex_set_color( parent, NS_MODEL_VERTEX_SEEN );

		edge = ns_model_vertex_find_edge( parent, root );
		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( parent ) ) );

		ns_model_edge_set_order( edge, NS_MODEL_EDGE_ORDER_FIRST, NS_TRUE );
		_ns_model_do_order_centrifugal( edge );
      }
	}


NS_PRIVATE void _ns_model_do_order_centripetal( nsmodeledge edge )
	{
	nsmodelvertex  vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nssize         count;
	nsushort       max_order;


	vertex = ns_model_edge_dest_vertex( edge );

	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
	ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

	/* First go to the tips of the tree structure. */
	curr_edge = ns_model_vertex_begin_edges( vertex );
	end_edges = ns_model_vertex_end_edges( vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		if( NS_MODEL_VERTEX_UNSEEN ==
			 ns_model_vertex_get_color( ns_model_edge_dest_vertex( curr_edge ) ) )
			_ns_model_do_order_centripetal( curr_edge );

	/* If no forward neighbors, i.e. tip of branch, the edge pointing
		here is first order. */
	if( ns_model_vertex_is_external( vertex ) )
		ns_model_edge_set_order( edge, NS_MODEL_EDGE_ORDER_FIRST, NS_TRUE );
	else
		{
		max_order = 0;
		curr_edge = ns_model_vertex_begin_edges( vertex );
		
		/* Get max branch order of neighbors. */
		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) ) 
			if( max_order < ns_model_edge_get_order( curr_edge ) )
				max_order = ns_model_edge_get_order( curr_edge );

		count     = 0;
		curr_edge = ns_model_vertex_begin_edges( vertex );

		/* See how many neighbors are of the 'max_order'. */
		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) ) 
			if( ns_model_edge_get_order( curr_edge ) == max_order )
				{
				++count;

				/* NOTE: 'count' should always be >= 1 but dont go past 2! */
				if( 1 < count )
					break;
				}

		/* NOTE:
			If count is 1, order is not incremented.
			If count is 2, order is incremented.
		*/
		ns_assert( 1 == count || 2 == count );
 		ns_model_edge_set_order( edge, max_order + ( nsushort )( count - 1 ), NS_TRUE );
		}
	}


NS_PRIVATE void _ns_model_order_centripetal( NsModel *model, NsProgress *progress )
	{
   nsmodeltree    curr;
   nsmodeltree    end;
	nsmodelvertex  parent;
	nsmodelvertex  root;
   nsmodeledge    edge;


   ns_assert( NULL != model );
   NS_USE_VARIABLE( progress );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_trees( model );
   end  = ns_model_end_trees( model );

   for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
      {
      parent = ns_model_tree_parent( curr );
		root   = ns_model_tree_root( curr );

      ns_model_vertex_set_color( parent, NS_MODEL_VERTEX_SEEN );

		edge = ns_model_vertex_find_edge( parent, root );
		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( parent ) ) );

		_ns_model_do_order_centripetal( edge );
      }
	}


NS_PRIVATE void _ns_model_order_none( NsModel *model, NsProgress *progress )
	{
	NS_USE_VARIABLE( progress );
	ns_model_order_edges( model, NS_MODEL_EDGE_ORDER_NONE );
	}


NsError ns_model_order
	(
	NsModel           *model,
	NsModelOrderType   type,
	NsProgress        *progress
	)
	{
	NsError error;

   ns_assert( NULL != model );

   ns_model_set_order_type( model, type );
	_ns_model_order_none( model, progress );

	/* Since we are only setting the order of the model we dont
		need the section centers to be regenerated. */
	ns_model_needs_edge_order_centers( model );

	if( NS_MODEL_ORDER_NONE == type )
		return ns_no_error();

	if( NS_FAILURE( ns_model_create_trees( model ), error ) )
		return error;

   switch( type )
      {
		case NS_MODEL_ORDER_CENTRIFUGAL:
			_ns_model_order_centrifugal( model, progress );
			break;

		case NS_MODEL_ORDER_CENTRIPETAL:
			_ns_model_order_centripetal( model, progress );
			break;

      default:
         ns_assert_not_reached();
      }

	return ns_no_error();
	}
