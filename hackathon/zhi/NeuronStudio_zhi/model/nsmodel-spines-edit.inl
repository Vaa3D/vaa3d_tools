
#define _NS_SPINE_PRINT_ID( spine )\

//	if( spine->is_selected )\
//		ns_println( "spine_id:" NS_FMT_INT, spine->id )

#define _NS_SPINE_PRINT_END()\

//	ns_print_newline();


NS_PRIVATE NsError _ns_model_do_add_spine( NsModel *model, nsspine *S, nsboolean set_id )
	{
	NsSpines  *spines;
	NsSpine   *spine;
	NsError    error;


	error = ns_no_error();
	ns_model_lock( model );

	spines = model->spines;

	if( NS_FAILURE( _ns_spine_new( &spine, spines ), error ) )
		goto _NS_MODEL_ADD_SPINE_EXIT;

	if( NS_FAILURE( ns_list_push_back( &spines->list, spine ), error ) )
		{
		_ns_spine_delete( spine );
		goto _NS_MODEL_ADD_SPINE_EXIT;
		}

	spine->type = NS_SPINE_OTHER;

	spine->iter = ns_list_rev_begin( &spines->list );
	ns_assert( ns_list_iter_get_object( spine->iter ) == spine );

	*S = spine->iter;

	if( set_id )
		_ns_spines_set_id( spines, *S );

	_NS_MODEL_ADD_SPINE_EXIT:

	if( ! ns_is_error( error ) )
		ns_spines_create_random_colors( ns_model_num_spines( model ) );

	ns_model_unlock( model );
	return error;
	}


NsError ns_model_add_spine( NsModel *model, nsspine *S )
	{  return _ns_model_do_add_spine( model, S, NS_TRUE );  }


NsError ns_model_add_spine_no_id( NsModel *model, nsspine *S )
	{  return _ns_model_do_add_spine( model, S, NS_FALSE );  }


void ns_model_remove_spine( NsModel *model, nsspine S )
	{  _ns_spines_erase_sync( model->spines, S );  }


void ns_model_select_spines( NsModel *model, nsboolean selected )
   {
   NsSpines    *spines;
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden )
			{
			spine->is_selected = selected;
			_NS_SPINE_PRINT_ID( spine );
			}
      }

	_NS_SPINE_PRINT_END();
   }


void ns_model_select_spines_by_type( NsModel *model, NsSpineType type )
	{
   NsSpines    *spines;
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden )
			{
			spine->is_selected = ( type == spine->type );
			_NS_SPINE_PRINT_ID( spine );
			}
      }

	_NS_SPINE_PRINT_END();
	}


nsboolean _ns_spines_read_merge_params( NsSpines *spines, const nschar *config_file );
NsError _ns_spines_run_merging( NsSpines *spines );

NsError ns_model_select_spine_bases( NsModel *model, const nschar *config_file )
	{
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

	if( ! _ns_spines_read_merge_params( model->spines, config_file ) )
		return ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	ns_model_select_spines( model, NS_FALSE );
	return _ns_spines_run_merging( model->spines );
	}


NsError ns_model_select_spines_by_aabbox( NsModel *model, const NsAABBox3d *B )
   {
   const NsSpines  *spines;
   nslistiter       curr;
   nslistiter       end;
   NsSpine         *spine;
   NsList           intersected;
   nslistiter       iter;
   nsboolean        any_selected;
   NsError          error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != B );

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the spines and store the ones that intersect
      this bounding box. */

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      if( ! spine->hidden && ns_aabbox3d_intersects_aabbox( &spine->bounding_box, B ) )
         {
         if( NS_FAILURE( ns_list_push_back( &intersected, spine ), error ) )
            {
            ns_list_destruct( &intersected );
            return error;
            }

         if( spine->is_selected )
            any_selected = NS_TRUE;
         }
      }

   /* If any of the intersecting spines are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		{
		spine = ns_list_iter_get_object( iter );
      spine->is_selected = ! any_selected;

		_NS_SPINE_PRINT_ID( spine );
		}

	_NS_SPINE_PRINT_END();

   ns_list_destruct( &intersected );
   return ns_no_error();
   }


NsError ns_model_select_spines_by_projection_onto_drag_rect
	(
	NsModel            *model,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj,
	NsSpineRenderMode   mode
	)
	{
   const NsSpines  *spines;
   nslistiter       curr;
   nslistiter       end;
   NsSpine         *spine;
   NsList           intersected;
   nslistiter       iter;
   nsboolean        any_selected;
	NsSphered        S;
   NsError          error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != drag_rect );
	ns_assert( NULL != proj );

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the spines and store the ones that intersect
      this bounding box. */

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      if( ! spine->hidden )
			if( ns_sphered_projects_onto_drag_rect(
					ns_spine_to_sphere_by_render_mode_ex( curr, mode, &S ),
					drag_rect,
					proj
					) )
				{
				if( NS_FAILURE( ns_list_push_back( &intersected, spine ), error ) )
					{
					ns_list_destruct( &intersected );
					return error;
					}

				if( spine->is_selected )
					any_selected = NS_TRUE;
				}
      }

   /* If any of the intersecting spines are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		{
		spine = ns_list_iter_get_object( iter );
      spine->is_selected = ! any_selected;

		_NS_SPINE_PRINT_ID( spine );
		}

	_NS_SPINE_PRINT_END();

   ns_list_destruct( &intersected );
   return ns_no_error();
	}


/* Select any spines whose attachment area is greater than its non-attachment area.
	All other spines become deselected. */
void ns_model_select_spines_with_invalid_surface_area( NsModel *model )
	{
   const NsSpines  *spines;
   nslistiter       curr;
   nslistiter       end;
   NsSpine         *spine;


   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden )
			spine->is_selected = ( spine->non_attach_sa < spine->attach_sa );
      }
	}


NS_PRIVATE NsError _ns_model_spine_do_find_attach_vertex_and_edge
   (
   NsSpines       *spines,
   NsSpine        *spine,
   NsVector       *nodes,
	nsmodelvertex  *ret_vertex,
	nsmodeledge    *ret_edge,
	NsVector3f     *ret_attach
   )
	{
	nsvectoriter         curr_node;
	nsvectoriter         end_nodes;
	const NsOctreeNode  *node;
	nsvectoriter         curr_object;
	nsvectoriter         end_objects;
	nsmodeledge          edge;
	nsfloat              distance;
	nsfloat              min_distance;
	nsmodelvertex        vertex;
	NsVector3f           attach;
	NsError              error;


	*ret_vertex = NS_MODEL_VERTEX_NIL;
	*ret_edge   = NS_MODEL_EDGE_NIL;

	if( NS_FAILURE(
			ns_model_octree_intersections(
				spines->model,
				NS_MODEL_EDGE_OCTREE,
				&spine->center,
				_ns_spine_voxel_octree_func_special,
				nodes
				),
			error ) )
		return error;

	min_distance = NS_FLOAT_MAX;

	curr_node = ns_vector_begin( nodes );
	end_nodes = ns_vector_end( nodes );

	for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
			 curr_node = ns_vector_iter_next( curr_node ) )
		{
		node = ns_vector_iter_get_object( curr_node );

		curr_object = ns_octree_node_begin_objects( node );
		end_objects = ns_octree_node_end_objects( node );

		for( ; ns_vector_iter_not_equal( curr_object, end_objects );
				 curr_object = ns_vector_iter_next( curr_object ) )
			{
			edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

			distance = _ns_spines_distance_to_conical_frustum(
							spines,
							&spine->center,
							edge,
							&attach,
							NULL,
							NULL,
							&vertex,
							NULL
							);

			if( distance < min_distance )
				{
				min_distance = distance;

				/* NOTE: Reset the spine order as well since we're
					looking at the edges anyway. */
				spine->order = ns_model_edge_get_order( edge );

				*ret_vertex = vertex;
				*ret_edge   = edge;

				if( NULL != ret_attach )
					*ret_attach = attach;
				}
			}
		}

	ns_verify( NS_SUCCESS( ns_vector_resize( nodes, 0 ), error ) );
	return ns_no_error();
	}


NsError ns_model_select_spines_on_selected_parts
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	)
	{
	NsVector        nodes;
   nslistiter      curr, end;
	NsSpine        *spine;
	nsmodelvertex   vertex;
	nsmodeledge     edge;
	NsError         error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != settings );

	error = ns_no_error();

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

	/* The build octree routine needs a few things set before calling. */
	model->spines->settings             = settings;
	model->spines->voxel_info           = ns_settings_voxel_info( settings );
	model->spines->progress             = progress;
	model->spines->max_octree_recursion = 5; /*TEMP?*/

	if( NS_FAILURE( _ns_spines_build_inflated_model_octree( model->spines ), error ) )
		goto _NS_MODEL_SELECT_SPINES_ON_SELECTED_PARTS_EXIT;

   ns_progress_set_title( progress, "Examining spines..." );
   ns_progress_num_iters( progress, ns_model_num_spines( model ) );
   ns_progress_begin( progress );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine  = ns_list_iter_get_object( curr );

		if( ! spine->hidden )
			{
			vertex = NS_MODEL_VERTEX_NIL;
			edge   = NS_MODEL_EDGE_NIL;

			if( NS_FAILURE(
					_ns_model_spine_do_find_attach_vertex_and_edge(
						model->spines,
						spine,
						&nodes,
						&vertex,
						&edge,
						NULL
						),
					error ) )
				goto _NS_MODEL_SELECT_SPINES_ON_SELECTED_PARTS_EXIT;

			/* NOTE: Check vertex attachment first! */
			if( ns_model_vertex_not_equal( vertex, NS_MODEL_VERTEX_NIL ) )
				{
				if( ns_model_vertex_is_selected( vertex ) )
					spine->is_selected = NS_TRUE;
				}
			else if( ns_model_edge_not_equal( edge, NS_MODEL_EDGE_NIL ) )
				{
				if( ns_model_vertex_is_selected( ns_model_edge_src_vertex( edge ) ) &&
					 ns_model_vertex_is_selected( ns_model_edge_dest_vertex( edge ) )  )
					spine->is_selected = NS_TRUE;
				}
			}

		ns_progress_next_iter( progress );
		}

	_NS_MODEL_SELECT_SPINES_ON_SELECTED_PARTS_EXIT:

	ns_progress_end( progress );

   ns_vector_destruct( &nodes );
	ns_model_clear_octree( model, NS_MODEL_EDGE_OCTREE );

	return error;
	}


void ns_model_mark_selected_spines_hidden( NsModel *model, nsboolean b )
	{
	nslistiter   curr, end;
	NsSpine     *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( spine->is_selected )
			{
			spine->hidden      = b;
			spine->is_selected = NS_FALSE;
			}
		}
	}


void ns_model_mark_spines_hidden( NsModel *model, nsboolean b )
	{
	nslistiter curr, end;

	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		( ( NsSpine* )ns_list_iter_get_object( curr ) )->hidden = b;
	}


/* NOTE: Avoid circular inclusion! */
#include <model/nsmodel-io-swc.h>

#define _NS_MODEL_VERTEX_SWC_ID( vertex )\
	NS_POINTER_TO_LONG( ns_model_vertex_get_data( (vertex), NS_MODEL_VERTEX_DATA_SLOT0 ) )

NsError ns_model_spines_set_swc_attachments
	(
	NsModel           *model,
	const NsSettings  *settings,
	nsboolean          set_soma_distances,
	NsProgress        *progress
	)
	{
	NsVector        nodes;
   nslistiter      curr, end;
	NsSpine        *spine;
	nsmodelvertex   vertex;
	nsmodelvertex   src_vertex;
	nsmodelvertex   dest_vertex;
	nslong          src_id;
	nslong          dest_id;
	nsmodelvertex   child_vertex;
	nsmodelvertex   parent_vertex;
	nsmodeledge     edge;
	NsVector3f      A, P, C, PC, AC;
	nsfloat         distance;
	NsError         error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != settings );

	ns_model_set_swc_labels( model, NS_FALSE );

	error = ns_no_error();

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

	/* The build octree routine needs a few things set before calling. */
	model->spines->settings             = settings;
	model->spines->voxel_info           = ns_settings_voxel_info( settings );
	model->spines->progress             = progress;
	model->spines->max_octree_recursion = 5; /*TEMP?*/

	if( NS_FAILURE( _ns_spines_build_inflated_model_octree( model->spines ), error ) )
		goto _NS_MODEL_SPINES_SET_SWC_ATTACHMENTS;

   ns_progress_set_title( progress, "Examining spines..." );
   ns_progress_num_iters( progress, ns_model_num_spines( model ) );
   ns_progress_begin( progress );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine  = ns_list_iter_get_object( curr );
		vertex = NS_MODEL_VERTEX_NIL;
		edge   = NS_MODEL_EDGE_NIL;

		if( NS_FAILURE(
				_ns_model_spine_do_find_attach_vertex_and_edge(
					model->spines,
					spine,
					&nodes,
					&vertex,
					&edge,
					&A
					),
				error ) )
			goto _NS_MODEL_SPINES_SET_SWC_ATTACHMENTS;

		/* NOTE: Check vertex attachment first! */
		if( ns_model_vertex_not_equal( vertex, NS_MODEL_VERTEX_NIL ) )
			{
			#ifdef NS_DEBUG
			ns_model_vertex_get_position( vertex, &C );
			ns_assert( NS_FLOAT_EQUAL( C.x, A.x ) );
			ns_assert( NS_FLOAT_EQUAL( C.y, A.y ) );
			ns_assert( NS_FLOAT_EQUAL( C.z, A.z ) );
			#endif

			spine->attach_swc_vertex_id = _NS_MODEL_VERTEX_SWC_ID( vertex );
			spine->attach_swc_distance  = 0.0f;

			if( set_soma_distances )
				spine->soma_distance = ns_model_vertex_get_soma_distance( vertex );
			}
		else if( ns_model_edge_not_equal( edge, NS_MODEL_EDGE_NIL ) )
			{
			src_vertex  = ns_model_edge_src_vertex( edge );
			dest_vertex = ns_model_edge_dest_vertex( edge );

			src_id  = _NS_MODEL_VERTEX_SWC_ID( src_vertex );
			dest_id = _NS_MODEL_VERTEX_SWC_ID( dest_vertex );

			ns_assert( src_id != dest_id );

			/* The vertex with the lower SWC label is the parent. */
			if( src_id < dest_id )
				{
				parent_vertex = src_vertex;
				child_vertex  = dest_vertex;
				}
			else
				{
				parent_vertex = dest_vertex;
				child_vertex  = src_vertex;
				}

			/* Find out how far along the edge (to the parent) the attachment
				point is from the child vertex. */

			ns_model_vertex_get_position( parent_vertex, &P );
			ns_model_vertex_get_position( child_vertex, &C );

			ns_vector3f_sub( &PC, &P, &C );
			ns_vector3f_sub( &AC, &A, &C );

			#ifdef NS_DEBUG
			{
			//nsfloat ic = ns_vector3f_dot( &PC, &AC ) / ( ns_vector3f_mag( &PC ) * ns_vector3f_mag( &AC ) );
			//ns_println( "ns_acosf( %.16f ) = %.16f", ic, ns_acosf( ic ) );
			//ns_assert( NS_FLOAT_EQUAL( ic, 1.0f ) );
			}
			#endif

			distance = ns_vector3f_mag( &AC ) / ns_vector3f_mag( &PC );

			/* Account for floating point error? */
			if( distance < 0.0f )
				distance = 0.0f;
			else if( distance > 1.0f )
				distance = 1.0f;

			spine->attach_swc_vertex_id = _NS_MODEL_VERTEX_SWC_ID( child_vertex );
			spine->attach_swc_distance  = distance;

			if( set_soma_distances )
				spine->soma_distance =
					ns_model_vertex_get_soma_distance( parent_vertex ) +
					( 1.0f - distance ) * ns_model_vertex_distance( parent_vertex, child_vertex );
			}

/*TEMP*/
//ns_println( " " NS_FMT_LONG " : " NS_FMT_DOUBLE, spine->attach_swc_vertex_id, spine->attach_swc_distance );

		ns_progress_next_iter( progress );
		}

	_NS_MODEL_SPINES_SET_SWC_ATTACHMENTS:

	ns_progress_end( progress );

   ns_vector_destruct( &nodes );
	ns_model_clear_octree( model, NS_MODEL_EDGE_OCTREE );

	return error;
	}


void ns_model_delete_selected_spines( NsModel *model )
   {
   NsSpines    *spines;
   nslistiter   curr;
   nslistiter   next;
   nslistiter   end;
   NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

	ns_model_lock( model );

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   while( ns_list_iter_not_equal( curr, end ) )
      {
      next  = ns_list_iter_next( curr );
      spine = ns_list_iter_get_object( curr );

      if( spine->is_selected )
         _ns_spines_erase_unsync( spines, curr );

      curr = next;
      }

	ns_model_unlock( model );
   }


nsspine ns_model_find_last_manual_spine( const NsModel *model )
	{
   NsSpines    *spines;
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   spines = model->spines;

   curr = ns_list_rev_begin( &spines->list );
   end  = ns_list_rev_end( &spines->list );

   for( ; ns_list_rev_iter_not_equal( curr, end ); curr = ns_list_rev_iter_next( curr ) )
      {
      spine = ns_list_rev_iter_get_object( curr );

      if( ! spine->auto_detected )
			return curr;
      }

	return NS_SPINE_NIL;
	}


void ns_model_type_selected_spines( NsModel *model, NsSpineType type )
   {
   NsSpines    *spines;
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      if( spine->is_selected )
			spine->type = type;
      }
   }


void ns_spine_mark_selected( nsspine S, nsboolean selected )
	{
	NsSpine *spine = ns_list_iter_get_object( S );
	spine->is_selected = selected;

	_NS_SPINE_PRINT_ID( spine );
	_NS_SPINE_PRINT_END();
	}


nsboolean ns_spine_is_selected( const nsspine S )
	{  return ( ( const NsSpine* )ns_list_iter_get_object( S ) )->is_selected;  }


NsAABBox3d* ns_spine_to_aabbox_ex( const nsspine S, NsAABBox3d *B )
	{
	*B = ( ( const NsSpine* )ns_list_iter_get_object( S ) )->bounding_box;
	return B;
	}


NsSphered* ns_spine_to_sphere_ex( const nsspine S, NsSphered *E )
	{
	NsSpine     *spine;
	NsVector3f   O;


	spine = ns_list_iter_get_object( S );
	ns_spine_get_position( S, &O );

	ns_vector3f_to_3d( &O, &E->O );
	E->r = spine->stats.head_diameter / 2.0;

	return E;
	}


NsSphered* ns_spine_to_sphere_by_render_mode_ex( const nsspine S, NsSpineRenderMode mode, NsSphered *E )
	{
	ns_spine_to_sphere_ex( S, E );
	E->r = ns_model_spine_render_radius_ex( S, mode );

	return E;
	}


NsError ns_model_buffer_selected_spines_ex
   (
   const NsModel  *model,
   NsList         *list
   )
	{
   nsspine  curr, end;
   NsError  error;


   ns_assert( NULL != model );
   ns_assert( NULL != list );
   ns_assert( ns_list_is_empty( list ) );
   ns_assert( NULL == ns_list_get_finalize_func( list ) );

   curr = ns_model_begin_spines( model );   
   end  = ns_model_end_spines( model );

   for( ; ns_spine_not_equal( curr, end ); curr = ns_spine_next( curr ) )
      if( ns_spine_is_selected( curr ) )
         if( NS_FAILURE( ns_list_push_back( list, curr ), error ) )
            return error;

   return ns_no_error();
	}


void ns_model_change_selected_spines_radii
	(
	NsModel  *model,
	nsfloat   percent,
	nsfloat   min_radius
	)
	{
   nsspine   curr, end;
	nsdouble  radius;


	ns_assert( NULL != model );

   curr = ns_model_begin_spines( model );   
   end  = ns_model_end_spines( model );

   for( ; ns_spine_not_equal( curr, end ); curr = ns_spine_next( curr ) )
      if( ns_spine_is_selected( curr ) )
			{
			/* Increase or decrease radius by a percentage. */
			radius = ( ns_spine_get_head_diameter( curr ) / 2.0 ) * ( percent / 100.0f );

			if( radius < min_radius )
				radius = min_radius;

			ns_spine_set_head_diameter( curr, radius * 2.0 );
			}
	}
