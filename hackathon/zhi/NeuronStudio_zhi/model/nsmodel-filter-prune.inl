
NS_PRIVATE void _ns_model_prune_init_lengths( NsModel *model )
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
         ns_model_edge_set_prune_length( curr_edge, NS_FLOAT_MAX, NS_FALSE );
      }
	}


NS_PRIVATE void _ns_model_prune_set_lengths( nsmodelvertex parent_vertex, nsmodelvertex curr_vertex )
   {
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	nsmodeledge    parent_edge;
   nsmodelvertex  dest_vertex;
	nsfloat        max_length, length;
	nsboolean      unremovable;


	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );

	unremovable =
		ns_model_vertex_is_tip( curr_vertex ) ?
			ns_model_vertex_is_unremovable( curr_vertex ) : NS_FALSE;

	parent_edge = NS_MODEL_EDGE_NIL;
	max_length  = ns_model_vertex_get_scoop( curr_vertex );

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			_ns_model_prune_set_lengths( curr_vertex, dest_vertex );

			unremovable |= ns_model_edge_is_unremovable( curr_edge );

			length = ns_model_edge_get_prune_length( curr_edge );

			if( length > max_length )
				max_length = length;
			}
		else
			parent_edge = curr_edge;
		}

	if( ns_model_edge_not_equal( parent_edge, NS_MODEL_EDGE_NIL ) )
		{
		length = max_length + ns_model_edge_length( parent_edge );

		if( length < ns_model_edge_get_prune_length( parent_edge ) )
			ns_model_edge_set_prune_length( parent_edge, length, NS_TRUE );

		ns_model_edge_mark_unremovable( parent_edge, unremovable, NS_TRUE );
		}
   }


NS_PRIVATE void _ns_model_prune_remove_unseen_vertices( NsModel *model )
   {
   nsmodelvertex  curr, next, end;
   nssize         n;


   n = ns_model_num_vertices( model );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   while( ns_model_vertex_not_equal( curr, end ) )
      {
      next = ns_model_vertex_next( curr );

      if( ! ns_model_vertex_is_manually_traced( curr ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr ) )
         {
         ns_assert( ! ns_model_vertex_is_origin( curr ) );
			ns_assert( ! ns_model_vertex_is_seed( curr ) );

         ns_model_remove_vertex( model, curr );
         }

      curr = next;
      }

/* TEMP */
ns_println( "Pruned off " NS_FMT_ULONG " vertices.", n - ns_model_num_vertices( model ) );
ns_println( "" );
   }


NS_PRIVATE nsmodelvertex _ns_model_prune_find_new_seen_origin( NsModelConnComp *cc )
	{
	nsmodelvertex  origin;
	nssize         count;
	nsfloat        radius;
	nsdouble       sum;
	nsdouble       average;
   nsmodelvertex  V;


	origin = NS_MODEL_VERTEX_NIL;
	count  = 0;
	sum    = 0.0;

   for( V = cc->list;
        ns_model_vertex_not_equal( V, NS_MODEL_VERTEX_NIL );
        V = ns_model_vertex_get_list( V ) )
		if( NS_MODEL_VERTEX_SEEN == ns_model_vertex_get_color( V ) )
			{
			radius = ns_model_vertex_get_scoop( V );

			/* Set the origin as the largest one. */
			if( ns_model_vertex_equal( origin, NS_MODEL_VERTEX_NIL ) ||
				 ns_model_vertex_get_scoop( origin ) < radius )
				origin = V;

			++count;
			sum += ( nsdouble )radius;
			}

	average = sum / ( nsdouble )count;

	ns_assert( ns_model_vertex_not_equal( origin, NS_MODEL_VERTEX_NIL ) );

	/* If this origin is not "significantly" bigger than the average,
		mark it as invalid. */
	if( ns_model_vertex_get_scoop( origin ) < NS_MODEL_ORIGIN_SCALAR * average )
		ns_model_vertex_mark_valid( origin, NS_FALSE );

	return origin;
	}


NS_PRIVATE void _ns_model_prune_reset_unseen_origins( NsModel *model )
   {
	nslistiter        curr, end;
	nsmodelorigin     origin;
	nsmodelvertex     vertex;
	NsModelConnComp  *cc;


	curr = ns_model_begin_conn_comps( model );
	end  = ns_model_end_conn_comps( model );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		cc = ns_list_iter_get_object( curr );

      if( ! ns_model_vertex_is_manually_traced( cc->origin ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( cc->origin ) )
         {
/*TEMP*/ns_println( "Pruned off origin for connected component " NS_FMT_ULONG, cc->id );

			vertex = _ns_model_prune_find_new_seen_origin( cc );
			ns_assert( ! ns_model_vertex_is_manually_traced( vertex ) );

			ns_assert( ns_model_vertex_not_equal( cc->origin, vertex ) );
			cc->origin = vertex;

			origin = ns_model_origin_by_conn_comp( model, cc->id );
			ns_assert( ns_model_origin_not_equal( NS_MODEL_ORIGIN_NIL, origin ) );

         ns_model_reset_origin( model, origin, vertex );
         }
		}
   }


NS_PRIVATE void _ns_model_prune_by_attach_ratio
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex,
	nsfloat        attach_ratio
	)
   {
   nsmodeledge    curr_edge, next_edge, end_edges;
   nsmodelvertex  dest_vertex;
	nsfloat        radius;


	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );
	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr_vertex ) );

	ns_model_vertex_set_color( curr_vertex, NS_MODEL_VERTEX_SEEN );

	radius = ns_model_vertex_get_scoop( curr_vertex );

	/* Chop off this vertices connections to "short" branches. */

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   while( ns_model_edge_not_equal( curr_edge, end_edges ) )
		{
		next_edge   = ns_model_edge_next( curr_edge );
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		/* NOTE: This routine assumes no vertices have edges pointing to themselves (i.e. self-loops). */
		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) )
			if( ns_model_edge_get_prune_length( curr_edge ) <= radius * attach_ratio )
				if( ! ns_model_edge_is_unremovable( curr_edge ) )
					ns_model_vertex_remove_edge( curr_vertex, curr_edge );

		curr_edge = next_edge;
		}

	/* Recurse into those that remained. */

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) )
			_ns_model_prune_by_attach_ratio( curr_vertex, dest_vertex, attach_ratio );
		}
   }


NS_PRIVATE nsmodeledge _ns_model_prune_get_largest_edge
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex,
	nsmodeledge    ignore_edge
	)
	{
	nsmodeledge    curr_edge, end_edges, largest_edge;
   nsmodelvertex  dest_vertex;


	largest_edge = NS_MODEL_EDGE_NIL;

	/* NOTE: Find the largest edge or any one that is marked as unremovable. */

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) )
			if( ns_model_edge_not_equal( curr_edge, ignore_edge ) )
				{
				if( ns_model_edge_is_unremovable( curr_edge ) )
					{
					largest_edge = curr_edge;
					break;
					}

				if( ns_model_edge_equal( largest_edge, NS_MODEL_EDGE_NIL ) ||
					 ns_model_edge_get_prune_length( curr_edge ) > ns_model_edge_get_prune_length( largest_edge ) )
					largest_edge = curr_edge;
				}
		}

	return largest_edge;
	}


NS_PRIVATE void _ns_model_prune_by_min_length
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex,
	nsfloat        min_length,
	nsmodelvertex  start_vertex
	)
   {
   nsmodeledge    curr_edge, next_edge, end_edges, largest_edge, next_largest_edge;
   nsmodelvertex  dest_vertex;
	nsfloat        radius;


	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );
	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( curr_vertex ) );

	ns_model_vertex_set_color( curr_vertex, NS_MODEL_VERTEX_SEEN );

	radius = ns_model_vertex_get_scoop( curr_vertex );

	/* Find the outgoing branch that is the longest. Note that for vertices with only
		one edge (a tip) the largest edge will remain as NULL. For vertices with 2 edges
		(a line) the largest edge will be the one outgoing edge and it will not be
		removed. */

	largest_edge = _ns_model_prune_get_largest_edge( parent_vertex, curr_vertex, NS_MODEL_EDGE_NIL );

	next_largest_edge =
		ns_model_vertex_equal( curr_vertex, start_vertex ) ?
			_ns_model_prune_get_largest_edge( parent_vertex, curr_vertex, largest_edge ) : NS_MODEL_EDGE_NIL;

	/* Chop off this vertices connections to "short" branches. */

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   while( ns_model_edge_not_equal( curr_edge, end_edges ) )
		{
		next_edge   = ns_model_edge_next( curr_edge );
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		/* NOTE: This routine assumes no vertices have edges pointing to themselves (i.e. self-loops). */
		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) &&
			 ns_model_edge_not_equal( curr_edge, largest_edge ) &&
			 ns_model_edge_not_equal( curr_edge, next_largest_edge ) )
			if( ns_model_edge_get_prune_length( curr_edge ) - radius < min_length )
				if( ! ns_model_edge_is_unremovable( curr_edge ) )
					ns_model_vertex_remove_edge( curr_vertex, curr_edge );

		curr_edge = next_edge;
		}

	/* Recurse into those that remained. */

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) &&
			 NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) )
			_ns_model_prune_by_min_length( curr_vertex, dest_vertex, min_length, start_vertex );
		}
   }


NS_PRIVATE nsmodeledge _ns_model_prune_get_furthest_edge
	(
	nsmodelvertex  parent_vertex,
	nsmodelvertex  curr_vertex
	)
	{
	nsmodeledge    curr_edge, end_edges, furthest_edge;
   nsmodelvertex  dest_vertex;


	/* NOTE: Find the largest edge or any one that is marked as unremovable. */

	furthest_edge = NS_MODEL_EDGE_NIL;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			if( ns_model_edge_is_unremovable( curr_edge ) )
				{
				furthest_edge = curr_edge;
				break;
				}

			if( ns_model_edge_equal( furthest_edge, NS_MODEL_EDGE_NIL ) ||
				 ns_model_edge_get_prune_length( curr_edge ) > ns_model_edge_get_prune_length( furthest_edge ) )
				furthest_edge = curr_edge;
			}
		}

	return furthest_edge;
	}


NS_PRIVATE nsmodelvertex _ns_model_prune_find_furthest( nsmodelvertex vertex )
	{
	nsmodeledge curr_edge, next_edge;

	curr_edge = _ns_model_prune_get_furthest_edge( NS_MODEL_VERTEX_NIL, vertex );

	if( ns_model_edge_equal( curr_edge, NS_MODEL_EDGE_NIL ) )
		return vertex;

	NS_INFINITE_LOOP
		{
		next_edge =
			_ns_model_prune_get_furthest_edge(
				ns_model_edge_src_vertex( curr_edge ),
				ns_model_edge_dest_vertex( curr_edge )
				);

		if( ns_model_edge_equal( next_edge, NS_MODEL_EDGE_NIL ) )
			break;

		curr_edge = next_edge;
		}

	ns_assert( ns_model_edge_not_equal( curr_edge, NS_MODEL_EDGE_NIL ) );
	return ns_model_edge_dest_vertex( curr_edge );
	}


NS_PRIVATE void _ns_model_prune_get_center_edge
	(
	nsmodelvertex   parent_vertex,
	nsmodelvertex   curr_vertex,
	nsmodeledge    *largest_edge
	)
	{
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	nsmodelvertex  dest_vertex;


   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			if( ns_model_edge_equal( *largest_edge, NS_MODEL_EDGE_NIL ) ||
				 ns_model_edge_get_prune_length( curr_edge ) > ns_model_edge_get_prune_length( *largest_edge ) )
				*largest_edge = curr_edge;

			_ns_model_prune_get_center_edge( curr_vertex, dest_vertex, largest_edge );
			}
		}
	}


NS_PRIVATE nsmodelvertex _ns_model_prune_find_center( nsmodelvertex vertex )
	{
	nsmodeledge    center_edge;
	nsmodelvertex  src_vertex;
	nsmodelvertex  dest_vertex;


	center_edge = NS_MODEL_EDGE_NIL;
	_ns_model_prune_get_center_edge( NS_MODEL_VERTEX_NIL, vertex, &center_edge );

	if( ns_model_edge_equal( center_edge, NS_MODEL_EDGE_NIL ) )
		return vertex;

	src_vertex  = ns_model_edge_src_vertex( center_edge );
	dest_vertex = ns_model_edge_dest_vertex( center_edge );

	return ns_model_vertex_get_scoop( src_vertex ) > ns_model_vertex_get_scoop( dest_vertex )
				? src_vertex : dest_vertex;
	}


NS_PRIVATE void _ns_model_prune_remove_unseen_seeds( NsModel *model )
	{
	nsmodelseed    curr, next, end;
	nsmodelvertex  vertex;


	curr = ns_model_begin_seeds( model );
	end  = ns_model_end_seeds( model );

   while( ns_model_seed_not_equal( curr, end ) )
		{
		next = ns_model_seed_next( curr );

		vertex = ns_model_seed_vertex( curr );
		ns_assert( ! ns_model_vertex_is_manually_traced( vertex ) );

		if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) )
			ns_model_remove_seed( model, curr );

		curr = next;
		}
	}


NS_PRIVATE NsError ns_model_filter_prune
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   )
   {
	nsmodelseed    curr, end;
	nsmodelvertex  vertex;
   NsError        error;


   ns_progress_set_title( progress, "Pruning Model..." );

   ns_progress_begin( progress );

   error = ns_no_error();

	_ns_model_prune_init_lengths( model );

	curr = ns_model_begin_seeds( model );
	end  = ns_model_end_seeds( model );

   for( ; ns_model_seed_not_equal( curr, end ); curr = ns_model_seed_next( curr ) )
		{
		vertex = ns_model_seed_vertex( curr );
		ns_assert( ! ns_model_vertex_is_manually_traced( vertex ) );
	
		if( ns_progress_cancelled( progress ) )
			goto _NS_MODEL_PRUNE_EXIT;

		_ns_model_prune_set_lengths( NS_MODEL_VERTEX_NIL, vertex );
		}

/*TEMP*/ns_println( "# of SEEDS = " NS_FMT_ULONG, ns_model_num_seeds( model ) );

	curr = ns_model_begin_seeds( model );
	end  = ns_model_end_seeds( model );

   for( ; ns_model_seed_not_equal( curr, end ); curr = ns_model_seed_next( curr ) )
		{
		if( ns_progress_cancelled( progress ) )
			goto _NS_MODEL_PRUNE_EXIT;

		vertex = ns_model_seed_vertex( curr );

		vertex = _ns_model_prune_find_furthest( vertex );
		_ns_model_prune_set_lengths( NS_MODEL_VERTEX_NIL, vertex );

		vertex = _ns_model_prune_find_center( vertex );
		//_ns_model_prune_set_lengths( NS_MODEL_VERTEX_NIL, vertex );

		ns_model_color_all_vertices_recursive( vertex, NS_MODEL_VERTEX_UNSEEN );
		_ns_model_prune_by_attach_ratio(
			NS_MODEL_VERTEX_NIL,
			vertex,
			ns_settings_get_neurite_attachment_ratio( settings )
			);

		ns_model_color_all_vertices_recursive( vertex, NS_MODEL_VERTEX_UNSEEN );
		_ns_model_prune_by_min_length(
			NS_MODEL_VERTEX_NIL,
			vertex,
			ns_settings_get_neurite_min_length( settings ),
			vertex
			);
		}

	_ns_model_prune_reset_unseen_origins( model );
	_ns_model_prune_remove_unseen_seeds( model );
	_ns_model_prune_remove_unseen_vertices( model );

   ns_progress_end( progress );

   _NS_MODEL_PRUNE_EXIT:

   return error;
   }
