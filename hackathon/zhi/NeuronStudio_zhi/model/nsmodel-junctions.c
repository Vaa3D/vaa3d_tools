#include "nsmodel-junctions.h"


#define NS_EDGE_LINE_NOT_JUNCTION( line )\
	( ! ns_model_vertex_is_junction( ns_model_edge_dest_vertex( (line)->edge ) ) )


typedef struct _NsEdgeLine
	{
	nsmodeledge  edge;
	NsLine3f     L;
	}
	NsEdgeLine;


NS_PRIVATE nsboolean ns_model_edge_line_compute_coord
	(
	NsEdgeLine     *line,
	nsmodelvertex   junction_vertex,
	NsVector3f     *coord
	)
	{
	nsmodelvertex  neighbor_vertex, dest_vertex;
	nsmodeledge    edge;


	ns_assert( ns_model_vertex_equal( junction_vertex, ns_model_edge_src_vertex( line->edge ) ) );
	neighbor_vertex = ns_model_edge_dest_vertex( line->edge );

	/* If the neighbor is a line vertex, return the position of the
		next vertex "after it" as the coordinate. */
	if( ! ns_model_vertex_is_origin( neighbor_vertex ) &&
		 ns_model_vertex_is_line( neighbor_vertex ) )
		{
		edge        = ns_model_vertex_begin_edges( neighbor_vertex );
		dest_vertex = ns_model_edge_dest_vertex( edge );

		if( ns_model_vertex_equal( dest_vertex, junction_vertex ) )
			{
			edge        = ns_model_edge_next( edge );
			dest_vertex = ns_model_edge_dest_vertex( edge );
			}

		ns_model_vertex_get_position( dest_vertex, coord );
		return NS_TRUE;
		}

	return NS_FALSE;
	}


NS_PRIVATE void ns_model_compute_junction_vertex_edge_lines
	(
	nsmodelvertex    junction_vertex,
	NsEdgeLine      *lines,
	nssize           num_lines
	)
	{
	nsmodeledge  curr_edge, end_edges;
	NsVector3f   C, N, D, P;
	nssize       index;


	ns_assert( num_lines == ns_model_vertex_num_edges( junction_vertex ) );

	ns_model_vertex_get_position( junction_vertex, &P );

	index     = 0;
	curr_edge = ns_model_vertex_begin_edges( junction_vertex );
	end_edges = ns_model_vertex_end_edges( junction_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &N );

		lines[ index ].edge = curr_edge;

		/* If cant find a next position then just use the direction from the junction
			vertex to this neighbor(the destination vertex of the current edge). */

		if( ns_model_edge_line_compute_coord( lines + index, junction_vertex, &C ) )
			ns_vector3f_sub( &D, &N, &C );
		else
			ns_vector3f_sub( &D, &P, &N );

		ns_vector3f_norm( &D );

		/* And finally make the line segment. */
		lines[ index ].L.P1 = N;		
		ns_vector3f_add( &(lines[ index ].L.P2), &N, &D );

		++index;
		}

	ns_assert( index == num_lines );
	}


NS_PRIVATE NsError ns_model_reposition_junction_vertex
	(
	NsModel           *model,
	nsmodelvertex      junction_vertex,
	const NsSettings  *settings,
	nssize             iterations
	);


NS_PRIVATE NsError ns_junction_vertex_project_to_closest
	(
	NsModel           *model,
	nsmodelvertex      junction_vertex,
	NsEdgeLine        *lines,
	nssize             num_lines,
	const NsSettings  *settings,
	nssize             iterations
	)
	{
	nssize         l, count;
	nsfloat        distance, min_distance;
	NsVector3f     P, A, N, C;
	nsfloat        real_u, u;
	nsmodelvertex  closest_vertex, dest_vertex;
	nsboolean      is_junction;
	NsError        error;


	ns_model_vertex_get_position( junction_vertex, &P );

	count          = 0;
	min_distance   = NS_FLOAT_MAX;
	real_u         = 1.0f;
	closest_vertex = NS_MODEL_VERTEX_NIL;

	for( l = 0; l < num_lines; ++l )
		{
		dest_vertex = ns_model_edge_dest_vertex( lines[l].edge );
		ns_model_vertex_get_position( dest_vertex, &N );

		distance = ns_vector3f_distance( &P, &N );

		if( distance < min_distance )
			{
			u = ns_point3f_project_onto_line( &P, &(lines[l].L), &A );

			min_distance   = distance;
			C              = A;
			real_u         = u;
			closest_vertex = dest_vertex;

			++count;
			}
		}

	if( 0 < count )
		{
		/* If the junction moves "behind its closest neighbor... */
		if( real_u <= 0.0f )
			{
			is_junction = ns_model_vertex_is_junction( closest_vertex );

			ns_model_vertex_set_position(
				junction_vertex,
				ns_model_vertex_get_position( closest_vertex, &C )
				);

			if( NS_FAILURE( ns_model_merge_vertices( model, junction_vertex, closest_vertex ), error ) )
				return error;

			/* Run this junction again if the one it absorbed was also a junction. */
			if( is_junction )
				if( NS_FAILURE(
						ns_model_reposition_junction_vertex(
							model,
							junction_vertex,
							settings,
							iterations
							),
						error ) )
					return error;
			}
		else if( ! ns_model_vertex_is_junction( closest_vertex ) )
			ns_model_vertex_set_position( junction_vertex, &C );
		}

	return ns_no_error();
	}
 

NS_PRIVATE NsError ns_model_reposition_junction_vertex
	(
	NsModel           *model,
	nsmodelvertex      junction_vertex,
	const NsSettings  *settings,
	nssize             iterations
	)
	{
	NsError      error;
	NsEdgeLine  *lines;
	nssize       num_lines;
	nssize       i, l;
	NsVector3f   O, P, C, A;
	nsfloat      radius;
	nssize       count;


	ns_assert( ! ns_model_vertex_is_origin( junction_vertex ) );
	ns_assert( ns_model_vertex_is_junction( junction_vertex ) );

	error     = ns_no_error();
	lines     = NULL;
	num_lines = ns_model_vertex_num_edges( junction_vertex );

	if( NULL == ( lines = ns_new_array( NsEdgeLine, num_lines ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_MODEL_REPOSITION_JUNCTION_VERTEX_EXIT;
		}

	ns_model_compute_junction_vertex_edge_lines( junction_vertex, lines, num_lines );

	ns_model_vertex_get_position( junction_vertex, &O );

	radius  = ns_model_vertex_get_radius( junction_vertex );
	radius *= ns_settings_get_neurite_vertex_radius_scalar( settings );

	for( i = 0; i < iterations; ++i )
		{
		ns_model_vertex_get_position( junction_vertex, &P );

		ns_vector3f_zero( &C );
		count = 0;

		for( l = 0; l < num_lines; ++l )
			if( NS_EDGE_LINE_NOT_JUNCTION( lines + l ) )
				{
				ns_point3f_project_onto_line( &P, &(lines[l].L), &A );
				ns_vector3f_cmpd_add( &C, &A );

				++count;
				}

		if( 0 < count )
			{
			ns_vector3f_cmpd_scale( &C, 1.0f / ( ( nsfloat )count ) );
			ns_model_vertex_set_position( junction_vertex, &C );

		/*
			ns_vector3f_sub( &D, &C, &O );
			distance = ns_vector3f_mag( &D );

			// Restrict how "far" the junction vertex can move.
			if( distance > radius )
				{
				factor = radius / distance;

				C.x = O.x + D.x * factor;
				C.y = O.y + D.y * factor;
				C.z = O.z + D.z * factor;

				ns_model_vertex_set_position( junction_vertex, &C );

				break;
				}
			else*/
			if( ns_vector3f_distance( &P, &C ) / ns_model_vertex_get_radius( junction_vertex ) * 100.0f < 0.5f )
				break;
			}
		}

	if( NS_FAILURE(
			ns_junction_vertex_project_to_closest(
				model,
				junction_vertex,
				lines,
				num_lines,
				settings,
				iterations
				),
			error ) )
		goto _NS_MODEL_REPOSITION_JUNCTION_VERTEX_EXIT;

	_NS_MODEL_REPOSITION_JUNCTION_VERTEX_EXIT:

	ns_delete( lines );
	return error;
	}


NsError ns_model_reposition_junction_vertices
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	)
	{
	nsmodelvertex  curr, end;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != settings );
	ns_assert( NULL != params );

	ns_progress_set_title( progress, "Positioning junction vertices..." );
	ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
	ns_progress_begin( progress );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		if( ! ns_model_vertex_is_manually_traced( curr ) &&
			 ! ns_model_vertex_is_origin( curr ) &&
			 ns_model_vertex_is_junction( curr ) )
			if( NS_FAILURE(
					ns_model_reposition_junction_vertex(
						model,
						curr,
						settings,
						params->reposition_iterations
						),
					error ) )
				return error;

		ns_progress_next_iter( progress );
		}

	ns_progress_end( progress );

	return ns_no_error();
	}


NS_PRIVATE NsError ns_model_resize_junction_vertex
	(
	nsmodelvertex               junction_vertex,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsSampler                  *sampler
	)
	{
	NsVector3i  jitter;
	NsVector3f  P;
	nsfloat     radius;
	nsfloat     length;
	nsboolean   did_create_sample;
	NsError     error;


	ns_assert( ! ns_model_vertex_is_origin( junction_vertex ) );
	ns_assert( ns_model_vertex_is_junction( junction_vertex ) );

	ns_model_vertex_get_position( junction_vertex, &P );

	ns_vector3i_zero( &jitter );

	if( NS_FAILURE(
			ns_sampler_run(
				sampler,
				params->volume,
				&P,
				&P,
				0,
				NS_FALSE,
				NS_RAYBURST_KERNEL_SUPER_VIZ,
				NS_RAYBURST_INTERP_BILINEAR,
				ns_settings_voxel_info( settings ),
				params->volume_average_intensity,
				&jitter,
				params->grafting_use_2d_sampling,
				params->grafting_aabbox_scalar,
				params->grafting_min_window,
				ns_settings_get_threshold_use_fixed( settings ),
				ns_settings_get_threshold_fixed_value( settings ),
				&radius,
				&length,
				&did_create_sample
				),
			error ) )
		return error;

	ns_model_vertex_set_radius( junction_vertex, radius );

	return ns_no_error();
	}


NsError ns_model_resize_junction_vertices
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	)
	{
	NsSampler      sampler;
	nsmodelvertex  curr, end;
	NsError        error;


	ns_assert( NULL != model );
	ns_assert( NULL != settings );
	ns_assert( NULL != params );

	error = ns_no_error();

	ns_sampler_construct( &sampler );

   ns_sampler_init(
      &sampler,
      ns_settings_voxel_info( settings ),
      params->volume,
      params->proj_xy,
      params->proj_zy,
      params->proj_xz
      );

	ns_progress_set_title( progress, "Resizing junction vertices..." );
	ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
	ns_progress_begin( progress );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		if( ! ns_model_vertex_is_manually_traced( curr ) &&
			 ! ns_model_vertex_is_origin( curr ) &&
			 ns_model_vertex_is_junction( curr ) )
			if( NS_FAILURE( ns_model_resize_junction_vertex( curr, settings, params, &sampler ), error ) )
				goto _NS_MODEL_RESIZE_JUNCTION_VERTICES_EXIT;

		ns_progress_next_iter( progress );
		}

	ns_progress_end( progress );

	_NS_MODEL_RESIZE_JUNCTION_VERTICES_EXIT:

	ns_sampler_destruct( &sampler );
	return error;
	}
