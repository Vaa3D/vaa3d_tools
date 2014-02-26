#include "nsmodel-section.h"


const nschar* ns_model_section_type_to_string( NsModelSectionType type )
	{
	NS_PRIVATE const nschar* ____ns_model_section_type_strings[ NS_MODEL_SECTION_NUM_TYPES ] =
		{
		"none",
		"depth-first",
		"breadth-first"
		};

	ns_assert( type < NS_MODEL_SECTION_NUM_TYPES );
	return ____ns_model_section_type_strings[ type ];
	}


NS_PRIVATE nsmodelvertex _ns_model_mark_section
	(
	nsmodelvertex  curr_vertex,
	nsmodeledge    curr_edge,
   nsulong        section
   )
   {
   nsmodelvertex dest_vertex;

	NS_INFINITE_LOOP
		{
      ns_assert( NS_MODEL_EDGE_UNSEEN == ns_model_edge_get_color( curr_edge ) );
      ns_model_edge_set_color( curr_edge, NS_MODEL_EDGE_SEEN, NS_TRUE );

      ns_model_edge_set_section( curr_edge, section, NS_TRUE );

      ns_assert( curr_vertex == ns_model_edge_src_vertex( curr_edge ) );
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ! ns_model_vertex_is_line( dest_vertex ) )
         break;

      /* Advance to the next vertex. Note that we have to find the
         edge that is not pointing back to the current vertex. */
      curr_edge = ns_model_vertex_begin_edges( dest_vertex );

      if( ns_model_vertex_equal( curr_vertex, ns_model_edge_dest_vertex( curr_edge ) ) )
         curr_edge = ns_model_edge_next( curr_edge );

      curr_vertex = dest_vertex;
      }

   return dest_vertex;
   }


#include <std/nsqueue.h>

NS_PRIVATE void _ns_model_section_breadth_first( NsModel *model, NsProgress *progress )
   {
   nsmodeltree    curr;
   nsmodeltree    end;
	nsmodelvertex  parent;
	nsmodelvertex  root;
	nsmodeledge    edge;
   nsmodelvertex  curr_vertex;
   nsmodelvertex  next_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nsulong        section;
   NsQueue        queue;


   ns_assert( NULL != model );
   NS_USE_VARIABLE( progress );

   section = NS_MODEL_EDGE_SECTION_NONE;

   ns_queue_construct( &queue, NULL );

   ns_model_color_edges( model, NS_MODEL_EDGE_UNSEEN );

	/* NOTE: The first branch of the tree is a special case. We cant
		traverse all of its edges since some could lead back into the
		soma. */

   curr = ns_model_begin_trees( model );
   end  = ns_model_end_trees( model );

   for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
      {
      parent = ns_model_tree_parent( curr );
		root   = ns_model_tree_root( curr );

		edge = ns_model_vertex_find_edge( parent, root );
		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( parent ) ) );

      ++section;

      next_vertex = _ns_model_mark_section( parent, edge, section );
      ns_queue_add( &queue, next_vertex ); /* IMPORTANT: Ignore failure? */
      }

   while( ! ns_queue_is_empty( &queue ) )
      {
      curr_vertex = ns_queue_front( &queue );
      ns_queue_remove( &queue );

      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         if( NS_MODEL_EDGE_UNSEEN == ns_model_edge_get_color( curr_edge ) )
            {
            ++section;

            next_vertex = _ns_model_mark_section( curr_vertex, curr_edge, section );
            ns_queue_add( &queue, next_vertex ); /* IMPORTANT: Ignore failure? */
            }
      }

   ns_queue_destruct( &queue );
   }


NS_PRIVATE void _ns_model_do_section_depth_first( nsmodeledge edge, nsulong *section )
   {
   nsmodelvertex  vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   vertex = ns_model_edge_dest_vertex( edge );

	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
   ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );
      
   curr_edge = ns_model_vertex_begin_edges( vertex );
   end_edges = ns_model_vertex_end_edges( vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) ) 
      if( NS_MODEL_VERTEX_UNSEEN ==
          ns_model_vertex_get_color( ns_model_edge_dest_vertex( curr_edge ) ) )
         {
         if( ns_model_vertex_is_junction( vertex ) )
            ++(*section);

         ns_model_edge_set_section( curr_edge, *section, NS_TRUE );
         _ns_model_do_section_depth_first( curr_edge, section );
         }
   }


NS_PRIVATE void _ns_model_section_depth_first( NsModel *model, NsProgress *progress )
   {
   nsmodeltree    curr;
   nsmodeltree    end;
	nsmodelvertex  parent;
	nsmodelvertex  root;
   nsmodeledge    edge;
   nsulong        section;


   ns_assert( NULL != model );
   NS_USE_VARIABLE( progress );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   section = NS_MODEL_EDGE_SECTION_NONE;

   curr = ns_model_begin_trees( model );
   end  = ns_model_end_trees( model );

   for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
      {
      parent = ns_model_tree_parent( curr );
		root   = ns_model_tree_root( curr );

      ns_model_vertex_set_color( parent, NS_MODEL_VERTEX_SEEN );

		edge = ns_model_vertex_find_edge( parent, root );
		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( parent ) ) );

		++section;
		ns_model_edge_set_section( edge, section, NS_TRUE );
      
		_ns_model_do_section_depth_first( edge, &section );
      }
   }


NS_PRIVATE void _ns_model_section_none( NsModel *model, NsProgress *progress )
   {
   NS_USE_VARIABLE( progress );
   ns_model_section_edges( model, NS_MODEL_EDGE_SECTION_NONE );
   }


NsError ns_model_section
   (
   NsModel             *model,
   NsModelSectionType   type,
   NsProgress          *progress
   )
   {
	NsError error;

   ns_assert( NULL != model );

   ns_model_set_section_type( model, type );
	_ns_model_section_none( model, progress );

	/* Since we are only setting the sections of the model we dont
		need the order centers to be regenerated. */
	ns_model_needs_edge_section_centers( model );

	if( NS_MODEL_SECTION_NONE == type )
		return ns_no_error();

	if( NS_FAILURE( ns_model_create_trees( model ), error ) )
		return error;

   switch( type )
      {
      case NS_MODEL_SECTION_DEPTH_FIRST:
         _ns_model_section_depth_first( model, progress );
         break;

      case NS_MODEL_SECTION_BREADTH_FIRST:
         _ns_model_section_breadth_first( model, progress );
         break;

      default:
         ns_assert_not_reached();
      }

	return ns_no_error();
   }

