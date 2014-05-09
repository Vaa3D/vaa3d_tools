

NS_PRIVATE void _ns_spine_clump_init_for_merging( NsSpineClump *clump )
	{
	ns_list_clear( &clump->seen );
	ns_list_clear( &clump->connections );

	clump->color = NS_SPINE_CLUMP_UNSEEN;
	}


NS_PRIVATE void _ns_spines_init_clumps_for_merging( NsSpines *spines )
	{
	nslistiter     iter;
	NsSpineClump  *clump;


	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );
		_ns_spine_clump_init_for_merging( clump );
		}
	}


NS_PRIVATE void _ns_spine_clump_voxel_clear_iface( NsVoxel *voxel )
	{  __CLEAR( voxel, __CLUMP_IFACE );  }


NS_PRIVATE NsError _ns_spine_clumps_eval_connectivity
	(
	const NsSpineClump  *A,
	const NsSpineClump  *B,
	NsSpines            *spines,
	nsboolean            print,
	nsboolean           *connect
	)
	{
	NsList               S1, S2;
	nslistiter           v;
	NsVoxel             *voxel, *neighbor, N;
	nssize               i;
	nsfloat              sum1, sum2;
	//nsdouble     max_dts, min_dts;
	//nsdouble     a, b, c;
	nsdouble             pinch, dts_range, intensity_range;
	const NsSpineClump  *higher;
	const NsSpineClump  *lower;
	const NsVoxel       *iface_brightest;
	nsfloat              range;
	NsError              error;


	*connect = NS_FALSE;
	error    = ns_no_error();

	iface_brightest = NULL;

	ns_list_construct( &S1, _ns_spine_clump_voxel_clear_iface );
	ns_list_construct( &S2, _ns_spine_clump_voxel_clear_iface );

	NS_LIST_FOREACH( &A->voxels, v )
		{
		voxel = ns_list_iter_get_object( v );
		ns_assert( __DATA( voxel, clump ) == A );

		for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
			{
			ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

			neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position );

			if( ( NULL != neighbor ) && ( __DATA( neighbor, clump ) == B ) && ( ! __GET( voxel, __CLUMP_IFACE ) ) )
				{
				if( NS_FAILURE( ns_list_push_back( &S1, voxel ), error ) )
					goto _NS_SPINES_CLUMPS_EVAL_CONNECTIVITY_EXIT;

				__SET( voxel, __CLUMP_IFACE );

				if( NULL == iface_brightest || voxel->intensity > iface_brightest->intensity )
					iface_brightest = voxel;
				}
			}
		}

	NS_LIST_FOREACH( &S1, v )
		{
		voxel = ns_list_iter_get_object( v );
		ns_assert( __DATA( voxel, clump ) == A );

		for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
			{
			ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

			neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position );

			if( ( NULL != neighbor ) && ( __DATA( neighbor, clump ) == A ) && ( ! __GET( neighbor, __CLUMP_IFACE ) ) )
				{
				if( NS_FAILURE( ns_list_push_back( &S2, neighbor ), error ) )
					goto _NS_SPINES_CLUMPS_EVAL_CONNECTIVITY_EXIT;

				__SET( neighbor, __CLUMP_IFACE );
				}
			}
		}

	//min_dts = NS_DOUBLE_MAX;
	//max_dts = -NS_DOUBLE_MAX;
	sum1    = 0.0f;
	NS_LIST_FOREACH( &S1, v )
		{
		voxel = ns_list_iter_get_object( v );
		ns_assert( __DATA( voxel, clump ) == A );
		sum1 += voxel->intensity;

		//if( __DATA( voxel, distance ) < min_dts )
		//	min_dts = __DATA( voxel, distance );

		//if( max_dts < __DATA( voxel, distance ) )
		//	max_dts = __DATA( voxel, distance );
		}

	sum2 = 0.0f;
	NS_LIST_FOREACH( &S2, v )
		{
		voxel = ns_list_iter_get_object( v );
		ns_assert( __DATA( voxel, clump ) == A );
		sum2 += voxel->intensity;
		}

	pinch = sum2 / sum1;

	//a = ( A->max_dts + A->min_dts ) / 2.0;
	//b = ( B->max_dts + B->min_dts ) / 2.0;
	//c = NS_ABS( a - b );
	//orientation = ( max_dts - min_dts ) / c;

	higher    = ( __DATA( A->maximum, distance ) > __DATA( B->maximum, distance ) ) ? A : B;
	lower     = ( __DATA( A->minimum, distance ) < __DATA( B->minimum, distance ) ) ? A : B;
	range     = __DATA( higher->minimum, distance ) - __DATA( lower->maximum, distance );
	range     = NS_ABS( range );
	dts_range = range / ( __DATA( higher->maximum, distance ) - __DATA( lower->minimum, distance ) );

	ns_assert( NULL != iface_brightest );
	intensity_range = ( A->max_intensity - iface_brightest->intensity ) / ( A->max_intensity - A->min_intensity );

	if( print )
		ns_print( " %d(%f,%f,%f)", B->id, pinch, dts_range, intensity_range );

	/* *connect =
		( ( ! A->touches_clump0 ) && ( pinch < spines->max_clump_pinch ) ) ||
		( dts_range < spines->max_clump_dts_range ) ||
		( intensity_range < spines->max_clump_intensity_range ); */

	//if( ! A->touches_clump0 )// Detached clump
	//	{
		*connect =
			( pinch < spines->max_clump_pinch ) ||
			( dts_range < spines->max_clump_dts_range );
	//	}
	//else // Attached clump
	//	{
	//	*connect =
	//		( dts_range < spines->max_clump_dts_range ) ||
	//		( intensity_range < spines->max_clump_intensity_range );
	//	}


	if( *connect && A->touches_clump0 && B->touches_clump0 )
		if( __DATA( A->origin, dts_id ) != __DATA( B->origin, dts_id ) )
			*connect = NS_FALSE;


	_NS_SPINES_CLUMPS_EVAL_CONNECTIVITY_EXIT:

	ns_list_destruct( &S1 );
	ns_list_destruct( &S2 );

	return error;
	}


NS_PRIVATE NsError _ns_spine_clump_compute_mergers( NsSpineClump *clump, NsSpines *spines, nsboolean print )
	{
	nslistiter   v;
	NsVoxel     *voxel, *neighbor, N;
	nsint        o;
	nsboolean    connect;
	NsError      error;


	if( ns_list_is_empty( &clump->voxels ) )
		return ns_no_error();

	ns_assert( ns_list_is_empty( &clump->seen ) );

	NS_LIST_FOREACH( &clump->voxels, v )
		{
		voxel = ns_list_iter_get_object( v );
		ns_assert( __DATA( voxel, clump ) == clump );

		for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
			{
			ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + o );

			if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) &&
				 NULL != __DATA( neighbor, clump ) &&
				 0 != __DATA( neighbor, clump )->id &&
				 __DATA( neighbor, clump ) != __DATA( voxel, clump ) &&
				 ! _ns_spine_clump_has_been_seen( clump, __DATA( neighbor, clump ) ) )
				{
				if( NS_FAILURE(
						_ns_spine_clumps_eval_connectivity(
							clump,
							__DATA( neighbor, clump ),
							spines,
							print,
							&connect
							),
						error ) )
					return error;

				//if( conn >= spines->min_clump_connectivity )
				//	{  /* Dont merge */  }
				//else if( ! clump->touches_clump0 )

				if( connect )
					{
					/* Form an undirected graph, essentially. */

					if( NS_FAILURE( ns_list_push_back( &clump->connections, __DATA( neighbor, clump ) ), error ) )
						return error;

					if( NS_FAILURE( ns_list_push_back( &__DATA( neighbor, clump )->connections, clump ), error ) )
						return error;
					}

				if( NS_FAILURE( ns_list_push_back( &clump->seen, __DATA( neighbor, clump ) ), error ) )
					return error;
				}
			}
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_spines_compute_clump_mergers( NsSpines *spines )
	{
	nslistiter  c;
	NsError     error;


	ns_assert( NULL != spines );

	ns_progress_set_title( spines->progress, "Computing clump mergers..." );
	ns_progress_num_iters( spines->progress, ns_list_size( &spines->clumps ) );
	ns_progress_begin( spines->progress );

	NS_LIST_FOREACH( &spines->clumps, c )
		{
		if( ns_progress_cancelled( spines->progress ) )
			return ns_no_error();

		if( NS_FAILURE( _ns_spine_clump_compute_mergers( ns_list_iter_get_object( c ), spines, NS_FALSE ), error ) )
			return error;

		ns_progress_next_iter( spines->progress );
		}

	ns_progress_end( spines->progress );

	return ns_no_error();
	}


NS_PRIVATE NsError ns_spines_find_all_clump_mergers( NsSpines *spines )
	{
	//nslistiter     iter;
	//NsSpineClump  *clump;
	NsError        error;


	_ns_spines_init_clumps_for_merging( spines );

	if( NS_FAILURE( _ns_spines_compute_clump_mergers( spines ), error ) )
		return error;

/*TEMP*///ns_println( "# of iface voxels = " NS_FMT_ULONG, ns_list_size( &spines->iface_voxels ) );

	//NS_LIST_FOREACH( &spines->clumps, iter )
	//	{
	//	clump = ns_list_iter_get_object( iter );

	//	if( NS_FAILURE( _ns_spines_do_execute_clump_mergers( spines, clump, clump ), error ) )
	//		return error;
	//	}

	return ns_no_error();
	}


NsError ns_model_spines_print_clump_mergers( NsModel *model, nsint x, nsint y, nsint z )
	{
	NsSpines      *spines;
	NsVector3i     V;
	NsVoxel       *voxel;
	NsSpineClump  *clump;
	NsError        error;


	ns_assert( NULL != model );

	spines = model->spines;
	ns_assert( NULL != spines );

	V.x = x;
	V.y = y;
	V.z = z;

	if( NULL == ( voxel = ns_voxel_table_find( &spines->voxel_table, &V ) ) )
		return ns_no_error();

	clump = __DATA( voxel, clump );

	if( NULL == clump )
		return ns_no_error();

	ns_assert( __DATA( voxel, clump_id ) == clump->id );

	if( 0 == clump->id )
		return ns_no_error();

	_ns_spine_clump_init_for_merging( clump );

	ns_print( NS_FMT_INT " :", clump->id );

	if( NS_FAILURE( _ns_spine_clump_compute_mergers( clump, spines, NS_TRUE ), error ) )
		return error;

	ns_print_newline();
	return ns_no_error();
	}
