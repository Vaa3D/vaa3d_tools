
NS_PRIVATE int _ns_spines_create_clumps_compare_intensity( const void *vp1, const void *vp2 )
	{
	NsVoxel *V1, *V2;

	V1 = *( ( NsVoxel** )vp1 );
	V2 = *( ( NsVoxel** )vp2 );

	if( V1->intensity > V2->intensity )
		return -1;
	else if( V1->intensity < V2->intensity )
		return 1;
	else
		return 0;
	}

/*
NS_PRIVATE int _ns_spines_create_clumps_compare_intensity_reverse( const void *vp1, const void *vp2 )
	{
	NsVoxel *V1, *V2;

	V1 = *( ( NsVoxel** )vp1 );
	V2 = *( ( NsVoxel** )vp2 );

	if( V1->intensity > V2->intensity )
		return 1;
	else if( V1->intensity < V2->intensity )
		return -1;
	else
		return 0;
	}
*/

/*
NS_PRIVATE int _ns_spines_create_clumps_compare_clump_size( const void *vp1, const void *vp2 )
	{
	NsVoxel *V1, *V2;

	V1 = *( ( NsVoxel** )vp1 );
	V2 = *( ( NsVoxel** )vp2 );

	if( _ns_spine_clump_num_voxels( __DATA( V1, clump ) ) > _ns_spine_clump_num_voxels( __DATA( V2, clump ) ) )
		return 1;
	else if( _ns_spine_clump_num_voxels( __DATA( V1, clump ) ) < _ns_spine_clump_num_voxels( __DATA( V2, clump ) ) )
		return -1;
	else
		return 0;
	}
*/


/*
NS_PRIVATE nssize _ns_spines_dump_non_maxima_clumps( NsSpines *spines )
	{
	nssize         num_voxels;
	NsSpineClump  *clump;
	nslistiter     curr_clump, next_clump, end_clumps;
	nslistiter     curr_voxel, end_voxels;
	NsVoxel       *voxel;
	nsboolean      has_true_maxima;


	num_voxels = 0;

	curr_clump = ns_list_begin( &spines->clumps );
	end_clumps  = ns_list_end( &spines->clumps );

	while( ns_list_iter_not_equal( curr_clump, end_clumps ) )
		{
		next_clump      = ns_list_iter_next( curr_clump );
		clump           = ns_list_iter_get_object( curr_clump );
		has_true_maxima = NS_FALSE;
		curr_voxel      = ns_list_begin( &clump->voxels );
		end_voxels      = ns_list_end( &clump->voxels );

		for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
				 curr_voxel = ns_list_iter_next( curr_voxel ) )
			{
			voxel = ns_list_iter_get_object( curr_voxel );

			if( __GET( voxel, __MAXIMA ) )
				if( __DATA( voxel, distance ) >= clump->max_dts )
					{
					has_true_maxima = NS_TRUE;
					break;
					}
			}

		if( ! has_true_maxima )
			{
			num_voxels += ns_list_size( &clump->voxels );

			curr_voxel = ns_list_begin( &clump->voxels );
			end_voxels = ns_list_end( &clump->voxels );

			for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
					 curr_voxel = ns_list_iter_next( curr_voxel ) )
				{
				voxel = ns_list_iter_get_object( curr_voxel );

				__DATA( voxel, clump_id ) = 0;
				__DATA( voxel, dts_id )   = 0;
				__DATA( voxel, clump )    = &spines->clump0;
				}

			ns_list_erase( &spines->clumps, curr_clump );
			}

		curr_clump = next_clump;
		}

	return num_voxels;
	}*/


NS_PRIVATE NsError _ns_spines_create_clumps_by_intensity( NsSpines *spines )
	{
	nssize         num_voxels;
	NsVoxel*      *voxels;
	NsVoxel       *voxel, *neighbor, *max_voxel;
	NsVoxel        N;
	nssize         x, v;
	nsint          o;
	nsint          clump_id;
	NsSpineClump  *clump;
	nsfloat        curr_intensity;
	//nsfloat        meet;
	nssize         start, end;
	NsQueue        Q;
	nstimer        t;
	//NsVector3f     max1, min1, max2, min2, A, B;
	//nsboolean      merge/*, merge_by_dts_range*/;
	//nsdouble       angle1, angle2;
	nssize         clump_zero_size;
	nsboolean      touches_clump0;
	//nslistiter     iter;
	NsError        error;


	/*TEMP!*/t = ns_timer();

	clump_zero_size = 0;

   ns_progress_set_title( spines->progress, "Creating spine clumps..." );

	error = ns_no_error();

	ns_list_clear( &spines->clumps );

	num_voxels = ns_voxel_table_size( &spines->voxel_table );

	if( NULL == ( voxels = ns_new_array( NsVoxel*, num_voxels ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_queue_construct( &Q, NULL );

	v = 0;

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		__DATA( voxel, clump ) = NULL;
		voxels[ v++ ] = voxel;
		}

	ns_assert( v == num_voxels );
	_ns_qsort( voxels, num_voxels, sizeof( NsVoxel* ), _ns_spines_create_clumps_compare_intensity );

	curr_intensity = -1.0f;
	clump_id       = 1;

	for( v = 0; v < num_voxels; ++v )
		{
		/* Ignore those voxels that were marked as "dendrite". */
		if( 0 == __DATA( voxels[v], clump_id ) )
			continue;

		if( ! NS_FLOAT_EQUAL( voxels[v]->intensity, curr_intensity ) )
			{
			ns_assert( ns_queue_is_empty( &Q ) );

			curr_intensity = voxels[v]->intensity;

			start = end = v;
			++end;

			while( end < num_voxels && NS_FLOAT_EQUAL( voxels[ end ]->intensity, curr_intensity ) )
				++end;

			/* Add all voxels at this intensity that are next to a clump. Attach to the neighbor
				with the maximum intensity. */
			for( x = start; x < end; ++x )
				{
				/* Ignore those voxels that were marked as "dendrite". */
				if( 0 == __DATA( voxels[x], clump_id ) )
					continue;

				touches_clump0 = NS_FALSE;

				max_voxel = NULL;

				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxels[x]->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						{
						if( 0 == __DATA( neighbor, clump_id ) )
							touches_clump0 = NS_TRUE;

						if( NULL != __DATA( neighbor, clump ) &&
							 0 != __DATA( neighbor, clump_id ) )
							if( NULL == max_voxel || neighbor->intensity > max_voxel->intensity )
								max_voxel = neighbor;
						}
					}

				if( NULL != max_voxel )
					{
					if( NS_FAILURE( _ns_spine_clump_add_voxel( __DATA( max_voxel, clump ), voxels[x], spines ), error ) )
						goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

					if( touches_clump0 )
						__DATA( voxels[x], clump )->touches_clump0 = NS_TRUE;

					if( NS_FAILURE( ns_queue_add( &Q, voxels[x] ), error ) )
						goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
					}
				}

			while( ! ns_queue_is_empty( &Q ) )
				{
				voxel = ns_queue_front( &Q );
				ns_queue_remove( &Q );

				/* Add all neighbors of this queue'd voxel that are at the current intensity and
					are not yet in a clump. Set the neighbors clump BEFORE entering the queue. */
				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxel->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						{
						if( 0 == __DATA( neighbor, clump_id ) )
							__DATA( voxel, clump )->touches_clump0 = NS_TRUE;

						if( NULL == __DATA( neighbor, clump ) &&
							 0 != __DATA( neighbor, clump_id ) &&
							 NS_FLOAT_EQUAL( neighbor->intensity, curr_intensity ) )
							{
							if( NS_FAILURE( _ns_spine_clump_add_voxel( __DATA( voxel, clump ), neighbor, spines ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

							if( NS_FAILURE( ns_queue_add( &Q, neighbor ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
							}
						}
					}
				}

			ns_queue_clear( &Q );
			}

		/* Flood fill at current intensity level starting at voxel v. */
		if( NULL == __DATA( voxels[v], clump ) )
			{
			ns_assert( ns_queue_is_empty( &Q ) );

			if( NS_FAILURE( _ns_spine_clump_new( &clump, clump_id ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			if( NS_FAILURE( ns_list_push_back( &spines->clumps, clump ), error ) )
				{
				_ns_spine_clump_delete( clump );
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
				}

			__SET( voxels[v], __CLUMP_SEED );
			if( NS_FAILURE( _ns_spine_clump_add_voxel( clump, voxels[v], spines ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			clump->origin = voxels[v];

			if( NS_FAILURE( ns_queue_add( &Q, voxels[v] ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			while( ! ns_queue_is_empty( &Q ) )
				{
				voxel = ns_queue_front( &Q );
				ns_queue_remove( &Q );

				/* Add neighbors of this voxels that meet current intensity. */
				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxel->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						{
						if( 0 == __DATA( neighbor, clump_id ) )
							__DATA( voxel, clump )->touches_clump0 = NS_TRUE;

						if( NULL == __DATA( neighbor, clump ) &&
							 0 != __DATA( neighbor, clump_id ) &&
							 NS_FLOAT_EQUAL( neighbor->intensity, voxel->intensity ) )
							{
							__SET( neighbor, __CLUMP_SEED );
							if( NS_FAILURE( _ns_spine_clump_add_voxel( clump, neighbor, spines ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

							if( NS_FAILURE( ns_queue_add( &Q, neighbor ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
							}
						}
					}
				}

			ns_queue_clear( &Q );

			++clump_id;
			}
		}

/*TEMP!*///ns_println( "# of clumps before any merging = " NS_FMT_ULONG, _ns_spines_num_unempty_clumps( spines ) );

	//if( spines->sort_clump_voxels )
	//	{
		//_ns_qsort( voxels, num_voxels, sizeof( NsVoxel* ), _ns_spines_set_clump_ids_compare_clump_size );
		//_ns_qsort( voxels, num_voxels, sizeof( NsVoxel* ), _ns_spines_set_clump_ids_compare_intensity_reverse );
	//	}


/*
	for( v = 0; v < num_voxels; ++v )
		{
		// Ignore those voxels that were marked as "dendrite".
		if( 0 == __DATA( voxels[v], clump_id ) )
			continue;

		for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
			{
			ns_vector3i_add( &N.position, &(voxels[v]->position), spines->voxel_offsets + o );

			if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) &&
				 NULL != __DATA( neighbor, clump ) &&
				 0 != __DATA( neighbor, clump_id ) &&
				 __DATA( neighbor, clump ) != __DATA( voxels[v], clump ) )
				{
				//merge = NS_FALSE; //merge_by_dts_range =



/* MERGE ALG #1 *//*
				meet = ( voxels[v]->intensity + neighbor->intensity ) / 2.0f;
				ns_assert( __DATA( voxels[v], clump )->max_intensity >= meet );

				//if( spines->clump_merge_by_intensity )
					//{
				merge =
					( ( __DATA( voxels[v], clump )->max_intensity - meet ) /
						( __DATA( voxels[v], clump )->max_intensity - __DATA( voxels[v], clump )->min_intensity )
						< spines->clump_intensity_merge );
					//}

				/* MERGE ALG #1 ADDITIONAL TEST *//*
				if( merge )
					if( __DATA( __DATA( voxels[v], clump )->origin, dts_id ) !=
						 __DATA( __DATA( neighbor, clump )->origin, dts_id ) )
						merge = NS_FALSE;




				/*
				if( spines->clump_merge_by_dts )
					{
					merge_by_dts_range =
						( ( __DATA( neighbor, clump )->max_dts - __DATA( voxels[v], clump )->min_dts ) /
							( __DATA( voxels[v], clump )->max_dts - __DATA( voxels[v], clump )->min_dts )
							< spines->clump_dts_merge )
						||
						( ( __DATA( neighbor, clump )->max_dts - __DATA( voxels[v], clump )->min_dts ) /
							( __DATA( neighbor, clump )->max_dts - __DATA( neighbor, clump )->min_dts )
							< spines->clump_dts_merge );
					}
				*/



				/*if( NULL != __DATA( voxels[v], clump )->maximum &&
					 NULL != __DATA( neighbor, clump )->maximum &&
					 __DATA( voxels[v], clump )->max_dts > __DATA( neighbor, clump )->max_dts )
					{
					ns_to_voxel_space( &(__DATA( voxels[v], clump )->maximum->position), &max1, spines->voxel_info );
					ns_to_voxel_space( &(__DATA( voxels[v], clump )->minimum->position), &min1, spines->voxel_info );
					ns_to_voxel_space( &(__DATA( neighbor, clump )->maximum->position), &max2, spines->voxel_info );
					ns_to_voxel_space( &(__DATA( neighbor, clump )->minimum->position), &min2, spines->voxel_info );

					ns_vector3f_sub( &A, &min1, &max1 );
					ns_vector3f_sub( &B, &max2, &max1 );

					angle1 = ns_vector3f_angle( &A, &B );
					angle1 = NS_RADIANS_TO_DEGREES( angle1 );

					ns_vector3f_sub( &A, &max2, &min2 );
					ns_vector3f_sub( &B, &max1, &min2 );

					angle2 = ns_vector3f_angle( &A, &B );
					angle2 = NS_RADIANS_TO_DEGREES( angle2 );

					merge_by_angle = ( angle1 < spines->parallel_range && angle2 < spines->parallel_range );
					}*/

				/*
					( ( neighbor == __DATA( neighbor, clump )->maximum ) &&
						( ! __GET( __DATA( neighbor, clump )->maximum, __MAXIMA ) ) &&
						( __DATA( voxels[v], clump )->mean_dts > __DATA( neighbor, clump )->max_dts ) );
				*/



/* MERGE ALG #2 *//*
				if( __DATA( voxels[v], clump )->touches_clump0 &&
					 __DATA( neighbor, clump )->touches_clump0 )
					{
					if( merge )
						if( __DATA( __DATA( voxels[v], clump )->origin, dts_id ) !=
							 __DATA( __DATA( neighbor, clump )->origin, dts_id ) )
							merge = NS_FALSE;
					}
				else
					{
					if( ! merge )
						if( __DATA( __DATA( voxels[v], clump )->origin, dts_id ) ==
							 __DATA( __DATA( neighbor, clump )->origin, dts_id ) )
							merge = NS_TRUE;
					}*/



/* MERGE ALG #3 *//*
				merge = ( __DATA( __DATA( voxels[v], clump )->origin, dts_id ) == __DATA( __DATA( neighbor, clump )->origin, dts_id ) );
*/


/*
				if( merge )//|| merge_by_dts_range
					{
					NsSpineClump *src, *dest;

					// NOTE: Merge the smaller clump onto the larger clump. This is done just for speed since it means
					//	less voxels will have to be flooded in the merge routine.
					if( _ns_spine_clump_num_voxels( __DATA( voxels[v], clump ) )
							>
						 _ns_spine_clump_num_voxels( __DATA( neighbor, clump ) ) )
						{
						dest = __DATA( voxels[v], clump );
						src  = __DATA( neighbor, clump );
						}
					else
						{
						dest = __DATA( neighbor, clump );
						src  = __DATA( voxels[v], clump );
						}

					//if( NS_FAILURE( _ns_spines_merge_clumps( spines, dest, src ), error ) )
					//	goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

					if( ! _ns_spine_clump_has_been_seen( dest, src ) )
						{
						/* Form an undirected graph, essentially. *//*

						if( NS_FAILURE( ns_list_push_back( &dest->connections, src ), error ) )
							goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

						if( NS_FAILURE( ns_list_push_back( &src->connections, dest ), error ) )
							goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

						if( NS_FAILURE( ns_list_push_back( &dest->seen, src ), error ) )
							goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
						}
					}
				}
			}
		}


	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );

		if( NS_FAILURE( _ns_spines_do_execute_clump_mergers( spines, clump, clump ), error ) )
			return error;
		}
*/

/*TEMP!*///ns_println( "# of clumps AFTER first merging = " NS_FMT_ULONG, _ns_spines_num_unempty_clumps( spines ) );


/*TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//_ns_spines_compute_and_save_clump_centroids( spines );
//_ns_spines_compute_and_save_clump_borderers( spines );

	error = ns_spines_find_all_clump_mergers( spines );

/*TEMP!*///ns_println( "# of clumps AFTER merging = " NS_FMT_ULONG, _ns_spines_num_unempty_clumps( spines ) );


	/* Save the clump ID's. */
	for( v = 0; v < num_voxels; ++v )
		if( 0 == __DATA( voxels[v], clump_id ) )
			{
			ns_assert( NULL == __DATA( voxels[v], clump ) );
			__DATA( voxels[v], clump ) = &spines->clump0;

			++clump_zero_size;
			}
		else
			{
			ns_assert( NULL != __DATA( voxels[v], clump ) );
			__DATA( voxels[v], clump_id ) = __DATA( voxels[v], clump )->id;

			/*TEMP*/
			if( __DATA( voxels[v], clump )->touches_clump0 )
				__SET( voxels[v], __TOUCH_CLUMP0 );
			}

/*TEMP*/ns_println( "Clump-0 size = " NS_FMT_ULONG, clump_zero_size );


	_NS_SPINES_SET_CLUMP_IDS_EXIT:

/*TEMP!*///ns_println( NS_FMT_ULONG " == " NS_FMT_ULONG, _ns_spines_count_clump_voxels( spines ), num_voxels );

	ns_queue_destruct( &Q );
	ns_free( voxels );

	_ns_spines_delete_empty_clumps( spines );


	//clump_zero_size += _ns_spines_dump_non_maxima_clumps( spines );
	///*TEMP*/ns_println( "Clump-0 size = " NS_FMT_ULONG, clump_zero_size );


	/* Dont need the clump voxel lists anymore. */
	//_ns_spines_clear_clump_voxels( spines );

/*TEMP!*/ns_println( "Spine INTENSITY declumping took %f seconds.", ns_difftimer( ns_timer(), t ) );

	return error;
	}


NS_PRIVATE int _ns_spines_create_clumps_compare_dts( const void *vp1, const void *vp2 )
	{
	NsVoxel *V1, *V2;

	V1 = *( ( NsVoxel** )vp1 );
	V2 = *( ( NsVoxel** )vp2 );

	if( __DATA( V1, distance ) > __DATA( V2, distance ) )
		return -1;
	else if( __DATA( V1, distance ) < __DATA( V2, distance ) )
		return 1;
	else
		return 0;
	}


/*
NS_PRIVATE void _ns_spines_clumps_print_lists( NsSpines *spines )
	{
	const NsSpineClump  *clump;
	nslistiter           iter;
	nslistiter           curr_voxel, end_voxels;
	const NsVoxel       *voxel;


	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );

		ns_print( "\n\n----------------------------------------\n" );

		curr_voxel = ns_list_rev_begin( &clump->voxels );
		end_voxels = ns_list_rev_end( &clump->voxels );

		for( ; ns_list_rev_iter_not_equal( curr_voxel, end_voxels );
				 curr_voxel = ns_list_rev_iter_next( curr_voxel ) )
			{
			voxel = ns_list_rev_iter_get_object( curr_voxel );
			ns_assert( __DATA( voxel, clump ) == clump );

			ns_print( "%f ", __DATA( voxel, distance ) );
			}
		}
	}


NS_PRIVATE void _ns_spines_clumps_print_num_voxels( NsSpines *spines )
	{
	const NsSpineClump  *clump;
	nslistiter           iter;
	nssize               count = 0;


	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );
		count += ns_list_size( &clump->voxels );
		}

	ns_println( NS_FMT_ULONG " = " NS_FMT_ULONG, ns_voxel_table_size( &spines->voxel_table ), count );
	}*/



#define FLOATEQUAL( f1, f2 )\
	( (f1) == (f2) )

NS_PRIVATE NsError _ns_spines_create_clumps_by_dts( NsSpines *spines )
	{
	nssize         num_voxels;
	NsVoxel*      *voxels;
	NsVoxel       *voxel, *neighbor, *max_voxel;
	NsVoxel        N;
	nssize         x, v;
	nsint          o;
	nsint          clump_id;
	NsSpineClump  *clump;
	nsfloat        curr_dts;
	//nsfloat        meet;
	nssize         start, end;
	NsQueue        Q;
	nstimer        t;
	//nsboolean      merge_by_dts_range;
	nssize         clump_zero_size;
	NsError        error;


	/*TEMP!*/t = ns_timer();

	clump_zero_size = 0;

   ns_progress_set_title( spines->progress, "Creating spine clumps..." );

	error = ns_no_error();

	ns_list_clear( &spines->clumps );

	num_voxels = ns_voxel_table_size( &spines->voxel_table );

	if( NULL == ( voxels = ns_new_array( NsVoxel*, num_voxels ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_queue_construct( &Q, NULL );

	v = 0;

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		__DATA( voxel, clump ) = NULL;
		voxels[ v++ ] = voxel;
		}

	ns_assert( v == num_voxels );
	_ns_qsort( voxels, num_voxels, sizeof( NsVoxel* ), _ns_spines_create_clumps_compare_dts );

	curr_dts = -1.0f;
	clump_id = 1;

	for( v = 0; v < num_voxels; ++v )
		{
		/* Ignore those voxels that were marked as "dendrite". */
		if( 0 == __DATA( voxels[v], clump_id ) )
			continue;

		if( ! FLOATEQUAL( __DATA( voxels[v], distance ), curr_dts ) )
			{
			ns_assert( ns_queue_is_empty( &Q ) );

			curr_dts = __DATA( voxels[v], distance );

			start = end = v;
			++end;

			while( end < num_voxels && FLOATEQUAL( __DATA( voxels[ end ], distance ), curr_dts ) )
				++end;

			/* Add all voxels at this dts that are next to a clump. Attach to the neighbor
				with the maximum intensity. */
			for( x = start; x < end; ++x )
				{
				/* Ignore those voxels that were marked as "dendrite". */
				if( 0 == __DATA( voxels[x], clump_id ) )
					continue;

				max_voxel = NULL;

				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxels[x]->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						if( NULL != __DATA( neighbor, clump ) && 0 != __DATA( neighbor, clump_id ) )
							if( NULL == max_voxel || //__DATA( neighbor, distance ) > __DATA( max_voxel, distance )
															 neighbor->intensity > max_voxel->intensity )
								max_voxel = neighbor;
					}

				if( NULL != max_voxel )
					{
					if( NS_FAILURE( _ns_spine_clump_add_voxel( __DATA( max_voxel, clump ), voxels[x], spines ), error ) )
						goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

					if( NS_FAILURE( ns_queue_add( &Q, voxels[x] ), error ) )
						goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
					}
				}

			while( ! ns_queue_is_empty( &Q ) )
				{
				voxel = ns_queue_front( &Q );
				ns_queue_remove( &Q );

				/* Add all neighbors of this queue'd voxel that are at the current dts and
					are not yet in a clump. Set the neighbors clump BEFORE entering the queue. */
				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxel->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						if( NULL == __DATA( neighbor, clump ) &&
							 0 != __DATA( neighbor, clump_id ) &&
							 FLOATEQUAL( __DATA( neighbor, distance ), curr_dts ) )
							{
							if( NS_FAILURE( _ns_spine_clump_add_voxel( __DATA( voxel, clump ), neighbor, spines ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

							if( NS_FAILURE( ns_queue_add( &Q, neighbor ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
							}
					}
				}

			ns_queue_clear( &Q );
			}

		/* Flood fill at current dts level starting at voxel v. */
		if( NULL == __DATA( voxels[v], clump ) )
			{
			ns_assert( ns_queue_is_empty( &Q ) );

			if( NS_FAILURE( _ns_spine_clump_new( &clump, clump_id ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			if( NS_FAILURE( ns_list_push_back( &spines->clumps, clump ), error ) )
				{
				_ns_spine_clump_delete( clump );
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
				}

			if( NS_FAILURE( _ns_spine_clump_add_voxel( clump, voxels[v], spines ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			if( NS_FAILURE( ns_queue_add( &Q, voxels[v] ), error ) )
				goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

			while( ! ns_queue_is_empty( &Q ) )
				{
				voxel = ns_queue_front( &Q );
				ns_queue_remove( &Q );

				/* Add neighbors of this voxels that meet current dts. */
				for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
					{
					ns_vector3i_add( &N.position, &(voxel->position), spines->voxel_offsets + o );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						if( NULL == __DATA( neighbor, clump ) &&
							 0 != __DATA( neighbor, clump_id ) &&
							 FLOATEQUAL( __DATA( neighbor, distance ), __DATA( voxel, distance ) ) )
							{
							if( NS_FAILURE( _ns_spine_clump_add_voxel( clump, neighbor, spines ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;

							if( NS_FAILURE( ns_queue_add( &Q, neighbor ), error ) )
								goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
							}
					}
				}

			ns_queue_clear( &Q );

			++clump_id;
			}
		}


//_ns_spines_clumps_print_lists( spines );
//_ns_spines_clumps_print_num_voxels( spines );


/*TEMP!*//*ns_println( "# of clumps before any merging = " NS_FMT_ULONG, _ns_spines_num_unempty_clumps( spines ) );

	for( v = 0; v < num_voxels; ++v )
		{
		/* Ignore those voxels that were marked as "dendrite". *//*
		if( 0 == __DATA( voxels[v], clump_id ) )
			continue;

		for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
			{
			ns_vector3i_add( &N.position, &(voxels[v]->position), spines->voxel_offsets + o );

			if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) &&
				 NULL != __DATA( neighbor, clump ) &&
				 0 != __DATA( neighbor, clump_id ) &&
				 __DATA( neighbor, clump ) != __DATA( voxels[v], clump ) )
				{
				merge_by_dts_range = NS_FALSE;

				meet = ( __DATA( voxels[v], distance ) + __DATA( neighbor, distance ) ) / 2.0f;

				merge_by_dts_range =
					( ( __DATA( voxels[v], clump )->max_dts - meet ) /*//*
						( __DATA( voxels[v], clump )->max_dts - __DATA( voxels[v], clump )->min_dts )*/
					/*	< spines->clump_dts_merge );

				if( merge_by_dts_range )
					{
					NsSpineClump *src, *dest;

					/* NOTE: Merge the smaller clump onto the larger clump. This is done just for speed since it means
						less voxels will have to be flooded in the merge routine. *//*
					if( _ns_spine_clump_num_voxels( __DATA( voxels[v], clump ) )
							>
						 _ns_spine_clump_num_voxels( __DATA( neighbor, clump ) ) )
						{
						dest = __DATA( voxels[v], clump );
						src  = __DATA( neighbor, clump );
						}
					else
						{
						dest = __DATA( neighbor, clump );
						src  = __DATA( voxels[v], clump );
						}

					if( NS_FAILURE( _ns_spines_merge_clumps( spines, dest, src ), error ) )
						goto _NS_SPINES_SET_CLUMP_IDS_EXIT;
					}
				}
			}
		}

/*TEMP!*//*ns_println( "# of clumps AFTER merging = " NS_FMT_ULONG, _ns_spines_num_unempty_clumps( spines ) );

	/* Save the clump (dts) ID's. */
	for( v = 0; v < num_voxels; ++v )
		{
		if( 0 == __DATA( voxels[v], clump_id ) )
			{
			ns_assert( NULL == __DATA( voxels[v], clump ) );
			__DATA( voxels[v], clump ) = &spines->clump0;

			++clump_zero_size;
			}
		else
			{
			ns_assert( NULL != __DATA( voxels[v], clump ) );
			__DATA( voxels[v], clump_id ) = __DATA( voxels[v], clump )->id;
			}

		__DATA( voxels[v], dts_id ) = __DATA( voxels[v], clump_id );
		
		//__DATA( voxels[v], clump_id ) = 1;
		}

/*TEMP*/ns_println( "Clump-0 size = " NS_FMT_ULONG, clump_zero_size );

	_NS_SPINES_SET_CLUMP_IDS_EXIT:

/*TEMP!*///ns_println( NS_FMT_ULONG " == " NS_FMT_ULONG, _ns_spines_count_clump_voxels( spines ), num_voxels );

	ns_queue_destruct( &Q );
	ns_free( voxels );

	_ns_spines_delete_empty_clumps( spines );

	/* Dont need the clump voxel lists anymore. */
	//_ns_spines_clear_clump_voxels( spines );
//_ns_spines_clumps_print_num_voxels( spines );

/*TEMP!*/ns_println( "Spine DTS declumping took %f seconds.", ns_difftimer( ns_timer(), t ) );

	return error;
	}
