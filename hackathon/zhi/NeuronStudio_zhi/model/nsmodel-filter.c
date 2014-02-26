#include "nsmodel-filter.h"


NS_PRIVATE void _ns_model_filter_do_smooth( NsModel *model, NsProgress *progress )
	{
	nsmodelvertex  curr, end, dest;
	nsmodeledge    edge;
	//NsVector3f     V1, V2, A;
	nsfloat        r1, r2;


	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		if( ! ns_model_vertex_is_manually_traced( curr ) && ! ns_model_vertex_is_origin( curr ) )
			{
			if( ns_model_vertex_is_line( curr ) )
				{
				edge = ns_model_vertex_begin_edges( curr );
				dest = ns_model_edge_dest_vertex( edge );
				ns_assert( ! ns_model_vertex_is_manually_traced( dest ) );

				//ns_model_vertex_get_position( dest, &V1 );
				r1 = ns_model_vertex_get_radius( dest );

				edge = ns_model_edge_next( edge );
				dest = ns_model_edge_dest_vertex( edge );
				ns_assert( ! ns_model_vertex_is_manually_traced( dest ) );

				//ns_model_vertex_get_position( dest, &V2 );
				r2 = ns_model_vertex_get_radius( dest );

				/* 2007-08-13 Removed smoothing of positions to avoid artifacts. */

				/* The new position is the average of the 2 neighbors positions. */
				/*ns_model_vertex_set_position(
					curr,
					ns_vector3f_cmpd_scale( ns_vector3f_add( &A, &V1, &V2 ), .5f )
					);*/

				/* The new radius if the average of this vertex and the neighbors radii. */
				ns_model_vertex_set_radius(
					curr,
					( ns_model_vertex_get_radius( curr ) + r1 + r2 ) / 3.0f
					);
				}
			else if( ns_model_vertex_is_external( curr ) )
				{
				/* NOTE: Dont alter external vertices' position. */
				r1 = ns_model_vertex_get_radius( curr );

				edge = ns_model_vertex_begin_edges( curr );
				dest = ns_model_edge_dest_vertex( edge );
				ns_assert( ! ns_model_vertex_is_manually_traced( dest ) );

				r2 = ns_model_vertex_get_radius( dest );

				ns_model_vertex_set_radius( curr, ( r1 + r2 ) / 2.0f );
				}
			}

		ns_progress_next_iter( progress );
		}
	}


/*
void ns_model_filter_smooth
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   )
   {
   nssize i;

   ns_progress_set_title( progress, "Smoothing model..." );

   ns_progress_num_iters(
      progress,
      ns_model_num_vertices( model ) * ns_settings_get_neurite_smooth_count( settings )
      );

   ns_progress_begin( progress );

   for( i = 0; i < ns_settings_get_neurite_smooth_count( settings ); ++i )
      {
      if( ns_progress_cancelled( progress ) )
         return;

      _ns_model_filter_do_smooth( model, progress );
      }

   ns_progress_end( progress );
   }
*/


NS_PRIVATE NsError _ns_model_filter_discretize_origins
   (
   NsModel     *model,
   nsfloat      radius_scalar,
   NsProgress  *progress
   )
   {
   nsmodelorigin  curr_origin;
   nsmodelorigin  end_origins;
   nsmodelvertex  origin_vertex;
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   NsVector3f     V1;
   NsVector3f     V2;
   nsfloat        radius;
   nsfloat        scaled_radius;
   NsError        error;


   NS_USE_VARIABLE( progress );

   curr_origin = ns_model_begin_origins( model );
   end_origins = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr_origin, end_origins );
          curr_origin = ns_model_origin_next( curr_origin ) )
		{
		origin_vertex = ns_model_origin_vertex( curr_origin );

		if( ! ns_model_vertex_is_manually_traced( origin_vertex ) )
			{
			ns_model_vertex_get_position( origin_vertex, &V1 );
			radius = ns_model_vertex_get_radius( origin_vertex );

			scaled_radius = radius * radius * radius_scalar;

			_NS_MODEL_DISCRETIZE_ORIGINS_REEXAMINE:

			curr_edge = ns_model_vertex_begin_edges( origin_vertex );
			end_edges = ns_model_vertex_end_edges( origin_vertex );

			for( ; ns_model_edge_not_equal( curr_edge, end_edges );
					 curr_edge = ns_model_edge_next( curr_edge ) )
				{
				dest_vertex = ns_model_edge_dest_vertex( curr_edge );
				ns_assert( ! ns_model_vertex_is_manually_traced( dest_vertex ) );

				ns_model_vertex_get_position( dest_vertex, &V2 );

				/* NOTE: The origin can remove anything. */
				if( NS_DISTANCE_SQUARED( V1.x, V1.y, V1.z, V2.x, V2.y, V2.z ) < scaled_radius )
					{
					if( NS_FAILURE( ns_model_merge_vertices( model, origin_vertex, dest_vertex ), error ) )
						return error;

					goto _NS_MODEL_DISCRETIZE_ORIGINS_REEXAMINE;
					}
				}
			}
		}

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_filter_discretize_junctions
   (
   NsModel     *model,
   nsfloat      radius_scalar,
   NsProgress  *progress
   )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   NsVector3f     V1;
   NsVector3f      V2;
   nsfloat        radius;
   nsfloat        scaled_radius;
   NsError        error;


	if( NULL != progress )
		{
		ns_progress_set_title( progress, "Discretizing junction vertices..." );
	   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
		ns_progress_begin( progress );
		}

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      if( ! ns_model_vertex_is_manually_traced( curr_vertex ) &&
			 ! ns_model_vertex_is_origin( curr_vertex ) &&
          ns_model_vertex_is_junction( curr_vertex ) )
         {
         ns_model_vertex_get_position( curr_vertex, &V1 );
         radius = ns_model_vertex_get_radius( curr_vertex );
      
         scaled_radius = radius * radius * radius_scalar;

         _NS_MODEL_DISCRETIZE_JUNCTIONS_REEXAMINE:

         curr_edge = ns_model_vertex_begin_edges( curr_vertex );
         end_edges = ns_model_vertex_end_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge, end_edges );
                curr_edge = ns_model_edge_next( curr_edge ) )
            {
            dest_vertex = ns_model_edge_dest_vertex( curr_edge );
				ns_assert( ! ns_model_vertex_is_manually_traced( dest_vertex ) );

				/* NOTE: Only allow removal of line vertices. */
				if( ! ns_model_vertex_is_origin( dest_vertex ) &&
					 ns_model_vertex_is_line( dest_vertex ) )
					{
					ns_model_vertex_get_position( dest_vertex, &V2 );

					if( NS_DISTANCE_SQUARED( V1.x, V1.y, V1.z, V2.x, V2.y, V2.z ) < scaled_radius )
						{
						if( NS_FAILURE( ns_model_merge_vertices( model, curr_vertex, dest_vertex ), error ) )
							return error;

						goto _NS_MODEL_DISCRETIZE_JUNCTIONS_REEXAMINE;
						}
					}
            }
         }

		if( NULL != progress )
			ns_progress_next_iter( progress );
      }

	if( NULL != progress )
		ns_progress_end( progress );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_filter_do_discretize_lines
   (
   NsModel        *model,
   nsmodelvertex   curr_vertex,
   nsfloat         radius_scalar,
   NsProgress     *progress
   )
   {
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   NsVector3f     V1;
   NsVector3f     V2;
   nsfloat        radius;
   nsfloat        scaled_radius;
   NsError        error;


   if( NS_MODEL_VERTEX_SEEN == ns_model_vertex_get_color( curr_vertex ) )
      return ns_no_error();

	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );
   ns_model_vertex_set_color( curr_vertex, NS_MODEL_VERTEX_SEEN );

	if( NULL != progress )
		ns_progress_next_iter( progress );

   if( ! ns_model_vertex_is_origin( curr_vertex ) &&
       ns_model_vertex_is_line( curr_vertex ) )
      {
      ns_model_vertex_get_position( curr_vertex, &V1 );
      radius = ns_model_vertex_get_radius( curr_vertex );
      
      scaled_radius = radius * radius * radius_scalar;

      _NS_MODEL_DISCRETIZE_LINES_REEXAMINE:

      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
         {
         dest_vertex = ns_model_edge_dest_vertex( curr_edge );
			ns_assert( ! ns_model_vertex_is_manually_traced( dest_vertex ) );

         /* NOTE: Only allow removal of line vertices. */
         if( NS_MODEL_VERTEX_UNSEEN == ns_model_vertex_get_color( dest_vertex ) &&
             ! ns_model_vertex_is_origin( dest_vertex ) &&
             ns_model_vertex_is_line( dest_vertex ) )
            {
            ns_model_vertex_get_position( dest_vertex, &V2 );

            if( NS_DISTANCE_SQUARED( V1.x, V1.y, V1.z, V2.x, V2.y, V2.z ) < scaled_radius )
               {
               if( NS_FAILURE( ns_model_merge_vertices( model, curr_vertex, dest_vertex ), error ) )
                  return error;

               goto _NS_MODEL_DISCRETIZE_LINES_REEXAMINE;
               }
            }
         }
      }

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      if( NS_FAILURE( _ns_model_filter_do_discretize_lines(
                        model,
                        ns_model_edge_dest_vertex( curr_edge ),
                        radius_scalar,
                        progress
                        ),
                        error ) )
         return error;

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_filter_discretize_lines
   (
   NsModel     *model,
   nsfloat      radius_scalar,
   NsProgress  *progress
   )
   {
   nsmodelorigin  curr;
   nsmodelorigin  end;
	nsmodelvertex  origin;
   NsError        error;


	if( NULL != progress )
		{
		ns_progress_set_title( progress, "Discretizing line vertices..." );
		ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
		ns_progress_begin( progress );
		}

   ns_model_color_vertices( model, NS_MODEL_VERTEX_UNSEEN );

   curr = ns_model_begin_origins( model );
   end  = ns_model_end_origins( model );

   for( ; ns_model_origin_not_equal( curr, end ); curr = ns_model_origin_next( curr ) )
		{
		origin = ns_model_origin_vertex( curr );

		if( ! ns_model_vertex_is_manually_traced( origin ) )
			if( NS_FAILURE( _ns_model_filter_do_discretize_lines(
									model,
									origin,
									radius_scalar,
									progress
									),
									error ) )
				return error;
		}

	if( NULL != progress )
		ns_progress_end( progress );

   return ns_no_error();
   }


NsError ns_model_filter_discretize
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   )
   {
   NsError error;

	ns_assert( NULL != model );
	ns_assert( NULL != settings );

	error = ns_no_error();

	ns_model_lock( model );

   if( NULL != progress && ns_progress_cancelled( progress ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

   if( NS_FAILURE(
			_ns_model_filter_discretize_origins(
				model,
				ns_settings_get_neurite_origin_radius_scalar( settings ),
				progress
				),
			error ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

   if( NULL != progress && ns_progress_cancelled( progress ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

   if( NS_FAILURE(
			_ns_model_filter_discretize_junctions(
				model,
				ns_settings_get_neurite_vertex_radius_scalar( settings ),
				progress
				),
			error ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

   if( NULL != progress && ns_progress_cancelled( progress ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

   if( NS_FAILURE(
			_ns_model_filter_discretize_lines(
				model,
				ns_settings_get_neurite_vertex_radius_scalar( settings ),
				progress
				),
			error ) )
      goto _NS_MODEL_FILTER_DISCRETIZE_EXIT;

	_NS_MODEL_FILTER_DISCRETIZE_EXIT:

	ns_model_unlock( model );
	return error;
   }


void ns_model_filter_fix_small_radii
	(
	NsModel           *model,
	const NsSettings  *settings
	)
   {
   nsmodelvertex  curr, end;
   nsfloat        tolerance;


   tolerance = ns_voxel_info_min_size( ns_settings_voxel_info( settings ) ) / 2.0f;

   /*TEMP*/ns_println( "tolerance = " NS_FMT_DOUBLE, tolerance );

	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		if( ! ns_model_vertex_is_manually_traced( curr ) )
			if( ns_model_vertex_get_radius( curr ) < tolerance )
				ns_model_vertex_set_radius( curr, tolerance );
   }


NS_PRIVATE void _ns_model_rec_do_average_thresholds_and_contrasts
   (
   const NsVector3f  *start,
   nsmodelvertex      parent_vertex,
   nsmodelvertex      curr_vertex,
   nsfloat            scaled_radius,
   nsfloat           *sum_thresholds,
   nsfloat           *sum_contrasts,
   nssize            *count
   )
   {
   NsVector3f     position;
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );
   ns_model_vertex_get_position( curr_vertex, &position );

   if( NS_DISTANCE_SQUARED( start->x, start->y, start->z, position.x, position.y, position.z )
       < scaled_radius )
      {
      (*sum_thresholds) += ns_model_vertex_get_threshold(
                  curr_vertex,
                  NS_MODEL_VERTEX_NEURITE_THRESHOLD
                  );

      (*sum_contrasts) += ns_model_vertex_get_contrast(
                  curr_vertex,
                  NS_MODEL_VERTEX_NEURITE_CONTRAST
                  );

      ++(*count);
      }
   else
      return;

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );
		ns_assert( ! ns_model_vertex_is_manually_traced( dest_vertex ) );

      /* Dont backtrack along parent edge! */
      if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
         _ns_model_rec_do_average_thresholds_and_contrasts(
            start,
            curr_vertex,
            dest_vertex,
            scaled_radius,
            sum_thresholds,
				sum_contrasts,
				count
            );   
      }   
   }


NS_PRIVATE void _ns_model_do_average_thresholds_and_contrasts
   (
   nsmodelvertex      curr_vertex,
   const NsSettings  *settings,
   nsfloat           *sum_thresholds,
   nsfloat           *sum_contrasts,
   nssize            *count
   )
   {
   nsfloat        radius_scalar;
   nsfloat        radius;
   nsfloat        scaled_radius;
   NsVector3f     start;
   nsmodelvertex  dest_vertex;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


	ns_assert( ! ns_model_vertex_is_manually_traced( curr_vertex ) );

   (*sum_thresholds) +=
			ns_model_vertex_get_threshold(
               curr_vertex,
               NS_MODEL_VERTEX_NEURITE_THRESHOLD
               );

   (*sum_contrasts) +=
			ns_model_vertex_get_contrast(
               curr_vertex,
               NS_MODEL_VERTEX_NEURITE_CONTRAST
               );

   ++(*count);

   radius_scalar =
      ns_model_vertex_is_origin( curr_vertex ) ?
         ns_settings_get_neurite_origin_radius_scalar( settings ) :
         ns_settings_get_neurite_vertex_radius_scalar( settings );

   radius = ns_model_vertex_get_radius( curr_vertex );
   ns_model_vertex_get_position( curr_vertex, &start );

   scaled_radius = radius * radius * radius_scalar;

   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
      {
      dest_vertex = ns_model_edge_dest_vertex( curr_edge );
		ns_assert( ! ns_model_vertex_is_manually_traced( dest_vertex ) );

      _ns_model_rec_do_average_thresholds_and_contrasts(
         &start,
         curr_vertex,
         dest_vertex,
         scaled_radius,
         sum_thresholds,
			sum_contrasts,
			count
         );   
      }
   }


/*
void ns_model_filter_average_thresholds_and_contrasts
   (
   NsModel           *model,
   const NsSettings  *settings,
   NsProgress        *progress
   )
   {
   nsmodelvertex  curr, end;
   nsfloat        sum_thresholds;
   nsfloat        sum_contrasts;
   nssize         count;


   ns_progress_set_title( progress, "Averaging vertex thresholds and contrasts..." );
   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
   ns_progress_begin( progress );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   /* First compute the average threshold for each vertex. Dont overwrite
      the actual neurite threshold yet. *//*
   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      {
		if( ! ns_model_vertex_is_manually_traced( curr ) )
			{
			sum_thresholds = 0.0f;
			sum_contrasts  = 0.0f;
			count          = 0;

			_ns_model_do_average_thresholds_and_contrasts(
				curr,
				settings,
				&sum_thresholds,
				&sum_contrasts,
				&count
				);

			/* NOTE: The 'if' statement is to shut up the compiler from warning
				us about a potential divide by zero. *//*
			if( 0 < count )
				{
				ns_assert( 1 <= count );

				ns_model_vertex_set_threshold(
					curr,
					NS_MODEL_VERTEX_AVERAGE_THRESHOLD,
					sum_thresholds / ( nsfloat )count
					);

				ns_model_vertex_set_contrast(
					curr,
					NS_MODEL_VERTEX_AVERAGE_CONTRAST,
					sum_contrasts / ( nsfloat )count
					);
				}
			}

      ns_progress_next_iter( progress );
      }

   ns_progress_end( progress );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   /* Now actually overwrite the neurite thresholds. *//*
   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		if( ! ns_model_vertex_is_manually_traced( curr ) )
			{
			ns_model_vertex_set_threshold(
				curr,
				NS_MODEL_VERTEX_NEURITE_THRESHOLD,
				ns_model_vertex_get_threshold(
					curr,
					NS_MODEL_VERTEX_AVERAGE_THRESHOLD
					)
				);

			ns_model_vertex_set_contrast(
				curr,
				NS_MODEL_VERTEX_NEURITE_CONTRAST,
				ns_model_vertex_get_contrast(
					curr,
					NS_MODEL_VERTEX_AVERAGE_CONTRAST
					)
				);
			}
   }
*/


NS_PRIVATE void _ns_model_filter_invalidate_origins( NsModel *model )
	{
	nslistiter              curr, end;
	const NsModelConnComp  *cc;
	nssize                  count;
	nsdouble                sum;
	nsdouble                average;
	nsmodelvertex           vertex;


	curr = ns_model_begin_conn_comps( model );
	end  = ns_model_end_conn_comps( model );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		cc = ns_list_iter_get_object( curr );

		if( ! ns_model_vertex_is_manually_traced( cc->origin ) )
			{
			count = 0;
			sum   = 0.0;

			/* Compute the average radius of all vertices, except the origin. */

			for( vertex = cc->list;
				  ns_model_vertex_not_equal( NS_MODEL_VERTEX_NIL, vertex );
				  vertex = ns_model_vertex_get_list( vertex ) )
				if( ! ns_model_vertex_is_origin( vertex ) )
					{
					ns_assert( ! ns_model_vertex_is_manually_traced( vertex ) );

					++count;
					sum += ( nsdouble )ns_model_vertex_get_radius( vertex );
					}

			/* If this origin is not "significantly" bigger than the average,
				mark it as invalid. */

			average = sum / ( nsdouble )count;

			if( ns_model_vertex_get_radius( cc->origin ) < NS_MODEL_ORIGIN_SCALAR * average )
				ns_model_vertex_mark_valid( cc->origin, NS_FALSE );
			}
		}
	}


NS_PRIVATE nsmodelvertex _ns_model_filter_find_largest_external_vertex( NsModel *model, nssize conn_comp )
   {
   nsmodelvertex  curr, end, largest;
   nsfloat        radius, max_radius;


   max_radius = 0.0f;
   largest    = NS_MODEL_VERTEX_NIL;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      if( ns_model_vertex_get_conn_comp( curr ) == conn_comp )
			{
			ns_assert( ! ns_model_vertex_is_manually_traced( curr ) );

         if( ns_model_vertex_is_external( curr ) )
            {
            radius = ns_model_vertex_get_radius( curr );

            if( max_radius < radius )
               {
               max_radius = radius;
               largest    = curr;
               }
            }
			}

   ns_assert( ns_model_vertex_not_equal( largest, NS_MODEL_VERTEX_NIL ) );
   return largest;
   }


NS_PRIVATE void _ns_model_filter_validate_origins( NsModel *model )
   {
   nsmodelorigin  origin;
   nsmodelvertex  old_vertex, new_vertex;
   nssize         conn_comp;


   NS_MODEL_ORIGIN_FOREACH( model, origin )
      {
      old_vertex = ns_model_origin_vertex( origin );

      if( ! ns_model_vertex_is_manually_traced( old_vertex ) &&
			 ! ns_model_vertex_is_valid( old_vertex ) )
         {
         conn_comp = ns_model_vertex_get_conn_comp( old_vertex );

         new_vertex = _ns_model_filter_find_largest_external_vertex( model, conn_comp );
			ns_assert( ! ns_model_vertex_is_manually_traced( new_vertex ) );

         ns_model_vertex_mark_valid( old_vertex, NS_TRUE );

         ns_assert( ns_model_vertex_is_valid( new_vertex ) );
         ns_model_reset_origin( model, origin, new_vertex );
         }
      }
   }


#include "nsmodel-filter-prune.inl"


/*
NS_PRIVATE NsError ns_model_filter_calc_diameters
	(
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
	)
	{
	NsRayburst     rayburst;
	nsmodelvertex  curr, end;
	NsVector3f     position;
	NsError        error;


   if( NS_FAILURE(
			ns_rayburst_construct(
				&rayburst,
				ns_settings_get_neurite_do_3d_radii( settings ) ?
					NS_RAYBURST_KERNEL_STD : NS_RAYBURST_KERNEL_2D,
				NS_RAYBURST_RADIUS_MLBD,
				params->rayburst_interp_type,
				params->volume,
				ns_settings_voxel_info( settings ),
				NULL
				),
         error ) )
      return error;

	ns_progress_set_title( progress, "Computing vertex diameters..." );
	ns_progress_num_iters( progress, ns_model_num_vertices( model ) );
	ns_progress_begin( progress );

	curr = ns_model_begin_vertices( model );
	end  = ns_model_end_vertices( model );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		if( ! ns_model_vertex_is_manually_traced( curr ) )
			{
			ns_model_vertex_get_position( curr, &position );

			ns_rayburst_set_threshold(
				&rayburst,
				ns_model_vertex_get_threshold( curr, NS_MODEL_VERTEX_NEURITE_THRESHOLD )
				);
		
			ns_rayburst( &rayburst, &position );

			ns_model_vertex_set_radius( curr, ns_rayburst_radius( &rayburst ) );
			}

	ns_progress_end( progress );

	ns_rayburst_destruct( &rayburst );

	return ns_no_error();
	}
*/


NsError ns_model_filter
   (
	NsModel                    *model,
	const NsSettings           *settings,
	const NsModelFilterParams  *params,
	NsProgress                 *progress
   )
   {
   NsError error;

   ns_assert( NULL != model );
   ns_assert( NULL != settings );
	ns_assert( NULL != params );

   ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( model=" NS_FMT_STRING_DOUBLE_QUOTED
		", settings=" NS_FMT_POINTER
		", progress=" NS_FMT_POINTER
		" )",
		ns_model_get_name( model ),
		settings,
		progress
		);

	error = ns_no_error();


	/* IMPORTANT: Windows has some sort of problem with the painting of the progress
		bar and this lock, so lets just NULL it out so that no updating occurs.
		These operations are usually pretty quick anyway, so updating a progress bar isnt
		really necessary. */
	progress = NULL;
	ns_model_lock( model );


	ns_model_needs_edge_centers( model );

	ns_model_filter_fix_small_radii( model, settings );


	ns_progress_set_title( progress, "Creating Connected Components..." );
	if( NS_FAILURE( ns_model_create_conn_comps( model ), error ) )
		goto _NS_MODEL_FILTER_EXIT;


	_ns_model_filter_invalidate_origins( model );
	if( ns_progress_cancelled( progress ) )
		goto _NS_MODEL_FILTER_EXIT;


	if( NS_FAILURE( ns_model_filter_prune( model, settings, progress ), error ) )
      goto _NS_MODEL_FILTER_EXIT;
	if( ns_progress_cancelled( progress ) )
		goto _NS_MODEL_FILTER_EXIT; 


	//ns_model_filter_smooth( model, settings, progress );
	//if( ns_progress_cancelled( progress ) )
	//	goto _NS_MODEL_FILTER_EXIT;


	//ns_model_filter_average_thresholds_and_contrasts( model, settings, progress );
	//if( ns_progress_cancelled( progress ) )
	//	goto _NS_MODEL_FILTER_EXIT;


	_ns_model_filter_validate_origins( model );
	if( ns_progress_cancelled( progress ) )
		goto _NS_MODEL_FILTER_EXIT;


	_NS_MODEL_FILTER_EXIT:


	ns_model_unlock( model );

	/* Dont need the seeds that were passed from the raw model to the filtered model anymore.
		This routine has its own synchronization. */
	ns_model_clear_seeds( model );

	if( ns_is_error( error ) )
		return error;


	/* This doesnt remove anything, so dont have to synchronize. */
	/*if( NS_FAILURE( ns_model_filter_calc_diameters( model, settings, params, progress ), error ) )
		return error;
	if( ns_progress_cancelled( progress ) )
		return ns_no_error();*/


	/* NOTE: The discretize filter has its own synchronization. */
	if( NS_FAILURE( ns_model_filter_discretize( model, settings, progress ), error ) )
		return error;
	if( ns_progress_cancelled( progress ) )
		return ns_no_error();


	if( ns_settings_get_neurite_realign_junctions( settings ) )
		{
		/* This doesnt remove anything, so dont have to synchronize. */
		if( NS_FAILURE( ns_model_reposition_junction_vertices( model, settings, params, progress ), error ) )
			return error;
		if( ns_progress_cancelled( progress ) )
			return ns_no_error();


		/* This doesnt remove anything, so dont have to synchronize. */
		if( NS_FAILURE( ns_model_resize_junction_vertices( model, settings, params, progress ), error ) )
			return error;
		if( ns_progress_cancelled( progress ) )
			return ns_no_error();


		/* NOTE: Have to re-discretize the junction vertices since they may have moved.
			Again this has its own synchronization. */
		if( NS_FAILURE(
				_ns_model_filter_discretize_junctions(
					model,
					ns_settings_get_neurite_vertex_radius_scalar( settings ),
					progress
					),
				error ) )
			return error;
		}


	ns_model_filter_fix_small_radii( model, settings );

   return ns_no_error();
   }


void ns_model_filter_adjust_shrinkage( NsModel *model, const NsVector3f *percent )
	{
	NsVector3f     P;
	nsfloat        d;
	nsmodelvertex  vertex;
	NsVector3f     position;
	nsfloat        diameter;


	ns_assert( NULL != model );
	ns_assert( NULL != percent );

	/* Formula by Christina Weaver, christina.weaver@mssm.edu (September 2005) */
	P.x = 100.0f / ( 100.0f - percent->x );
	P.y = 100.0f / ( 100.0f - percent->y );
	P.z = 100.0f / ( 100.0f - percent->z );
	d   = 0.5f * ( P.x + P.y );

	NS_MODEL_VERTEX_FOREACH( model, vertex )
		{
		ns_model_vertex_get_position( vertex, &position );
		diameter = ns_model_vertex_get_radius( vertex ) * 2.0f;

		ns_vector3f_cmpd_non_uni_scale( &position, &P );
		diameter *= d;

		ns_model_vertex_set_position( vertex, &position );
		ns_model_vertex_set_radius( vertex, diameter / 2.0f );
		}
	}


void ns_model_filter_translate_vertices( NsModel *model, const NsVector3f *T )
	{
	nsmodelvertex  vertex;
	NsVector3f     position;


	ns_assert( NULL != model );
	ns_assert( NULL != T );

	NS_MODEL_VERTEX_FOREACH( model, vertex )
		{
		ns_model_vertex_get_position( vertex, &position );
		ns_vector3f_cmpd_add( &position, T );
		ns_model_vertex_set_position( vertex, &position );
		}
	}
