#include "nsmodel.h"

/* NOTE: Avoid circular inclusion! */
#include "nsmodel-graft.h"
#include "nsmodel-spines.h"
#include "nssettings.h"


const nschar* ns_model_vertex_type_to_string( NsModelVertexType type )
   {
   NS_PRIVATE const nschar* _ns_model_vertex_type_strings[ NS_MODEL_VERTEX_NUM_TYPES ] =
      {
      "origin",
      "external",
      "junction",
      "line"
      };

   ns_assert( ( nssize )type < NS_MODEL_VERTEX_NUM_TYPES );
   return _ns_model_vertex_type_strings[ type ];
   }


const nschar* ns_model_function_type_to_string( NsModelFunctionType type )
   {
   NS_PRIVATE const nschar* _ns_model_function_type_strings[ NS_MODEL_FUNCTION_NUM_TYPES ] =
      {
		"unknown",
		"soma",
		"basal_dendrite",
		"apical_dendrite",
		"axon",
		"invalid"
      };

   ns_assert( ( nssize )type < NS_MODEL_FUNCTION_NUM_TYPES );
   return _ns_model_function_type_strings[ type ];
   }


/* An ever-increasing unique integer. Returns current
   value and then increments the value. In the unlikely
   case that wrap-around occurs it shouldnt matter. */
NS_PRIVATE nsulong ns_model_id( void )
   {
   NS_PRIVATE nsulong _ns_model_id = 101;
   return _ns_model_id++;
   }


#define NS_MODEL_BIT_HAS_EDGE_ORDER_CENTERS    0x001
#define NS_MODEL_BIT_HAS_EDGE_SECTION_CENTERS  0x002
#define NS_MODEL_BIT_IS_FILTERING              0x004


#define NS_MODEL_OBJECT_BIT_DELETED  0x80000000


#define NS_MODEL_VERTEX_BIT_ORIGIN           0x001
#define NS_MODEL_VERTEX_BIT_SELECTED         0x002
#define NS_MODEL_VERTEX_BIT_VALID            0x004
#define NS_MODEL_VERTEX_BIT_MANUALLY_TRACED  0x008
#define NS_MODEL_VERTEX_BIT_JOINABLE         0x010
#define NS_MODEL_VERTEX_BIT_HIDDEN           0x020
#define NS_MODEL_VERTEX_BIT_SEED             0x040
#define NS_MODEL_VERTEX_BIT_UNREMOVABLE      0x080


#define NS_MODEL_EDGE_BIT_ORDER_CENTER    0x001
#define NS_MODEL_EDGE_BIT_SECTION_CENTER  0x002
#define NS_MODEL_EDGE_BIT_UNREMOVABLE     0x004


NsModelVertex* ____ns_model_vertex( nsmodelvertex V, const nschar *details )
   {
   NsModelVertex *vertex;

   ns_assert_with_details( ns_model_vertex_not_equal( V, NS_MODEL_VERTEX_NIL ), details );

   vertex = ns_graph_vertex_iter_get_object( V );

   ns_assert_with_details( NULL != vertex, details );
	ns_assert( ! ( ( nsboolean )( vertex->flags & NS_MODEL_OBJECT_BIT_DELETED ) ) );

   return vertex;
   }


NsModelEdge* ____ns_model_edge( nsmodeledge E, const nschar *details )
   {
   NsModelEdge *edge;

   ns_assert_with_details( ns_model_edge_not_equal( E, NS_MODEL_EDGE_NIL ), details );

   edge = ns_graph_edge_iter_get_object( E );

   ns_assert_with_details( NULL != edge, details );
	ns_assert( ! ( ( nsboolean )( edge->flags & NS_MODEL_OBJECT_BIT_DELETED ) ) );

   return edge;
   }


NS_PRIVATE NsModelVertex* ns_model_vertex_assign( NsModelVertex *dest, const NsModelVertex *src )
   {
   ns_assert( NULL != dest );
   ns_assert( NULL != src );

   *dest = *src;

   return dest;
   }


NS_PRIVATE NsModelEdge* ns_model_edge_assign( NsModelEdge *dest, const NsModelEdge *src )
   {
   ns_assert( NULL != dest );
   ns_assert( NULL != src );

   *dest = *src;

   return dest;
   }


NS_PRIVATE NsError ns_model_vertex_new( NsModelVertex **V )
   {
   ns_assert( NULL != V );

   if( NULL == ( *V = ns_new0( NsModelVertex ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   return ns_no_error();
   }


NS_PRIVATE NsError ns_model_edge_new( NsModelEdge **E )
   {
   ns_assert( NULL != E );

   if( NULL == ( *E = ns_new0( NsModelEdge ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   return ns_no_error();
   }


NS_PRIVATE NsError ns_model_vertex_new_assign
   (
   NsModelVertex        **dest,
   const NsModelVertex   *src
   )
   {
   NsError error;

   ns_assert( NULL != dest );
   ns_assert( NULL != src );

   if( NS_FAILURE( ns_model_vertex_new( dest ), error ) )
      return error;

   ns_model_vertex_assign( *dest, src );
   return ns_no_error();
   }


NS_PRIVATE NsError ns_model_edge_new_assign
   (
   NsModelEdge        **dest,
   const NsModelEdge   *src
   )
   {
   NsError error;

   ns_assert( NULL != dest );
   ns_assert( NULL != src );

   if( NS_FAILURE( ns_model_edge_new( dest ), error ) )
      return error;

   ns_model_edge_assign( *dest, src );
   return ns_no_error();
   }


NS_PRIVATE void ns_model_vertex_delete( NsModelVertex *V )
   {
   ns_assert( NULL != V );

   /* IMPORTANT: The origin's must be cleared before the vertices! */
   ns_assert( ! ( ( nsboolean )( V->flags & NS_MODEL_VERTEX_BIT_ORIGIN ) ) );

	V->flags |= NS_MODEL_OBJECT_BIT_DELETED;

   ns_delete( V );
   }


NS_PRIVATE void ns_model_edge_delete( NsModelEdge *E )
   {
   ns_assert( NULL != E );

	E->flags |= NS_MODEL_OBJECT_BIT_DELETED;

   ns_delete( E );
   }


NS_PRIVATE void _ns_model_vertex_set_origin( NsModelVertex *vertex )
	{
   vertex->flags     |= NS_MODEL_VERTEX_BIT_ORIGIN;
	vertex->func_type  = NS_MODEL_FUNCTION_SOMA;
	}


NS_PRIVATE void _ns_model_vertex_clear_origin( NsModelVertex *vertex )
	{
   vertex->flags     &= ~NS_MODEL_VERTEX_BIT_ORIGIN;
	vertex->func_type  = NS_MODEL_FUNCTION_UNKNOWN;
	}


NS_PRIVATE void _ns_model_vertex_set_seed( NsModelVertex *vertex )
	{  vertex->flags |= NS_MODEL_VERTEX_BIT_SEED;  }


NS_PRIVATE void _ns_model_vertex_clear_seed( NsModelVertex *vertex )
	{  vertex->flags &= ~NS_MODEL_VERTEX_BIT_SEED;  }


NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsmodelvertex, nspointer );


NS_PRIVATE void ns_model_origin_finalize_func( nspointer object )
   {
   nsmodelvertex V;

   ns_assert( NULL != object );
   V = object;

   ns_assert( ns_model_vertex_is_origin( V ) );
   _ns_model_vertex_clear_origin( ns_model_vertex( V ) );
   }


NS_PRIVATE void ns_model_seed_finalize_func( nspointer object )
   {
   nsmodelvertex V;

   ns_assert( NULL != object );
   V = object;

   ns_assert( ns_model_vertex_is_seed( V ) );
   _ns_model_vertex_clear_seed( ns_model_vertex( V ) );
   }


void ns_model_construct( NsModel *model )
   {
   NsError error;

   ns_assert( NULL != model );
   ns_memzero( model, sizeof( NsModel ) );

   ns_verify( NS_SUCCESS( ns_string_construct_init( &model->name, "" ), error ) );

   ns_list_construct(
      &model->origins,
      ns_model_origin_finalize_func
      );

	ns_list_construct(
		&model->seeds,
		ns_model_seed_finalize_func
		);

   ns_graph_construct(
      &model->graph,
      NS_FALSE,
      ns_model_vertex_delete,
      ns_model_edge_delete
      );

   ns_list_construct( &model->data, NULL );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &model->buffer, 0 ), error ) );
   
   ns_list_construct( &model->conn_comps, ns_free );
	ns_list_construct( &model->trees, ns_free );

   ns_octree_construct( model->octrees + NS_MODEL_EDGE_OCTREE );
   ns_octree_construct( model->octrees + NS_MODEL_VERTEX_OCTREE );

   ns_value_construct( &model->value );

   ns_verify( NS_SUCCESS( ns_model_new_grafter( model ), error ) );
   ns_verify( NS_SUCCESS( ns_model_new_spines( model ), error ) );

	ns_model_needs_edge_centers( model );

   ns_mutex_construct( &model->mutex );
   ns_mutex_set_name( &model->mutex, "model->mutex" );
   ns_mutex_create( &model->mutex ); /* NOTE: Ignore failure. */
   }


void ns_model_destruct( NsModel *model )
   {
   ns_assert( NULL != model );

   ns_string_destruct( &model->name );

   /* NOTE: The origin and seed lists must be destructed before the graph
		since vertices are accessed during their destruction.  */

   ns_list_destruct( &model->origins );
	ns_list_destruct( &model->seeds );

   ns_graph_destruct( &model->graph );

   ns_list_destruct( &model->data );

   ns_byte_array_destruct( &model->buffer );

   ns_list_destruct( &model->conn_comps );
	ns_list_destruct( &model->trees );

   ns_octree_destruct( model->octrees + NS_MODEL_EDGE_OCTREE );
   ns_octree_destruct( model->octrees + NS_MODEL_VERTEX_OCTREE );

   ns_value_destruct( &model->value );

   ns_model_delete_grafter( model );
   ns_model_delete_spines( model );

   ns_mutex_destruct( &model->mutex );
   }


void ns_model_lock( NsModel *model )
   {
   NsError error;

   ns_assert( NULL != model );

   if( ! ns_mutex_is_created( &model->mutex ) ||
       NS_FAILURE( ns_mutex_lock( &model->mutex ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );
   }


nsboolean ns_model_try_lock( NsModel *model )
   {
   NsError    error;
   nsboolean  did_lock = NS_FALSE;


   ns_assert( NULL != model );

   if( ! ns_mutex_is_created( &model->mutex ) ||
       NS_FAILURE( ns_mutex_try_lock( &model->mutex, &did_lock ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );

   return did_lock;
   }


void ns_model_unlock( NsModel *model )
   {
   NsError error;

   ns_assert( NULL != model );

   if( ! ns_mutex_is_created( &model->mutex ) ||
       NS_FAILURE( ns_mutex_unlock( &model->mutex ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );
   }


NsValue* ns_model_value( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ( NsValue* )&model->value;
   }


nsuint ns_model_get_thresh_count( const NsModel *model )
	{
	ns_assert( NULL != model );
	return model->thresh_count;
	}


void ns_model_set_thresh_count( NsModel *model, nsuint thresh_count )
	{
	ns_assert( NULL != model );
	model->thresh_count = thresh_count;
	}


void ns_model_up_thresh_count( NsModel *model )
	{
	ns_assert( NULL != model );
	++(model->thresh_count);
	}


const nschar* ns_model_get_name( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_string_get( &model->name );
   }


NsError ns_model_set_name( NsModel *model, const nschar *name )
   {
   ns_assert( NULL != model );
   return ns_string_set( &model->name, name );
   }


NsVector3f* ns_model_find_center( const NsModel *model, NsVector3f *C )
   {
   nslistiter  curr;
   nslistiter  end;
   nsfloat     radius;
   nsfloat     sum;
   NsVector3f  P;


	ns_assert( NULL != C );

   sum = 0.0f;
   ns_vector3f_zero( C );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      {         
      radius = /*ns_powf( */ns_model_vertex_get_radius( curr )/*, power )*/;

      sum += radius;

      ns_model_vertex_get_position( curr, &P );

      ns_vector3f_cmpd_scale( &P, radius );
      ns_vector3f_cmpd_add( C, &P );
      }

   ns_vector3f_cmpd_scale( C, 1.0f / sum );
	return C;
   }


nsint ns_model_get_order_type( const NsModel *model )
   {
   ns_assert( NULL != model );
   return model->order_type;
   }


void ns_model_set_order_type( NsModel *model, nsint type )
   {
   ns_assert( NULL != model );
   model->order_type = type;
   }


nsint ns_model_get_section_type( const NsModel *model )
   {
   ns_assert( NULL != model );
   return model->section_type;
   }


void ns_model_set_section_type( NsModel *model, nsint type )
   {
   ns_assert( NULL != model );
   model->section_type = type;
   }


void ns_model_needs_edge_centers( NsModel *model )
	{
   ns_assert( NULL != model );

	model->flags &= ~NS_MODEL_BIT_HAS_EDGE_ORDER_CENTERS;
	model->flags &= ~NS_MODEL_BIT_HAS_EDGE_SECTION_CENTERS;
	}


void ns_model_needs_edge_order_centers( NsModel *model )
	{
   ns_assert( NULL != model );
	model->flags &= ~NS_MODEL_BIT_HAS_EDGE_ORDER_CENTERS;
	}


void ns_model_needs_edge_section_centers( NsModel *model )
	{
   ns_assert( NULL != model );
	model->flags &= ~NS_MODEL_BIT_HAS_EDGE_SECTION_CENTERS;
	}


nsboolean ns_model_has_edge_order_centers( const NsModel *model )
	{
   ns_assert( NULL != model );
	return ( nsboolean )( model->flags & NS_MODEL_BIT_HAS_EDGE_ORDER_CENTERS );
	}


NS_PRIVATE void _ns_model_clear_edge_order_centers( NsModel *model )
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         ns_model_edge_mark_order_center( curr_edge, NS_FALSE, NS_FALSE );
      }
	}


NS_PRIVATE void _ns_model_find_edge_order_center( NsList *edges, nsfloat *distance )
	{
	nsfloat      half_distance;
	nslistiter   iter;
	nsmodeledge  scan;
	nsfloat      delta;


	ns_assert( ! ns_list_is_empty( edges ) );

	/* Hit a "stopping point". Find center from the list of edges. Lets be
		extra paranoid and make sure that if only one edge then it is marked
		as the center. */

	if( 1 == ns_list_size( edges ) )
		ns_model_edge_mark_order_center(
			ns_list_iter_get_object( ns_list_begin( edges ) ),
			NS_TRUE,
			NS_TRUE
			);
	else
		{
		half_distance = (*distance) / 2.0f;
		delta         = 0.0f;

		NS_LIST_FOREACH( edges, iter )
			{
			scan   = ns_list_iter_get_object( iter );
			delta += ns_model_edge_length( scan );

			if( delta >= half_distance )
				{
				ns_model_edge_mark_order_center( scan, NS_TRUE, NS_TRUE );
				break;
				}
			}
		}

	/* Reset before returning. */
	ns_list_clear( edges );
	*distance = 0.0f;
	}


NS_PRIVATE NsError _ns_model_do_calc_edge_order_centers
	(
	NsList       *edges,
	nsmodeledge   edge,
	nsfloat       distance
	)
	{
	nsmodelvertex  vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	NsError        error;


	vertex = ns_model_edge_dest_vertex( edge );

	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
	ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

	if( NS_FAILURE( ns_list_push_back( edges, edge ), error ) )
		return error;

	distance += ns_model_edge_length( edge );

	if( ns_model_vertex_is_external( vertex ) || ns_model_vertex_is_junction( vertex ) )
		_ns_model_find_edge_order_center( edges, &distance );

	/* Recursively traverse the model. */

	curr_edge = ns_model_vertex_begin_edges( vertex );
	end_edges = ns_model_vertex_end_edges( vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		if( NS_MODEL_VERTEX_UNSEEN ==
			ns_model_vertex_get_color( ns_model_edge_dest_vertex( curr_edge ) ) )
			{
			/* Other condition for setting an order center is if we're not at an
				external or junction point (i.e. line) and the next edge has a 
				different order than the current edge. */
			if( ns_model_vertex_is_line( vertex ) &&
				 ns_model_edge_get_order( curr_edge ) != ns_model_edge_get_order( edge ) )
				_ns_model_find_edge_order_center( edges, &distance );

			if( NS_FAILURE( _ns_model_do_calc_edge_order_centers( edges, curr_edge, distance ), error ) )
				return error;
			}

	return ns_no_error();
	}


NsError ns_model_calc_edge_order_centers( NsModel *model )
	{
	NsList         edges;
	nsmodelorigin  curr_origin, end_origins;
	nsmodeledge    curr_edge, end_edges;
	nsmodelvertex  vertex;
	NsError        error;


	ns_assert( NULL != model );

/*TEMP*/ns_print( "Calculating edge order centers..." );

	_ns_model_clear_edge_order_centers( model );

	ns_list_construct( &edges, NULL );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		vertex = ns_model_origin_vertex( curr_origin );
		ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			{
			ns_list_clear( &edges );

			if( NS_FAILURE( _ns_model_do_calc_edge_order_centers( &edges, curr_edge, 0.0f ), error ) )
				{
				ns_list_destruct( &edges );
				return error;
				}
			}
		}

	model->flags |= NS_MODEL_BIT_HAS_EDGE_ORDER_CENTERS;

/*TEMP*/ns_println( " done" );

	ns_list_destruct( &edges );
	return ns_no_error();
	}


nsboolean ns_model_has_edge_section_centers( const NsModel *model )
	{
   ns_assert( NULL != model );
	return ( nsboolean )( model->flags & NS_MODEL_BIT_HAS_EDGE_SECTION_CENTERS );
	}


NS_PRIVATE void _ns_model_clear_edge_section_centers( NsModel *model )
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         ns_model_edge_mark_section_center( curr_edge, NS_FALSE, NS_FALSE );
      }
	}


NS_PRIVATE void _ns_model_find_edge_section_center( NsList *edges, nsfloat *distance )
	{
	nsfloat      half_distance;
	nslistiter   iter;
	nsmodeledge  scan;
	nsfloat      delta;


	ns_assert( ! ns_list_is_empty( edges ) );

	/* Hit a "stopping point". Find center from the list of edges. Lets be
		extra paranoid and make sure that if only one edge then it is marked
		as the center. */

	if( 1 == ns_list_size( edges ) )
		ns_model_edge_mark_section_center(
			ns_list_iter_get_object( ns_list_begin( edges ) ),
			NS_TRUE,
			NS_TRUE
			);
	else
		{
		half_distance = (*distance) / 2.0f;
		delta         = 0.0f;

		NS_LIST_FOREACH( edges, iter )
			{
			scan   = ns_list_iter_get_object( iter );
			delta += ns_model_edge_length( scan );

			if( delta >= half_distance )
				{
				ns_model_edge_mark_section_center( scan, NS_TRUE, NS_TRUE );
				break;
				}
			}
		}

	/* Reset before returning. */
	ns_list_clear( edges );
	*distance = 0.0f;
	}


NS_PRIVATE NsError _ns_model_do_calc_edge_section_centers
	(
	NsList       *edges,
	nsmodeledge   edge,
	nsfloat       distance
	)
	{
	nsmodelvertex  vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	NsError        error;


	vertex = ns_model_edge_dest_vertex( edge );

	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
	ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

	if( NS_FAILURE( ns_list_push_back( edges, edge ), error ) )
		return error;

	distance += ns_model_edge_length( edge );

	if( ns_model_vertex_is_external( vertex ) || ns_model_vertex_is_junction( vertex ) )
		_ns_model_find_edge_section_center( edges, &distance );

	/* Recursively traverse the model. */

	curr_edge = ns_model_vertex_begin_edges( vertex );
	end_edges = ns_model_vertex_end_edges( vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		if( NS_MODEL_VERTEX_UNSEEN ==
			ns_model_vertex_get_color( ns_model_edge_dest_vertex( curr_edge ) ) )
			if( NS_FAILURE( _ns_model_do_calc_edge_section_centers( edges, curr_edge, distance ), error ) )
				return error;

	return ns_no_error();
	}


NsError ns_model_calc_edge_section_centers( NsModel *model )
	{
	NsList         edges;
	nsmodelorigin  curr_origin, end_origins;
	nsmodeledge    curr_edge, end_edges;
	nsmodelvertex  vertex;
	NsError        error;


	ns_assert( NULL != model );

/*TEMP*/ns_print( "Calculating edge section centers..." );

	_ns_model_clear_edge_section_centers( model );

	ns_list_construct( &edges, NULL );

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		vertex = ns_model_origin_vertex( curr_origin );
		ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			{
			ns_list_clear( &edges );

			if( NS_FAILURE( _ns_model_do_calc_edge_section_centers( &edges, curr_edge, 0.0f ), error ) )
				{
				ns_list_destruct( &edges );
				return error;
				}
			}
		}

	model->flags |= NS_MODEL_BIT_HAS_EDGE_SECTION_CENTERS;

/*TEMP*/ns_println( " done" );

	ns_list_destruct( &edges );
	return ns_no_error();
	}


void ns_model_set_is_filtering( NsModel *model, nsboolean b )
	{
	ns_assert( NULL != model );

	if( b )
		model->flags |= NS_MODEL_BIT_IS_FILTERING;
	else
		model->flags &= (~NS_MODEL_BIT_IS_FILTERING);
	}


nsboolean ns_model_get_is_filtering( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ( nsboolean )( model->flags & NS_MODEL_BIT_IS_FILTERING );
	}


nsfloat ns_model_vertex_distance( const nsmodelvertex V1, const nsmodelvertex V2 )
	{
	NsVector3f P1, P2;

	return ns_vector3f_distance(
				ns_model_vertex_get_position( V1, &P1 ),
				ns_model_vertex_get_position( V2, &P2 )
				);
	}


NS_PRIVATE _ns_model_do_set_soma_distances
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex,
	nsfloat        distance
	)
	{
	nsmodeledge    curr_edge, end_edges;
	nsmodelvertex  dest_vertex;


	/* NOTE: The distance only starts increasing after we have traversed out of the soma. */
	if( NS_MODEL_FUNCTION_SOMA != ns_model_vertex_get_function_type( curr_vertex ) )
		if( ns_model_vertex_not_equal( parent_vertex, NS_MODEL_VERTEX_NIL ) )
			distance += ns_model_vertex_distance( curr_vertex, parent_vertex );

	ns_model_vertex_set_soma_distance( curr_vertex, distance );

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_do_set_soma_distances( curr_vertex, dest_vertex, distance );
		}
	}


void ns_model_set_soma_distances( NsModel *model )
	{
	nsmodelorigin curr, end;

	ns_assert( NULL != model );

/*TEMP*/ns_print( "Setting soma distances..." );

	curr = ns_model_begin_origins( model );
	end  = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
		_ns_model_do_set_soma_distances(
			NS_MODEL_VERTEX_NIL,
			ns_model_origin_vertex( curr ),
			0.0f
			);

/*TEMP*/ns_println( " done" );
	}


void ns_model_resize
	(
	NsModel           *model,
	const NsVector3f  *old_voxel_size,
	const NsVector3f  *new_voxel_size
	)
	{
	nsmodelvertex  curr_vertex, end_vertices;
	NsVector3f     ratio, position;


	ns_assert( NULL != model );
	ns_assert( NULL != old_voxel_size );
	ns_assert( NULL != new_voxel_size );

	ratio.x = new_voxel_size->x / old_voxel_size->x;
	ratio.y = new_voxel_size->y / old_voxel_size->y;
	ratio.z = new_voxel_size->z / old_voxel_size->z;

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		ns_model_vertex_set_position(
			curr_vertex,
			ns_vector3f_cmpd_non_uni_scale( ns_model_vertex_get_position( curr_vertex, &position ), &ratio )
			);

		/* Note that we're assuming that the radius is calculated in 2D, i.e.
			in the XY plane. Also assuming x and y voxel size are the same, so
			it doesnt matter which one is used to rescale the radius. */
		ns_model_vertex_set_radius(
			curr_vertex,
			ns_model_vertex_get_radius( curr_vertex ) * ratio.x
			);
		}
	}


void ns_model_auto_iter_up( NsModel *model )
   {
   ns_assert( NULL != model );
   ++(model->auto_iter);
   }


void ns_model_auto_iter_down( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( 0 < model->auto_iter );

   --(model->auto_iter);
   }


nssize ns_model_auto_iter( const NsModel *model )
   {
   ns_assert( NULL != model );
   return model->auto_iter;
   }


nssize ns_model_vertex_get_auto_iter( const nsmodelvertex V )
   {  return ns_model_vertex( V )->auto_iter;  }


void ns_model_vertex_set_auto_iter( nsmodelvertex V )
   {
   NsModelVertex *vertex = ns_model_vertex( V );

   ns_assert( NULL != vertex->model );
   vertex->auto_iter = vertex->model->auto_iter;
   }


NsError ns_model_auto_iter_list_add_by_vertex( NsList *list, nsmodelvertex V )
   {
   ns_assert( NULL != list );
   return ns_model_auto_iter_list_add_by_number( list, ns_model_vertex_get_auto_iter( V ) );
   }


NsError ns_model_auto_iter_list_add_by_number( NsList *list, nssize auto_iter )
   {
   ns_assert( NULL != list );
   return ns_list_push_back( list, NS_SIZE_TO_POINTER( auto_iter ) );
   }


NS_PRIVATE NsError ns_model_assign_origins_and_seeds
   (
   NsModel        *lhs,
   nsmodelvertex   lhs_curr,
   nsmodelvertex   lhs_end,
   const NsModel  *rhs,
   nsmodelvertex   rhs_curr,
   nsmodelvertex   rhs_end
   )
   {
   NsError error;

   NS_USE_VARIABLE( rhs );

   for( ; ns_model_vertex_not_equal( rhs_curr, rhs_end );
          rhs_curr = ns_model_vertex_next( rhs_curr ) )
      {
		/* NOTE: Have to clear the origin and/or seed flag before adding
			since bitwise copies were made of the source vertices. */

      if( ns_model_vertex_is_origin( rhs_curr ) )
         {
         _ns_model_vertex_clear_origin( ns_model_vertex( lhs_curr ) );

         if( NS_FAILURE( ns_model_add_origin( lhs, lhs_curr, NULL ), error ) )
            return error;
         }

      if( ns_model_vertex_is_seed( rhs_curr ) )
         {
         _ns_model_vertex_clear_seed( ns_model_vertex( lhs_curr ) );

         if( NS_FAILURE( ns_model_add_seed( lhs, lhs_curr, NULL ), error ) )
            return error;
         }

      ns_assert( ns_model_vertex_not_equal( lhs_curr, lhs_end ) );
      lhs_curr = ns_model_vertex_next( lhs_curr );
      }

   ns_assert( ns_model_vertex_equal( lhs_curr, lhs_end ) );

   return ns_no_error();
   } 


NS_PRIVATE NsError ns_model_assign_vertex( nspointer *dest, const NsModelVertex *src )
   {
   NsModelVertex  *vertex;
   NsError         error;


   if( NS_FAILURE( ns_model_vertex_new_assign( &vertex, src ), error ) )
      return error;

   /* NOTE: Set the vertices "owner" model pointer! */
   vertex->model = src->data[ NS_MODEL_VERTEX_DATA_RESERVED ];

   *dest = vertex;

   /* TODO: Update progress? */

   return ns_no_error();
   }


NS_PRIVATE NsError ns_model_assign_edge( nspointer *dest, const NsModelEdge *src )
   {
   NsModelEdge  *edge;
   NsError       error;


   if( NS_FAILURE( ns_model_edge_new_assign( &edge, src ), error ) )
      return error;

   *dest = edge;

   return ns_no_error();
   }


NsError ns_model_assign( NsModel *lhs, const NsModel *rhs )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   NsError        error;


   ns_assert( NULL != lhs );
   ns_assert( NULL != rhs );

   ns_model_clear( lhs );

	error = ns_no_error();

	ns_model_lock( lhs );
	ns_model_lock( ( NsModel* )rhs );

   curr_vertex  = ns_model_begin_vertices( rhs );
   end_vertices = ns_model_end_vertices( rhs );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      ns_model_vertex_set_data( curr_vertex, NS_MODEL_VERTEX_DATA_RESERVED, lhs );/* TEMP? */

   if( NS_FAILURE( ns_graph_assign(
                     &lhs->graph,
                     &rhs->graph,
                     ns_model_assign_vertex,
                     ns_model_assign_edge
                     ),
                     error ) )
      goto _NS_MODEL_ASSIGN_EXIT;

   ns_assert( ns_model_num_vertices( lhs ) == ns_model_num_vertices( rhs ) );

   if( NS_FAILURE(
         ns_model_assign_origins_and_seeds(
            lhs,
            ns_model_begin_vertices( lhs ),
            ns_model_end_vertices( lhs ),
            rhs,
            ns_model_begin_vertices( rhs ),
            ns_model_end_vertices( rhs )
            ),
         error ) )
      goto _NS_MODEL_ASSIGN_EXIT;

   ns_assert( ns_model_num_origins( lhs ) == ns_model_num_origins( rhs ) );
   ns_assert( ns_model_num_seeds( lhs ) == ns_model_num_seeds( rhs ) );

	_NS_MODEL_ASSIGN_EXIT:

	ns_model_unlock( lhs );
	ns_model_unlock( ( NsModel* )rhs );

   return error;
   }


NsError ns_model_merge( NsModel *lhs, const NsModel *rhs )
   {
   nssize         old_num_vertices;
   nssize         new_num_vertices;
   nssize         old_num_origins;
   nssize         new_num_origins;
   nssize         old_num_seeds;
   nssize         new_num_seeds;
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   NsError        error;


   ns_assert( NULL != lhs );
   ns_assert( NULL != rhs );

	error = ns_no_error();

	ns_model_lock( lhs );
	ns_model_lock( ( NsModel* )rhs );

	ns_model_needs_edge_centers( lhs );

   old_num_vertices = ns_model_num_vertices( lhs );
   old_num_origins  = ns_model_num_origins( lhs );
	old_num_seeds    = ns_model_num_seeds( lhs );

   curr_vertex  = ns_model_begin_vertices( rhs );
   end_vertices = ns_model_end_vertices( rhs );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      ns_model_vertex_set_data( curr_vertex, NS_MODEL_VERTEX_DATA_RESERVED, lhs );/* TEMP? */

   if( NS_FAILURE( ns_graph_add(
                     &lhs->graph,
                     &rhs->graph,
                     ns_model_assign_vertex,
                     ns_model_assign_edge
                     ),
                     error ) )
      goto _NS_MODEL_MERGE_EXIT;

   new_num_vertices = ns_model_num_vertices( lhs );
   ns_assert( new_num_vertices == old_num_vertices + ns_model_num_vertices( rhs ) );

   if( old_num_vertices < new_num_vertices )
      if( NS_FAILURE(
            ns_model_assign_origins_and_seeds(
               lhs,
               ns_model_vertex_at( lhs, old_num_vertices ),
               ns_model_end_vertices( lhs ),
               rhs,
               ns_model_begin_vertices( rhs ),
               ns_model_end_vertices( rhs )
               ),
            error ) )
         goto _NS_MODEL_MERGE_EXIT;

   new_num_origins = ns_model_num_origins( lhs );
   ns_assert( new_num_origins == old_num_origins + ns_model_num_origins( rhs ) );

   new_num_seeds = ns_model_num_seeds( lhs );
   ns_assert( new_num_seeds == old_num_seeds + ns_model_num_seeds( rhs ) );

	_NS_MODEL_MERGE_EXIT:

	ns_model_unlock( lhs );
	ns_model_unlock( ( NsModel* )rhs );

   return error;
   }


void ns_model_clear( NsModel *model )
   {
   ns_assert( NULL != model );

   ns_model_lock( model );

   /* NOTE: The origin and seed lists must be cleared before the graph since
      vertices are accessed while they are being cleared.  */

   ns_list_clear( &model->origins );
	ns_list_clear( &model->seeds );

   ns_graph_clear( &model->graph );

   ns_list_clear( &model->data );

   ns_byte_array_clear( &model->buffer );

   ns_list_clear( &model->conn_comps );
	ns_list_clear( &model->trees );

   ns_octree_clear( model->octrees + NS_MODEL_EDGE_OCTREE );
   ns_octree_clear( model->octrees + NS_MODEL_VERTEX_OCTREE );

   ns_model_set_data_finalize_func( model, NULL );

	ns_model_needs_edge_centers( model );

   ns_model_unlock( model );
   }


NS_PRIVATE void _ns_model_clear_non_manually_traced_origins( NsModel *model )
   {
   nsmodelorigin curr, next, end;

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   while( ns_model_origin_not_equal( curr, end ) )
      {
      next = ns_model_origin_next( curr );

      if( ! ns_model_vertex_is_manually_traced( ns_model_origin_vertex( curr ) ) )
          ns_list_erase( &model->origins, curr );

      curr = next;
      }
   }


NS_PRIVATE void _ns_model_clear_non_manually_traced_seeds( NsModel *model )
   {
   nsmodelseed curr, next, end;

   curr = ns_model_begin_seeds( model );
   end  = ns_model_end_seeds( model );

   while( ns_model_seed_not_equal( curr, end ) )
      {
      next = ns_model_seed_next( curr );

      if( ! ns_model_vertex_is_manually_traced( ns_model_seed_vertex( curr ) ) )
          ns_list_erase( &model->seeds, curr );

      curr = next;
      }
   }


NS_PRIVATE void _ns_model_clear_non_manually_traced_vertices( NsModel *model )
   {
   nsmodelvertex curr, next, end;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   while( ns_model_vertex_not_equal( curr, end ) )
      {
      next = ns_model_vertex_next( curr );

      if( ! ns_model_vertex_is_manually_traced( curr ) )
         ns_graph_remove_vertex( &model->graph, curr, NS_FALSE );

      curr = next;
      }
   }


void ns_model_clear_non_manually_traced( NsModel *model )
   {
   ns_assert( NULL != model );

   ns_model_lock( model );

   /* NOTE: The origin and seed lists must be cleared before the graph since
      vertices are accessed while its being cleared.  */
   _ns_model_clear_non_manually_traced_origins( model );
   _ns_model_clear_non_manually_traced_seeds( model );

   _ns_model_clear_non_manually_traced_vertices( model );

   ns_list_clear( &model->data );

   ns_byte_array_clear( &model->buffer );

   ns_list_clear( &model->conn_comps );

   ns_octree_clear( model->octrees + NS_MODEL_EDGE_OCTREE );
   ns_octree_clear( model->octrees + NS_MODEL_VERTEX_OCTREE );

   ns_model_set_data_finalize_func( model, NULL );

   ns_model_unlock( model );
   }


nssize ns_model_num_vertices( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_graph_num_vertices( &model->graph );
   }


NsList* ns_model_vertex_list( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_graph_vertex_list( &model->graph );
   }


const NsVoxelInfo* ns_model_get_voxel_info( const NsModel *model )
   {
   ns_assert( NULL != model );
   return model->voxel_info;
   }


void ns_model_set_voxel_info( NsModel *model, const NsVoxelInfo *voxel_info )
   {
   ns_assert( NULL != model );
   model->voxel_info = voxel_info;
   }


nssize ns_model_num_edges( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_graph_num_edges( &model->graph );
   }


NS_PRIVATE void _ns_model_do_num_loops
   (
   nsmodelvertex   parent_vertex,
   nsmodelvertex   curr_vertex,
   nssize          *num_loops
   )
   {
   nsmodeledge     curr_edge;
   nsmodeledge     end_edges;
   nsmodelvertex   dest_vertex;
   nslong          color;


   ns_assert( NS_MODEL_COLOR_WHITE == ns_model_vertex_get_color( curr_vertex ) );
   ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_GREY );

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
         {
         color = ns_model_vertex_get_color( dest_vertex );

         if( NS_MODEL_COLOR_GREY == color )
            *num_loops = *num_loops + 1;
         else if( NS_MODEL_COLOR_WHITE == color )
            _ns_model_do_num_loops( curr_vertex, dest_vertex, num_loops );
         }
      }

   ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_BLACK );
   }


nssize ns_model_num_loops( NsModel *model )
   {
   nsmodelorigin  curr;
   nsmodelorigin  end;
   nssize         num_loops;


   num_loops = 0;

   ns_model_color_vertices( model, NS_MODEL_COLOR_WHITE );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );
                                        
   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      _ns_model_do_num_loops(
         ns_model_end_vertices( model ),
         ns_model_origin_vertex( curr ),
         &num_loops
         );

   return num_loops;
   }


void ns_model_color_vertices( NsModel *model, nslong color )
   {
   ns_assert( NULL != model );
   ns_graph_color_vertices( &model->graph, color );
   }


void ns_model_color_edges( NsModel *model, nslong color )
   {
   ns_assert( NULL != model );
   ns_graph_color_edges( &model->graph, color );
   }


void ns_model_color_vertices_recursive( nsmodelvertex V, nslong color )
   {  ns_graph_color_vertices_recursive( V, color );  }



NS_PRIVATE void ns_model_do_color_all_vertices_recursive
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex,
	nslong         color
	)
	{
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nsmodelvertex  dest_vertex;


	ns_model_vertex_set_color( curr_vertex, color );

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			ns_model_do_color_all_vertices_recursive( curr_vertex, dest_vertex, color );
		}
	}


void ns_model_color_all_vertices_recursive( nsmodelvertex V, nslong color )
	{  ns_model_do_color_all_vertices_recursive( NS_MODEL_VERTEX_NIL, V, color );  }


NS_PRIVATE NsError _ns_model_conn_comp_new( NsModelConnComp **cc, nssize id )
   {
   if( NULL == ( *cc = ns_new( NsModelConnComp ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   (*cc)->id     = id;
   (*cc)->origin = NS_MODEL_VERTEX_NIL;
   (*cc)->list   = NS_MODEL_VERTEX_NIL;

   return ns_no_error();
   }


NS_PRIVATE void _ns_model_do_create_conn_comp( NsModelConnComp *cc, nsmodelvertex vertex )
   {
   nsmodeledge curr, end;

   if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) )
      {
      ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

      ns_model_vertex_set_list( vertex, cc->list );
      cc->list = vertex;

      ns_model_vertex_set_conn_comp( vertex, cc->id );

      if( ns_model_vertex_is_origin( vertex ) )
         {
         ns_assert( ns_model_vertex_equal( cc->origin, NS_MODEL_VERTEX_NIL ) );
         cc->origin = vertex;
         }

      curr = ns_model_vertex_begin_edges( vertex );
      end  = ns_model_vertex_end_edges( vertex );

      for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
         _ns_model_do_create_conn_comp( cc, ns_model_edge_dest_vertex( curr ) );
      }
   }


NS_IMPEXP NsError ns_model_create_conn_comps( NsModel *model )
   {
   nsmodelvertex     curr, end;
   nssize            id;
   NsModelConnComp  *cc;
   NsError           error;


   ns_assert( NULL != model );

   ns_list_clear( &model->conn_comps );

   id = 0;

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr ) )
         {
         if( NS_FAILURE( _ns_model_conn_comp_new( &cc, id ), error ) )
            return error;

         if( NS_FAILURE( ns_list_push_back( &model->conn_comps, cc ), error ) )
            {
            ns_delete( cc );
            return error;
            }

         ++id;

         _ns_model_do_create_conn_comp( cc, curr );
         }

   return ns_no_error();
   }


nssize ns_model_num_conn_comps( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_size( &model->conn_comps );
   }


nslistiter ns_model_begin_conn_comps( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_begin( &model->conn_comps );
   }


nslistiter ns_model_end_conn_comps( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_end( &model->conn_comps );
   }


NS_PRIVATE void _ns_model_vertex_do_set_conn_comp_number_recursive
   (
   nsmodelvertex  parent_vertex,
   nsmodelvertex  curr_vertex,
   nssize         conn_comp
   )
   {
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nsmodelvertex  dest_vertex;


   ns_model_vertex_set_color( curr_vertex, NS_MODEL_VERTEX_SEEN );
   ns_model_vertex_set_conn_comp( curr_vertex, conn_comp );

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
         _ns_model_vertex_do_set_conn_comp_number_recursive(
            curr_vertex,
            dest_vertex,
            conn_comp
            );
      }
   }


nssize ns_model_set_conn_comp_numbers( NsModel *model )
   {
   nsmodelvertex  curr, end;
   nssize         conn_comp;


   ns_assert( NULL != model );

   conn_comp = 0;

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr ) )
         {
         _ns_model_vertex_do_set_conn_comp_number_recursive(
            NS_MODEL_VERTEX_NIL,
            curr,
            conn_comp
            );

         ++conn_comp;
         }

   return conn_comp;
   }


void ns_model_vertex_set_conn_comp_number_recursive( nsmodelvertex V, nssize conn_comp )
   {  _ns_model_vertex_do_set_conn_comp_number_recursive( NS_MODEL_VERTEX_NIL, V, conn_comp );  }


void ns_model_order_edges( NsModel *model, nsushort order )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         ns_model_edge_set_order( curr_edge, order, NS_FALSE );
      }
   }


void ns_model_section_edges( NsModel *model, nsulong section )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         ns_model_edge_set_section( curr_edge, section, NS_FALSE );
      }
   }


nssize ns_model_num_sections( const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nsulong        section;
   nsulong        max_section;


   max_section = 0;

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         {
         section = ns_model_edge_get_section( curr_edge );

         if( max_section < section )
            max_section = section;
         }
      }

   return max_section;
   }


#define _ns_model_section_lengths_get_size( model )\
   ( ns_byte_array_size( &(model)->section_lengths ) / sizeof( nsfloat ) )

#define _ns_model_section_lengths_set_size( model, size )\
   ns_byte_array_resize( &(model)->section_lengths, (size) * sizeof( nsfloat ) )

#define _ns_model_section_lengths_valid( model, section )\
   ( (section) < _ns_model_section_lengths_get_size( (model) ) )

NS_PRIVATE nsfloat* _ns_model_section_lengths_at( const NsModel *model, nsulong section )
   {
   ns_assert( _ns_model_section_lengths_valid( model, section ) );

#ifdef NS_DEBUG
   return ( nsfloat* )ns_byte_array_at( &model->section_lengths, section * sizeof( nsfloat ) );
#else
   return ( ( nsfloat* )ns_byte_array_begin( &model->section_lengths ) ) + section;
#endif
   }


#define _ns_model_section_lengths_get( model, section )\
   ( *( _ns_model_section_lengths_at( (model), (section) ) ) )

#define _ns_model_section_lengths_set( model, section, length )\
   ( *( _ns_model_section_lengths_at( (model), (section) ) ) = (length) )

#define _ns_model_section_lengths_add( model, section, length )\
   ( *( _ns_model_section_lengths_at( (model), (section) ) ) += (length) )

NsError ns_model_calc_section_lengths( NsModel *model )
   {
   nssize         num_sections;
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nsulong        section;
   nsfloat        length;
   NsError        error;


   num_sections = ns_model_num_sections( model );

   /* The section number is 1 based so we need to add
      +1 for the 0 index. */
   if( NS_FAILURE( _ns_model_section_lengths_set_size( model, num_sections + 1 ), error ) )
      return error;

   /* Initialize to zero... */
   for( section = 0; section <= num_sections; ++section )
      _ns_model_section_lengths_set( model, section, 0.0f );

   /* Add all the edges lengths... */
   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         {
         section = ns_model_edge_get_section( curr_edge );
         length  = ns_model_edge_length( curr_edge );

         _ns_model_section_lengths_add( model, section, length );
         }
      }

   /* We have to divide by 2 since there are mirror edges. */
   for( section = 0; section <= num_sections; ++section )
      {
      length = _ns_model_section_lengths_get( model, section ) / 2.0f;
      _ns_model_section_lengths_set( model, section, length );
      }

   return ns_no_error();
   }


nsfloat ns_model_section_length( const NsModel *model, nsulong section )
   {
   ns_assert( NULL != model );

   return _ns_model_section_lengths_valid( model, section ) ?
          _ns_model_section_lengths_get( model, section ) : 0.0f;
   }


nsdouble ns_model_total_dendritic_length( const NsModel *model )
	{
	nsmodelvertex  curr_vertex, end_vertices;
	nsmodeledge    curr_edge, end_edges;
	nsdouble       total;


	ns_assert( NULL != model );

	total = 0.0;

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			total += ns_model_edge_length( curr_edge );
		}

	/* Half the total since the sum includes the length
		of mirrored edges. */
	return total / 2.0;
	}


nsboolean ns_model_has_unset_sections( const NsModel *model )
	{
	nsmodelvertex  curr_vertex, end_vertices;
	nsmodeledge    curr_edge, end_edges;


	ns_assert( NULL != model );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         if( NS_MODEL_EDGE_SECTION_NONE == ns_model_edge_get_section( curr_edge ) )
				return NS_TRUE;
		}

	return NS_FALSE;
	}


NsError ns_model_add_vertex( NsModel *model, nsmodelvertex *V )
   {
   NsModelVertex  *vertex;
   NsError         error;


   ns_assert( NULL != model );

   if( NS_FAILURE( ns_model_vertex_new( &vertex ), error ) )
      return error;

   if( NS_FAILURE( ns_graph_add_vertex( &model->graph, vertex, V ), error ) )
      {
      ns_model_vertex_delete( vertex );
      return error;
      }

   vertex->model = model;
   vertex->flags = NS_MODEL_VERTEX_BIT_VALID;
   vertex->id    = ns_model_id();

   return ns_no_error();
   }


void ns_model_remove_vertex( NsModel *model, nsmodelvertex V )
   {
   ns_assert( NULL != model );
   ns_graph_remove_vertex( &model->graph, V, NS_FALSE );
   }


nsmodeledge ns_model_vertex_find_edge( const nsmodelvertex from, const nsmodelvertex to )
   {
   nsmodeledge curr, end;

   curr = ns_model_vertex_begin_edges( from );
   end  = ns_model_vertex_end_edges( from );

   for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
      if( ns_model_vertex_equal( to, ns_model_edge_dest_vertex( curr ) ) )
         return curr;

   return end;
   }


nsboolean ns_model_vertex_edge_exists( const nsmodelvertex from, const nsmodelvertex to )
   {
   return ns_model_edge_not_equal(
            ns_model_vertex_end_edges( from ),
            ns_model_vertex_find_edge( from, to )
            );
   }


void ns_model_vertex_transpose_edges( nsmodelvertex V )
	{
	NsGraphVertex *vertex = ns_graph_vertex( V );
	ns_list_splice_reverse( &vertex->edges );
	}


nsmodeledge ns_model_vertex_edge_at( const nsmodelvertex V, nssize index )
   {
   nsmodeledge E;

   ns_assert( index < ns_model_vertex_num_edges( V ) );

   E = ns_model_vertex_begin_edges( V );
   return ns_model_edge_offset( E, ( nslong )index );
   }


NsError ns_model_merge_vertices
   (
   NsModel       *model,
   nsmodelvertex  dest,
   nsmodelvertex  src
   )
   {
   nsmodeledge    src_curr;
   nsmodeledge    src_end;
   nsmodeledge    dest_curr;
   nsmodeledge    dest_end;
   nsboolean      do_add;
   nsmodelvertex  vertex;
   NsError        error;


   ns_assert( NULL != model );
   ns_assert( ns_model_vertex_not_equal( dest, src ) );

   src_curr = ns_model_vertex_begin_edges( src );
   src_end  = ns_model_vertex_end_edges( src );

   for( ; ns_model_edge_not_equal( src_curr, src_end );
          src_curr = ns_model_edge_next( src_curr ) )
      {
      do_add = NS_TRUE;
      vertex = ns_model_edge_dest_vertex( src_curr );

      /* If 'src' edge points to the 'dest' then skip it. */
      if( ns_model_vertex_equal( vertex, dest ) )
         do_add = NS_FALSE;
      else
         {
         /* Check if the 'src' edge points to any neighbors of 'dest'. */         
         dest_curr = ns_model_vertex_begin_edges( dest );
         dest_end  = ns_model_vertex_end_edges( dest );

         for( ; ns_model_edge_not_equal( dest_curr, dest_end );
                dest_curr = ns_model_edge_next( dest_curr ) )
            if( ns_model_vertex_equal( vertex, ns_model_edge_dest_vertex( dest_curr ) ) )
               {
               do_add = NS_FALSE;
               break;
               }
         }

      if( do_add )
         if( NS_FAILURE( ns_model_vertex_add_edge( dest, vertex, NULL, NULL ), error ) )
            return error;
      }

   ns_assert( ! ns_model_vertex_is_origin( src ) );
   ns_model_remove_vertex( model, src );

   return ns_no_error();
   }


nssize ns_model_num_origins( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_size( &model->origins );
   }


nsmodelorigin ns_model_origin_by_conn_comp( const NsModel *model, nssize conn_comp )
   {
   nsmodelorigin curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      if( ns_model_vertex_get_conn_comp( ns_model_origin_vertex( curr ) ) == conn_comp )
         return curr;

   return NS_MODEL_ORIGIN_NIL;
   }


NsError ns_model_add_origin( NsModel *model, nsmodelvertex V, nsmodelorigin *O )
   {
   NsError error;

   ns_assert( NULL != model );
   ns_assert( ! ns_model_vertex_is_origin( V ) );

   if( NS_FAILURE( ns_list_push_back( &model->origins, V ), error ) )
		return error;

   _ns_model_vertex_set_origin( ns_model_vertex( V ) );

   if( NULL != O )
      *O = ns_list_rev_begin( &model->origins );

	return ns_no_error();
   }


void ns_model_remove_origin( NsModel *model, nsmodelorigin O )
   {
   ns_assert( NULL != model );
   ns_list_erase( &model->origins, O );
   }


void ns_model_remove_origin_by_conn_comp( NsModel *model, nssize conn_comp )
   {
   nsmodelorigin curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      if( ns_model_vertex_get_conn_comp( ns_model_origin_vertex( curr ) ) == conn_comp )
         {
         ns_model_remove_origin( model, curr );
         return;
         }

   ns_assert_not_reached();
   }


void ns_model_remove_origin_by_vertex( NsModel *model, nsmodelvertex V )
   {
   nsmodelorigin curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      if( ns_model_vertex_equal( ns_model_origin_vertex( curr ), V ) )
         {
         ns_model_remove_origin( model, curr );
         return;
         }

   ns_assert_not_reached();
   }


nsmodelvertex ns_model_find_vertex_by_position( const NsModel *model, const NsVector3f *position )
	{
   nsmodelvertex         curr, end;
	const NsModelVertex  *vertex;	


	ns_assert( NULL != model );
	ns_assert( NULL != position );

   curr = ns_model_rev_begin_vertices( model );
   end  = ns_model_rev_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_prev( curr ) )
		{
		vertex = ns_model_vertex( curr );

		if( NS_FLOAT_EQUAL( position->x, vertex->position.x ) &&
			 NS_FLOAT_EQUAL( position->y, vertex->position.y ) &&
			 NS_FLOAT_EQUAL( position->z, vertex->position.z )   )
			return curr;
		}

	return NS_MODEL_VERTEX_NIL;
	}


nsmodelorigin ns_model_find_origin_by_conn_comp( const NsModel *model, nssize conn_comp )
   {
   nsmodelorigin curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      if( ns_model_vertex_get_conn_comp( ns_model_origin_vertex( curr ) ) == conn_comp )
         return curr;

   return NS_MODEL_ORIGIN_NIL;
   }


nsmodelorigin ns_model_find_origin_by_vertex( const NsModel *model, nsmodelvertex V )
   {
   nsmodelorigin curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
      if( ns_model_vertex_equal( ns_model_origin_vertex( curr ), V ) )
         return curr;

   return NS_MODEL_ORIGIN_NIL;
   }


void ns_model_reset_origin( NsModel *model, nsmodelorigin O, nsmodelvertex V )
   {
   ns_assert( NULL != model );

   ns_assert(
      ns_model_vertex_get_conn_comp( ns_model_origin_vertex( O ) ) ==
      ns_model_vertex_get_conn_comp( V )
      );      

   ns_assert( ns_model_vertex_is_origin( ns_model_origin_vertex( O ) ) );
	_ns_model_vertex_clear_origin( ns_model_vertex( ns_model_origin_vertex( O ) ) );

   ns_assert( ! ns_model_vertex_is_origin( V ) );
	_ns_model_vertex_set_origin( ns_model_vertex( V ) );

   ns_list_iter_set_object( O, V );
   }


nssize ns_model_num_seeds( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_size( &model->seeds );
   }


NsError ns_model_add_seed( NsModel *model, nsmodelvertex V, nsmodelseed *S )
   {
   NsError error;

   ns_assert( NULL != model );
   ns_assert( ! ns_model_vertex_is_seed( V ) );

   if( NS_FAILURE( ns_list_push_back( &model->seeds, V ), error ) )
		return error;

   _ns_model_vertex_set_seed( ns_model_vertex( V ) );

   if( NULL != S )
      *S = ns_list_rev_begin( &model->seeds );

	return ns_no_error();
   }


void ns_model_remove_seed( NsModel *model, nsmodelseed S )
   {
   ns_assert( NULL != model );

	/*TEMP*/
   ns_println(
      "Removed seed from model " NS_FMT_STRING_DOUBLE_QUOTED
      " with automatic iteration number " NS_FMT_ULONG ".",
      ns_model_get_name( model ),
      ns_model_vertex_get_auto_iter( ns_model_seed_vertex( S ) )
      );

   ns_list_erase( &model->seeds, S );
   }


void ns_model_clear_seeds( NsModel *model )
   {
   ns_assert( NULL != model );

   ns_model_lock( model );
   ns_list_clear( &model->seeds );
   ns_model_unlock( model );
   }


void ns_model_reset_seed( NsModel *model, nsmodelseed S, nsmodelvertex V )
   {
   ns_assert( NULL != model );

   ns_assert(
      ns_model_vertex_get_conn_comp( ns_model_seed_vertex( S ) ) ==
      ns_model_vertex_get_conn_comp( V )
      );      

   ns_assert( ns_model_vertex_is_seed( ns_model_seed_vertex( S ) ) );
	_ns_model_vertex_clear_seed( ns_model_vertex( ns_model_seed_vertex( S ) ) );

   ns_assert( ! ns_model_vertex_is_seed( V ) );
	_ns_model_vertex_set_seed( ns_model_vertex( V ) );

   ns_list_iter_set_object( S, V );
   }


NS_PRIVATE void _ns_model_do_remove_conn_comp( NsModel *model, nsmodelvertex curr_vertex )
   {
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr_vertex ) );
   ns_model_vertex_set_color( curr_vertex, NS_MODEL_VERTEX_SEEN );

   _NS_MODEL_DO_REMOVE_CONN_COMP_RESTART:

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      /* NOTE: Iterators can become invalid! */
      if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) )
         {
         _ns_model_do_remove_conn_comp( model, dest_vertex );
         goto _NS_MODEL_DO_REMOVE_CONN_COMP_RESTART;
         }
      }

   ns_assert( ! ns_model_vertex_is_origin( curr_vertex ) );
   ns_assert( ! ns_model_vertex_is_seed( curr_vertex ) );

   ns_model_remove_vertex( model, curr_vertex );
   }


NS_PRIVATE void ns_model_remove_conn_comp( NsModel *model, nsmodelorigin O, nsmodelseed S )
   {
   nsmodelvertex origin;

   ns_assert( NULL != model );

   origin = ns_model_origin_vertex( O );

   ns_model_remove_origin( model, O );
	ns_model_remove_seed( model, S );

   ns_model_color_vertices_recursive( origin, NS_MODEL_VERTEX_UNSEEN );
   _ns_model_do_remove_conn_comp( model, origin );
   }


NS_PRIVATE nsmodelorigin _ns_model_find_origin_by_auto_iter
   (
   const NsModel  *model,
   nssize          auto_iter
   )
   {
   nsmodelorigin curr;

   NS_MODEL_ORIGIN_FOREACH( model, curr )
      if( ns_model_vertex_get_auto_iter( ns_model_origin_vertex( curr ) ) == auto_iter )
         return curr;

   return NS_MODEL_ORIGIN_NIL;
   }


NS_PRIVATE nsmodelseed _ns_model_find_seed_by_auto_iter
   (
   const NsModel  *model,
   nssize          auto_iter
   )
   {
   nsmodelseed curr;

   NS_MODEL_SEED_FOREACH( model, curr )
      if( ns_model_vertex_get_auto_iter( ns_model_seed_vertex( curr ) ) == auto_iter )
         return curr;

   return NS_MODEL_SEED_NIL;
   }


NS_PRIVATE void ns_model_remove_conn_comp_by_auto_iter( NsModel *model, nssize auto_iter )
   {
   nsmodelorigin  O;
	nsmodelseed    S;


   ns_assert( NULL != model );

   O = _ns_model_find_origin_by_auto_iter( model, auto_iter );

   if( ns_model_origin_not_equal( O, NS_MODEL_ORIGIN_NIL ) )
      {
		S = _ns_model_find_seed_by_auto_iter( model, auto_iter );
		ns_assert( ns_model_seed_not_equal( S, NS_MODEL_SEED_NIL ) );

      ns_model_remove_conn_comp( model, O, S );

      ns_log_echo( NS_TRUE );

      ns_log_entry(
         NS_LOG_ENTRY_NOTE,
         NS_FUNCTION
         ":: Removed connected component from model " NS_FMT_STRING_DOUBLE_QUOTED
         " with automatic iteration number " NS_FMT_ULONG ".",
         ns_model_get_name( model ),
         auto_iter
         );

      ns_log_echo( NS_FALSE );
      }
   }


NsError ns_model_remove_conn_comp_by_auto_iter_list( NsModel *model, const NsList *list )
   {
   nslistiter   iter;
   nspointer    auto_iter;
   NsHashTable  table;
   NsError      error;


   if( NS_FAILURE(
         ns_hash_table_construct(
            &table,
            ns_pointer_hash_func,
            ns_pointer_equal_func,
            NULL,
            NULL
            ),
         error ) )
      return error;

	ns_model_lock( model );

   NS_LIST_FOREACH( list, iter )
      {
      auto_iter = ns_list_iter_get_object( iter );

      /* NOTE: Manually traced vertices should have an 'auto_iter'
         number of zero, so we can avoid a hash by checking for
         this. */
      if( 0 < NS_POINTER_TO_SIZE( auto_iter ) &&
          ! ns_hash_table_exists( &table, auto_iter ) )
         {
         /* NOTE: Ignore error on add() since the worst that would
            happen is that the vertex list will be traversed if the
            same 'auto_iter' number comes up again in the list. */
         ns_hash_table_add( &table, auto_iter, auto_iter );

         ns_model_remove_conn_comp_by_auto_iter(
            model,
            NS_POINTER_TO_SIZE( auto_iter )
            );
         }
      }

   ns_hash_table_destruct( &table );

	ns_model_unlock( model );

   return ns_no_error();
   }


void ns_model_clear_origins( NsModel *model )
   {
   ns_assert( NULL != model );

   ns_model_lock( model );
   ns_list_clear( &model->origins );
   ns_model_unlock( model );
   }


void ns_model_deselect_all_hidden_vertices( NsModel *model )
	{
   nsmodelvertex curr, end;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_is_hidden( curr ) )
         ns_model_vertex_mark_selected( curr, NS_FALSE );
	}


void ns_model_select_vertices( NsModel *model, nsboolean selected )
   {
   nsmodelvertex curr, end;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ! ns_model_vertex_is_hidden( curr ) )
         ns_model_vertex_mark_selected( curr, selected );
   }


NS_PRIVATE NsError _ns_model_do_select_vertices_by_aabbox
	(
	NsModel           *model,
	const NsAABBox3d  *B,
	nssize            *num_intersections
	)
   {
   nsmodelvertex  curr, end;
   NsList         intersected;
   nslistiter     iter;
   nsboolean      any_selected;
   NsSphered      S;
   NsError        error;


   ns_assert( NULL != model );
   ns_assert( NULL != B );

	if( NULL != num_intersections )
		*num_intersections = 0;

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the vertices and store the ones that intersect
      this bounding box. */

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ! ns_model_vertex_is_hidden( curr ) )
         if( ns_sphered_intersects_aabbox(
               ns_model_vertex_to_sphere_ex( curr, &S ),
               B
               ) )
            {
            if( NS_FAILURE( ns_list_push_back( &intersected, curr ), error ) )
               {
               ns_list_destruct( &intersected );
               return error;
               }

            if( ns_model_vertex_is_selected( curr ) )
               any_selected = NS_TRUE;

				if( NULL != num_intersections )
					++(*num_intersections);
            }

   /* If any of the intersecting vertices are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		{
      ns_model_vertex_mark_selected(
         ns_list_iter_get_object( iter ),
         ! any_selected
         );

		/*TEMP!!!!!*/
		/*if( ! any_selected )
			ns_println(
				"T:%f   C:%f",
				ns_model_vertex_get_threshold( ns_list_iter_get_object( iter ), NS_MODEL_VERTEX_NEURITE_THRESHOLD ),
				ns_model_vertex_get_contrast( ns_list_iter_get_object( iter ), NS_MODEL_VERTEX_NEURITE_CONTRAST )
				);*/
		}

   ns_list_destruct( &intersected );
   return ns_no_error();
   }


NsError ns_model_select_vertices_by_aabbox( NsModel *model, const NsAABBox3d *B )
	{  return _ns_model_do_select_vertices_by_aabbox( model, B, NULL );  }


NsError ns_model_select_vertices_by_aabbox_ex
	(
	NsModel           *model,
	const NsAABBox3d  *B,
	nssize            *num_intersections
	)
	{  return _ns_model_do_select_vertices_by_aabbox( model, B, num_intersections );  }


NsError ns_model_select_vertices_by_projection_onto_drag_rect
	(
	NsModel            *model,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj
	)
	{
   nsmodelvertex  curr, end;
   NsList         intersected;
   nslistiter     iter;
   nsboolean      any_selected;
   NsSphered      S;
   NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != drag_rect );
	ns_assert( NULL != proj );

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the vertices and store the ones that intersect
      this drag rectangle. */

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ! ns_model_vertex_is_hidden( curr ) )
         if( ns_sphered_projects_onto_drag_rect(
               ns_model_vertex_to_sphere_ex( curr, &S ),
               drag_rect,
					proj
               ) )
            {
            if( NS_FAILURE( ns_list_push_back( &intersected, curr ), error ) )
               {
               ns_list_destruct( &intersected );
               return error;
               }

            if( ns_model_vertex_is_selected( curr ) )
               any_selected = NS_TRUE;
            }

   /* If any of the intersecting vertices are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		{
      ns_model_vertex_mark_selected(
         ns_list_iter_get_object( iter ),
         ! any_selected
         );
		}

   ns_list_destruct( &intersected );
   return ns_no_error();
	}


void ns_model_select_vertices_by_type( NsModel *model, NsModelVertexType type )
	{
   nsmodelvertex curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		if( ! ns_model_vertex_is_hidden( curr ) )
			ns_model_vertex_mark_selected( curr, type == ns_model_vertex_get_type( curr ) );
	}


void ns_model_select_vertices_by_function( NsModel *model, NsModelFunctionType type )
	{
   nsmodelvertex curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		if( ! ns_model_vertex_is_hidden( curr ) )
			ns_model_vertex_mark_selected( curr, type == ns_model_vertex_get_function_type( curr ) );
	}


nsboolean ns_model_has_any_selected_vertices( const NsModel *model )
   {
   nsmodelvertex curr, end;

   ns_assert( NULL != model );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_is_selected( curr ) )
         return NS_TRUE;

   return NS_FALSE;
   }


nsboolean ns_model_buffer_selected_vertices
   (
   const NsModel  *model,
   nsmodelvertex  *vertices,
   nssize          count
   )
   {
   nsmodelvertex  curr, end;
   nssize         index;


   ns_assert( NULL != model );
   ns_assert( NULL != vertices );

   index = 0;
   curr  = ns_model_begin_vertices( model );   
   end   = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_is_selected( curr ) )
         {
         if( index < count )
            vertices[ index ] = curr;

         ++index;

         if( count < index )
            break;
         }

   return index == count;
   }


NsError ns_model_buffer_selected_vertices_ex
   (
   const NsModel  *model,
   NsList         *list
   )
   {
   nsmodelvertex  curr, end;
   NsError        error;


   ns_assert( NULL != model );
   ns_assert( NULL != list );
   ns_assert( ns_list_is_empty( list ) );
   ns_assert( NULL == ns_list_get_finalize_func( list ) );

   curr = ns_model_begin_vertices( model );   
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_is_selected( curr ) )
         if( NS_FAILURE( ns_list_push_back( list, curr ), error ) )
            return error;

   return ns_no_error();
   }


nssize ns_model_num_external_vertices( const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nssize         count;


   count        = 0;
   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ns_model_vertex_is_external( curr_vertex ) )
         ++count;

   return count;
   }


nssize ns_model_num_junction_vertices( const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nssize         count;


   count        = 0;
   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ns_model_vertex_is_junction( curr_vertex ) )
         ++count;

   return count;
   }


nssize ns_model_num_line_vertices( const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nssize         count;


   count        = 0;
   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ns_model_vertex_is_line( curr_vertex ) )
         ++count;

   return count;
   }


#define _NS_MODEL_IS_BIT( object_func, O, bit )\
   ( ( object_func( (O) )->flags & (bit) ) ? NS_TRUE : NS_FALSE )


#define _NS_MODEL_MARK_BIT( object_func, O, b, bit )\
   if( (b) )\
      object_func( (O) )->flags |= (bit);\
   else\
      object_func( (O) )->flags &= ~(bit)


#define _NS_MODEL_VERTEX_IS_BIT( V, bit )\
   _NS_MODEL_IS_BIT( ns_model_vertex, (V), (bit) )


#define _NS_MODEL_VERTEX_MARK_BIT( V, b, bit )\
   _NS_MODEL_MARK_BIT( ns_model_vertex, (V), (b), (bit) )


#define _NS_MODEL_EDGE_IS_BIT( E, bit )\
   _NS_MODEL_IS_BIT( ns_model_edge, (E), (bit) )


#define _NS_MODEL_EDGE_MARK_BIT( E, b, bit )\
   _NS_MODEL_MARK_BIT( ns_model_edge, (E), (b), (bit) )


nsboolean ns_model_vertex_is_origin( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_ORIGIN );  }


nsboolean ns_model_vertex_is_seed( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_SEED );  }


nsboolean ns_model_vertex_is_selected( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_SELECTED );  }


void ns_model_vertex_mark_selected( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_SELECTED );  }


void ns_model_vertex_toggle_selected( nsmodelvertex V )
   {  ns_model_vertex_mark_selected( V, ! ns_model_vertex_is_selected( V ) );  }


nsboolean ns_model_vertex_is_valid( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_VALID );  }


void ns_model_vertex_mark_valid( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_VALID );  }


nsboolean ns_model_vertex_is_unremovable( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_UNREMOVABLE );  }


void ns_model_vertex_mark_unremovable( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_UNREMOVABLE );  }


void ns_model_mark_joinable( NsModel *model, nsboolean b )
   {
   nsmodelvertex curr;

   ns_assert( NULL != model );

   NS_MODEL_VERTEX_FOREACH( model, curr )
      ns_model_vertex_mark_joinable( curr, b );
   }


nsboolean ns_model_vertex_is_joinable( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_JOINABLE );  }


void ns_model_vertex_mark_joinable( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_JOINABLE );  }


nsboolean ns_model_vertex_is_manually_traced( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_MANUALLY_TRACED );  }


void ns_model_vertex_mark_manually_traced( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_MANUALLY_TRACED );  }


NS_PRIVATE void _ns_model_vertex_do_mark_recursive
   (
   nsmodelvertex  parent_vertex,
   nsmodelvertex  curr_vertex,
   nsboolean      b,
   nsulong        bit
   )
   {
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge, end_edges;


   _NS_MODEL_VERTEX_MARK_BIT( curr_vertex, b, bit );

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
         _ns_model_vertex_do_mark_recursive( curr_vertex, dest_vertex, b, bit );
      }
   }


void ns_model_vertex_mark_manually_traced_recursive( nsmodelvertex V, nsboolean b )
   {
   _ns_model_vertex_do_mark_recursive(
      NS_MODEL_VERTEX_NIL,
      V,
      b,
      NS_MODEL_VERTEX_BIT_MANUALLY_TRACED
      );
   }


void ns_model_mark_manually_traced( NsModel *model, nsboolean b )
   {
   nsmodelvertex curr;

   ns_assert( NULL != model );

   NS_MODEL_VERTEX_FOREACH( model, curr )
      ns_model_vertex_mark_manually_traced( curr, b );
   }


nsboolean ns_model_vertex_is_hidden( const nsmodelvertex V )
   {  return _NS_MODEL_VERTEX_IS_BIT( V, NS_MODEL_VERTEX_BIT_HIDDEN );  }


void ns_model_vertex_mark_hidden( nsmodelvertex V, nsboolean b )
   {  _NS_MODEL_VERTEX_MARK_BIT( V, b, NS_MODEL_VERTEX_BIT_HIDDEN );  }


void ns_model_mark_hidden( NsModel *model, nsboolean b )
   {
   nsmodelvertex curr;

   ns_assert( NULL != model );

   NS_MODEL_VERTEX_FOREACH( model, curr )
      ns_model_vertex_mark_hidden( curr, b );
   }


void ns_model_mark_selected_vertices_hidden( NsModel *model, nsboolean b )
   {
   nsmodelvertex curr;

   ns_assert( NULL != model );

   NS_MODEL_VERTEX_FOREACH( model, curr )
      if( ns_model_vertex_is_selected( curr ) )
         {
         ns_model_vertex_mark_hidden( curr, b );
         ns_model_vertex_mark_selected( curr, NS_FALSE );
         }
   }


nsuint ns_model_vertex_get_file_num( const nsmodelvertex V )
   {  return ns_model_vertex( V )->file_num;  }


void ns_model_vertex_set_file_num( nsmodelvertex V, nsuint file_num )
   {  ns_model_vertex( V )->file_num = file_num;  }


void ns_model_set_file_num( NsModel *model, nsuint file_num )
   {
   nsmodelvertex curr;

   ns_assert( NULL != model );

   NS_MODEL_VERTEX_FOREACH( model, curr )
      ns_model_vertex_set_file_num( curr, file_num );
   }


NsModelFunctionType ns_model_vertex_get_function_type( const nsmodelvertex V )
	{  return ns_model_vertex( V )->func_type;  }


void ns_model_vertex_set_function_type( nsmodelvertex V, NsModelFunctionType func_type )
	{  ns_model_vertex( V )->func_type = func_type;  }



/*
nsfloat ns_model_vertex_get_size_ratio( const nsmodelvertex V )
	{  return ns_model_vertex( V )->size_ratio;  }

void ns_model_vertex_set_size_ratio( nsmodelvertex V, nsfloat size_ratio )
	{  ns_model_vertex( V )->size_ratio = size_ratio;  }
*/


nsfloat ns_model_vertex_get_soma_distance( const nsmodelvertex V )
	{  return ns_model_vertex( V )->soma_distance;  }


void ns_model_vertex_set_soma_distance( nsmodelvertex V, nsfloat soma_distance )
	{  ns_model_vertex( V )->soma_distance = soma_distance;  }


void ns_model_set_function_type( NsModel *model, NsModelFunctionType func_type )
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
		ns_model_vertex_set_function_type( curr_vertex, func_type );

      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			ns_model_edge_set_function_type( curr_edge, func_type, NS_FALSE );
      }
	}


NS_PRIVATE void _ns_model_do_set_function_type_recursive
   (
   nsmodelvertex        parent_vertex,
   nsmodelvertex        curr_vertex,
	NsModelFunctionType  func_type
   )
   {
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge, end_edges;


   ns_model_vertex_set_function_type( curr_vertex, func_type );

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
		ns_model_edge_set_function_type( curr_edge, func_type, NS_TRUE );

      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
         _ns_model_do_set_function_type_recursive( curr_vertex, dest_vertex, func_type );
      }
   }


void ns_model_set_function_type_recursive( nsmodelvertex V, NsModelFunctionType func_type )
	{  _ns_model_do_set_function_type_recursive( NS_MODEL_VERTEX_NIL, V, func_type );  }


nsboolean ns_model_are_function_types_complete( const NsModel *model )
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
		if( NS_MODEL_FUNCTION_UNKNOWN == ns_model_vertex_get_function_type( curr_vertex ) )
			return NS_FALSE;

      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			if( NS_MODEL_FUNCTION_UNKNOWN == ns_model_edge_get_function_type( curr_edge ) )
				return NS_FALSE;
      }

	return NS_TRUE;
	}


NS_PRIVATE NsError _ns_model_do_find_trees
   (
	NsModel        *model,
	nsmodelvertex   parent_vertex,
   nsmodelvertex   curr_vertex
   )
	{
	NsModelTree    *tree;
	nsmodelvertex   dest_vertex;
	nsmodeledge     curr_edge, end_edges;
	NsError         error;


	ns_assert( NS_MODEL_FUNCTION_SOMA == ns_model_vertex_get_function_type( parent_vertex ) );

	if( NS_MODEL_FUNCTION_SOMA != ns_model_vertex_get_function_type( curr_vertex ) )
		{
		if( NULL == ( tree = ns_new( NsModelTree ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

		if( NS_FAILURE( ns_list_push_back( &model->trees, tree ), error ) )
			{
			ns_free( tree );
			return error;
			}

		tree->parent      = parent_vertex;
		tree->root        = curr_vertex;
		tree->initialized = NS_FALSE;

		return ns_no_error();
		}

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			if( NS_FAILURE( _ns_model_do_find_trees( model, curr_vertex, dest_vertex ), error ) )
				return error;
      }

	return ns_no_error();
	}


NS_PRIVATE nsboolean _ns_model_do_valid_tree
	(
	nsmodelvertex        parent_vertex,
	nsmodelvertex        curr_vertex,
	NsModelFunctionType  func_type
	)
	{
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


	if( ns_model_vertex_get_function_type( curr_vertex ) != func_type )
		return NS_FALSE;

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
		/* NOTE: Its OK to check the function type of the edge that leads
			back to the parent, but we obviously dont recurse down it. */
		if( ns_model_edge_get_function_type( curr_edge ) != func_type )
			return NS_FALSE;

      dest_vertex = ns_model_edge_dest_vertex( curr_edge );

      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			if( ! _ns_model_do_valid_tree( curr_vertex, dest_vertex, func_type ) )
				return NS_FALSE;
		}

	return NS_TRUE;
	}


NS_PRIVATE NsError _ns_model_do_create_trees
	(
	NsModel        *model,
	nsmodelvertex   origin_vertex,
	nssize          conn_comp,
	nssize         *total_index
	)
	{
	nssize          cell_index;
	nssize          unknown_index;
	nssize          basal_dendrite_index;
	nssize          apical_dendrite_index;
	nssize          axon_index;
	nssize          invalid_index;
	NsModelTree    *tree;
	nsmodelvertex   dest_vertex;
	nslistiter      iter;
	nsmodeledge     curr_edge, end_edges;
	nsmodeledge     edge;
	NsError         error;


	cell_index            = 0;
	unknown_index         = 0;
	basal_dendrite_index  = 0;
	apical_dendrite_index = 0;
	axon_index            = 0;
	invalid_index         = 0;

   curr_edge = ns_model_vertex_begin_edges( origin_vertex );
   end_edges = ns_model_vertex_end_edges( origin_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( NS_FAILURE( _ns_model_do_find_trees( model, origin_vertex, dest_vertex ), error ) )
			return error;

		NS_LIST_FOREACH( &model->trees, iter )
			{
			tree = ns_list_iter_get_object( iter );

			if( ! tree->initialized )
				{
				edge = ns_model_vertex_find_edge( tree->parent, tree->root );
				ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( tree->parent ) ) );

					/* NOTE: The first edge of the tree indicates the function. */
				tree->func_type = ns_model_edge_get_function_type( edge );
				ns_assert( NS_MODEL_FUNCTION_SOMA != tree->func_type );

				if( ! _ns_model_do_valid_tree( tree->parent, tree->root, tree->func_type ) )
					tree->func_type = NS_MODEL_FUNCTION_INVALID;

				switch( tree->func_type )
					{
					case NS_MODEL_FUNCTION_UNKNOWN:
						tree->type_index = unknown_index++;
						break;

					case NS_MODEL_FUNCTION_BASAL_DENDRITE:
						tree->type_index = basal_dendrite_index++;
						break;

					case NS_MODEL_FUNCTION_APICAL_DENDRITE:
						tree->type_index = apical_dendrite_index++;
						break;

					case NS_MODEL_FUNCTION_AXON:
						tree->type_index = axon_index++;
						break;

					case NS_MODEL_FUNCTION_INVALID:
						tree->type_index = invalid_index++;
						break;

					default:
						ns_assert_not_reached();
					}

				tree->conn_comp   = conn_comp;
				tree->total_index = (*total_index)++;
				tree->cell_index  = cell_index++;
				tree->initialized = NS_TRUE;

				ns_assert( tree->conn_comp == ns_model_vertex_get_conn_comp( origin_vertex ) );
				}
			}
		}

	return ns_no_error();
	}


NsError ns_model_create_trees( NsModel *model )
	{
   nsmodelorigin  curr, end;
	nsmodelvertex  origin;
	nssize         total_index;
   NsError        error;


	error = ns_no_error();

	ns_model_set_conn_comp_numbers( model );

	ns_list_clear( &model->trees );

	total_index = 0;

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
		{
		origin = ns_model_origin_vertex( curr );

      if( NS_FAILURE(
				_ns_model_do_create_trees(
					model,
					origin,
					ns_model_vertex_get_conn_comp( origin ),
					&total_index
					),
				error ) )
			break;
		}

	return error;
	}


nssize ns_model_num_trees( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ns_list_size( &model->trees );
	}


nsmodeltree ns_model_begin_trees( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ns_list_begin( &model->trees );
	}


nsmodeltree ns_model_end_trees( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ns_list_end( &model->trees );
	}


NS_PRIVATE NsModelTree* ns_model_tree( const nsmodeltree T )
	{
	NsModelTree *tree = ns_list_iter_get_object( T );

	ns_assert( NULL != tree );
	return tree;
	}


NsModelFunctionType ns_model_tree_function_type( const nsmodeltree T )
	{  return ns_model_tree( T )->func_type;  }


nssize ns_model_tree_conn_comp( const nsmodeltree T )
	{  return ns_model_tree( T )->conn_comp;  }


nssize ns_model_tree_total_index( const nsmodeltree T )
	{  return ns_model_tree( T )->total_index;  }


nssize ns_model_tree_cell_index( const nsmodeltree T )
	{  return ns_model_tree( T )->cell_index;  }


nssize ns_model_tree_type_index( const nsmodeltree T )
	{  return ns_model_tree( T )->type_index;  }


nsmodelvertex ns_model_tree_parent( const nsmodeltree T )
	{  return ns_model_tree( T )->parent;  }


nsmodelvertex ns_model_tree_root( const nsmodeltree T )
	{  return ns_model_tree( T )->root;  }


nsmodelvertex ns_model_rev_begin_vertices( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_graph_rev_begin_vertices( &model->graph );
   }


nsmodelvertex ns_model_rev_end_vertices( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_graph_rev_end_vertices( &model->graph );
   }


nsmodelvertex ns_model_vertex_at( const NsModel *model, nssize index )
   {
   nsmodelvertex V;

   ns_assert( index < ns_model_num_vertices( model ) );

   V = ns_model_begin_vertices( model );
   return ns_model_vertex_offset( V, ( nslong )index );
   }


NsModelVertexType ns_model_vertex_get_type( const nsmodelvertex V )
   {
   NsModelVertexType type;

   if( ns_model_vertex_is_origin( V ) )
      type = NS_MODEL_VERTEX_ORIGIN;
   else
      {
      switch( ns_model_vertex_num_edges( V ) )
         {
         case 0:
         case 1:
            type = NS_MODEL_VERTEX_EXTERNAL;
            break;

         case 2:
            type = NS_MODEL_VERTEX_LINE;
            break;

         default:
            type = NS_MODEL_VERTEX_JUNCTION;
         }
      }

   return type;
   }


nsmodelvertex ns_model_vertex_get_list( const nsmodelvertex V )
   {  return ns_model_vertex( V )->next;  }


void ns_model_vertex_set_list( nsmodelvertex V, nsmodelvertex next )
   {  ns_model_vertex( V )->next = next;  }


nsulong ns_model_vertex_get_id( const nsmodelvertex V )
   {  return ns_model_vertex( V )->id;  }


void ns_model_vertex_set_id( nsmodelvertex V, nsulong id )
   {  ns_model_vertex( V )->id = id;  }


NsModel* ns_model_vertex_model( const nsmodelvertex V )
   {  return ns_model_vertex( V )->model;  }


nssize ns_model_vertex_get_conn_comp( const nsmodelvertex V )
   {  return ns_model_vertex( V )->conn_comp;  }


void ns_model_vertex_set_conn_comp( nsmodelvertex V, nssize conn_comp )
   {  ns_model_vertex( V )->conn_comp = conn_comp;  }


NsVector3f* ns_model_vertex_get_position( const nsmodelvertex V, NsVector3f *P )
   {
   ns_assert( NULL != P );

   *P = ns_model_vertex( V )->position;
   return P;
   }


NsVector3d* ns_model_vertex_get_position_ex( const nsmodelvertex V, NsVector3d *P )
   {
   NsVector3f T;

   ns_model_vertex_get_position( V, &T );

   ns_vector3f_to_3d( &T, P );
   return P;
   }


void ns_model_vertex_set_position( nsmodelvertex V, const NsVector3f *P )
   {
   ns_assert( NULL != P );
   ns_model_vertex( V )->position = *P;
   }


NsSpheref* ns_model_vertex_to_sphere( const nsmodelvertex V, NsSpheref *sphere )
   {
   ns_assert( NULL != sphere );

   return ns_spheref(
            sphere,
            ns_model_vertex_get_position_x( V ),
            ns_model_vertex_get_position_y( V ),
            ns_model_vertex_get_position_z( V ),
            ns_model_vertex_get_radius( V )
            );
   }


NsSphered* ns_model_vertex_to_sphere_ex( const nsmodelvertex V, NsSphered *sphere )
   {
   ns_assert( NULL != sphere );

	ns_sphered(
		sphere,
		( nsdouble )ns_model_vertex_get_position_x( V ),
		( nsdouble )ns_model_vertex_get_position_y( V ),
		( nsdouble )ns_model_vertex_get_position_z( V ),
		( nsdouble )ns_model_vertex_get_radius( V )
		);

//ns_println( "S = %f,%f,%f  %f", sphere->O.x, sphere->O.y, sphere->O.z, sphere->r );

	return sphere;
   }


nsfloat ns_model_vertex_get_position_x( const nsmodelvertex V )
   {  return ns_model_vertex( V )->position.x;  }


nsfloat ns_model_vertex_get_position_y( const nsmodelvertex V )
   {  return ns_model_vertex( V )->position.y;  }


nsfloat ns_model_vertex_get_position_z( const nsmodelvertex V )
   {  return ns_model_vertex( V )->position.z;  }


void ns_model_vertex_set_position_x( nsmodelvertex V, nsfloat x )
   {  ns_model_vertex( V )->position.x = x;  }


void ns_model_vertex_set_position_y( nsmodelvertex V, nsfloat y )
   {  ns_model_vertex( V )->position.y = y;  }


void ns_model_vertex_set_position_z( nsmodelvertex V, nsfloat z )
   {  ns_model_vertex( V )->position.z = z;  }


nsfloat ns_model_vertex_get_radius( const nsmodelvertex V )
   {  return ns_model_vertex( V )->radius;  }


void ns_model_vertex_set_radius( nsmodelvertex V, nsfloat radius )
   {  ns_model_vertex( V )->radius = radius;  }


nsfloat ns_model_vertex_get_scoop( const nsmodelvertex V )
   {  return ns_model_vertex( V )->scoop;  }


void ns_model_vertex_set_scoop( nsmodelvertex V, nsfloat scoop )
   {  ns_model_vertex( V )->scoop = scoop;  }


nsfloat ns_model_vertex_get_stretch( const nsmodelvertex V )
	{  return ns_model_vertex( V )->stretch;  }


void ns_model_vertex_set_stretch( nsmodelvertex V, nsfloat stretch )
	{  ns_model_vertex( V )->stretch = stretch;  }


nsfloat ns_model_vertex_get_threshold( const nsmodelvertex V, nssize which )
   {
   ns_assert( which < NS_MODEL_VERTEX_NUM_THRESHOLDS );
   return ns_model_vertex( V )->thresholds[ which ];
   }


void ns_model_vertex_set_threshold( nsmodelvertex V, nssize which, nsfloat threshold )
   {
   ns_assert( which < NS_MODEL_VERTEX_NUM_THRESHOLDS );
   ns_model_vertex( V )->thresholds[ which ] = threshold; 
   }


nsfloat ns_model_vertex_get_contrast( const nsmodelvertex V, nssize which )
	{
	ns_assert( which < NS_MODEL_VERTEX_NUM_CONTRASTS );
	return ns_model_vertex( V )->contrasts[ which ];
	}


void ns_model_vertex_set_contrast( nsmodelvertex V, nssize which, nsfloat contrast )
	{
	ns_assert( which < NS_MODEL_VERTEX_NUM_CONTRASTS );
	ns_model_vertex( V )->contrasts[ which ] = contrast;
	}


nspointer ns_model_vertex_get_data( const nsmodelvertex V, nssize index )
   {
   ns_assert( index < NS_MODEL_VERTEX_NUM_DATA );
   return ns_model_vertex( V )->data[ index ];
   }


void ns_model_vertex_set_data( nsmodelvertex V, nssize index, nspointer data )
   {
   ns_assert( index < NS_MODEL_VERTEX_NUM_DATA );
   ns_model_vertex( V )->data[ index ] = data;
   }


NsError ns_model_vertex_add_edge
   (
   nsmodelvertex   from,
   nsmodelvertex   to,
   nsmodeledge    *FT,
   nsmodeledge    *TF
   )
   {
   NsModel      *model;
   NsModelEdge  *from_to_edge;
   NsModelEdge  *to_from_edge;
   NsError       error;


   ns_assert( ns_model_vertex_model( from ) == ns_model_vertex_model( to ) );
   model = ns_model_vertex_model( from );

   from_to_edge = NULL;
   to_from_edge = NULL;

   if( NS_FAILURE( ns_model_edge_new( &from_to_edge ), error ) ||
       NS_FAILURE( ns_model_edge_new( &to_from_edge ), error )   )
      {
      ns_model_edge_delete( from_to_edge );
      ns_model_edge_delete( to_from_edge );

      return error;
      }

   if( NS_FAILURE( ns_graph_vertex_add_undir_edge(
                     from,
                     to,
                     from_to_edge,
                     to_from_edge,
                     FT,
                     TF
                     ),
                     error ) )
      {
      ns_model_edge_delete( from_to_edge );
      ns_model_edge_delete( to_from_edge );

      return error;
      }

   return ns_no_error();
   }


void ns_model_vertex_remove_edge( nsmodelvertex V, nsmodeledge E )
   {  ns_graph_vertex_remove_edge( V, E );  }


nsfloat ns_model_edge_length( const nsmodeledge E )
   {
   NsVector3f V1, V2;

   ns_model_vertex_get_position( ns_model_edge_src_vertex( E ), &V1 );
   ns_model_vertex_get_position( ns_model_edge_dest_vertex( E ), &V2 );

   return ns_vector3f_distance( &V1, &V2 );
   }


nslong ns_model_edge_get_color( const nsmodeledge E )
   {  return ns_graph_edge_get_color( E );  }


void ns_model_edge_set_color( nsmodeledge E, nslong color, nsboolean mirrored )
   {
   ns_graph_edge_set_color( E, color );

   if( mirrored )
      ns_graph_edge_set_color( ns_model_edge_mirror( E ), color );
   }


nsushort ns_model_edge_get_order( const nsmodeledge E )
   {  return ns_model_edge( E )->order;  }


void ns_model_edge_set_order( nsmodeledge E, nsushort order, nsboolean mirrored )
   {
   ns_model_edge( E )->order = order;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->order = order;
   }


nsboolean ns_model_edge_is_order_center( const nsmodeledge E )
	{  return _NS_MODEL_EDGE_IS_BIT( E, NS_MODEL_EDGE_BIT_ORDER_CENTER );  }


void ns_model_edge_mark_order_center( nsmodeledge E, nsboolean b, nsboolean mirrored )
	{
	_NS_MODEL_EDGE_MARK_BIT( E, b, NS_MODEL_EDGE_BIT_ORDER_CENTER );

	if( mirrored )
		_NS_MODEL_EDGE_MARK_BIT( ns_model_edge_mirror( E ), b, NS_MODEL_EDGE_BIT_ORDER_CENTER );
	}


nsboolean ns_model_edge_is_section_center( const nsmodeledge E )
	{  return _NS_MODEL_EDGE_IS_BIT( E, NS_MODEL_EDGE_BIT_SECTION_CENTER );  }


void ns_model_edge_mark_section_center( nsmodeledge E, nsboolean b, nsboolean mirrored )
	{
	_NS_MODEL_EDGE_MARK_BIT( E, b, NS_MODEL_EDGE_BIT_SECTION_CENTER );

	if( mirrored )
		_NS_MODEL_EDGE_MARK_BIT( ns_model_edge_mirror( E ), b, NS_MODEL_EDGE_BIT_SECTION_CENTER );
	}


nsboolean ns_model_edge_is_unremovable( const nsmodeledge E )
	{  return _NS_MODEL_EDGE_IS_BIT( E, NS_MODEL_EDGE_BIT_UNREMOVABLE );  }


void ns_model_edge_mark_unremovable( nsmodeledge E, nsboolean b, nsboolean mirrored )
	{
	_NS_MODEL_EDGE_MARK_BIT( E, b, NS_MODEL_EDGE_BIT_UNREMOVABLE );

	if( mirrored )
		_NS_MODEL_EDGE_MARK_BIT( ns_model_edge_mirror( E ), b, NS_MODEL_EDGE_BIT_UNREMOVABLE );
	}


nsulong ns_model_edge_get_section( const nsmodeledge E )
   {  return ns_model_edge( E )->section;  }


void ns_model_edge_set_section( nsmodeledge E, nsulong section, nsboolean mirrored )
   {
   ns_model_edge( E )->section = section;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->section = section;
   }


nspointer ns_model_edge_get_data( const nsmodeledge E )
   {  return ns_model_edge( E )->data;  }


void ns_model_edge_set_data( nsmodeledge E, nspointer data, nsboolean mirrored )
   {
   ns_model_edge( E )->data = data;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->data = data;
   }


NsConicalFrustumd* ns_model_edge_to_conical_frustum( const nsmodeledge E, NsConicalFrustumd *frustum )
   {
   nsmodelvertex src_vertex, dest_vertex;

   ns_assert( NULL != frustum );

   src_vertex  = ns_model_edge_src_vertex( E );
   dest_vertex = ns_model_edge_dest_vertex( E );

   /* NOTE: The edge's shape is that of a tapered cylinder, i.e. conical frustum. */
   ns_model_vertex_get_position_ex( src_vertex, &frustum->C1 );
   frustum->r1 = ns_model_vertex_get_radius( src_vertex );

   ns_model_vertex_get_position_ex( dest_vertex, &frustum->C2 );
   frustum->r2 = ns_model_vertex_get_radius( dest_vertex );

	return frustum;
   }


nsdouble ns_model_edge_angle_between( const nsmodeledge E1, const nsmodeledge E2 )
   {
   NsConicalFrustumd  Fa, Fb;
   NsVector3d         Va, Vb;


   ns_model_edge_to_conical_frustum( E1, &Fa );
   ns_model_edge_to_conical_frustum( E2, &Fb );

   ns_vector3d_sub( &Va, &Fa.C2, &Fa.C1 );
   ns_vector3d_sub( &Vb, &Fb.C2, &Fb.C1 );

   return NS_RADIANS_TO_DEGREES( ns_vector3d_angle( &Va, &Vb ) );
   }


NsModelFunctionType ns_model_edge_get_function_type( const nsmodeledge E )
	{  return ns_model_edge( E )->func_type;  }


void ns_model_edge_set_function_type( nsmodeledge E, NsModelFunctionType func_type, nsboolean mirrored )
	{
   ns_model_edge( E )->func_type = func_type;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->func_type = func_type;
	}


nsfloat ns_model_edge_get_threshold( const nsmodeledge E, nssize which )
	{
   ns_assert( which < NS_MODEL_EDGE_NUM_THRESHOLDS );
	return ns_model_edge( E )->thresholds[ which ];
	}


void ns_model_edge_set_threshold( nsmodeledge E, nssize which, nsfloat threshold, nsboolean mirrored )
	{
	ns_assert( which < NS_MODEL_EDGE_NUM_THRESHOLDS );

   ns_model_edge( E )->thresholds[ which ] = threshold;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->thresholds[ which ] = threshold;
	}


nsfloat ns_model_edge_get_contrast( const nsmodeledge E, nssize which )
	{
	ns_assert( which < NS_MODEL_EDGE_NUM_CONTRASTS );
	return ns_model_edge( E )->contrasts[ which ];
	}


void ns_model_edge_set_contrast( nsmodeledge E, nssize which, nsfloat contrast, nsboolean mirrored )
	{
	ns_assert( which < NS_MODEL_EDGE_NUM_CONTRASTS );

   ns_model_edge( E )->contrasts[ which ] = contrast;

   if( mirrored )
      ns_model_edge( ns_model_edge_mirror( E ) )->contrasts[ which ] = contrast;
	}


NsFinalizeFunc ns_model_get_data_finalize_func( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_get_finalize_func( &model->data );
   }


void ns_model_set_data_finalize_func( NsModel *model, NsFinalizeFunc finalize_func )
   {
   ns_assert( NULL != model );
   ns_list_set_finalize_func( &model->data, finalize_func );
   }


NsError ns_model_add_data( NsModel *model, nspointer data, nsmodeldata *D )
   {
   NsError error;

   ns_assert( NULL != model );

   if( NS_FAILURE( ns_list_push_back( &model->data, data ), error ) )
      return error;

   if( NULL != D )
      *D = ns_list_rev_begin( &model->data );

   return ns_no_error();
   }


void ns_model_remove_data( NsModel *model, nsmodeldata D )
   {
   ns_assert( NULL != model );
   ns_list_erase( &model->data, D );
   }


nssize ns_model_data_size( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_size( &model->data );
   }


nsmodeldata ns_model_begin_data( NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_begin( &model->data );
   }


nsmodeldata ns_model_end_data( NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_end( &model->data );
   }


void ns_model_clear_data( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_list_clear( &model->data );
   }


nsuint8* ns_model_begin_buffer( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_byte_array_begin( &model->buffer );
   }


nsuint8* ns_model_end_buffer( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_byte_array_end( &model->buffer );
   }


nssize ns_model_buffer_size( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_byte_array_size( &model->buffer );
   }


NsError ns_model_resize_buffer( NsModel *model, nssize bytes )
   {
   ns_assert( NULL != model );
   return ns_byte_array_resize( &model->buffer, bytes );
   }


void ns_model_clear_buffer( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_byte_array_clear( &model->buffer );
   }


void ns_model_init_voxel_bounds
   (
   NsVector3f  *min,
   NsVector3f  *max
   )
   {
   ns_vector3f( min, NS_FLOAT_MAX, NS_FLOAT_MAX, NS_FLOAT_MAX );
   ns_vector3f( max, -NS_FLOAT_MAX, -NS_FLOAT_MAX, -NS_FLOAT_MAX );
   }


void ns_model_update_voxel_bounds
   (
   NsVector3f        *min,
   NsVector3f        *max,
   const NsVector3f  *V,
   nsfloat            offset
   )
   {
   if( V->x - offset < min->x )min->x = V->x - offset;
   if( V->y - offset < min->y )min->y = V->y - offset;
   if( V->z - offset < min->z )min->z = V->z - offset;
   if( V->x + offset > max->x )max->x = V->x + offset;
   if( V->y + offset > max->y )max->y = V->y + offset;
   if( V->z + offset > max->z )max->z = V->z + offset;
   }


void ns_model_voxel_bounds_to_aabbox
   (
   const NsVector3f  *min,
   const NsVector3f  *max,
   NsAABBox3d        *box
   )
   {
   ns_aabbox3d(
      box,
      ( nsdouble )min->x,
      ( nsdouble )max->y,
      ( nsdouble )max->z,
      ( nsdouble )( max->x - min->x ),
      ( nsdouble )( max->y - min->y ),
      ( nsdouble )( max->z - min->z )
      );
   }


void ns_model_voxel_bounds_to_aabbox_ex
   (
   const NsVector3i   *C1i,
   const NsVector3i   *C2i,
	const NsVoxelInfo  *voxel_info,
   NsAABBox3d         *box
   )
	{
	NsVector3f C1f, C2f;

	ns_assert( C1i->x <= C2i->x );
	ns_assert( C1i->y <= C2i->y );
	ns_assert( C1i->z <= C2i->z );

	ns_to_voxel_space( C1i, &C1f, voxel_info );
	ns_to_voxel_space( C2i, &C2f, voxel_info );

	ns_model_voxel_bounds_to_aabbox( &C1f, &C2f, box );
	}


void ns_model_image_space_drag_rect_to_aabbox
   (
   const NsDragRect   *drag_rect,
   const NsImage      *image,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
   NsAABBox3d         *B
   )
   {
   NsVector3f  min, max;
   nsint       sx, sy, ex, ey;
   //nsint       width, height, length;


   ns_assert( NULL != drag_rect );
   ns_assert( NULL != image );
   ns_assert( NULL != voxel_info );
   ns_assert( NULL != B );

   sx = ns_drag_rect_start_x( drag_rect );
   sy = ns_drag_rect_start_y( drag_rect );
   ex = ns_drag_rect_end_x( drag_rect );
   ey = ns_drag_rect_end_y( drag_rect );

   //ns_assert( ns_image_width( image )  <= ( nssize )NS_INT_MAX );
   //ns_assert( ns_image_height( image ) <= ( nssize )NS_INT_MAX );
   //ns_assert( ns_image_length( image ) <= ( nssize )NS_INT_MAX );

   //width  = ( nsint )ns_image_width( image );
   //height = ( nsint )ns_image_height( image );
   //length = ( nsint )ns_image_length( image );

   /* BUG FIX 2007-04-05: Allowing bounding box to be of "infinite" size
      in the third dimension, e.g. in Z for the XY plane.

      Note that NS_FLOAT_MAX/2.0f - ( -NS_FLOAT_MAX/2.0f ) = NS_FLOAT_MAX
      Hopefully the accuracy is maintained and we dont overflow?
   */
   switch( which )
      {
      case NS_XY:
         min.x = ns_to_voxel_space_component( sx, voxel_info, NS_COMPONENT_X );
         max.x = ns_to_voxel_space_component( ex, voxel_info, NS_COMPONENT_X );
         min.y = ns_to_voxel_space_component( sy, voxel_info, NS_COMPONENT_Y );
         max.y = ns_to_voxel_space_component( ey, voxel_info, NS_COMPONENT_Y );
         //min.z = 0.0f;
         //max.z = ns_to_voxel_space_component( length, voxel_info, NS_COMPONENT_Z );
         min.z = -NS_FLOAT_MAX / 2.0f;
         max.z = NS_FLOAT_MAX / 2.0f;
         break;

      case NS_ZY:
         //min.x = 0.0f;
         //max.x = ns_to_voxel_space_component( width, voxel_info, NS_COMPONENT_X );
         min.x = -NS_FLOAT_MAX / 2.0f;
         max.x = NS_FLOAT_MAX / 2.0f;
         min.y = ns_to_voxel_space_component( sy, voxel_info, NS_COMPONENT_Y );
         max.y = ns_to_voxel_space_component( ey, voxel_info, NS_COMPONENT_Y );
         min.z = ns_to_voxel_space_component( sx, voxel_info, NS_COMPONENT_Z );
         max.z = ns_to_voxel_space_component( ex, voxel_info, NS_COMPONENT_Z );
         break;

      case NS_XZ:
         min.x = ns_to_voxel_space_component( sx, voxel_info, NS_COMPONENT_X );
         max.x = ns_to_voxel_space_component( ex, voxel_info, NS_COMPONENT_X );
         //min.y = 0.0f;
         //max.y = ns_to_voxel_space_component( height, voxel_info, NS_COMPONENT_Y );
         min.y = -NS_FLOAT_MAX / 2.0f;
         max.y = NS_FLOAT_MAX / 2.0f;
         min.z = ns_to_voxel_space_component( sy, voxel_info, NS_COMPONENT_Z );
         max.z = ns_to_voxel_space_component( ey, voxel_info, NS_COMPONENT_Z );
         break;

      default:
         ns_assert_not_reached();
      }

   ns_model_voxel_bounds_to_aabbox( &min, &max, B );
   }


void ns_model_client_space_drag_rect_to_aabbox
   (
   const NsDragRect   *drag_rect,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
	const NsPoint2i    *corner,
	nsfloat             zoom,
   NsAABBox3d         *B
   )
	{
	NsPoint2i   S, E;
   NsVector3f  min, max;


   ns_assert( NULL != drag_rect );
   ns_assert( NULL != voxel_info );
   ns_assert( NULL != B );

   S.x = ns_drag_rect_start_x( drag_rect );
   S.y = ns_drag_rect_start_y( drag_rect );
   E.x = ns_drag_rect_end_x( drag_rect );
   E.y = ns_drag_rect_end_y( drag_rect );

	ns_client_to_voxel_space( &S, &min, voxel_info, which, corner, zoom );
	ns_client_to_voxel_space( &E, &max, voxel_info, which, corner, zoom );

   switch( which )
      {
      case NS_XY:
         min.z = -NS_FLOAT_MAX / 2.0f;
         max.z = NS_FLOAT_MAX / 2.0f;
         break;

      case NS_ZY:
         min.x = -NS_FLOAT_MAX / 2.0f;
         max.x = NS_FLOAT_MAX / 2.0f;
         break;

      case NS_XZ:
         min.y = -NS_FLOAT_MAX / 2.0f;
         max.y = NS_FLOAT_MAX / 2.0f;
         break;

      default:
         ns_assert_not_reached();
      }

   ns_model_voxel_bounds_to_aabbox( &min, &max, B );

//ns_println( "B = %f,%f,%f  %f,%f,%f", B->O.x, B->O.y, B->O.z, B->width, B->height, B->length );
	}


NsAABBox3d* ns_model_position_and_radius_to_aabbox
   (
   const NsVector3f  *Pf,
   nsfloat            radius,
   NsAABBox3d        *B
   )
   {
   NsVector3d Pd;

   return ns_model_position_and_radius_to_aabbox_ex(
            ns_vector3f_to_3d( Pf, &Pd ),
            ( nsdouble )radius,
            B
            );
   }


NsAABBox3d* ns_model_position_and_radii_to_aabbox
   (
   const NsVector3f  *P,
   const NsVector3f  *R,
   NsAABBox3d        *B
   )
   {
   ns_aabbox3d(
      B,
      P->x - R->x,
      P->y + R->y,
      P->z + R->z,
      R->x * 2.0,
      R->y * 2.0,
      R->z * 2.0
      );

   return B;
   }


NsAABBox3d* ns_model_position_and_radius_to_aabbox_ex
   (
   const NsVector3d  *P,
   nsdouble           radius,
   NsAABBox3d        *B
   )
   {
   ns_aabbox3d(
      B,
      P->x - radius,
      P->y + radius,
      P->z + radius,
      radius * 2.0,
      radius * 2.0,
      radius * 2.0
      );

   return B;
   }


NsAABBox3d* ns_model_vertex_to_aabbox_ex( const nsmodelvertex V, NsAABBox3d *B )
   {
   NsVector3d P;

   return ns_model_position_and_radius_to_aabbox_ex(
            ns_model_vertex_get_position_ex( V, &P ),
            ( nsdouble )ns_model_vertex_get_radius( V ),
            B
            );
   }


NS_PRIVATE void _ns_model_calc_aabbox( NsModel *model, NsAABBox3d *box )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   NsVector3f     min;
   NsVector3f     max;
   NsVector3f     V;
   nsfloat        radius;
   NsVector3d     origin;
   nsdouble       width;
   nsdouble       height;
   nsdouble       length;
   nsdouble       max_dim;


   ns_assert( NULL != model );
   ns_assert( NULL != box );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   /* No vertices, just return. Unlikely but could happen. */
   if( ns_model_vertex_equal( curr_vertex, end_vertices ) )
      return;

   ns_model_init_voxel_bounds( &min, &max );

   /* Get the bounding box for the model, which is not
      necessarily a cube. */
   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
         curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      ns_model_vertex_get_position( curr_vertex, &V );
      radius = ns_model_vertex_get_radius( curr_vertex );

      ns_model_update_voxel_bounds( &min, &max, &V, radius );
      }

   ns_model_voxel_bounds_to_aabbox( &min, &max, box );

   /* Convert the bounding box to a true cube by finding
      the maximum dimension and offsetting the origin. */
   ns_aabbox3d_origin( box, &origin );

   width  = ns_aabbox3d_width( box );
   height = ns_aabbox3d_height( box );
   length = ns_aabbox3d_length( box );

   max_dim = NS_MAX3( width, height, length );

   ns_aabbox3d(
      box,
      origin.x - ( max_dim - width  ) / 2.0,
      origin.y + ( max_dim - height ) / 2.0,
      origin.z + ( max_dim - length ) / 2.0,
      max_dim,
      max_dim,
      max_dim
      );
   }


/* The octree stores model edge iterators. */
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsmodeledge, nspointer );


NsError ns_model_build_edge_octree_ex
   (
   NsModel                *model,
   nssize                  min_node_objects,
   nssize                  max_recursion,
   nssize                  init_node_capacity,
   NsOctreeIntersectFunc   intersect_func,
   nsdouble                aabbox_increment,
	nsdouble                min_aabbox_size,
   NsProgress             *progress
   )
   {
   nsmodelvertex  curr_vertex, end_vertices;
   nsmodeledge    curr_edge, end_edges;
   nsint          which;
   nsboolean      was_added;
   NsError        error;


   which = NS_MODEL_EDGE_OCTREE;

   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );
   ns_assert( NULL != intersect_func );

   if( 0 == ns_model_num_vertices( model ) )
      return ns_no_error();

   ns_octree_clear( model->octrees + which );

   _ns_model_calc_aabbox( model, model->boxes + which );
   ns_aabbox3d_cmpd_xflate( model->boxes + which, aabbox_increment );

   ns_octree_set_root_aabbox( model->octrees + which, model->boxes + which );
   ns_octree_set_min_node_objects( model->octrees + which, min_node_objects );
   ns_octree_set_max_recursion( model->octrees + which, max_recursion );
   ns_octree_set_init_node_capacity( model->octrees + which, init_node_capacity );
	ns_octree_set_min_aabbox_size( model->octrees + which, min_aabbox_size );
   ns_octree_set_intersect_func( model->octrees + which, intersect_func );

   ns_progress_num_iters( progress, ns_model_num_edges( model ) );
   ns_progress_begin( progress );

   ns_model_color_edges( model, NS_MODEL_EDGE_UNSEEN );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         {
			if( NS_MODEL_EDGE_UNSEEN == ns_model_edge_get_color( curr_edge ) )
				{
				ns_model_edge_set_color( curr_edge, NS_MODEL_EDGE_SEEN, NS_TRUE );

				if( NS_FAILURE( ns_octree_add( model->octrees + which, curr_edge, &was_added ), error ) )
					return error;

				ns_assert( was_added );
				}

         ns_progress_next_iter( progress );
         }
      }

   ns_progress_end( progress );
   return ns_no_error();
   }


NsError ns_model_build_vertex_octree_ex
   (
   NsModel                *model,
   nssize                  min_node_objects,
   nssize                  max_recursion,
   nssize                  init_node_capacity,
   NsOctreeIntersectFunc   intersect_func,
   nsdouble                aabbox_increment,
   NsProgress             *progress
   )
   {
   nsmodelvertex  curr_vertex, end_vertices;
   nsint          which;
   nsboolean      was_added;
   NsError        error;


   which = NS_MODEL_VERTEX_OCTREE;

   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );
   ns_assert( NULL != intersect_func );

   if( 0 == ns_model_num_vertices( model ) )
      return ns_no_error();

   ns_octree_clear( model->octrees + which );

   _ns_model_calc_aabbox( model, model->boxes + which );
   ns_aabbox3d_cmpd_xflate( model->boxes + which, aabbox_increment );

   ns_octree_set_root_aabbox( model->octrees + which, model->boxes + which );
   ns_octree_set_min_node_objects( model->octrees + which, min_node_objects );
   ns_octree_set_max_recursion( model->octrees + which, max_recursion );
   ns_octree_set_init_node_capacity( model->octrees + which, init_node_capacity );
   ns_octree_set_intersect_func( model->octrees + which, intersect_func );

   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
   ns_progress_begin( progress );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      if( NS_FAILURE( ns_octree_add( model->octrees + which, curr_vertex, &was_added ), error ) )
         return error;

      ns_assert( was_added );

      ns_progress_next_iter( progress );
      }

   ns_progress_end( progress );
   return ns_no_error();
   }


NS_PRIVATE nsboolean _ns_model_edge_octree_intersect_func
   (
   nsconstpointer     object,
   const NsAABBox3d  *box
   )
   {
	nsmodeledge         edge;
	const NsModel      *model;
   NsConicalFrustumd   cf;
   NsVector3d          N;
   NsVector3d          C;
   nsdouble            radius;
   nsdouble            length;
   nsdouble            d;
   nsdouble            u;
   NsSphered           sphere;
   nsboolean           done;


	edge  = ( nsmodeledge )object;
	model = ns_model_vertex_model( ns_model_edge_src_vertex( edge ) );

	ns_assert( NULL != model );

	if( NULL != model->edge_filter_func )
		if( ! ( model->edge_filter_func )( edge ) )
			return NS_FALSE;

   ns_model_edge_to_conical_frustum( edge, &cf );

   /* Get direction vector between the two endpoints. */
   ns_vector3d_sub( &N, &cf.C2, &cf.C1 );
   length = ns_vector3d_mag( &N );

   if( length <= NS_FLOAT_ALMOST_ZERO )
      {
      /* If the frustum has no length, just take the larger of
         the spheres and use as the intersection test. */
      if( cf.r1 > cf.r2 )
         ns_sphered( &sphere, cf.C1.x, cf.C1.y, cf.C1.z, cf.r1 );
      else
         ns_sphered( &sphere, cf.C2.x, cf.C2.y, cf.C2.z, cf.r2 );

      if( ns_sphered_intersects_aabbox( &sphere, box ) )
         return NS_TRUE;
      }
   else
      {
      ns_vector3d_norm( &N );

      /* Check intersection between spheres, which approximate the
         conical frustum, and the bounding box since there is no easy
         intersection test for a conical frustum and axis-aligned
         bounding box. */

      done = NS_FALSE;
      d    = 0.0;

      while( ! done )
         {
         /* Create a sphere by interpolating the current coordinate and radius. */
         ns_vector3d_scale( &C, &N, d );
         ns_vector3d_cmpd_add( &C, &cf.C1 );

         u = d / length;

         if( 1.0 <= u )
            {
            u    = 1.0;
            done = NS_TRUE;
            }

         /* If u = 0, then radius == radius1
            If u = 1, then radius == radius2 */
         radius = ( 1.0 - u ) * cf.r1 + u * cf.r2;

         /* Increase radius by 5% to account for "space" between spheres. */
         ns_sphered( &sphere, C.x, C.y, C.z, radius + radius * .05 );

         if( ns_sphered_intersects_aabbox( &sphere, box ) )
            return NS_TRUE;

			/* The 'd' variable wont advance if the radius goes to zero and
				we will end up in an infinite loop. */
			if( NS_FLOAT_EQUAL( radius, 0.0f ) )
				return NS_FALSE;

         /* Move half the radius of the previous sphere. */
         d += radius / 2.0;
         }
      }

   return NS_FALSE;
   }


NsError ns_model_build_edge_octree
   (
	NsModel           *model,
	const NsAABBox3d  *bbox,
	nssize             min_node_objects,
	nssize             max_recursion,
	nssize             init_node_capacity,
	nsboolean          ( *filter_func )( nsmodeledge ),
	NsProgress        *progress
   )
	{
   nsmodelvertex  curr_vertex, end_vertices;
   nsmodeledge    curr_edge, end_edges;
   nsint          which;
   nsboolean      was_added;
   NsError        error;


   which = NS_MODEL_EDGE_OCTREE;

   ns_assert( NULL != model );
	ns_assert( NULL != bbox );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   ns_octree_clear( model->octrees + which );

	model->edge_filter_func = filter_func;

	model->boxes[ which ] = *bbox;

   ns_octree_set_root_aabbox( model->octrees + which, model->boxes + which );
   ns_octree_set_min_node_objects( model->octrees + which, min_node_objects );
   ns_octree_set_max_recursion( model->octrees + which, max_recursion );
   ns_octree_set_init_node_capacity( model->octrees + which, init_node_capacity );
   ns_octree_set_intersect_func( model->octrees + which, _ns_model_edge_octree_intersect_func );

   ns_progress_num_iters( progress, ns_model_num_edges( model ) );
   ns_progress_begin( progress );

   ns_model_color_edges( model, NS_MODEL_EDGE_UNSEEN );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         {
			if( NS_MODEL_EDGE_UNSEEN == ns_model_edge_get_color( curr_edge ) )
				{
				ns_model_edge_set_color( curr_edge, NS_MODEL_EDGE_SEEN, NS_TRUE );

				if( NS_FAILURE( ns_octree_add( model->octrees + which, curr_edge, &was_added ), error ) )
					return error;
				}

         ns_progress_next_iter( progress );
         }
      }

   ns_progress_end( progress );
   return ns_no_error();
	}


NS_PRIVATE nsboolean _ns_model_vertex_octree_intersect_func
   (
   nsconstpointer     object,
   const NsAABBox3d  *box
   )
	{
	nsmodelvertex   vertex;
	const NsModel  *model;
   NsSphered       sphere;


	vertex = ( nsmodelvertex )object;
	model  = ns_model_vertex_model( vertex );

	ns_assert( NULL != model );

	if( NULL != model->vertex_filter_func )
		if( ! ( model->vertex_filter_func )( vertex ) )
			return NS_FALSE;

	return ns_sphered_intersects_aabbox(
				ns_model_vertex_to_sphere_ex( vertex, &sphere ),
				box
				);
	}


NsError ns_model_build_vertex_octree
   (
	NsModel           *model,
	const NsAABBox3d  *bbox,
	nssize             min_node_objects,
	nssize             max_recursion,
	nssize             init_node_capacity,
	nsboolean          ( *filter_func )( nsmodelvertex ),
	NsProgress        *progress
   )
	{
   nsmodelvertex  curr_vertex, end_vertices;
   nsint          which;
   nsboolean      was_added;
   NsError        error;


   which = NS_MODEL_VERTEX_OCTREE;

   ns_assert( NULL != model );
	ns_assert( NULL != bbox );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   ns_octree_clear( model->octrees + which );

	model->vertex_filter_func = filter_func;

	model->boxes[ which ] = *bbox;

   ns_octree_set_root_aabbox( model->octrees + which, model->boxes + which );
   ns_octree_set_min_node_objects( model->octrees + which, min_node_objects );
   ns_octree_set_max_recursion( model->octrees + which, max_recursion );
   ns_octree_set_init_node_capacity( model->octrees + which, init_node_capacity );
   ns_octree_set_intersect_func( model->octrees + which, _ns_model_vertex_octree_intersect_func );

   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
   ns_progress_begin( progress );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      if( NS_FAILURE( ns_octree_add( model->octrees + which, curr_vertex, &was_added ), error ) )
         return error;

      ns_progress_next_iter( progress );
      }

   ns_progress_end( progress );
   return ns_no_error();
	}


void ns_model_clear_octree( NsModel *model, nsint which )
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   ns_octree_clear( model->octrees + which );
   }


void ns_model_clear_octrees( NsModel *model )
	{
	ns_model_clear_octree( model, NS_MODEL_EDGE_OCTREE );
	ns_model_clear_octree( model, NS_MODEL_VERTEX_OCTREE );
	}


nssize ns_model_octree_size( const NsModel *model, nsint which )
	{
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

	return ns_octree_num_objects( model->octrees + which );
	}


nsboolean ns_model_octree_is_empty( const NsModel *model, nsint which )
	{
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

	return NULL == ns_octree_root( model->octrees + which );
	}


NsError ns_model_octree_add( NsModel *model, nsint which, nspointer object )
	{
	nsboolean was_added;

   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

	return ns_octree_add( model->octrees + which, object, &was_added );
	}


NS_PRIVATE void _ns_model_octree_do_calc_thresholds_and_contrasts
	(
	const NsOctreeNode  *node,
	const NsSettings    *settings,
	nsboolean            include_edges
	)
   {
   NsValue       *value;
   nsint          octant;
   nsvectoriter   curr;
   nsvectoriter   end;
   nsmodeledge    edge;
   nsfloat        threshold;
	nsfloat        contrast;
	nsfloat        multiplier;


   if( NULL != node )
      {
      if( ns_octree_node_is_leaf( node ) )
         {
         if( 0 == ns_octree_node_num_objects( node ) )
            return;

			multiplier = ns_settings_get_threshold_multiplier( settings );

			value = ns_octree_node_value( node, NS_MODEL_OCTREE_THRESHOLD );

         if( ! ns_value_is( value, NS_VALUE_FLOAT ) )
            ns_value_set_float( value, NS_FLOAT_MAX );

         /* Find the vertex in the octree node with the lowest SCALED threshold. */

         curr = ns_octree_node_begin_objects( node );
         end  = ns_octree_node_end_objects( node );

         for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
            {
            edge = ( nsmodeledge )ns_vector_iter_get_object( curr );

            threshold = ns_model_vertex_get_threshold(
									ns_model_edge_src_vertex( edge ),
									NS_MODEL_VERTEX_NEURITE_THRESHOLD
									);

            contrast = ns_model_vertex_get_contrast(
									ns_model_edge_src_vertex( edge ),
									NS_MODEL_VERTEX_NEURITE_CONTRAST
									);

				threshold = ns_model_scale_threshold( threshold, contrast, multiplier );

            if( threshold < ns_value_get_float( value ) )
               ns_value_set_float( value, threshold );

            threshold = ns_model_vertex_get_threshold(
									ns_model_edge_dest_vertex( edge ),
									NS_MODEL_VERTEX_NEURITE_THRESHOLD
									);

            contrast = ns_model_vertex_get_contrast(
									ns_model_edge_dest_vertex( edge ),
									NS_MODEL_VERTEX_NEURITE_CONTRAST
									);

				threshold = ns_model_scale_threshold( threshold, contrast, multiplier );

            if( threshold < ns_value_get_float( value ) )
               ns_value_set_float( value, threshold );

				if( include_edges )
					{
					threshold = ns_model_edge_get_threshold( edge, NS_MODEL_EDGE_NEURITE_THRESHOLD );
					contrast  = ns_model_edge_get_contrast( edge, NS_MODEL_EDGE_NEURITE_CONTRAST );
					threshold = ns_model_scale_threshold( threshold, contrast, multiplier );

					if( threshold < ns_value_get_float( value ) )
						ns_value_set_float( value, threshold );
					}
            }
         }
      else
         for( octant = 0; octant < 8; ++octant )
            _ns_model_octree_do_calc_thresholds_and_contrasts(
					ns_octree_node_child( node, octant ),
					settings,
					include_edges
					);
      }
   }


void ns_model_octree_calc_thresholds_and_contrasts
	(
	NsModel           *model,
	const NsSettings  *settings,
	nsint              which,
	nsboolean          include_edges
	)
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   _ns_model_octree_do_calc_thresholds_and_contrasts(
		ns_octree_root( model->octrees + which ),
		settings,
		include_edges
		);
   }


const NsOctreeNode* ns_model_octree_root( const NsModel *model, nsint which )
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   return ns_octree_root( model->octrees + which );
   }


NsError ns_model_octree_intersections
   (
   const NsModel          *model,
   nsint                   which,
   nspointer               object,
   NsOctreeIntersectFunc   new_intersect_func,
   NsVector               *nodes
   )
   {
   NsOctreeIntersectFunc  old_intersect_func;
   NsError                error;


   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );
   ns_assert( NULL != nodes );

   old_intersect_func = ns_octree_get_intersect_func( model->octrees + which );
   ns_octree_set_intersect_func( ( NsOctree* )model->octrees + which, new_intersect_func );

   error = ns_octree_intersections( model->octrees + which, object, nodes );

   ns_octree_set_intersect_func( ( NsOctree* )model->octrees + which, old_intersect_func );

   return error;
   }


NS_PRIVATE void _ns_model_do_render_octree
   (
   const NsOctreeNode  *node,
   void                 ( *render_func )( const NsOctreeNode*, nspointer ),
	void                 ( *set_color )( nsfloat r, nsfloat g, nsfloat b, nspointer ),
   nspointer            user_data
   )
   {
   nsint octant;

   if( NULL != node )
      {
      if( ns_octree_node_is_leaf( node ) )
			{
			if( 0 == ns_octree_node_num_objects( node ) )
				( set_color )( 0.0f, 0.0f, 0.1f, user_data );
			else
				( set_color )( 1.0f, 0.0f, 0.0f, user_data );

         ( render_func )( node, user_data );
			}
      else
         for( octant = 0; octant < 8; ++octant )
            _ns_model_do_render_octree(
               ns_octree_node_child( node, octant ),
               render_func,
					set_color,
               user_data
               );
      }
   }


void ns_model_render_octree
   (
   const NsModel  *model,
   nsint           which,
   void            ( *render_func )( const NsOctreeNode*, nspointer ),
	void            ( *set_color )( nsfloat r, nsfloat g, nsfloat b, nspointer ),
   nspointer       user_data
   )
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );
   ns_assert( NULL != render_func );

   _ns_model_do_render_octree(
      ns_octree_root( model->octrees + which ),
      render_func,
		set_color,
      user_data
      );
   }


void ns_model_octree_stats( const NsModel *model, nsint which, NsOctreeStats *stats )
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );
   ns_assert( NULL != stats );

   ns_octree_stats( model->octrees + which, stats );
   }


const NsAABBox3d* ns_model_octree_aabbox( const NsModel *model, nsint which )
   {
   ns_assert( NULL != model );
   ns_assert( which < NS_MODEL_NUM_OCTREES );

   return model->boxes + which;
   }


void ns_model_vertex_get( const nsmodelvertex V, NsModelVertex *vertex )
   {
   ns_assert( NULL != vertex );
   ns_model_vertex_assign( vertex, ns_model_vertex( V ) );
   }


void ns_model_vertex_set( nsmodelvertex V, const NsModelVertex *vertex )
   {
   ns_assert( NULL != vertex );
   ns_model_vertex_assign( ns_model_vertex( V ), vertex );
   }


void ns_model_edge_get( const nsmodeledge E, NsModelEdge *edge )
   {
   ns_assert( NULL != edge );
   ns_model_edge_assign( edge, ns_model_edge( E ) );
   }


void ns_model_edge_set( nsmodeledge E, const NsModelEdge *edge )
   {
   ns_assert( NULL != edge );
   ns_model_edge_assign( ns_model_edge( E ), edge );
   }


void ns_model_info
	(
	const NsModel  *model,
	NsModelInfo    *info,
	nsuint          flags
	)
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	nsmodeltree    curr_tree;
	nsmodeltree    end_trees;
	NsError        error;


   ns_assert( NULL != model );
   ns_assert( NULL != info );

   ns_memzero( info, sizeof( NsModelInfo ) );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
		if( ( nsboolean )( flags & NS_MODEL_INFO_VERTICES ) )
			{
			++(info->num_vertices);

			if( ns_model_vertex_is_origin( curr_vertex ) )
				++(info->num_origin_vertices);

			if( ns_model_vertex_is_external( curr_vertex ) )
				++(info->num_external_vertices);

			if( ns_model_vertex_is_junction( curr_vertex ) )
				++(info->num_junction_vertices);

			if( ns_model_vertex_is_line( curr_vertex ) )
				++(info->num_line_vertices);

			switch( ns_model_vertex_get_function_type( curr_vertex ) )
				{
				case NS_MODEL_FUNCTION_UNKNOWN:
					++(info->num_unknown_vertices);
					break;

				case NS_MODEL_FUNCTION_SOMA:
					++(info->num_soma_vertices);
					break;

				case NS_MODEL_FUNCTION_BASAL_DENDRITE:
					++(info->num_basal_vertices);
					break;

				case NS_MODEL_FUNCTION_APICAL_DENDRITE:
					++(info->num_apical_vertices);
					break;

				case NS_MODEL_FUNCTION_AXON:
					++(info->num_axon_vertices);
					break;

				default:
					ns_assert_not_reached();
				}
			}

		if( ( nsboolean )( flags & NS_MODEL_INFO_EDGES ) )
			{
			curr_edge = ns_model_vertex_begin_edges( curr_vertex );
			end_edges = ns_model_vertex_end_edges( curr_vertex );

			for( ; ns_model_edge_not_equal( curr_edge, end_edges );
					 curr_edge = ns_model_edge_next( curr_edge ) )
				{
				++(info->num_edges);
				}
			}
      }

   /* Undirected graph so divide the number of edges by 2. */
   info->num_edges /= 2;

	if( ( nsboolean )( flags & NS_MODEL_INFO_TREES ) )
		if( NS_SUCCESS( ns_model_create_trees( ( NsModel* )model ), error ) )
			{
			curr_tree = ns_model_begin_trees( model );
			end_trees = ns_model_end_trees( model );

			for( ; ns_model_tree_not_equal( curr_tree, end_trees );
					 curr_tree = ns_model_tree_next( curr_tree ) )
				{
				++(info->num_trees);

				switch( ns_model_tree_function_type( curr_tree ) )
					{
					case NS_MODEL_FUNCTION_UNKNOWN:
						++(info->num_unknown_trees);
						break;

					case NS_MODEL_FUNCTION_BASAL_DENDRITE:
						++(info->num_basal_trees);
						break;

					case NS_MODEL_FUNCTION_APICAL_DENDRITE:
						++(info->num_apical_trees);
						break;

					case NS_MODEL_FUNCTION_AXON:
						++(info->num_axon_trees);
						break;

					case NS_MODEL_FUNCTION_INVALID:
						++(info->num_invalid_trees);
						break;

					default:
						ns_assert_not_reached();
					}
				}
			}
   }


NS_PRIVATE nsdouble _ns_model_vertex_get_avg_angle_to_z_axis( nsmodelvertex vertex )
	{
	nsmodeledge  curr, end;
	nsdouble     sum_angles, angle;
	nssize       count;
	NsVector3d   P1, P2, V, Z;


	sum_angles = 0.0;
	count      = 0;

	ns_vector3d( &Z, 0.0, 0.0, 1.0 );
	ns_model_vertex_get_position_ex( vertex, &P1 );

	curr = ns_model_vertex_begin_edges( vertex );
	end  = ns_model_vertex_end_edges( vertex );

	for( ; ns_model_edge_not_equal( curr, end ); curr = ns_model_edge_next( curr ) )
		{
		ns_model_vertex_get_position_ex( ns_model_edge_dest_vertex( curr ), &P2 );

		ns_vector3d_sub( &V, &P2, &P1 );

		angle = ns_vector3d_angle( &V, &Z );

		/* Account for floating point error? */
		if( angle < 0.0 )
			angle = 0.0;
		else if( NS_PI_D < angle )
			angle = NS_PI_D;

		/* Make sure the angle is between 0 and 90. */
		if( angle > NS_PI_D / 2.0 )
			angle = NS_PI_D - angle;

		sum_angles += angle;
		++count;
		}

	return 0 < count ? ( sum_angles / ( nsdouble )count ) : NS_PI_D / 2.0;
	}


NsError ns_model_compute_z_spread_divisor
	(
	NsModel           *model,
	const NsSettings  *settings,
	const NsImage     *image,
	const NsCubei     *roi,
	nsboolean          use_2d_sampling,
	nsfloat            aabbox_scalar,
	nsint              min_window,
	nsfloat           *z_spread_divisor,
	NsProgress        *progress
	)
	{
	NsRayburst     rayburst;
	nsmodelvertex  curr, end;
	nsdouble       stretch;
	nsdouble       average;
	nsdouble       half_the_average;
	nsdouble       sum;
	nsdouble       angle;
	nssize         count;
	NsVector3f     P;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != settings );
	ns_assert( NULL != image );
	ns_assert( NULL != roi );
	ns_assert( NULL != z_spread_divisor );

	*z_spread_divisor = NS_SPINE_Z_SPREAD_DIVISOR_DEFAULT;

	if( NS_FAILURE(
			ns_grafting_establish_thresholds_and_contrasts( 
				settings,
				image,
				roi,
				use_2d_sampling,
				aabbox_scalar,
				min_window,
				model,
				progress
				),
			error ) )
		return error;

   if( NS_FAILURE(
			ns_rayburst_construct(
				&rayburst,
				NS_RAYBURST_KERNEL_SINGLE_3D,
				NS_RAYBURST_RADIUS_MLBD,
				NS_RAYBURST_INTERP_BILINEAR,
				image,
				ns_settings_voxel_info( settings ),
				NULL
				),
				error ) )
      return error;

   ns_progress_set_title( progress, "Computing z-axis spread divisor..." );
   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
	ns_progress_begin( progress );

	/* Ray travels parallel with the Z-axis. */
	ns_rayburst_init_single_ex( &rayburst, 0.0f, 0.0f, 1.0f );

	/* 1) Compute the stretch for each vertex in the model. */
	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

/* TEMP */
//ns_println( "\nmodel rayburst" );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		ns_rayburst_set_threshold(
			&rayburst,
			ns_model_vertex_get_threshold( curr, NS_MODEL_VERTEX_NEURITE_THRESHOLD )
			);

		ns_rayburst( &rayburst, ns_model_vertex_get_position( curr, &P ) );

/* TEMP */
//ns_println( "%f %f", ns_model_vertex_get_radius( curr )*2.0f, ns_rayburst_radius( &rayburst )*2.0f );

		angle   = _ns_model_vertex_get_avg_angle_to_z_axis( curr );
		stretch = ( nsdouble )( ( ns_rayburst_radius( &rayburst ) * 2.0f ) / ( ns_model_vertex_get_radius( curr ) * 2.0f ) );

		stretch *= ns_sin( angle );

		ns_model_vertex_set_stretch( curr, ( nsfloat )stretch );

		/* This allows 2D spine detection as well as compensating for tiny rayburst values, or
			if the data is really flat. */
		if( ns_model_vertex_get_stretch( curr ) < 1.0f )
			ns_model_vertex_set_stretch( curr, 1.0f );

      ns_progress_next_iter( progress );
		}

   ns_progress_end( progress );

	/* 2) Find the average of those stretch values. */
	curr  = ns_model_begin_vertices( model );
	end   = ns_model_end_vertices( model );
	sum   = 0.0;
	count = 0;

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		sum += ( nsdouble )ns_model_vertex_get_stretch( curr );
		++count;
		}

	if( 0 == count )
		goto _NS_MODEL_COMPUTE_Z_SPREAD_DIVISOR_EXIT;

	average          = sum / ( nsdouble )count;
	half_the_average = average / 2.0;

/*TEMP*/ns_println( "The average z-spread-divisor is %f (count=" NS_FMT_ULONG ")", average, count );

	/* 3) Find the average of those stretch values that are NOT above the average
			and that are NOT below half the average. i.e. remove those values that
			are outside the norm. */
	curr  = ns_model_begin_vertices( model );
	end   = ns_model_end_vertices( model );
	sum   = 0.0;
	count = 0;

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		stretch = ( nsdouble )ns_model_vertex_get_stretch( curr );

		if( half_the_average <= stretch && stretch <= average )
			{
			sum += stretch;
			++count;
			}
		}

	if( 0 == count )
		goto _NS_MODEL_COMPUTE_Z_SPREAD_DIVISOR_EXIT;

	*z_spread_divisor = ( nsfloat )( sum / ( nsdouble )count );

/*TEMP*/ns_println( "The z-spread-divisor is %f (count=" NS_FMT_ULONG ")", *z_spread_divisor, count );
	
	_NS_MODEL_COMPUTE_Z_SPREAD_DIVISOR_EXIT:

	ns_rayburst_destruct( &rayburst );
	return ns_no_error();
	}


nsboolean ns_model_vertex_intersects_ray
   (
   const nsmodelvertex   V,
   const NsRay3f        *R
   )
   {
   NsSpheref   S;
   NsVector3f  I;


   ns_assert( NULL != R );

   return ns_ray3f_intersects_sphere(
            R,
            ns_model_vertex_to_sphere( V, &S ),
            &I
            );
   }


nsboolean ns_model_vertices_intersect_ray
   (
   const NsModel  *model,
   const NsRay3f  *R,
   nsmodelvertex  *which
   )
   {
   nsmodelvertex  curr, end, closest;
   nsdouble       min_distance, distance;
   NsVector3f     V;


   ns_assert( NULL != model );
   ns_assert( NULL != R );

   closest      = NS_MODEL_VERTEX_NIL;
   min_distance = NS_DOUBLE_MAX;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_intersects_ray( curr, R ) )
         {
         /* Only select the closest one! */
         distance = ns_vector3f_distance(
                     &R->O,
                     ns_model_vertex_get_position( curr, &V )
                     );

         if( distance < min_distance )
            {
            min_distance = distance;
            closest      = curr;
            }
         }

   if( NULL != which )
      *which = closest;

   return NS_MODEL_VERTEX_NIL != closest;
   }


NS_PRIVATE void _ns_model_vertices_render_as_spheres_begin( NsRenderState *state )
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		CirclesBegin( state->constants.polygon_complexity, state->constants.line_size );
	}


NS_PRIVATE void _ns_model_vertices_render_as_spheres_end( NsRenderState *state )
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		CirclesEnd();
	}


NS_PRIVATE void _ns_model_vertex_render_as_sphere
	(
	NsRenderState  *state,
	nsmodelvertex   vertex,
	nsfloat         radius
	)
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		{
		NsVector3d Pd;

		ns_model_vertex_get_position_ex( vertex, &Pd );
		CircleRender( &Pd, radius, ( const nsuchar* )( &( state->variables.color ) ) );
		}
	else
		{
		NsVector3f Pf;

		ns_model_vertex_get_position( vertex, &Pf );
		NS_RENDER( state, sphere_func )( &Pf, radius, state );
		}
	}


NS_PRIVATE void _ns_model_do_render_vertices_spheres_recursive
	( 
	nsmodelvertex   parent_vertex,
	nsmodelvertex   curr_vertex,
	NsRenderState  *state,
	nsint           edge_color_scheme
	)
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;


	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_do_render_vertices_spheres_recursive( curr_vertex, dest_vertex, state, edge_color_scheme );
		else if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_edge_color_ex( state, edge_color_scheme, curr_edge );
			_ns_model_vertex_render_as_sphere( state, curr_vertex, ns_model_vertex_get_radius( curr_vertex ) );
			}
		}
	}


NS_PRIVATE void _ns_model_render_vertices_spheres_recursive
	(
   const NsModel  *model,
   NsRenderState  *state,
	nsint           edge_color_scheme
	)
	{
	nsmodelorigin  curr_origin;
	nsmodelorigin  end_origins;
	nsmodelvertex  vertex;


	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		vertex = ns_model_origin_vertex( curr_origin );

		if( ! ns_model_vertex_is_hidden( vertex ) || state->constants.draw_hidden_objects )
			{
			state->variables.color = NS_COLOR4UB_BLACK;
			_ns_model_vertex_render_as_sphere( state, vertex, ns_model_vertex_get_radius( vertex ) );
			}

		_ns_model_do_render_vertices_spheres_recursive(
			NS_MODEL_VERTEX_NIL,
			vertex,
			state,
			edge_color_scheme
			);
		}
	}


void ns_model_render_vertices_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, sphere_func ) );

   state->variables.shape = NS_RENDER_SHAPE_SPHERE;
   NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_vertices_render_as_spheres_begin( state );

	if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_ORDER )
		_ns_model_render_vertices_spheres_recursive( model, state, state->constants.EDGE_COLOR_BY_ORDER );
	else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_SECTION )
		_ns_model_render_vertices_spheres_recursive( model, state, state->constants.EDGE_COLOR_BY_SECTION );
	else
		{
		curr_vertex  = ns_model_begin_vertices( model );
		end_vertices = ns_model_end_vertices( model );

		for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
				 curr_vertex = ns_model_vertex_next( curr_vertex ) )
			if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
				{
				state->variables.color = ns_render_state_vertex_color( state, curr_vertex );
				_ns_model_vertex_render_as_sphere( state, curr_vertex, ns_model_vertex_get_radius( curr_vertex ) );
				}
		}

	_ns_model_vertices_render_as_spheres_end( state );

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );

	/* Draw points in 2D so that the vertices are visible no matter
		what the zoom factor. */
	if( 2 == state->constants.dimensions )
		ns_model_render_vertices_points( model, state );
   }


void ns_model_render_vertices_fixed_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
	nssize         auto_iter;
	NsVector3f     V;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, sphere_func ) );

   state->variables.shape = NS_RENDER_SHAPE_SPHERE;
	state->variables.color = state->constants.vertex_single_color;

	auto_iter = ns_model_auto_iter( model );

   NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_vertices_render_as_spheres_begin( state );
	
	curr_vertex  = ns_model_begin_vertices( model );
	end_vertices = ns_model_end_vertices( model );

	for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		if( ns_model_vertex_get_auto_iter( curr_vertex ) == auto_iter )
			_ns_model_vertex_render_as_sphere( state, curr_vertex, NS_MODEL_VERTEX_FIXED_RADIUS );

	_ns_model_vertices_render_as_spheres_end( state );

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );

	/* Draw points in 2D so that the vertices are visible no matter
		what the zoom factor. */
	if( 2 == state->constants.dimensions )
		{
		NsVector3b N;

		ns_assert( NULL != NS_RENDER( state, point_func ) );

		/* Just create a "dummy" normal for the vertices. */
		N.x = 1; N.y = 0; N.z = 0;

		state->variables.shape = NS_RENDER_SHAPE_POINT;
		NS_RENDER_PRE_OR_POST( state, pre_points_func );

		curr_vertex  = ns_model_begin_vertices( model );
		end_vertices = ns_model_end_vertices( model );

		for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
				 curr_vertex = ns_model_vertex_next( curr_vertex ) )
			if( ns_model_vertex_get_auto_iter( curr_vertex ) == auto_iter )
				NS_RENDER( state, point_func )(
					&N,
					ns_model_vertex_get_position( curr_vertex, &V ),
					state
					);

		NS_RENDER_PRE_OR_POST( state, post_points_func );
		}
   }


void ns_model_render_vertex_list_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  next_vertex;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, sphere_func ) );

   state->variables.shape = NS_RENDER_SHAPE_SPHERE;
   NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_vertices_render_as_spheres_begin( state );

   curr_vertex = ns_model_grafter_begin_vertices( model );

   while( ns_model_vertex_not_equal( curr_vertex, NS_MODEL_VERTEX_NIL ) )
      {
      next_vertex = ns_model_vertex_get_list( curr_vertex );

      state->variables.color = ns_render_state_vertex_color( state, curr_vertex );
		_ns_model_vertex_render_as_sphere( state, curr_vertex, ns_model_vertex_get_radius( curr_vertex ) );

      curr_vertex = next_vertex;
      }

	_ns_model_vertices_render_as_spheres_end( state );

   NS_RENDER_PRE_OR_POST( state, post_spheres_func );
   }


NS_PRIVATE void _ns_model_do_render_vertices_points_recursive
	(
	const NsVector3b  *N,
	nsmodelvertex      parent_vertex,
	nsmodelvertex      curr_vertex,
	NsRenderState     *state,
	nsint              edge_color_scheme
	)
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
   NsVector3f     V;


	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_do_render_vertices_points_recursive( N, curr_vertex, dest_vertex, state, edge_color_scheme );
		else if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_edge_color_ex( state, edge_color_scheme, curr_edge );

			NS_RENDER( state, point_func )(
				N,
				ns_model_vertex_get_position( curr_vertex, &V ),
				state
				);
			}
		}
	}


NS_PRIVATE void _ns_model_render_vertices_points_recursive
	(
   const NsModel     *model,
	const NsVector3b  *N,
   NsRenderState     *state,
	nsint              edge_color_scheme
	)
	{
	nsmodelorigin  curr_origin;
	nsmodelorigin  end_origins;
	nsmodelvertex  vertex;
	NsVector3f     V;


	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		vertex = ns_model_origin_vertex( curr_origin );

		if( ! ns_model_vertex_is_hidden( vertex ) || state->constants.draw_hidden_objects )
			{
			state->variables.color = NS_COLOR4UB_BLACK;

			NS_RENDER( state, point_func )(
				N,
				ns_model_vertex_get_position( vertex, &V ),
				state
				);
			}

		_ns_model_do_render_vertices_points_recursive(
			N,
			NS_MODEL_VERTEX_NIL,
			vertex,
			state,
			edge_color_scheme
			);
		}
	}


void ns_model_render_vertices_points
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   NsVector3f     V;
   NsVector3b     N;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, point_func ) );

   /* Just create a "dummy" normal for the vertices. */
   N.x = 1; N.y = 0; N.z = 0;

   state->variables.shape = NS_RENDER_SHAPE_POINT;
   NS_RENDER_PRE_OR_POST( state, pre_points_func );

	if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_ORDER )
		_ns_model_render_vertices_points_recursive( model, &N, state, state->constants.EDGE_COLOR_BY_ORDER );
	else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_SECTION )
		_ns_model_render_vertices_points_recursive( model, &N, state, state->constants.EDGE_COLOR_BY_SECTION );
	else
		{
		curr_vertex  = ns_model_begin_vertices( model );
		end_vertices = ns_model_end_vertices( model );

		for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
				 curr_vertex = ns_model_vertex_next( curr_vertex ) )
			if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
				{
				state->variables.color = ns_render_state_vertex_color( state, curr_vertex );
      
				NS_RENDER( state, point_func )(
					&N,
					ns_model_vertex_get_position( curr_vertex, &V ),
					state
					);
				}
		}

   NS_RENDER_PRE_OR_POST( state, post_points_func );
   }


void ns_model_render_vertex_list_points
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  next_vertex;
   NsVector3f     V;
   NsVector3b     N;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, point_func ) );

   /* Just create a "dummy" normal for the vertices. */
   N.x = 1; N.y = 0; N.z = 0;

   state->variables.shape = NS_RENDER_SHAPE_POINT;
   NS_RENDER_PRE_OR_POST( state, pre_points_func );

   curr_vertex = ns_model_grafter_begin_vertices( model );

   while( ns_model_vertex_not_equal( curr_vertex, NS_MODEL_VERTEX_NIL ) )
      {
      next_vertex = ns_model_vertex_get_list( curr_vertex );

      state->variables.color = ns_render_state_vertex_color( state, curr_vertex );
      
      NS_RENDER( state, point_func )(
         &N,
         ns_model_vertex_get_position( curr_vertex, &V ),
         state
         );

      curr_vertex = next_vertex;
      }

   NS_RENDER_PRE_OR_POST( state, post_points_func );
   }


void ns_model_render_vertices_selected
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   NsAABBox3d     B;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

   state->variables.color = NS_COLOR4UB_WHITE;
   state->variables.shape = NS_RENDER_SHAPE_AABBOX;

   NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ns_model_vertex_is_selected( curr_vertex ) )
         NS_RENDER( state, aabbox_func )(
            ns_model_vertex_to_aabbox_ex( curr_vertex, &B ),
            state
            );

   NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
   }


#define _ns_model_edge_get_render_iter( E )\
   ( ns_model_edge( E )->render_iter )


/* NOTE: Avoid drawing each edge twice by setting a number for each edge
   and its mirror as it is traversed. */
NS_PRIVATE void _ns_model_edge_set_render_iter( nsmodeledge E, nsshort render_iter )
   {
   ns_model_edge( E )->render_iter = ( nsshort )render_iter;
   ns_model_edge( ns_model_edge_mirror( E ) )->render_iter = ( nsshort )render_iter;
   }


NS_PRIVATE void _ns_model_next_render_iter( NsModel *model )
   {
   if( NS_SHORT_MAX == model->render_iter )
      model->render_iter = 0;
   else
      ++(model->render_iter);
   }


void ns_model_render_edges_lines
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	NsVector3f     V1;
	NsVector3f     V2;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, line_func ) );

   _ns_model_next_render_iter( ( NsModel* )model );

   state->variables.shape = NS_RENDER_SHAPE_LINE;
   NS_RENDER_PRE_OR_POST( state, pre_lines_func );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
         {
         curr_edge = ns_model_vertex_begin_edges( curr_vertex );
         end_edges = ns_model_vertex_end_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge, end_edges );
                 curr_edge = ns_model_edge_next( curr_edge ) )
            if( _ns_model_edge_get_render_iter( curr_edge ) != model->render_iter )
               if( ! ns_model_vertex_is_hidden( ns_model_edge_dest_vertex( curr_edge ) )
						 || state->constants.draw_hidden_objects )
                  {
                  _ns_model_edge_set_render_iter( curr_edge, model->render_iter );

                  state->variables.color = ns_render_state_edge_color( state, curr_edge );

                  NS_RENDER( state, line_func )(
                     ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 ),
                     ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 ),
                     state
                     );
                  }
         }

   NS_RENDER_PRE_OR_POST( state, post_lines_func );
   }


void ns_model_render_edges_frustums
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   NsVector3f     V1;
   NsVector3f     V2;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, frustum_func ) );

   state->variables.shape = NS_RENDER_SHAPE_FRUSTUM;
   NS_RENDER_PRE_OR_POST( state, pre_frustums_func );

   _ns_model_next_render_iter( ( NsModel* )model );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
         {
         curr_edge = ns_model_vertex_begin_edges( curr_vertex );
         end_edges = ns_model_vertex_end_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge, end_edges );
                 curr_edge = ns_model_edge_next( curr_edge ) )
            if( _ns_model_edge_get_render_iter( curr_edge ) != model->render_iter )
               if( ! ns_model_vertex_is_hidden( ns_model_edge_dest_vertex( curr_edge ) )
						 || state->constants.draw_hidden_objects )
                  {
                  _ns_model_edge_set_render_iter( curr_edge, model->render_iter );

                  state->variables.color = ns_render_state_edge_color( state, curr_edge );

                  NS_RENDER( state, frustum_func )(
                     ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 ),
                     ns_model_vertex_get_radius( ns_model_edge_src_vertex( curr_edge ) ),
                     ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 ),
                     ns_model_vertex_get_radius( ns_model_edge_dest_vertex( curr_edge ) ),
                     state
                     );
                  }
         }

   NS_RENDER_PRE_OR_POST( state, post_frustums_func );
   }


void ns_model_render_edge_order_centers
   (
   const NsModel  *model,
	void            ( *callback )( const NsVector3f*, nsulong, nspointer ),
	nspointer       user_data
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	NsVector3f     V1, V2, V;


   ns_assert( NULL != model );

   _ns_model_next_render_iter( ( NsModel* )model );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
              curr_edge = ns_model_edge_next( curr_edge ) )
         if( _ns_model_edge_get_render_iter( curr_edge ) != model->render_iter )
            {
            _ns_model_edge_set_render_iter( curr_edge, model->render_iter );

				if( ns_model_edge_is_order_center( curr_edge ) )
					{
					ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 );
					ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 );

					/* Find the center of the edge. */
					ns_vector3f_add( &V, &V1, &V2 );
					ns_vector3f_cmpd_scale( &V, 0.5f );

					( callback )( &V, ( nsulong )ns_model_edge_get_order( curr_edge ), user_data );
					}
            }
      }
   }


void ns_model_render_edge_section_centers
   (
   const NsModel  *model,
	void            ( *callback )( const NsVector3f*, nsulong, nspointer ),
	nspointer       user_data
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	NsVector3f     V1, V2, V;


   ns_assert( NULL != model );

   _ns_model_next_render_iter( ( NsModel* )model );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
              curr_edge = ns_model_edge_next( curr_edge ) )
         if( _ns_model_edge_get_render_iter( curr_edge ) != model->render_iter )
            {
            _ns_model_edge_set_render_iter( curr_edge, model->render_iter );

				if( ns_model_edge_is_section_center( curr_edge ) )
					{
					ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 );
					ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 );

					/* Find the center of the edge. */
					ns_vector3f_add( &V, &V1, &V2 );
					ns_vector3f_cmpd_scale( &V, 0.5f );

					( callback )( &V, ns_model_edge_get_section( curr_edge ), user_data );
					}
            }
      }
   }


/*
void ns_model_render_msh_zone
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   NsVector3f     V1;
   NsVector3f     V2;
	nsboolean      polygon_borders;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, frustum_func ) );

   state->variables.shape = NS_RENDER_SHAPE_FRUSTUM;
	state->variables.color = NS_COLOR4UB_DARK_GREY;

	polygon_borders = state->constants.polygon_borders;

   NS_RENDER_PRE_OR_POST( state, pre_frustums_func );

   _ns_model_next_render_iter( ( NsModel* )model );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      if( ! ns_model_vertex_is_hidden( curr_vertex ) || state->constants.draw_hidden_objects )
         {
         curr_edge = ns_model_vertex_begin_edges( curr_vertex );
         end_edges = ns_model_vertex_end_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge, end_edges );
                 curr_edge = ns_model_edge_next( curr_edge ) )
            if( _ns_model_edge_get_render_iter( curr_edge ) != model->render_iter )
               if( ! ns_model_vertex_is_hidden( ns_model_edge_dest_vertex( curr_edge ) )
						 || state->constants.draw_hidden_objects )
                  {
                  _ns_model_edge_set_render_iter( curr_edge, model->render_iter );

                  NS_RENDER( state, frustum_func )(
                     ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 ),
                     ns_model_vertex_get_radius( ns_model_edge_src_vertex( curr_edge ) ) + msh,
                     ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 ),
                     ns_model_vertex_get_radius( ns_model_edge_dest_vertex( curr_edge ) ) + msh,
                     state
                     );
                  }
         }

   NS_RENDER_PRE_OR_POST( state, post_frustums_func );
   }
*/


typedef struct _NsModelVertexOffset
	{
	nsmodelvertex  vertex;
	nsmodelvertex  parent;
	NsVector3f     position;
	}
	NsModelVertexOffset;


NS_PRIVATE void _ns_model_offset_xy_random_init
	(
	nsmodelvertex         curr_vertex,
	nsmodelvertex         parent_vertex,
	NsModelVertexOffset  *offsets,
	nssize               *index
	)
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge, end_edges;


	offsets[ *index ].vertex = curr_vertex;
	offsets[ *index ].parent = parent_vertex;

	ns_model_vertex_get_position( curr_vertex, &( offsets[ *index ].position ) );

	++(*index);

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			_ns_model_offset_xy_random_init(
				dest_vertex,
				curr_vertex,
				offsets,
				index
				);
		}
	}


NsError ns_model_offset_xy_random( NsModel *model, nsfloat max_xy_offset )
	{
	NsModelVertexOffset  *offsets;
	nssize                index;
	nssize                num_vertices;
	nsmodelorigin         curr_origin, end_origins;
	NsVector3f            P1, P2, V, C, N;
	nsfloat               r;


	ns_assert( NULL != model );

	num_vertices = ns_model_num_vertices( model );

	if( NULL == ( offsets = ns_new_array( NsModelVertexOffset, num_vertices ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	index       = 0;
	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		_ns_model_offset_xy_random_init(
			ns_model_origin_vertex( curr_origin ),
			NS_MODEL_VERTEX_NIL,
			offsets,
			&index
			);

	ns_assert( index == num_vertices );

	N.x = 0.0f; N.y = 0.0f; N.z = 1.0f;

	for( index = 0; index < num_vertices; ++index )
		if( ns_model_vertex_not_equal( offsets[ index ].parent, NS_MODEL_VERTEX_NIL ) )
			{
			ns_model_vertex_get_position( offsets[ index ].vertex, &P1 );
			ns_model_vertex_get_position( offsets[ index ].parent, &P2 );

			ns_vector3f_sub( &V, &P1, &P2 );
			ns_vector3f_cross( &C, &V, &N );
			ns_vector3f_norm( &C );

			//if( 0.5f < ns_randf( &R ) )
			//	ns_vector3f_cmpd_rev( &C );
			//C.x = C.x * ( ns_randf( &R ) * max_xy_offset );
			//C.y = C.y * ( ns_randf( &R ) * max_xy_offset );

			r = ns_global_randf();
//ns_println( "%f", r );

			C.x = C.x * ( r * max_xy_offset );
			C.y = C.y * ( r * max_xy_offset );
			C.z = 0.0f; /* Should be zero anyway, but lets be sure. */

			r = ns_global_randf();
//ns_println( "%f", r );

			if( r > 0.5f )
				ns_vector3f_cmpd_rev( &C );

			ns_vector3f_add( &( offsets[ index ].position ), &P1, &C );
			ns_assert( NS_FLOAT_EQUAL( P1.z, offsets[ index ].position.z ) );
			}

	for( index = 0; index < num_vertices; ++index )
		ns_model_vertex_set_position(
			offsets[ index ].vertex,
			&( offsets[ index ].position )
			);

	ns_delete( offsets );

	return ns_no_error();
	}


NS_PRIVATE nsfloat _ns_model_vertex_distance_to_conical_frustum
   (
   const NsPoint3f    *V,
   const nsmodeledge   edge,
	nsboolean           is_2d
   )
   {
	NsPoint3f  P;
	NsLine3f   L;
	nsfloat    d1, d2, d;


	P = *V;
	
   ns_model_vertex_get_position( ns_model_edge_src_vertex( edge ), &L.P1 );
   ns_model_vertex_get_position( ns_model_edge_dest_vertex( edge ), &L.P2 );

	/* NOTE: Setting these all to zero effectively makes this a 2D calculation. */
	if( is_2d )
		P.z = L.P1.z = L.P2.z = 0.0f;

	if( ns_point3f_projects_onto_line( &P, &L ) )
		d = ns_point3f_distance_to_line( &P, &L );
	else
		{
		d1 = ns_point3f_distance_to_point( &P, &L.P1 );
		d2 = ns_point3f_distance_to_point( &P, &L.P2 );

		d = d1 < d2 ? d1 : d2;
		}

	return d;
   }


NS_PRIVATE nsdouble _ns_model_vertex_medial_axis_error
	(
	nsmodelvertex   vertex,
	NsModel        *ref_model,
	nsulong         section,
	nsboolean       is_2d
	)
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	nsfloat        distance;
	nsfloat        min_distance;
	NsVector3f     P;


	min_distance = NS_FLOAT_MAX;

	ns_model_vertex_get_position( vertex, &P );

   curr_vertex  = ns_model_begin_vertices( ref_model );
   end_vertices = ns_model_end_vertices( ref_model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
              curr_edge = ns_model_edge_next( curr_edge ) )
			if( ns_model_edge_get_section( curr_edge ) == section )
				{
				distance = _ns_model_vertex_distance_to_conical_frustum( &P, curr_edge, is_2d );

				if( distance < min_distance )
					min_distance = distance;
				}
		}

	return ( nsdouble )min_distance;
	}


NS_PRIVATE nsdouble _ns_model_do_calc_medial_axis_error
	( 
	nsmodelvertex   parent_vertex,
	nsmodelvertex   curr_vertex,
	NsModel        *ref_model,
	nsboolean       is_2d
	)
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nsmodeledge    parent_edge;
	nsulong        section;
	nsdouble       error;


	error = 0.0;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			error += _ns_model_do_calc_medial_axis_error( curr_vertex, dest_vertex, ref_model, is_2d );
		else if( ns_model_vertex_not_equal( parent_vertex, NS_MODEL_VERTEX_NIL ) )
			{
			parent_edge = ns_model_vertex_find_edge( curr_vertex, parent_vertex );

			ns_assert( ns_model_edge_not_equal( parent_edge, ns_model_vertex_end_edges( curr_vertex ) ) );
			ns_assert( ns_model_edge_not_equal( parent_edge, NS_MODEL_EDGE_NIL ) );

			section = ns_model_edge_get_section( parent_edge );

			error += _ns_model_vertex_medial_axis_error( curr_vertex, ref_model, section, is_2d );
			}
		}

	return error;
	}


NS_PRIVATE nsdouble _ns_model_calc_medial_axis_error( NsModel *new_model, NsModel *ref_model, nsboolean is_2d )
	{
	nsmodelorigin  curr, end;
	nsdouble       error;


	ns_assert( NULL != new_model );
	ns_assert( NULL != ref_model );

	if( 0 == ns_model_num_vertices( ref_model ) )
		return 0.0;

	ns_assert( ns_model_num_origins( new_model ) == ns_model_num_origins( ref_model ) );
	//ns_assert( ns_model_num_vertices( new_model ) == ns_model_num_vertices( ref_model ) );
	//ns_assert( ns_model_num_edges( new_model ) == ns_model_num_edges( ref_model ) );

	error = 0.0;

	curr = ns_model_begin_origins( new_model );
	end  = ns_model_end_origins( new_model );

	for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
		error +=
			_ns_model_do_calc_medial_axis_error(
				NS_MODEL_VERTEX_NIL,
				ns_model_origin_vertex( curr ),
				ref_model,
				is_2d
				);

	/* This is the Mean Absolute Error. */
	return error / ( nsdouble )ns_model_num_vertices( new_model );
	}


nsdouble ns_model_calc_medial_axis_error_2d( NsModel *new_model, NsModel *ref_model )
	{  return _ns_model_calc_medial_axis_error( new_model, ref_model, NS_TRUE );  }


nsdouble ns_model_calc_medial_axis_error_3d( NsModel *new_model, NsModel *ref_model )
	{  return _ns_model_calc_medial_axis_error( new_model, ref_model, NS_FALSE );  }
