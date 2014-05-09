#include "nsmodel-stats.h"


typedef struct _NsModelStatsEntry
	{
	nslong   order;
	nsfloat  length;
	nsfloat	diameter;
	}
	NsModelStatsEntry;


typedef struct _NsModelStatsAccum
	{
	NsVector3f  last;
	nssize      count;
	nsfloat     distance;
	nsfloat     sum_diameters;
	}
	NsModelStatsAccum;


NS_PRIVATE NsList* _ns_model_stats_conn_comp_list
	(
	const NsModelStats  *stats,
	nssize               conn_comp
	)
	{
	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	return ( ( NsList* )ns_byte_array_begin( &stats->buffer ) ) + conn_comp;
	}


NS_PRIVATE NsError _ns_model_stats_entry_new
	(
	NsModelStats  *stats,
	nssize         conn_comp,
	nslong         order,
	nsfloat        length,
	nsfloat        diameter
	)
	{
	NsModelStatsEntry  *entry;
	NsError             error;


	ns_assert( NULL != stats );

	if( NULL == ( entry = ns_new( NsModelStatsEntry ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	entry->order    = order;
	entry->length   = length;
	entry->diameter = diameter;

	if( NS_FAILURE( ns_list_push_back(
							_ns_model_stats_conn_comp_list( stats, conn_comp ),
							entry
							),
							error ) )
		{
		ns_delete( entry );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE void _ns_model_stats_entry_delete( nspointer entry )
	{  ns_delete( entry );  }


NS_PRIVATE NsError _ns_model_stats_do_create
	( 
	NsModelStats       *stats,
	nssize              conn_comp,
	nslong              order,
	nsmodelvertex       vertex,
	NsModelStatsAccum   accum
	)
	{
	NsVector3f       curr;
	nsmodeledge      curr_edge;
	nsmodeledge      end_edges;
	nsmodelvertex    dest_vertex;
	nsboolean        has_neighbors_of_same_order;
	nsboolean        did_add_entry;
	nssize           neighbors_of_same_order_count;
	NsError          error;


	ns_assert( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( vertex ) );
	ns_model_vertex_set_color( vertex, NS_MODEL_VERTEX_SEEN );

	ns_model_vertex_get_position( vertex, &curr );
	accum.distance += ns_vector3f_distance( &curr, &accum.last );

	/* Change 04/25/05: Dont include the junctions diameter into
		the average diamter for any given branch. */
	if( ! ns_model_vertex_is_origin( vertex ) /* && ! ns_model_vertex_is_junction( vertex ) */ )
		{
		++accum.count;
		accum.sum_diameters += 2 * ns_model_vertex_get_radius( vertex );
		}

	accum.last = curr;

	/*
	TODO: If previous vertex is inside the soma or we're
	entering a new order AND the previous vertex is a junction,
	we need to find where the branch actually begins, i.e. offset
	the length by the radius of the previous vertex
	*/

	/* 4 cases at any vertex:

		1) The vertex is a leaf node, i.e. the very tip of a branch.
			Add stats to list and return.

		2) The vertex is an interior node that does not have any neighbors
			of a different order. Just keep recursing.

		3) The vertex is an interior node that only has neighbors of a
			different order. Add stats to list and recurse into neighbors.

		4) The vertex is an interior node that has a neighbor of the same
			order and one or more neighbors of a diffent order. Recurse into
			the branches but DONT add stats.
	*/

	if( ns_model_vertex_is_external( vertex ) ) /* Case 1. */
		{
		ns_assert( 0 < accum.count );

		return _ns_model_stats_entry_new(
						stats,
						conn_comp,
						order,
						accum.distance + ns_model_vertex_get_radius( vertex ),
						accum.sum_diameters / accum.count
						);
		}
	else
		{
		/* Search for neighbors of the current vertex of the same order. */
		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		neighbors_of_same_order_count = 0;
		has_neighbors_of_same_order   = NS_FALSE;

		while( ns_model_edge_not_equal( curr_edge, end_edges ) )
			{
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) &&
				 order == ns_model_edge_get_order( curr_edge ) )
				{
				++neighbors_of_same_order_count;
				has_neighbors_of_same_order = NS_TRUE; /* Case 2 or 4. */

				if( NS_FAILURE( _ns_model_stats_do_create(
										stats,
										conn_comp,
										order,
										dest_vertex,
										accum
										),
										error ) )
					return error;
				}

			curr_edge = ns_model_edge_next( curr_edge );
			}

		/* NOTE: This condition must be true for this algorithm to
			work properly. */
		if( neighbors_of_same_order_count > 1 )
			ns_warning( NS_WARNING_LEVEL_RECOVERABLE NS_MODULE " Loop detected." );

		/* Search for neighbors of the current vertex of a different order. */
		curr_edge = ns_model_vertex_begin_edges( vertex );
		end_edges = ns_model_vertex_end_edges( vertex );

		did_add_entry = NS_FALSE;

		while( ns_model_edge_not_equal( curr_edge, end_edges ) )
			{
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) &&
				 order != ns_model_edge_get_order( curr_edge ) )
				{
				/* Case 3 or 4. If case 3 then the following if will execute. */
				if( ! did_add_entry && ! has_neighbors_of_same_order )
					{
					ns_assert( 0 < accum.count );

					if( NS_FAILURE( _ns_model_stats_entry_new(
											stats,
											conn_comp,
											order,
											accum.distance,
											accum.sum_diameters / accum.count
											),
											error ) )
						return error;
					else
						did_add_entry = NS_TRUE;
					}

				/* NOTE: We will be recursing down a branch of a different order,
					so we need to reset the following values. */
				accum.count         = 0;
				accum.distance      = 0.0f;
				accum.sum_diameters = 0.0f;

				if( NS_FAILURE( _ns_model_stats_do_create(
										stats,
										conn_comp,
										ns_model_edge_get_order( curr_edge ),
										dest_vertex,
										accum
										),
										error ) )
					return error;
				}

			curr_edge = ns_model_edge_next( curr_edge );
			}
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_model_stats_create( NsModelStats *stats, NsModel *model )
	{
   nsmodeltree        curr;
   nsmodeltree        end;
	nsmodelvertex      parent;
	nsmodelvertex      root;
   nsmodeledge        edge;
	NsModelStatsAccum  accum;
	NsError            error;


	ns_assert( NULL != stats );
	ns_assert( NULL != model );

	if( NS_FAILURE( ns_model_create_trees( model ), error ) )
		return error;

	ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_trees( model );
   end  = ns_model_end_trees( model );

   for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
      {
      parent = ns_model_tree_parent( curr );
		root   = ns_model_tree_root( curr );

      ns_model_vertex_set_color( parent, NS_MODEL_VERTEX_SEEN );
		ns_model_vertex_get_position( parent, &accum.last );

		accum.count         = 0;
		accum.distance      = 0;
		accum.sum_diameters = 0;

		edge = ns_model_vertex_find_edge( parent, root );
		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( parent ) ) );

		if( NS_FAILURE(
				_ns_model_stats_do_create(
					stats,
					ns_model_tree_conn_comp( curr ),
					ns_model_edge_get_order( edge ),
					ns_model_edge_dest_vertex( edge ),
					accum
					),
				error ) )
			return error;
      }

	return ns_no_error();
	}


NsError ns_model_stats_construct
	(
	NsModelStats  *stats,
	NsModel       *model,
	NsProgress    *progress
	)
	{
	NsError  error;
	nssize   conn_comp;


	NS_USE_VARIABLE( progress );

	ns_assert( NULL != stats );
	ns_assert( NULL != model );

	stats->num_conn_comp = ns_model_num_origins( model );

	if( NS_FAILURE( ns_byte_array_construct(
							&stats->buffer,
							stats->num_conn_comp * sizeof( NsList )
							),
							error ) )
		return error;

	for( conn_comp = 0; conn_comp < stats->num_conn_comp; ++conn_comp )
		ns_list_construct(
			_ns_model_stats_conn_comp_list( stats, conn_comp ),
			_ns_model_stats_entry_delete
			);

	if( NS_FAILURE( _ns_model_stats_create( stats, model ), error ) )
		{
		ns_model_stats_destruct( stats );
		return error;
		}

	return ns_no_error();
	}


void ns_model_stats_destruct( NsModelStats *stats )
	{
	nssize conn_comp;

	ns_assert( NULL != stats );

	for( conn_comp = 0; conn_comp < stats->num_conn_comp; ++conn_comp )
		ns_list_destruct( _ns_model_stats_conn_comp_list( stats, conn_comp ) );

	ns_byte_array_destruct( &stats->buffer );
	}


nssize ns_model_stats_num_conn_comp( const NsModelStats *stats )
	{
	ns_assert( NULL != stats );
	return stats->num_conn_comp;
	}


nslong ns_model_stats_min_order
	(
	const NsModelStats  *stats,
	nssize               conn_comp
	)
	{
	NsList             *list;
	nslong              min;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	min  = NS_INT_MAX;
	curr = ns_list_begin( list );
	end  = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( entry->order < min )
			min = entry->order;
		}

	return min;
	}


nslong ns_model_stats_max_order
	(
	const NsModelStats  *stats,
	nssize               conn_comp
	)
	{
	NsList             *list;
	nslong              max;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	max  = NS_INT_MIN;
	curr = ns_list_begin( list );
	end  = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( max < entry->order )
			max = entry->order;
		}

	return max;
	}


nssize ns_model_stats_num_edges
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	)
	{
	NsList             *list;
	nssize              count;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	count = 0;
	curr  = ns_list_begin( list );
	end   = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( order == entry->order )
			++count;
		}

	return count;
	}


nsfloat ns_model_stats_avg_length
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	)
	{
	NsList             *list;
	nsfloat             sum;
	nssize              count;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	sum   = 0;
	count = 0;
	curr  = ns_list_begin( list );
	end   = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( order == entry->order )
			{
			++count;
			sum += entry->length;
			}
		}

	if( 0 == count )
		return 0;

	return sum / count;
	}


nsfloat ns_model_stats_avg_diameter
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	)
	{
	NsList             *list;
	nsfloat             sum;
	nssize              count;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	sum   = 0;
	count = 0;
	curr  = ns_list_begin( list );
	end   = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( order == entry->order )
			{
			++count;
			sum += entry->diameter;
			}
		}

	if( 0 == count )
		return 0;

	return sum / count;
	}


nsfloat ns_model_stats_std_dev_length
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	)
	{
	NsList             *list;
	nsfloat             average;
	nsfloat             sum;
	nssize              count;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	average = ns_model_stats_avg_length( stats, conn_comp, order );
	sum     = 0;
	count   = 0;
	curr    = ns_list_begin( list );
	end     = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( order == entry->order )
			{
			++count;
			sum += NS_POW2( entry->length - average );
			}
		}

	if( 0 == count )
		return 0;

	return ns_sqrtf( sum / count );
	}


nsfloat ns_model_stats_std_dev_diameter
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	)
	{
	NsList             *list;
	nsfloat             average;
	nsfloat             sum;
	nssize              count;
	NsModelStatsEntry  *entry;
	nslistiter          curr;
	nslistiter          end;


	ns_assert( NULL != stats );
	ns_assert( conn_comp < stats->num_conn_comp );

	list = _ns_model_stats_conn_comp_list( stats, conn_comp );

	average = ns_model_stats_avg_diameter( stats, conn_comp, order );
	sum     = 0;
	count   = 0;
	curr    = ns_list_begin( list );
	end     = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		entry = ( NsModelStatsEntry* )ns_list_iter_get_object( curr );

		if( order == entry->order )
			{
			++count;
			sum += NS_POW2( entry->diameter - average );
			}
		}

	if( 0 == count )
		return 0;

	return ns_sqrtf( sum / count );
	}


#define _NS_MODEL_STATS_INT_WIDTH        ( ( nsint ) 6 )
#define _NS_MODEL_STATS_FLOAT_WIDTH      ( ( nsint )10 )
#define _NS_MODEL_STATS_FLOAT_PRECISION  ( ( nsint ) 2 )

NsError ns_model_stats_write( const NsModelStats *stats, const nschar *name )
	{
	NsFile              file;
	NsModelStatsEntry  *entry;
	nssize              conn_comp;
	NsList             *list;
	nslistiter          curr;
	nslistiter          end;
	nslong              min_order;
	nslong              max_order;
	nslong              order;
	NsError             error;


	ns_assert( NULL != stats );
	ns_assert( NULL != name );

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, name, NS_FILE_MODE_WRITE ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	if( NS_FAILURE( ns_file_print(
							&file,
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_NEWLINE,
							_NS_MODEL_STATS_INT_WIDTH,
							"CONCOM",
							_NS_MODEL_STATS_INT_WIDTH,
							"ORDER",
							_NS_MODEL_STATS_INT_WIDTH,
							"COUNT",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"AVGLEN",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"STDDEVLEN",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"AVGDIAM",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"STDDEVDIAM"
							),
							error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	for( conn_comp = 0; conn_comp < stats->num_conn_comp; ++conn_comp )
		{
		list = _ns_model_stats_conn_comp_list( stats, conn_comp );

		if( 0 < ns_list_size( list ) )
			{
			min_order = ns_model_stats_min_order( stats, conn_comp );
			max_order = ns_model_stats_max_order( stats, conn_comp );

			for( order = min_order; order <= max_order; ++order )
				if( NS_FAILURE( ns_file_print(
										&file,
										NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_ULONG )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_LONG )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_ULONG )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_NEWLINE,
										_NS_MODEL_STATS_INT_WIDTH,
										conn_comp,
										_NS_MODEL_STATS_INT_WIDTH,
										order,
										_NS_MODEL_STATS_INT_WIDTH,
										ns_model_stats_num_edges( stats, conn_comp, order ),
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										ns_model_stats_avg_length( stats, conn_comp, order ),
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										ns_model_stats_std_dev_length( stats, conn_comp, order ),
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										ns_model_stats_avg_diameter( stats, conn_comp, order ),
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										ns_model_stats_std_dev_diameter( stats, conn_comp, order )
										),
										error ) )
					{
					ns_file_destruct( &file );
					return error;
					}
			}
		}

	if( NS_FAILURE( ns_file_print(
							&file,
							NS_STRING_NEWLINE
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_TAB
							NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
							NS_STRING_NEWLINE,
							_NS_MODEL_STATS_INT_WIDTH,
							"CONCOM",
							_NS_MODEL_STATS_INT_WIDTH,
							"ORDER",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"LENGTH",
							_NS_MODEL_STATS_FLOAT_WIDTH,
							"AVGDIAM"
							),
							error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	for( conn_comp = 0; conn_comp < stats->num_conn_comp; ++conn_comp )
		{
		list = _ns_model_stats_conn_comp_list( stats, conn_comp );

		if( 0 < ns_list_size( list ) )
			{
			curr = ns_list_begin( list );
			end  = ns_list_end( list );

			for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
				{
				entry = ns_list_iter_get_object( curr );

				if( NS_FAILURE( ns_file_print(
										&file,
										NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_ULONG )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_LONG )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_TAB
										NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
										NS_STRING_NEWLINE,
										_NS_MODEL_STATS_INT_WIDTH,
										conn_comp,
										_NS_MODEL_STATS_INT_WIDTH,
										entry->order,
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										entry->length,
										_NS_MODEL_STATS_FLOAT_WIDTH, _NS_MODEL_STATS_FLOAT_PRECISION,
										entry->diameter
										),
										error ) )
					{
					ns_file_destruct( &file );
					return error;
					}
				}
			}
		}

	ns_file_destruct( &file );
	return ns_no_error();
	}
