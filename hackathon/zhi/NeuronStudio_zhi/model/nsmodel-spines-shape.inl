/*
NS_PRIVATE NsError _ns_spines_create_dendrite_clump( NsSpines *spines )
   {
   NsSpineEvent          event;
	nslistiter            curr;
	const NsSpineMaxima  *M;
   NsVoxel              *voxel;
   NsError               error;


   ns_progress_set_title( spines->progress, "Creating dendrite clump..." );
   ns_progress_num_iters( spines->progress, spines->num_maxima );
   ns_progress_begin( spines->progress );

   _ns_spine_event_construct( &event );

   NS_LIST_FOREACH( &spines->maxima, curr )
		{
		M = ns_list_iter_get_object( curr );

		/* Check if the maxima falls within the user-defined maximum distance by
			applying its correction factor to its distance-to-surface. *//*
		if( ( __DATA( M->voxel, distance ) * M->correction_factor <=
			   ns_settings_get_spine_max_voxel_distance( spines->settings ) )
			  &&
			 ( 0 == __DATA( M->voxel, clump_id ) )
			)	
			{
			event.correction_factor = M->correction_factor;

			voxel = M->voxel;
			ns_assert( __GET( voxel, __MAXIMA ) );

			if( ns_progress_cancelled( spines->progress ) )
				{
				_ns_spine_event_destruct( &event );
				return ns_no_error();
				}

			if( NS_FAILURE(
					_ns_spines_create_dendrite_event(
						spines,
						voxel,
						&event
						),
					error ) )
				{
				_ns_spine_event_destruct( &event );
				return error;
				}

			_ns_spine_event_clear( &event );
			}

      ns_progress_next_iter( spines->progress );
		}

   ns_progress_end( spines->progress );

   _ns_spine_event_destruct( &event );
   return ns_no_error();
   }
*/

NS_PRIVATE NsError _ns_spines_create_shapes( NsSpines *spines )
   {
   NsSpineEvent          event;
	nslistiter            curr;
	const NsSpineMaxima  *M;
   NsVoxel              *voxel;
   NsSpineType           spine_type;
   nssize                num_voxels;
	nsboolean             is_attached;
	nsboolean             do_store;
   NsError               error;


   ns_progress_set_title( spines->progress, "Creating spine events..." );
   ns_progress_num_iters( spines->progress, spines->num_maxima );
   ns_progress_begin( spines->progress );

   _ns_spine_event_construct( &event );

   NS_LIST_FOREACH( &spines->maxima, curr )
		{
		M = ns_list_iter_get_object( curr );

		/* Check if the maxima falls within the user-defined maximum distance by
			applying its correction factor to its distance-to-surface. */
		if( __DATA( M->voxel, distance ) * M->correction_factor <=
			 ns_settings_get_spine_max_voxel_distance( spines->settings ) )	
			{
			event.correction_factor = M->correction_factor;

			voxel = M->voxel;
			ns_assert( __GET( voxel, __MAXIMA ) );

			if( ns_progress_cancelled( spines->progress ) )
				{
				_ns_spine_event_destruct( &event );
				return ns_no_error();
				}

			if( NULL == __DATA( voxel, spine ) && 0 != __DATA( voxel, clump_id ) )
				{
				if( NS_FAILURE(
						_ns_spines_create_spine_event(
							spines,
							voxel,
							&event
							),
							error ) )
					{
					_ns_spine_event_destruct( &event );
					return error;
					}

				if( NS_FAILURE( _ns_spines_calc_event_widths( spines, &event ), error ) )
					{
					_ns_spine_event_destruct( &event );
					return error;
					}

				spine_type = _ns_spines_analyze_event( spines, &event );

				//ns_settings_get_spine_min_volume( spines->settings ) <= volume &&
				//volume <= ns_settings_get_spine_max_volume( spines->settings ) )

				if( NS_SPINE_INVALID != spine_type )
					{
					do_store    = NS_TRUE;
					num_voxels  = _ns_spine_event_num_voxels( &event );
					is_attached = _ns_spine_event_is_attached( &event );

					if( /*NS_SPINE_STUBBY == spine_type*/ is_attached )
						{
						if( num_voxels < ns_settings_get_spine_min_attached_voxels( spines->settings ) )
							do_store = NS_FALSE;
						}
					else
						{
						if( num_voxels < ns_settings_get_spine_min_detached_voxels( spines->settings ) )
							do_store = NS_FALSE;
						}

					if( do_store )
						if( NS_FAILURE( _ns_spines_store_event( spines, &event, spine_type ), error ) )
							{
							_ns_spine_event_destruct( &event );
							return error;
							}
					}

				_ns_spine_event_clear( &event );
				}
			}

      ns_progress_next_iter( spines->progress );
		}

   ns_progress_end( spines->progress );

   _ns_spine_event_destruct( &event );
   return ns_no_error();
   }


#define _ns_spines_size( spines )\
   ns_list_size( &(spines)->list )


NS_PRIVATE NsError _ns_spines_run_merging( NsSpines *spines )
   {
   nslistiter   curr1, curr2, end;
   NsSpine     *spine;
	NsSpine     *detached_spine, *attached_spine;
	NsSpine     *merge_spine;
	nsfloat      min_distance;
	nsfloat      distance_to_m;
	nsfloat      distance_to_line;
	nssize       count;
	NsLine3f     L;
	NsVector3f   A, B;
	nsfloat      mag_of_A;
	nsfloat      u;
	nsfloat      projection_length;
	NsVector3f   M;
	nsfloat      m;


   //ns_progress_set_title( spines->progress, "Merging spines..." );
   //ns_progress_num_iters( spines->progress, _ns_spines_size( spines ) );
   //ns_progress_begin( spines->progress );

	count = 0;

   end = ns_list_end( &spines->list );

   curr1 = ns_list_begin( &spines->list );
   for( ; ns_list_iter_not_equal( curr1, end ); curr1 = ns_list_iter_next( curr1 ) )
      {
		merge_spine  = NULL;
		min_distance = NS_FLOAT_MAX;

      spine = ns_list_iter_get_object( curr1 );

		if( ! spine->is_attached )
			{
			detached_spine = spine;

			M = detached_spine->min;//detached_spine->base;
			m = detached_spine->min_distance;//detached_spine->base_distance;

			L.P1 = M;
			L.P2 = detached_spine->surface;

			ns_vector3f_sub( &A, &detached_spine->surface, &M );
			mag_of_A = ns_vector3f_mag( &A );

			curr2 = ns_list_begin( &spines->list );
			for( ; ns_list_iter_not_equal( curr2, end ); curr2 = ns_list_iter_next( curr2 ) )
				{
				spine = ns_list_iter_get_object( curr2 );

				if( spine->is_attached && ( ! spine->stats.has_neck ) )/* Test 4, See below. */
					{
					attached_spine = spine;
					ns_assert( detached_spine != attached_spine );

					/* 4 tests ( added 4th test on 2007-06-08 )

						1) Make sure the attached spine's maximum voxel is within a conceptual
							sphere that has a radius equal to the DTS of the detached spines base.

						2) Make sure the attached spine's maximum voxel projects onto the line
							formed between the detached spine's base and surface points.

						3) Make sure the attached spine's maximum voxel falls within a conecptual
							cone whose tip is the detached spines base, and whose radius at the
							base is equal to user-defined 'merge_value'. Note that we can compute
							this by finding out how far(in the range [0-1]) the maximum point is along
							the vector formed beteen the detached spines base and surface point.

						4) Make sure the attached spine doesnt have a neck.
					*/

					distance_to_m = ns_vector3f_distance( &attached_spine->max, &M );

					if( distance_to_m <= m ) /* Test 1 */
						if( ns_point3f_projects_onto_line( &attached_spine->max, &L ) )/* Test 2 */
							{
							distance_to_line = ns_point3f_distance_to_line( &attached_spine->max, &L );

							ns_vector3f_sub( &B, &attached_spine->max, &M );
							projection_length = ns_vector3f_dot( &A, &B ) / mag_of_A;

							/* NOTE: 'u' should be in the range [0-1]. */
							u = projection_length / m;

							/* Account for floating point error? */
							if( u < 0.0f )
								u = 0.0f;
							else if( 1.0f < u )
								u = 1.0f;

							/* Since u is in the range [0-1], multiplying by the 'merge_value', i.e.
								the radius of the base of the cone, is like linearly interpolating
								the radius of the cone. */

							if( distance_to_line <= ns_powf( u, spines->merge_power ) * spines->merge_value )/* Test 3 */
								if( distance_to_line < min_distance )
									{
									/* NOTE: We take the minimum one as the one to merge. */
									min_distance = distance_to_line;
									merge_spine  = attached_spine;
									}
							}
					}
				}
			}

		if( NULL != merge_spine )
			{
			++count;

			/*TEMP*/if( ! merge_spine->hidden )merge_spine->is_selected = NS_TRUE;
			}

      //ns_progress_next_iter( spines->progress );
      }

/*TEMP*/ns_println( "MERGE " NS_FMT_ULONG " spines.", count );

   //ns_progress_end( spines->progress );
	return ns_no_error();
   }


NS_PRIVATE void _ns_spine_calc_threshold_and_contrast
	(
	NsSpine   *spine,
	nsfloat   *threshold,
	nsfloat   *contrast,
	NsSpines  *spines
	)
   {  _ns_spines_calc_threshold_and_contrast_at( spines, &spine->center, threshold, contrast );  }
