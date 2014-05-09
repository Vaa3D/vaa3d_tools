#include "nsmodel-join.h"


typedef struct _NsModelJoinEntry
	{
	nsmodelvertex  curr_vertex;
	nsmodelvertex  closest_vertex;
	nsfloat        min_overlap;
	}
	NsModelJoinEntry;


NS_PRIVATE NsError _ns_model_join_entry_new
	(
	NsModelJoinEntry  **entry,
	nsmodelvertex       curr_vertex,
	nsmodelvertex       closest_vertex,
	nsfloat             min_overlap
	)
	{
	if( NULL == ( *entry = ns_new( NsModelJoinEntry ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*entry)->curr_vertex    = curr_vertex;
	(*entry)->closest_vertex = closest_vertex;
	(*entry)->min_overlap    = min_overlap;

	return ns_no_error();
	}


NS_PRIVATE void _ns_model_join_entry_delete( NsModelJoinEntry *entry )
	{  ns_delete( entry );  }


NsError ns_model_join( NsModel *dest, const NsModel *src, NsList *list )
   {
	NsList             entries;
	nslistiter         curr_entry;
	nslistiter         scan_entry;
	nslistiter         end_entries;
	NsModelJoinEntry  *curr;
	NsModelJoinEntry  *scan;
   nsmodelvertex      curr_vertex;
   nsmodelvertex      next_vertex;
   nsmodelvertex      end_vertices;
   nsmodelvertex      scan_vertex;
	nsmodelvertex      deleted_vertex;
   NsSpheref          curr_sphere;
	NsSpheref          closest_sphere;
   NsSpheref          scan_sphere;
	NsVector3f         curr_position;
   nsfloat            distance;
   nsmodelvertex     *closest_vertices;
   nsfloat           *min_overlaps;
   nsfloat            overlap;
   nssize             curr_conn_comp;
	nssize             scan_conn_comp;
   nssize             closest_conn_comp;
	nssize             num_conn_comps;
	nssize             cc;
	nsboolean          found;
	nsboolean          mark;
	nsboolean          merge;
   NsError            error;



	ns_assert( NULL != dest );
	ns_assert( NULL != src );
	ns_assert( 1 == ns_model_num_origins( src ) );
	ns_assert( NULL != list );

	error = ns_no_error();

	/* First duplicate all the vertices of the source model
		into the destination model. */
	ns_model_mark_joinable( ( NsModel* )src, NS_TRUE );

	if( NS_FAILURE( ns_model_merge( dest, src ), error ) )
		return error;

	ns_model_lock( dest );

	ns_list_construct( &entries, _ns_model_join_entry_delete );

	closest_vertices = NULL;
	min_overlaps     = NULL;

	num_conn_comps = ns_model_set_conn_comp_numbers( dest );

	if( NULL == ( closest_vertices = ns_new_array( nsmodelvertex, num_conn_comps ) ) )
		goto _NS_MODEL_JOIN_EXIT;

	if( NULL == ( min_overlaps = ns_new_array( nsfloat, num_conn_comps ) ) )
		goto _NS_MODEL_JOIN_EXIT;

   curr_vertex  = ns_model_begin_vertices( dest );
   end_vertices = ns_model_end_vertices( dest );

   while( ns_model_vertex_not_equal( curr_vertex, end_vertices ) )
      {
      next_vertex = ns_model_vertex_next( curr_vertex );

      if( ns_model_vertex_is_joinable( curr_vertex ) )
         {
			ns_model_vertex_mark_joinable( curr_vertex, NS_FALSE );
         ns_model_vertex_to_sphere( curr_vertex, &curr_sphere );

         curr_conn_comp = ns_model_vertex_get_conn_comp( curr_vertex );

			for( cc = 0; cc < num_conn_comps; ++cc )
				{
				min_overlaps[cc]     = NS_FLOAT_MAX;
				closest_vertices[cc] = NS_MODEL_VERTEX_NIL;
				}

         scan_vertex = ns_model_begin_vertices( dest );

         /* NOTE: Dont test against any vertices in the same connected
            component. */
         for( ; ns_model_vertex_not_equal( scan_vertex, end_vertices );
                scan_vertex = ns_model_vertex_next( scan_vertex ) )
				{
				scan_conn_comp = ns_model_vertex_get_conn_comp( scan_vertex );
				ns_assert( scan_conn_comp < num_conn_comps );

            if( scan_conn_comp != curr_conn_comp )
               {
               ns_model_vertex_to_sphere( scan_vertex, &scan_sphere );

               if( ns_spheref_intersects_sphere( &curr_sphere, &scan_sphere ) )
                  {
                  distance = ns_vector3f_distance( &curr_sphere.O, &scan_sphere.O );
                  overlap  = distance - curr_sphere.r - scan_sphere.r;

                  ns_assert( overlap <= 0.0f );

                  if( ns_model_vertex_equal( closest_vertices[ scan_conn_comp ], NS_MODEL_VERTEX_NIL ) ||
                      overlap < min_overlaps[ scan_conn_comp ] )
                     {
                     min_overlaps[ scan_conn_comp ]     = overlap;
                     closest_vertices[ scan_conn_comp ] = scan_vertex;
                     }
                  }
               }
				}

			for( cc = 0; cc < num_conn_comps; ++cc )
				if( ns_model_vertex_not_equal( closest_vertices[cc], NS_MODEL_VERTEX_NIL ) )
					{
					/* Traverse the list of entries and update an entry where these overlaps
						are closer than a previous overlap, to a given 'closest_vertex'. If not
						in the list yet, then add it. */

					found = NS_FALSE;

					curr_entry  = ns_list_begin( &entries );
					end_entries = ns_list_end( &entries );

					for( ; ns_list_iter_not_equal( curr_entry, end_entries );
							 curr_entry = ns_list_iter_next( curr_entry ) )
						{
						curr = ns_list_iter_get_object( curr_entry );

						if( ns_model_vertex_equal( closest_vertices[cc], curr->closest_vertex ) )
							{
							if( min_overlaps[cc] < curr->min_overlap )
								{
								curr->curr_vertex = curr_vertex;
								curr->min_overlap = min_overlaps[cc];
								}

							found = NS_TRUE;
							break;
							}
						}

					if( ! found )
						{
						if( NS_FAILURE(
								_ns_model_join_entry_new(
									&curr,
									curr_vertex,
									closest_vertices[cc],
									min_overlaps[cc]
									),
								error ) )
							goto _NS_MODEL_JOIN_EXIT;

						if( NS_FAILURE( ns_list_push_back( &entries, curr ), error ) )
							{
							_ns_model_join_entry_delete( curr );
							goto _NS_MODEL_JOIN_EXIT;
							}
						}
					}
         }

      curr_vertex = next_vertex;
      }

	/* Now go through the entries and do the actual join operations. */

	curr_entry  = ns_list_begin( &entries );
	end_entries = ns_list_end( &entries );

	for( ; ns_list_iter_not_equal( curr_entry, end_entries );
			 curr_entry = ns_list_iter_next( curr_entry ) )
		{
		curr = ns_list_iter_get_object( curr_entry );

		if( ns_model_vertex_not_equal( curr->curr_vertex, NS_MODEL_VERTEX_NIL ) &&
			 ns_model_vertex_not_equal( curr->closest_vertex, NS_MODEL_VERTEX_NIL ) )
			{
			curr_conn_comp    = ns_model_vertex_get_conn_comp( curr->curr_vertex );
			closest_conn_comp = ns_model_vertex_get_conn_comp( curr->closest_vertex );

			if( curr_conn_comp != closest_conn_comp )
				{
				/* Mark the component we're connecting to as manually traced
					if the source component is marked. */
				mark = ns_model_vertex_is_manually_traced( curr->curr_vertex );

				/* Remove the components origin since its going to connect. */
				ns_model_remove_origin_by_conn_comp( dest, curr_conn_comp );

				merge = ns_point3f_inside_sphere(
								ns_model_vertex_get_position( curr->curr_vertex, &curr_position ),
								ns_model_vertex_to_sphere( curr->closest_vertex, &closest_sphere )
								);

				if( merge )
					{
					deleted_vertex = curr->curr_vertex;

					if( NS_FAILURE( ns_model_merge_vertices( dest, curr->closest_vertex, curr->curr_vertex ), error ) )
						goto _NS_MODEL_JOIN_EXIT;

					/* Go through the entries and "null out" any entries that have a 'curr_vertex' equal to
						this entries 'curr_vertex' since that vertex was just deleted in ns_model_merge_vertices(). */

					scan_entry = ns_list_begin( &entries );

					for( ; ns_list_iter_not_equal( scan_entry, end_entries );
							 scan_entry = ns_list_iter_next( scan_entry ) )
						{
						scan = ns_list_iter_get_object( scan_entry );

						if( ns_model_vertex_equal( scan->curr_vertex, deleted_vertex ) )
							scan->curr_vertex = NS_MODEL_VERTEX_NIL;
						}
					}
				else
					{
					if( NS_FAILURE( ns_model_vertex_add_edge( curr->curr_vertex, curr->closest_vertex, NULL, NULL ), error ) )
						goto _NS_MODEL_JOIN_EXIT;
					}

				/* Re-establish connected component number since we connected a component. */
				ns_model_vertex_set_conn_comp_number_recursive( curr->closest_vertex, closest_conn_comp );

				if( mark )
					ns_model_vertex_mark_manually_traced_recursive( curr->closest_vertex, NS_TRUE );

				if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, curr->closest_vertex ), error ) )
					goto _NS_MODEL_JOIN_EXIT;
				}
			}
		}

	_NS_MODEL_JOIN_EXIT:

	ns_list_destruct( &entries );

	ns_free( closest_vertices );
	ns_free( min_overlaps );

	ns_model_unlock( dest );

	return error;
   }

