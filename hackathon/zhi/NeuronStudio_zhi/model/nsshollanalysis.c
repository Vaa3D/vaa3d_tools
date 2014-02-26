#include "nsshollanalysis.h"


typedef struct _NsShollAnnulus
	{
	nssize     index;
	nsfloat    start_radius;
	nsfloat    end_radius;
	nsuint     total_num_spines;
	nsuint     def_clssfr_num_spines[ NS_SPINE_NUM_TYPES ];
	nsuint    *ann_clssfr_num_spines;
	nsdouble   total_dend_volume;
	nsdouble   total_dend_sa;
	nsdouble   total_dend_length;
	nsuint     num_branch_points;
	}
	NsShollAnnulus;


NS_PRIVATE ns_sholl_annulus_new
	(
	NsShollAnnulus  **ret_annulus,
	nssize            index,
	nsfloat           start_radius,
	nsfloat           end_radius,
	NsShollAnalysis  *sa
	)
	{
	NsShollAnnulus *annulus;

	ns_assert( NULL != ret_annulus );
	ns_assert( NULL != sa );

	if( NULL == ( annulus = ns_new0( NsShollAnnulus ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	annulus->index                 = index;
	annulus->start_radius          = start_radius;
	annulus->end_radius            = end_radius;
	annulus->ann_clssfr_num_spines = NULL;

	if( NULL != sa->classifier )
		if( NULL == ( annulus->ann_clssfr_num_spines =
				ns_new_array0( nsuint, ns_spines_classifier_num_classes( sa->classifier ) ) ) )
			{
			ns_delete( annulus );
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}

	*ret_annulus = annulus;
	return ns_no_error();
	}


NS_PRIVATE ns_sholl_annulus_delete( NsShollAnnulus *annulus )
	{
	ns_assert( NULL != annulus );

	ns_delete( annulus->ann_clssfr_num_spines );
	ns_delete( annulus );
	}


void ns_sholl_analysis_construct( NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );

	ns_list_construct( &sa->annuli, ns_sholl_annulus_delete );
	sa->classifier = NULL;
	}


void ns_sholl_analysis_destruct( NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );
	ns_list_destruct( &sa->annuli );
	}


nssize ns_sholl_analysis_size( const NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );
	return ns_list_size( &sa->annuli );
	}	


const NsVector3f* ns_sholl_analysis_origin( const NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );
	return &sa->origin;
	}


void ns_sholl_analysis_clear( NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );

	ns_list_clear( &sa->annuli );
	sa->classifier = NULL;
	}


NS_PRIVATE NsError _ns_sholl_analysis_init( NsShollAnalysis *sa )
	{
	NsShollAnnulus  *annulus;
	nsfloat          start_radius, end_radius;
	nssize           i;
	NsError          error;


	ns_assert( NULL != sa );
	ns_assert( ns_list_is_empty( &sa->annuli ) );

	ns_assert( NULL == sa->classifier );
	sa->classifier = ns_spines_classifier_get();

	start_radius = sa->min_radius;
	end_radius   = start_radius + sa->radius_delta;

	ns_assert( 1 <= sa->num_annuli );

	for( i = 0; i < sa->num_annuli; ++i )
		{
		/* The radius of the last annulus can be different than the radius delta. */
		if( i == sa->num_annuli - 1 )
			end_radius = sa->max_radius;

		/*TEMP*/
		/*ns_println(
			"Annulus " NS_FMT_ULONG ": >= " NS_FMT_DOUBLE " and < " NS_FMT_DOUBLE,
			i + 1,
			start_radius,
			end_radius
			);*/

		if( NS_FAILURE( ns_sholl_annulus_new( &annulus, i, start_radius, end_radius, sa ), error ) )
			return error;

		if( NS_FAILURE( ns_list_push_back( &sa->annuli, annulus ), error ) )
			{
			ns_sholl_annulus_delete( annulus );
			return error;
			}

		start_radius += sa->radius_delta;
		end_radius   += sa->radius_delta;
		}

	return ns_no_error();
	}


#define _NS_MODEL_VERTEX_SET_SHOLL_DISTANCE( curr_vertex, distance )\
	ns_model_vertex_set_data( (curr_vertex), NS_MODEL_VERTEX_DATA_SLOT1, NS_FLOAT_TO_POINTER( (distance) ) )

#define _NS_MODEL_VERTEX_GET_SHOLL_DISTANCE( curr_vertex )\
	NS_POINTER_TO_FLOAT( ns_model_vertex_get_data( (curr_vertex), NS_MODEL_VERTEX_DATA_SLOT1 ) )


NS_PRIVATE nsfloat _ns_sholl_analysis_max_distance( NsShollAnalysis *sa )
	{
	nsfloat        max_distance;
	nsfloat        distance;
	nsmodelvertex  curr_vertex, end_vertices;
	nsspine        curr_spine, end_spines;
	NsVector3f     P;


	ns_progress_set_title( sa->progress, "Finding Sholl analysis maximum distance..." );

	/*TEMP???*/
	ns_assert( 1 == ns_model_num_origins( sa->model ) );
	ns_model_vertex_get_position( ns_model_origin_vertex( ns_model_begin_origins( sa->model ) ), &sa->origin );

	max_distance = -NS_FLOAT_MAX;

	curr_vertex  = ns_model_begin_vertices( sa->model );
	end_vertices = ns_model_end_vertices( sa->model );

	for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		ns_model_vertex_get_position( curr_vertex, &P );

		distance = ns_vector3f_distance( &P, &sa->origin );

		/* NOTE: We save the sholl distance to avoid having to call ns_vector3f_distance()
			(and hence a square root) many times later when the actual analysis is run. */
		_NS_MODEL_VERTEX_SET_SHOLL_DISTANCE( curr_vertex, distance );

		/* NOTE: Add the radius to give "room" for the center of the vertex to fall in the last annulus. */
		distance += ns_model_vertex_get_radius( curr_vertex );

		if( max_distance < distance )
			max_distance = distance;
		}

	curr_spine = ns_model_begin_spines( sa->model );
	end_spines = ns_model_end_spines( sa->model );

	for( ; ns_spine_not_equal( curr_spine, end_spines ); curr_spine = ns_spine_next( curr_spine ) )
		{
		ns_spine_get_position( curr_spine, &P );

		distance = ns_vector3f_distance( &P, &sa->origin );

		/* NOTE: We save the sholl distance to avoid having to call ns_vector3f_distance()
			(and hence a square root) many times later when the actual analysis is run. */
		ns_spine_set_sholl_distance( curr_spine, distance );

		if( max_distance < distance )
			max_distance = distance;
		}

	return max_distance;
	}


NS_PRIVATE void _ns_sholl_analysis_run_spines( NsShollAnalysis *sa )
	{
	nslistiter       curr_annulus, end_annuli;
	NsShollAnnulus  *annulus;
	nsspine          curr_spine, end_spines;	
	nsfloat          distance;
	nsint            num_classes;
	nsint            type;


	ns_progress_set_title( sa->progress, "Running Sholl analysis on spines..." );
   ns_progress_num_iters( sa->progress, ns_list_size( &sa->annuli ) );
	ns_progress_begin( sa->progress );

	num_classes = ( nsint )( NULL != sa->classifier ? ns_spines_classifier_num_classes( sa->classifier ) : 0 );

	curr_annulus = ns_list_begin( &sa->annuli );
	end_annuli   = ns_list_end( &sa->annuli );

	for( ; ns_list_iter_not_equal( curr_annulus, end_annuli );
			 curr_annulus = ns_list_iter_next( curr_annulus ) )
		{
      if( ns_progress_cancelled( sa->progress ) )
         return;

		annulus = ns_list_iter_get_object( curr_annulus );

		curr_spine = ns_model_begin_spines( sa->model );
		end_spines = ns_model_end_spines( sa->model );

		for( ; ns_spine_not_equal( curr_spine, end_spines ); curr_spine = ns_spine_next( curr_spine ) )
			{		
			distance = ns_spine_get_sholl_distance( curr_spine );

			if( annulus->start_radius <= distance && distance < annulus->end_radius )
				{
				++(annulus->total_num_spines);

				type = ( nsint )ns_spine_get_type( curr_spine );

				if( NULL == sa->classifier )
					{
					ns_assert( type < ( nsint )NS_SPINE_NUM_TYPES );
					ns_assert( ( nsint )NS_SPINE_INVALID != type );

					++(annulus->def_clssfr_num_spines[ type ]);
					}
				else
					{
					ns_assert( 0 < type );
					ns_assert( type <= num_classes );

					++(annulus->ann_clssfr_num_spines[ type - 1 ]);
					}
				}
			}

		ns_progress_next_iter( sa->progress );
		}

	ns_progress_end( sa->progress );
	}


typedef struct _NsShollResult
	{  
	nsdouble  volume;
	nsdouble  surface;
	nsdouble  length;
	}
	NsShollResult;


NS_PRIVATE nsboolean _ns_ray3d_intersects_sphere_ex
	(
	const NsRay3d    *R,
	const NsSphered  *S,
	nsdouble         *t0,
	nsdouble         *t1
	)
	{
	nsdouble    a, b, c, d, sqrt_of_d;
	NsVector3d  V;


	ns_vector3d_sub( &V, &R->O, &S->O );

	a = ns_vector3d_dot( &R->D, &R->D );
	b = ns_vector3d_dot( &R->D, &V ) * 2.0;
	c = ns_vector3d_dot( &V, &V ) - NS_POW2( S->r );
	
	d = NS_POW2( b ) - 4.0 * a * c;

	if( d < 0.0 )
		return NS_FALSE;
	else
		{
		sqrt_of_d = ns_sqrt( d );

		*t0 = ( -b + sqrt_of_d ) / ( 2.0 * a );
		*t1 = ( -b - sqrt_of_d ) / ( 2.0 * a );
		}

	return NS_TRUE;
	}


enum{ NS_SHOLL_SMALL, NS_SHOLL_BIG };

typedef struct _NsShollHit
	{
	nsdouble  t;
	nsenum    s;
	}
	NsShollHit;


NS_PRIVATE int _ns_sholl_hit_compare_func( const NsShollHit *h1, const NsShollHit *h2 )
	{
	if( h1->t > h2->t )
		return 1;
	else if( h1->t < h2->t )
		return -1;
	else
		return 0;
	}


NS_PRIVATE void _ns_sholl_analysis_run_frustum
	(
	NsShollAnalysis          *sa,
	const NsShollAnnulus     *annulus,
	const NsConicalFrustumd  *F,
	NsShollResult            *result
	)
	{
	NsRay3d     R;
	NsSphered   S;
	NsShollHit  H[4];
	nsint       i, n;
	nsdouble    min, max;
	nsdouble    r1, r2;


	ns_assert( NULL != sa );
	ns_assert( NULL != annulus );
	ns_assert( NULL != F );
	ns_assert( NULL != result );

	result->volume  = 0.0;
	result->surface = 0.0;
	result->length  = 0.0;

	ns_vector3f_to_3d( &sa->origin, &S.O );

	R.O = F->C1;
	ns_vector3d_sub( &R.D, &F->C2, &F->C1 );

	n = 0;

	H[0].s = H[1].s = NS_SHOLL_SMALL;
	H[2].s = H[3].s = NS_SHOLL_BIG;

	S.r = annulus->start_radius;
	if( _ns_ray3d_intersects_sphere_ex( &R, &S, &(H[0].t), &(H[1].t) ) )
		{
		H[ n     ] = H[0];
		H[ n + 1 ] = H[1];
		n += 2;
		}

	S.r = annulus->end_radius;
	if( _ns_ray3d_intersects_sphere_ex( &R, &S, &(H[2].t), &(H[3].t) ) )
		{
		H[ n     ] = H[2];
		H[ n + 1 ] = H[3];
		n += 2;
		}

	_ns_qsort( H, n, sizeof( NsShollHit ), _ns_sholl_hit_compare_func );

	for( i = 0; i < n - 1; ++i )
		if( H[ i ].s == NS_SHOLL_BIG || H[ i + 1 ].s == NS_SHOLL_BIG )
			{
			min = NS_MAX( 0.0, H[ i     ].t );
			max = NS_MIN( 1.0, H[ i + 1 ].t );

			if( max > min )
				{
				result->length = ( max - min ) * ns_vector3d_mag( &R.D );

				r1 = ( 1.0 - min ) * F->r1 + min * F->r2;
				r2 = ( 1.0 - max ) * F->r1 + max * F->r2;

				result->surface = NS_PI_D * ( r1 + r2 ) * ns_sqrt( NS_POW2( r1 - r2 ) + NS_POW2( result->length ) );

				result->volume = ( 1.0 / 3.0 ) * NS_PI_D * result->length * ( NS_POW2( r1 ) + ( r1 * r2 ) + NS_POW2( r2 ) );
				}
			}
	}


NS_PRIVATE void _ns_sholl_analysis_do_run_dendrites
	(
	NsShollAnalysis  *sa,
	NsShollAnnulus   *annulus,
	nsmodelvertex     parent_vertex,
	nsmodelvertex     curr_vertex
	)
	{
	nsmodeledge        curr_edge;
	nsmodeledge        end_edges;
	nsmodelvertex      dest_vertex;
	NsConicalFrustumd  F;
	NsShollResult      result;
	nsfloat            distance;


	if( ns_model_vertex_is_junction( curr_vertex ) )
		{
		distance = _NS_MODEL_VERTEX_GET_SHOLL_DISTANCE( curr_vertex );

		if( annulus->start_radius <= distance && distance < annulus->end_radius )
			++(annulus->num_branch_points);
		}

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			ns_model_edge_to_conical_frustum( curr_edge, &F );

			_ns_sholl_analysis_run_frustum( sa, annulus, &F, &result );

			/*TEMP*///ns_println( NS_FMT_DOUBLE " " NS_FMT_DOUBLE " " NS_FMT_DOUBLE, result.volume, result.surface, result.length );

			annulus->total_dend_volume += result.volume;
			annulus->total_dend_sa     += result.surface;
			annulus->total_dend_length += result.length;

			_ns_sholl_analysis_do_run_dendrites( sa, annulus, curr_vertex, dest_vertex );
			}
		}
	}


NS_PRIVATE void _ns_sholl_analysis_run_dendrites( NsShollAnalysis *sa )
	{
	nslistiter       curr_annulus, end_annuli;
	NsShollAnnulus  *annulus;
	nsmodelorigin    curr_origin;
	nsmodelorigin    end_origins;


	ns_progress_set_title( sa->progress, "Running Sholl analysis on dendrites..." );
   ns_progress_num_iters( sa->progress, ns_list_size( &sa->annuli ) );
	ns_progress_begin( sa->progress );

	curr_annulus = ns_list_begin( &sa->annuli );
	end_annuli   = ns_list_end( &sa->annuli );

	for( ; ns_list_iter_not_equal( curr_annulus, end_annuli );
			 curr_annulus = ns_list_iter_next( curr_annulus ) )
		{
      if( ns_progress_cancelled( sa->progress ) )
         return;

		annulus = ns_list_iter_get_object( curr_annulus );

		/*TEMP*///ns_println( "Annulus " NS_FMT_ULONG, annulus->index );

		curr_origin = ns_model_begin_origins( sa->model );
		end_origins = ns_model_end_origins( sa->model );

		for( ; ns_model_origin_not_equal( curr_origin, end_origins );
				 curr_origin = ns_model_origin_next( curr_origin ) )
			_ns_sholl_analysis_do_run_dendrites(
				sa,
				annulus,
				NS_MODEL_VERTEX_NIL,
				ns_model_origin_vertex( curr_origin )
				);

		/*TEMP*///ns_print_newline();

		ns_progress_next_iter( sa->progress );
		}

	ns_progress_end( sa->progress );
	}


NsError ns_sholl_analysis_run
	(
	NsShollAnalysis  *sa,
	NsModel          *model,
	nsfloat           min_radius,
	nsfloat           max_radius,
	nsfloat           radius_delta,
	NsProgress       *progress
	)
	{
	nsfloat  max_distance;
	NsError  error;


	ns_assert( NULL != sa );
	ns_assert( NULL != model );

	ns_sholl_analysis_clear( sa );

	sa->model        = model;
	sa->min_radius   = min_radius;
	sa->max_radius   = max_radius;
	sa->radius_delta = radius_delta;
	sa->progress     = progress;

	if( 0 == ns_model_num_vertices( model ) )
		return ns_no_error();

	/*TEMP!!!!!!!*/
	ns_println( "Sholl PASSED Min Radius   = " NS_FMT_DOUBLE, sa->min_radius );
	ns_println( "Sholl PASSED Max Radius   = " NS_FMT_DOUBLE, sa->max_radius );
	ns_println( "Sholl PASSED Radius Delta = " NS_FMT_DOUBLE, sa->radius_delta );

	max_distance = _ns_sholl_analysis_max_distance( sa );
	ns_assert( 0.0f <= max_distance );

	/*TEMP!!!!!!!*/
	ns_println( "Sholl Max Distance = " NS_FMT_DOUBLE, max_distance );

	/* Protect against the user entering invalid values. */

	if( max_distance < sa->max_radius )
		sa->max_radius = max_distance;

	if( sa->min_radius > sa->max_radius )
		NS_SWAP( nsfloat, sa->min_radius, sa->max_radius );

	if( sa->radius_delta > sa->max_radius - sa->min_radius )
		sa->radius_delta = sa->max_radius - sa->min_radius;

	/*TEMP!!!!!!!*/
	ns_println( "Sholl ACTUAL Min Radius   = " NS_FMT_DOUBLE, sa->min_radius );
	ns_println( "Sholl ACTUAL Max Radius   = " NS_FMT_DOUBLE, sa->max_radius );
	ns_println( "Sholl ACTUAL Radius Delta = " NS_FMT_DOUBLE, sa->radius_delta );

	sa->num_annuli = ( nssize )ns_ceil( ( sa->max_radius - sa->min_radius ) / sa->radius_delta );

	/*TEMP!!!!!!!*/
	ns_println( "Sholl # of Annuli = " NS_FMT_ULONG, sa->num_annuli );

	if( NS_FAILURE( _ns_sholl_analysis_init( sa ), error ) )
		return error;

	_ns_sholl_analysis_run_spines( sa );
	_ns_sholl_analysis_run_dendrites( sa );

	/* NULL these out so these pointers are only valid while the analysis
		is running. */
	sa->model    = NULL;
	sa->progress = NULL;

	return ns_no_error();
	}


NsError ns_sholl_analysis_write( const NsShollAnalysis *sa, const nschar *file_name )
	{
	NsFile                 file;
	NsString               name;
	nsint                  max_class_name_length;
	nsint                  class_value;
	nsint                  num_classes;
	const nschar          *class_name;
	nslistiter             curr_annulus, end_annuli;
	const NsShollAnnulus  *annulus;
	NsError                error;


	#define _NS_SHOLL_ANALYSIS_WRITE_WIDTH      16
	#define _NS_SHOLL_ANALYSIS_WRITE_PRECISION   4

	ns_assert( NULL != sa );
	ns_assert( NULL != file_name );

	error = ns_no_error();

   ns_file_construct( &file );
	ns_string_construct( &name );

	if( NULL != sa->classifier )
		if( NS_FAILURE( ns_spines_classifier_init_lookup( sa->classifier ), error ) )
			goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

   if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_WRITE ), error ) )
      goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

   if( NS_FAILURE(
         ns_file_print(
            &file,
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
				NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "START-RADIUS",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "END-RADIUS",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "DEND-LENGTH",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "DEND-VOLUME",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "DEND-SA",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "BRANCH-POINTS",
				_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "SPINE-COUNT"
            ),
         error ) )
      goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

	/* This just "quiets" the compiler about possible uninitialized variables. */
	num_classes           = 0;
	max_class_name_length = 0;

	if( NULL != sa->classifier )
		{
		max_class_name_length  = ( nsint )ns_spines_classifier_max_class_name_length( sa->classifier );
		max_class_name_length += ( nsint )ns_ascii_strlen( "-COUNT" );

		if( max_class_name_length < _NS_SHOLL_ANALYSIS_WRITE_WIDTH )
			max_class_name_length = _NS_SHOLL_ANALYSIS_WRITE_WIDTH;
		else
			++max_class_name_length; /* Add a space. */

		num_classes = ( nsint )ns_spines_classifier_num_classes( sa->classifier );

		for( class_value = 1; class_value <= num_classes; ++class_value )
			{
			class_name = ns_spines_classifier_lookup( sa->classifier, class_value );
			ns_verify( ! ns_ascii_streq( class_name, NS_SPINES_CLASSIFIER_NA ) );

			if( NS_FAILURE( ns_string_set( &name, class_name ), error ) )
				goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

			ns_string_upper( &name );

			if( NS_FAILURE( ns_string_append( &name, "-COUNT" ), error ) )
				goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

			if( NS_FAILURE(
					ns_file_print(
						&file,
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
						max_class_name_length, ns_string_get( &name )
						),
					error ) )
				goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;
			}
		}
	else
		{
		if( NS_FAILURE(
				ns_file_print(
					&file,
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "STUBBY-COUNT",
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "THIN-COUNT",
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "MUSHROOM-COUNT",
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, "OTHER-COUNT"
					),
				error ) )
			goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;
		}

	if( NS_FAILURE( ns_file_print( &file, NS_STRING_NEWLINE ), error ) )
      goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

	curr_annulus = ns_list_begin( &sa->annuli );
	end_annuli   = ns_list_end( &sa->annuli );

	for( ; ns_list_iter_not_equal( curr_annulus, end_annuli );
			 curr_annulus = ns_list_iter_next( curr_annulus ) )
		{
		annulus = ns_list_iter_get_object( curr_annulus );

		if( NS_FAILURE(
				ns_file_print(
					&file,
					NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
					NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
					NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
					NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
					NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT )
					NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT ),
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, _NS_SHOLL_ANALYSIS_WRITE_PRECISION, annulus->start_radius,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, _NS_SHOLL_ANALYSIS_WRITE_PRECISION, annulus->end_radius,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, _NS_SHOLL_ANALYSIS_WRITE_PRECISION, annulus->total_dend_length,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, _NS_SHOLL_ANALYSIS_WRITE_PRECISION, annulus->total_dend_volume,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, _NS_SHOLL_ANALYSIS_WRITE_PRECISION, annulus->total_dend_sa,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->num_branch_points,
					_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->total_num_spines
					),
				error ) )
			goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;

		if( NULL != sa->classifier )
			{
			for( class_value = 1; class_value <= num_classes; ++class_value )
				if( NS_FAILURE(
					ns_file_print(
						&file,
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT ),
						/* NOTE: Subtract one since the classifier class values are 1-based but the
							C array of unsigned integers is 0-based. */
						max_class_name_length, annulus->ann_clssfr_num_spines[ class_value - 1 ]
						),
					error ) )
				goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;
			}
		else
			{
			if( NS_FAILURE(
					ns_file_print(
						&file,
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT )
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT )
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT )
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_UINT ),
						_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->def_clssfr_num_spines[ NS_SPINE_STUBBY   ],
						_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->def_clssfr_num_spines[ NS_SPINE_THIN     ],
						_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->def_clssfr_num_spines[ NS_SPINE_MUSHROOM ],
						_NS_SHOLL_ANALYSIS_WRITE_WIDTH, annulus->def_clssfr_num_spines[ NS_SPINE_OTHER    ]
						),
					error ) )
				goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;
			}

		if( NS_FAILURE( ns_file_print( &file, NS_STRING_NEWLINE ), error ) )
			goto _NS_SHOLL_ANALYSIS_WRITE_EXIT;
		}

	_NS_SHOLL_ANALYSIS_WRITE_EXIT:

   ns_file_destruct( &file );
	ns_string_destruct( &name );

	if( NULL != sa->classifier )
		ns_spines_classifier_finalize_lookup( sa->classifier );

   return error;
	}


NS_PRIVATE const NsShollAnnulus* _ns_sholl_annulus( const nsshollannulus A )
	{  return ns_list_iter_get_object( A );  }


nsshollannulus ns_sholl_analysis_begin( const NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );
	return ns_list_begin( &sa->annuli );
	}


nsshollannulus ns_sholl_analysis_end( const NsShollAnalysis *sa )
	{
	ns_assert( NULL != sa );
	return ns_list_end( &sa->annuli );
	}


nssize ns_sholl_annulus_index( const nsshollannulus A )
	{  return _ns_sholl_annulus( A )->index;  }


nsfloat ns_sholl_annulus_start_radius( const nsshollannulus A )
	{  return _ns_sholl_annulus( A )->start_radius;  }


nsfloat ns_sholl_annulus_end_radius( const nsshollannulus A )
	{  return _ns_sholl_annulus( A )->end_radius;  }
