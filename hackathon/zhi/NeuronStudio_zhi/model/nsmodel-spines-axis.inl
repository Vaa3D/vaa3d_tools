
NS_PRIVATE nsfloat _ns_spines_calc_distance_with_residual_smear
	(
	const NsSpines    *spines,
	const NsVector3f  *V1,
	const NsVector3f  *V2
	)
	{
	nsfloat dx, dy, dz;

	dx = ( V1->x - V2->x );
	dy = ( V1->y - V2->y );
	dz = ( V1->z - V2->z ) / spines->residual_smear_value;

	return ns_sqrtf( dx * dx + dy * dy + dz * dz );

	//return ns_vector3f_distance( V1, V2 );
	}


NS_PRIVATE NsError _ns_spine_calc_distance_to_branch_for_axis_modulated
	(
	NsSpine         *spine,
	const NsSpines  *spines,
	NsVoxel         *farthest
	)
	{
	NsQueue      Q[2], *Qc, *Qn;
	nslistiter   curr, end;
	NsVoxel     *voxel, *neighbor;
	nssize       i;
	NsVector3f   Vf, Nf;
	NsVector3i   Ni;
	nsfloat      d, a;
	NsError      error;


	ns_assert( NULL != farthest );

	error = ns_no_error();

	Qc = Q + 0;
	Qn = Q + 1;

	ns_queue_construct( Qc, NULL );
	ns_queue_construct( Qn, NULL );

	curr = ns_list_begin( &spine->voxels );
	end  = ns_list_end( &spine->voxels );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		voxel = ns_list_iter_get_object( curr );

		__CLEAR( voxel, __ENQUEUED );
		__CLEAR( voxel, __FINALIZED );

		/* IMPORTANT: Voxel db's (and a's) have been altered in "unmodulated" routine, so we need
			to re-initalize them! See "unmodulated" routine below. */

		__DATA( voxel, a )    = 0.0f;
		__DATA( voxel, next ) = NULL;

		if( __GET( voxel, __LAST_LAYER ) )
			{
			__DATA( voxel, db ) = 0.0f;

			if( NS_FAILURE( ns_queue_add( Qc, voxel ), error ) )
				goto _NS_SPINE_CALC_DB_FOR_AXIS_EXIT;

			__SET( voxel, __ENQUEUED );
			__SET( voxel, __FINALIZED );
			}
		else
			__DATA( voxel, db ) = -1.0f;
		}

	NS_INFINITE_LOOP
		{
		while( ! ns_queue_is_empty( Qc ) )
			{
			voxel = ns_queue_front( Qc );
			ns_queue_remove( Qc );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( ! __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );

							d = _ns_spines_calc_distance_with_residual_smear( spines, &Nf, &Vf );
							//a = __DATA( voxel, ds );//( __DATA( neighbor, ds ) + __DATA( voxel, ds ) ) / 2.0f;
							a = __DATA( voxel, a ) + d / ( __DATA( neighbor, ds ) + __DATA( voxel, ds ) ) + d;

							//if( __DATA( neighbor, a ) < 0.0f || a > __DATA( neighbor, a ) )
							if( __DATA( neighbor, a ) <= 0.0f || a < __DATA( neighbor, a ) )
								{
								__DATA( neighbor, a )    = a;
								__DATA( neighbor, db )   = d + __DATA( voxel, db );
								__DATA( neighbor, next ) = voxel;
								}

							if( ! __GET( neighbor, __ENQUEUED ) )
								{
								if( NS_FAILURE( ns_queue_add( Qn, neighbor ), error ) )
									goto _NS_SPINE_CALC_DB_FOR_AXIS_EXIT;

								__SET( neighbor, __ENQUEUED );
								}
							}
				}
			}

		if( ns_queue_is_empty( Qn ) )
			break;

		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );
			__SET( voxel, __FINALIZED );
			}

		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );

							d = _ns_spines_calc_distance_with_residual_smear( spines, &Nf, &Vf );
							//a = __DATA( neighbor, ds );
							a = __DATA( neighbor, a ) + d / ( __DATA( neighbor, ds ) + __DATA( voxel, ds ) ) + d;

							//if( ( __DATA( voxel, a ) < 0.0f || a > __DATA( voxel, a ) )
							if( __DATA( voxel, a ) <= 0.0f || a < __DATA( voxel, a ) )
									//&&
								 //__DATA( neighbor, next ) != voxel )
								{
								__DATA( voxel, a )    = a;
								__DATA( voxel, db )   = d + __DATA( neighbor, db );
								__DATA( voxel, next ) = neighbor;
								}
							}
				}
			}

		NS_SWAP( NsQueue*, Qc, Qn );
		}

//ns_print( "Tracing back axis..." );

	ns_to_voxel_space( &farthest->position, &spine->max, spines->voxel_info );
	spine->maxima = farthest;

	for( voxel = farthest; NULL != voxel; voxel = __DATA( voxel, next ) )
		{
		ns_assert( __DATA( voxel, spine ) == spine );

		ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

		if( NS_FAILURE( ns_spine_axis_add_point( &spine->axis, &Vf ), error ) )
			goto _NS_SPINE_CALC_DB_FOR_AXIS_EXIT;
		}

//ns_println( " done" );

	_NS_SPINE_CALC_DB_FOR_AXIS_EXIT:

	ns_queue_destruct( Qc );
	ns_queue_destruct( Qn );

	return error;
	}


NS_PRIVATE NsError _ns_spine_calc_distance_to_branch_for_axis_unmodulated
	(
	NsSpine          *spine,
	const NsSpines   *spines,
	NsVoxel         **farthest
	)
	{
	NsQueue      Q[2], *Qc, *Qn;
	nslistiter   curr, end;
	NsVoxel     *voxel, *neighbor, *max_db;
	nssize       i;
	NsVector3f   Vf, Nf;
	NsVector3i   Ni;
	nsfloat      d, a;
	NsError      error;


	error = ns_no_error();

	Qc = Q + 0;
	Qn = Q + 1;

	ns_queue_construct( Qc, NULL );
	ns_queue_construct( Qn, NULL );

	curr = ns_list_begin( &spine->voxels );
	end  = ns_list_end( &spine->voxels );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		voxel = ns_list_iter_get_object( curr );

		__CLEAR( voxel, __ENQUEUED );
		__CLEAR( voxel, __FINALIZED );

		if( 0.0f <= __DATA( voxel, db ) )
			{
			if( NS_FAILURE( ns_queue_add( Qc, voxel ), error ) )
				goto _NS_SPINE_CALC_DB_FOR_AXIS_EXIT;

			__SET( voxel, __ENQUEUED );
			__SET( voxel, __FINALIZED );
			}
		}

	NS_INFINITE_LOOP
		{
		while( ! ns_queue_is_empty( Qc ) )
			{
			voxel = ns_queue_front( Qc );
			ns_queue_remove( Qc );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( ! __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );

							d = _ns_spines_calc_distance_with_residual_smear( spines, &Nf, &Vf );
							a = __DATA( voxel, a ) + d;

							if( __DATA( neighbor, a ) <= 0.0f || a < __DATA( neighbor, a ) )
								{
								__DATA( neighbor, a )    = a;
								__DATA( neighbor, db )   = d + __DATA( voxel, db );
								__DATA( neighbor, next ) = voxel;
								}

							if( ! __GET( neighbor, __ENQUEUED ) )
								{
								if( NS_FAILURE( ns_queue_add( Qn, neighbor ), error ) )
									goto _NS_SPINE_CALC_DB_FOR_AXIS_EXIT;

								__SET( neighbor, __ENQUEUED );
								}
							}
				}
			}

		if( ns_queue_is_empty( Qn ) )
			break;

		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );
			__SET( voxel, __FINALIZED );
			}

		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );

							d = _ns_spines_calc_distance_with_residual_smear( spines, &Nf, &Vf );
							a = __DATA( neighbor, a ) + d;

							if( __DATA( voxel, a ) <= 0.0f || a < __DATA( voxel, a ) )
								{
								__DATA( voxel, a )    = a;
								__DATA( voxel, db )   = d + __DATA( neighbor, db );
								__DATA( voxel, next ) = neighbor;
								}
							}
				}
			}

		NS_SWAP( NsQueue*, Qc, Qn );
		}

	max_db = NULL;
	curr   = ns_list_begin( &spine->voxels );
	end    = ns_list_end( &spine->voxels );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		voxel = ns_list_iter_get_object( curr );

		if( NULL == max_db || __DATA( voxel, db ) > __DATA( max_db, db ) )
			max_db = voxel;
		}

	*farthest = max_db;

	_NS_SPINE_CALC_DB_FOR_AXIS_EXIT:

	ns_queue_destruct( Qc );
	ns_queue_destruct( Qn );

	return error;
	}


NS_PRIVATE NsError _ns_spine_calc_distance_to_surface_for_axis( NsSpine *spine, NsSpines *spines )
	{
	NsQueue      Q[2], *Qc, *Qn;
	nslistiter   curr, end;
	NsVoxel     *voxel, *neighbor;
	nssize       i;
	NsVector3f   Vf, Nf;
	NsVector3i   Ni;
	nsfloat      d;
	nsint        width, height, length;
	NsError      error;


	error = ns_no_error();

	width  = ( nsint )ns_image_width( spines->image );
	height = ( nsint )ns_image_height( spines->image );
	length = ( nsint )ns_image_length( spines->image );

	Qc = Q + 0;
	Qn = Q + 1;

	ns_queue_construct( Qc, NULL );
	ns_queue_construct( Qn, NULL );

	curr = ns_list_begin( &spine->voxels );
	end  = ns_list_end( &spine->voxels );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		voxel = ns_list_iter_get_object( curr );

		__CLEAR( voxel, __ENQUEUED );
		__CLEAR( voxel, __FINALIZED );

		ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

		for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
			{
			ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

			if( ( 0 <= Ni.x && Ni.x < width  ) &&
				 ( 0 <= Ni.y && Ni.y < height ) &&
				 ( 0 <= Ni.z && Ni.z < length )   )
				if( ( NULL == ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) ) ||
					 ( ( __DATA( neighbor, clump_id ) != __DATA( voxel, clump_id ) ) && ( 0 != __DATA( neighbor, clump_id ) ) ) )
					{
					ns_to_voxel_space( &Ni, &Nf, spines->voxel_info );
					d = ns_vector3f_distance( &Nf, &Vf );

					if( __DATA( voxel, ds ) < 0.0f || d < __DATA( voxel, ds ) )
						__DATA( voxel, ds ) = d;

					if( ! __GET( voxel, __ENQUEUED ) )
						{
						if( NS_FAILURE( ns_queue_add( Qc, voxel ), error ) )
							goto _NS_SPINE_CALC_DS_FOR_AXIS_EXIT;

						__SET( voxel, __ENQUEUED );
						}
					}
			}

		if( __GET( voxel, __ENQUEUED ) )
			__SET( voxel, __FINALIZED );
		}

	/*TEMP!!!!*/
	/*ns_println(
		NS_FMT_ULONG " out of " NS_FMT_ULONG " %% first shell = " NS_FMT_DOUBLE,
		ns_queue_size( Qc ),
		ns_list_size( &spine->voxels ),
		( ( nsdouble )ns_queue_size( Qc ) / ( nsdouble )ns_list_size( &spine->voxels ) ) * 100.0
		);*/

	NS_INFINITE_LOOP
		{
		while( ! ns_queue_is_empty( Qc ) )
			{
			voxel = ns_queue_front( Qc );
			ns_queue_remove( Qc );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( ! __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );
							d = ns_vector3f_distance( &Nf, &Vf );

							if( __DATA( neighbor, ds ) < 0.0f || d + __DATA( voxel, ds ) < __DATA( neighbor, ds ) )
								__DATA( neighbor, ds ) = d + __DATA( voxel, ds );

							if( ! __GET( neighbor, __ENQUEUED ) )
								{
								if( NS_FAILURE( ns_queue_add( Qn, neighbor ), error ) )
									goto _NS_SPINE_CALC_DS_FOR_AXIS_EXIT;

								__SET( neighbor, __ENQUEUED );
								}
							}
				}
			}

		if( ns_queue_is_empty( Qn ) )
			break;

		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );
			__SET( voxel, __FINALIZED );
			}
/*
		curr = ns_list_begin( &Qn->list );
		end  = ns_list_end( &Qn->list );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );

			ns_to_voxel_space( &voxel->position, &Vf, spines->voxel_info );

			for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
				{
				ns_vector3i_add( &Ni, &(voxel->position), spines->voxel_offsets + i );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
					if( NULL != __DATA( neighbor, spine ) && __DATA( neighbor, spine ) == __DATA( voxel, spine ) )
						if( __GET( neighbor, __FINALIZED ) )
							{
							ns_to_voxel_space( &neighbor->position, &Nf, spines->voxel_info );
							d = ns_vector3f_distance( &Nf, &Vf );

							if( __DATA( voxel, ds ) < 0.0f || d + __DATA( neighbor, ds ) < __DATA( voxel, ds ) )
								__DATA( voxel, ds ) = d + __DATA( neighbor, ds );
							}
				}
			}
*/
		NS_SWAP( NsQueue*, Qc, Qn );
		}

	_NS_SPINE_CALC_DS_FOR_AXIS_EXIT:

	ns_queue_destruct( Qc );
	ns_queue_destruct( Qn );

	return error;
	}


NS_PRIVATE NsError _ns_spine_calc_distances_for_axis( NsSpine *spine, NsSpines *spines )
	{
	NsVoxel  *farthest;
	NsError   error;


	if( NS_FAILURE( _ns_spine_calc_distance_to_surface_for_axis( spine, spines ), error ) )
		return error;

	farthest = NULL;

	if( NS_FAILURE( _ns_spine_calc_distance_to_branch_for_axis_unmodulated( spine, spines, &farthest ), error ) )
		return error;

	if( NS_FAILURE( _ns_spine_calc_distance_to_branch_for_axis_modulated( spine, spines, farthest ), error ) )
		return error;

	return ns_no_error();
	}


NS_PRIVATE void _ns_spine_do_smooth_axis( NsSpine *spine )
	{
	nslistiter               curr, end;
	NsSpineAxisPoint        *curr_point;
	const NsSpineAxisPoint  *prev_point, *next_point;


	curr = ns_list_begin( &spine->axis.points );
	end  = ns_list_end( &spine->axis.points );

	/* NOTE: Cant smooth the first and last points. */
	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		if( ns_list_iter_not_equal( curr, ns_list_begin( &spine->axis.points ) ) &&
			 ns_list_iter_not_equal( curr, ns_list_rev_begin( &spine->axis.points ) ) )
			{
			prev_point = ns_list_iter_get_object( ns_list_iter_prev( curr ) );
			curr_point = ns_list_iter_get_object( curr );
			next_point = ns_list_iter_get_object( ns_list_iter_next( curr ) );

			/* The following statements are equivalent to finding the midpoint between
				the previous and next point, and then setting the current point to the
				midpoint of that point and the current points position. */
			curr_point->P.x = ( prev_point->P.x + next_point->P.x + 2.0f * curr_point->P.x ) / 4.0f;
			curr_point->P.y = ( prev_point->P.y + next_point->P.y + 2.0f * curr_point->P.y ) / 4.0f;
			curr_point->P.z = ( prev_point->P.z + next_point->P.z + 2.0f * curr_point->P.z ) / 4.0f;
			}
	}


NS_PRIVATE void _ns_spine_smooth_axis( NsSpine *spine, const NsSpines *spines )
	{
	nsint i;

   for( i = 0; i < spines->axis_smoothing; ++i )
		_ns_spine_do_smooth_axis( spine );
	}


NS_PRIVATE NsError _ns_spines_calc_axis( NsSpines *spines )
	{
	nslistiter   curr, end;
	NsSpine     *spine;
	NsError      error;


	ns_assert( NULL != spines );

   ns_progress_set_title( spines->progress, "Finding spine axes..." );
   ns_progress_num_iters( spines->progress, _ns_spines_size( spines ) );
   ns_progress_begin( spines->progress );

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      if( ns_progress_cancelled( spines->progress ) )
         return ns_no_error();

      spine = ns_list_iter_get_object( curr );

		if( NS_FAILURE( _ns_spine_calc_distances_for_axis( spine, spines ), error ) )
			return error;

		_ns_spine_smooth_axis( spine, spines );

      ns_progress_next_iter( spines->progress );
		}

   ns_progress_end( spines->progress );

	return ns_no_error();
	}
