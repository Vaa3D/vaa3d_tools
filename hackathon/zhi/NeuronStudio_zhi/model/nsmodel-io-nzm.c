#include "nsmodel-io-nzm.h"


#define _NS_NZM_MAX_LINE  1024


#define _NS_NZM_NO_SECTION        0
#define _NS_NZM_SECTION_SPINE     1
#define _NS_NZM_SECTION_SOMA      2
#define _NS_NZM_SECTION_DENDRITE  3


#define _NS_NZM_TAG_SPINE     "POINT"
#define _NS_NZM_TAG_SOMA      "CLOSED_CONTOUR"
#define _NS_NZM_TAG_DENDRITE  "ARBOR_WITH_DIAMETER"


NS_PRIVATE NsError _ns_nzm_parse_xyz( const nschar *ptr, NsVector4f *V )
	{
	return 3 == _ns_sscan(
					ptr,
					NS_FMT_DOUBLE
					NS_FMT_DOUBLE
					NS_FMT_DOUBLE,
					&V->x,
					&V->y,
					&V->z
					) ?
		ns_no_error() : ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
	}


NS_PRIVATE NsError _ns_nzm_parse_uint( const nschar *ptr, nsuint *u )
	{
	return 1 == _ns_sscan( ptr, NS_FMT_UINT, u ) ?
			ns_no_error() : ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
	}


NS_PRIVATE NsError _ns_nzm_parse_int( const nschar *ptr, nsint *i )
	{
	return 1 == _ns_sscan( ptr, NS_FMT_INT, i ) ?
			ns_no_error() : ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
	}


NS_PRIVATE NsError _ns_nzm_parse_float( const nschar *ptr, nsfloat *f )
	{
	return 1 == _ns_sscan( ptr, NS_FMT_DOUBLE, f ) ?
			ns_no_error() : ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
	}


NS_PRIVATE NsError _ns_nzm_add_vector( NsList *vectors, const NsVector4f *Vo )
	{
	NsVector4f  *Vc;
	NsError      error;


	if( NULL == ( Vc = ns_new( NsVector4f ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	*Vc = *Vo;

	if( NS_FAILURE( ns_list_push_back( vectors, Vc ), error ) )
		{
		ns_delete( Vc );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE void _ns_nzm_calc_centroid( const NsList *vectors, NsVector4f *centroid )
	{
	nslistiter  curr, next, end;
	nsfloat     circumference;


	ns_vector4f_zero( centroid );
	circumference = 0.0f;

	curr = ns_list_begin( vectors );
	end  = ns_list_end( vectors );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		ns_vector4f_cmpd_add(
			centroid,
			( const NsVector4f* )ns_list_iter_get_object( curr )
			);

		next = ns_list_iter_next( curr );
	
		if( ns_list_iter_equal( next, end ) )
			next = ns_list_begin( vectors );
	
		circumference += ns_vector4f_distance(
								( const NsVector4f* )ns_list_iter_get_object( curr ),
								( const NsVector4f* )ns_list_iter_get_object( next )
								);
		}

	/* The centroid is the "average" vector. */
	ns_vector4f_cmpd_scale( centroid, 1.0f / ns_list_size( vectors ) );

	/* Convert the circumference to a diameter using the formula C = PI * D */
	centroid->w = circumference / NS_PI_F;
	}


NS_PRIVATE void _ns_nzm_delete_vector( nspointer vector )
	{
	ns_assert( NULL != vector );
	ns_delete( vector );
	}


NS_PRIVATE NsError _ns_nzm_parse_branch_info
	(
	nschar  *ptr,
	nsuint  *num_vectors,
	nsint   *parent_cyl,
	nsuint  *num_children
	)
	{
	NS_PRIVATE const nschar* _ns_nzm_branch_seps = " \t";

	NsError error;

	if( NULL == ( ptr = ns_ascii_strtok( ptr, _ns_nzm_branch_seps ) ) )
		return ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NS_FAILURE( _ns_nzm_parse_uint( ptr, num_vectors ), error ) )
		return error;

	if( NULL == ( ptr = ns_ascii_strtok( NULL, _ns_nzm_branch_seps ) ) )
		return ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( ns_ascii_streq( ptr, "nil" ) )
		*parent_cyl = -1;
	else if( NS_FAILURE( _ns_nzm_parse_int( ptr, parent_cyl ), error ) )
		return error;

	if( NULL == ( ptr = ns_ascii_strtok( NULL, _ns_nzm_branch_seps ) ) )
		return ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NS_FAILURE( _ns_nzm_parse_uint( ptr, num_children ), error ) )
		return error;

	if( *num_vectors < 2 )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( 0 == *parent_cyl )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	return ns_no_error();
	}


typedef struct _NzmBranch
	{
	NsList      vertices;
	nslistiter  iter;
	nsuint      num_children;
	nsint       parent_cyl;
	nsint       num_cyls;
	}
	NzmBranch;


NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsmodelvertex, nspointer );


#define _nzm_branch_model_vertex( V )\
	( ( nsmodelvertex )ns_list_iter_get_object( (V) ) )


NS_PRIVATE nsmodelvertex _nzm_branch_curr_model_vertex( NzmBranch *branch )
	{
	ns_assert( ns_list_iter_not_equal( branch->iter, ns_list_end( &branch->vertices ) ) );
	return _nzm_branch_model_vertex( branch->iter );
	}


NS_PRIVATE NsError _nzm_branch_new
	(
	NzmBranch    **p_branch,
	nsuint         num_children,
	nsint          parent_cyl
	)
	{
	NzmBranch *branch;

	*p_branch = NULL;

	if( NULL == ( branch = ns_new( NzmBranch ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_list_construct( &branch->vertices, NULL );

	branch->iter         = ns_list_end( &branch->vertices );
	branch->num_children = num_children;
	branch->parent_cyl   = parent_cyl;

	*p_branch = branch;
	return ns_no_error();
	}


NS_PRIVATE void _nzm_branch_delete( NzmBranch *branch )
	{
	ns_list_destruct( &branch->vertices );
	ns_delete( branch );
	}


NS_PRIVATE NsError _ns_nzm_push_branch( NsStack *branches, nsuint num_children, nsint parent_cyl )
	{
	NzmBranch  *branch;
	NsError     error;


	if( NS_FAILURE( _nzm_branch_new(
							&branch,
							num_children,
							parent_cyl
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_stack_push( branches, branch ), error ) )
		{
		_nzm_branch_delete( branch );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE void _ns_nzm_delete_branch( nspointer branch )
	{  _nzm_branch_delete( ( NzmBranch* )branch );  }


#ifdef NS_DEBUG

/* A branch is initially a series of vertices that looks like...

			  cy1           cy2           cy3      cyN-1         cyN
	(first)-----(first+1)-----(first+2)----- ... -----(last-1)-----(last)

	*cy = a cylinder

	The edges are undirected so the exterior vertices, first and
	last, should have one edge while the interior vertices should
	all have 2 edges.
*/
NS_PRIVATE nsboolean _ns_nzm_validate_branch( const NzmBranch *branch )
	{
	nslistiter     first;
	nslistiter     last;
	nslistiter     curr;
	nslistiter     next;
	nslistiter     end;
	nsmodelvertex  vertex;
	nsmodeledge    edge;
	nsint          cyl_num;


	first = ns_list_begin( &branch->vertices );
	last  = ns_list_rev_begin( &branch->vertices );
	end   = ns_list_end( &branch->vertices );

	ns_assert( ns_list_iter_not_equal( first, end ) );
	ns_assert( ns_list_iter_not_equal( last, end ) );

	if( 1 != ns_model_vertex_num_edges( _nzm_branch_model_vertex( first ) ) )
		return NS_FALSE;

	if( 1 != ns_model_vertex_num_edges( _nzm_branch_model_vertex( last ) ) )
		return NS_FALSE;

	for( curr = ns_list_iter_next( first );
		  ns_list_iter_not_equal( curr, last );
		  curr = ns_list_iter_next( curr ) )
		{
		ns_assert( ns_list_iter_not_equal( curr, end ) );

		if( 2 != ns_model_vertex_num_edges( _nzm_branch_model_vertex( curr ) ) )
			return NS_FALSE;
		}

	/* NOTE: Cylinder numbers are 1-based! */
	cyl_num = 1;

	curr = ns_list_begin( &branch->vertices );
	last = ns_list_rev_begin( &branch->vertices );

	/* Each edge holds a cylinder number. */
	for( ; ns_list_iter_not_equal( curr, last ); curr = next )
		{
		next   = ns_list_iter_next( curr );
		vertex = _nzm_branch_model_vertex( curr );
		edge   = ns_model_vertex_find_edge( vertex, _nzm_branch_model_vertex( next ) );

		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( vertex ) ) );

		if( cyl_num != NS_POINTER_TO_INT( ns_model_edge_get_data( edge ) ) )
			return NS_FALSE;

		++cyl_num;
		}

	return NS_TRUE;
	}

#endif/* NS_DEBUG */


NS_PRIVATE void _ns_nzm_set_branch_cylinders( NzmBranch *branch )
	{
	nslistiter     curr;
	nslistiter     next;
	nslistiter     last;
	nsmodelvertex  vertex;
	nsmodeledge    edge;
	nsint          cyl_num;


	cyl_num = 1;

	curr = ns_list_begin( &branch->vertices );
	last = ns_list_rev_begin( &branch->vertices );

	/* Each edge holds a cylinder number. */
	for( ; ns_list_iter_not_equal( curr, last ); curr = next )
		{
		next   = ns_list_iter_next( curr );
		vertex = _nzm_branch_model_vertex( curr );
		edge   = ns_model_vertex_find_edge( vertex, _nzm_branch_model_vertex( next ) );

		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( vertex ) ) );

		ns_model_edge_set_data( edge, NS_INT_TO_POINTER( cyl_num ), NS_TRUE );
		++cyl_num;
		}
	}


NS_PRIVATE nslistiter _ns_nzm_find_first_cylinder_vertex
	(
	NzmBranch  *branch,
	nsint       cyl_num
	)
	{
	nslistiter     curr;
	nslistiter     next;
	nslistiter     last;
	nsmodelvertex  vertex;
	nsmodeledge    edge;


	curr = ns_list_begin( &branch->vertices );
	last = ns_list_rev_begin( &branch->vertices );

	for( ; ns_list_iter_not_equal( curr, last ); curr = next )
		{
		next   = ns_list_iter_next( curr );
		vertex = _nzm_branch_model_vertex( curr );
		edge   = ns_model_vertex_find_edge( vertex, _nzm_branch_model_vertex( next ) );

		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( vertex ) ) );

		if( cyl_num == NS_POINTER_TO_INT( ns_model_edge_get_data( edge ) ) )
			return curr;
		}

	return NULL;
	}


NS_PRIVATE nslistiter _ns_nzm_find_last_cylinder_vertex
	(
	NzmBranch   *branch,
	nsint        cyl_num
	)
	{
	nslistiter     curr;
	nslistiter     next;
	nslistiter     last;
	nsmodelvertex  vertex;
	nsmodeledge    edge;


	curr = ns_list_rev_begin( &branch->vertices );
	last = ns_list_begin( &branch->vertices );

	for( ; ns_list_iter_not_equal( curr, last ); curr = next )
		{
		next   = ns_list_rev_iter_next( curr );
		vertex = _nzm_branch_model_vertex( curr );
		edge   = ns_model_vertex_find_edge( vertex, _nzm_branch_model_vertex( next ) );

		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( vertex ) ) );

		if( cyl_num == NS_POINTER_TO_INT( ns_model_edge_get_data( edge ) ) )
			return curr;
		}

	return NULL;
	}


#ifdef NS_DEBUG

NS_PRIVATE nsboolean _ns_nzm_validate_cylinder
	(
	nslistiter  first,
	nslistiter  last,
	nsint       cyl_num
	)
	{
	nslistiter     curr;
	nslistiter     next;
	nsmodelvertex  vertex;
	nsmodeledge    edge;


	for( curr = first; ns_list_iter_not_equal( curr, last ); curr = next )
		{
		next   = ns_list_iter_next( curr );
		vertex = _nzm_branch_model_vertex( curr );
		edge   = ns_model_vertex_find_edge( vertex, _nzm_branch_model_vertex( next ) );

		ns_assert( ns_model_edge_not_equal( edge, ns_model_vertex_end_edges( vertex ) ) );

		if( cyl_num != NS_POINTER_TO_INT( ns_model_edge_get_data( edge ) ) )
			return NS_FALSE;
		}

	return NS_TRUE;
	}

#endif/* NS_DEBUG */


NS_PRIVATE nsdouble _ns_nzm_vertex_distance( nslistiter a, nslistiter b )
	{
	NsVector3f A, B;

	ns_model_vertex_get_position( _nzm_branch_model_vertex( a ), &A );
	ns_model_vertex_get_position( _nzm_branch_model_vertex( b ), &B );	

	return ns_vector3f_distance( &A, &B );
	}


NS_PRIVATE nslistiter _ns_nzm_find_closest_cylinder_vertex
	(
	nslistiter  first,
	nslistiter  last,
	nslistiter  target
	)
	{
	nslistiter  stop;
	nslistiter  curr;
	nslistiter  closest;
	nsdouble    min_distance;
	nsdouble    distance;


	stop = ns_list_iter_next( last );
	curr = first;

	min_distance = _ns_nzm_vertex_distance( target, curr );
	closest      = curr;

	curr = ns_list_iter_next( curr );

	for( ; ns_list_iter_not_equal( curr, stop ); curr = ns_list_iter_next( curr ) )
		{
		distance = _ns_nzm_vertex_distance( target, curr );

		if( distance < min_distance )
			{
			min_distance = distance;
			closest      = curr;
			}
		}

	return closest;
	}


NS_PRIVATE NsError _ns_nzm_attach_branch_to_parent( NsModel *model, NzmBranch *child, NzmBranch *parent )
	{
	nslistiter  first;
	nslistiter  last;
	nslistiter  closest;
	nslistiter  root;


	ns_assert( 0 < child->parent_cyl );

	if( parent->num_cyls < child->parent_cyl )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Find the start and end vertex of the parent cylinder
		that child attaches to. */
	first = _ns_nzm_find_first_cylinder_vertex( parent, child->parent_cyl );
	ns_verify( NULL != first );

	last  = _ns_nzm_find_last_cylinder_vertex( parent, child->parent_cyl );
	ns_verify( NULL != last );

	ns_assert( ns_list_iter_not_equal( first, last ) );
	ns_assert( _ns_nzm_validate_cylinder( first, last, child->parent_cyl ) );

	/* Find the closest vertex in the cylinder to the first
		vertex of the child branch. */
	root    = ns_list_begin( &child->vertices );
	closest = _ns_nzm_find_closest_cylinder_vertex( first, last, root );
	
	/* If the closest vertex is "close enough" then merge the
		first vertex of the child to it.	Otherwise insert a new
		vertex into the cylinder and connect the first vertex of
		the child. */
	return ns_model_merge_vertices(
				model,
				_nzm_branch_model_vertex( closest ),
				_nzm_branch_model_vertex( root )
				);
	}


NS_PRIVATE nsboolean _ns_nzm_compare_vector_z( const NsVector4f *V1, const NsVector4f *V2 )
	{  return V1->z < V2->z;  }


NS_PRIVATE NsError _ns_nzm_create_soma( NsModel *model, NsList *centroids )
	{
	nsfloat        x;
	nsfloat        y;
	nsfloat        z;
	nslistiter     curr;
	nslistiter     next;
	nslistiter     end;
	NsVector4f    *C;
	NsVector4f    *V1;
	NsVector4f    *V2;
	NsVector3f     T1;
	NsVector3f     T2;
	nsdouble       r1;
	nsdouble       r2;
	nsdouble       h;
	nsdouble       volume;
	nsdouble       radius;
	nsmodelvertex  soma;
	NsError        error;
	nsmodelorigin  curr_orig;
	nsmodelorigin  end_origs;


	/* Sort the centroid list by their Z coordinate. */
	ns_list_sort( centroids, _ns_nzm_compare_vector_z );
	end = ns_list_end( centroids );

	/* Find the average X and Y value. */
	x = 0.0f;
	y = 0.0f;

	curr = ns_list_begin( centroids );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		C = ( NsVector4f* )ns_list_iter_get_object( curr );

		x += C->x;
		y += C->y;
		}

	x /= ns_list_size( centroids );
	y /= ns_list_size( centroids );

	/* Set the X and Y of the centroids. This assures the centers of
		the soma contours are aligned. */

	curr = ns_list_begin( centroids );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		C = ( NsVector4f* )ns_list_iter_get_object( curr );

		C->x = x;
		C->y = y;
		}

	/* Calculate the combined volume of the conical frustums formed
		between contours. */
	volume = 0.0;

	curr = ns_list_begin( centroids );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		next = ns_list_iter_next( curr );

		if( ns_list_iter_not_equal( next, end ) )
			{
			V1 = ( NsVector4f* )ns_list_iter_get_object( curr );
			V2 = ( NsVector4f* )ns_list_iter_get_object( next );

			/* Remember the 'w' stored a diameter, not a radius! */
			r1 = V1->w / 2.0;
			r2 = V2->w / 2.0;
			h  = ns_vector3f_distance(
						ns_vector4f_to_3f( V1, &T1 ),
						ns_vector4f_to_3f( V2, &T2 )
						);

			/* Got formula off the web at http://jwilson.coe.uga.edu/emt725/Frustum/Frustum.cone.html */
			volume += ( ( NS_PI * h / 3.0 ) * ( r1 * r1 + r1 * r2 + r2 * r2 ) );
			}
		}

	/* Reorder the formula for the volume of a sphere to solve for the radius. */
	radius = pow( ( volume * 3.0 ) / ( 4.0 * NS_PI ), 1.0 / 3.0 );

	/* The z position for the soma should be right in the middle. */
	z = ( ( NsVector4f* )ns_list_iter_get_object( ns_list_begin( centroids ) ) )->z 
			+
		 ( ( NsVector4f* )ns_list_iter_get_object( ns_list_rev_begin( centroids ) ) )->z;

	z /= 2.0f;

	/* Now create a soma, i.e. model vertex and origin, and remove the current
		origins which are really just the tips of the dendrite trees, and connect
		them to the soma vertex. */
	if( NS_FAILURE( ns_model_add_vertex( model, &soma ), error ) )
		return error;

	ns_model_vertex_set_position( soma, ns_vector3f( &T1, x, y, z ) );
	ns_model_vertex_set_radius( soma, ( nsfloat )radius );

	curr_orig = ns_model_begin_origins( model );
	end_origs = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_orig, end_origs );
			 curr_orig = ns_model_origin_next( curr_orig ) )
		if( NS_FAILURE( ns_model_vertex_add_edge(
								soma,
								ns_model_origin_vertex( curr_orig ),
								NULL,
								NULL
								),
								error ) )
			return error;

	ns_model_clear_origins( model );

	if( NS_FAILURE( ns_model_add_origin( model, soma, NULL ), error ) )
		return error;

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_nzm_parse_file
	(
	NsFile   *file,
	NsModel  *model,
	nslong   *line_num
	)
	{
	nsboolean        eof;
	nschar          *ptr;
	NsError          error;
	nsint            section;
	NsList           vectors;
	NsList           centroids;
	nsuint           count;
	nsuint           num_children;
	nsint            parent_cyl;
	nsboolean        reading_diams;
	NsVector4f       V;
	NsVector3f       P;
	nsfloat          diameter;
	NsStack          branches;
	nsmodelvertex    curr_vertex;
	NzmBranch       *curr_branch;
	NzmBranch       *parent_branch;
	NsFinalizeFunc   finalize_func;
	nschar           line[ _NS_NZM_MAX_LINE ];


	eof           = NS_FALSE;
	section       = _NS_NZM_NO_SECTION;
	count         = 0;
	reading_diams = NS_FALSE;
	error         = ns_no_error();

	ns_list_construct( &vectors, _ns_nzm_delete_vector );
	ns_list_construct( &centroids, _ns_nzm_delete_vector );
	ns_stack_construct( &branches, _ns_nzm_delete_branch );

	while( ! eof )
		{
		*line = NS_ASCII_NULL;

		if( NS_FAILURE( ns_file_readline( file, line, _NS_NZM_MAX_LINE, &eof ), error ) )
			goto _NS_NZM_PARSE_FILE_EXIT;

		if( NULL != line_num )
			*line_num = *line_num + 1;

		ptr = line;

		/* Skip leading white-space. */
		while( *ptr && ns_ascii_isspace( *ptr ) )
			++ptr;

		/* Skip empty lines. */
		if( ! *ptr )
			continue;

		switch( section )
			{
			case _NS_NZM_SECTION_SPINE:
				if( NS_FAILURE( _ns_nzm_parse_xyz( ptr, &V ), error ) )
					goto _NS_NZM_PARSE_FILE_EXIT;

				/* NOTE: Currently spines not retained in model. */
				section = _NS_NZM_NO_SECTION;
				break;

			case _NS_NZM_SECTION_DENDRITE:
				if( 0 < count )
					{
					if( ns_stack_is_empty( &branches ) )
						{
						error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
						goto _NS_NZM_PARSE_FILE_EXIT;
						}

					curr_branch = ( NzmBranch* )ns_stack_peek( &branches );

					if( reading_diams )
						{
						/* Read and set a radius. NOTE: Since there are N vertices and
							N-1 diameters, the last vertex will not have one set. */
						if( NS_FAILURE( _ns_nzm_parse_float( ptr, &diameter ), error ) )
							goto _NS_NZM_PARSE_FILE_EXIT;

						/* NOTE: NeuroZoom file stores diameters! */
						ns_model_vertex_set_radius(
							_nzm_branch_curr_model_vertex( curr_branch ),
							diameter / 2.0f
							);

						curr_branch->iter = ns_list_iter_next( curr_branch->iter );
						}
					else
						{
						/* Read and set a coordinate, while creating a model vertex as well. */
						if( NS_FAILURE( _ns_nzm_parse_xyz( ptr, &V ), error ) )
							goto _NS_NZM_PARSE_FILE_EXIT;

						if( NS_FAILURE( ns_model_add_vertex( model, &curr_vertex ), error ) )
							goto _NS_NZM_PARSE_FILE_EXIT;

						ns_model_vertex_set_position( curr_vertex, ns_vector4f_to_3f( &V, &P ) );

						/* Add an edge from the current vertex to the previous one if the
							previous has been set. */
						if( ns_list_iter_not_equal(
								curr_branch->iter,
								ns_list_end( &curr_branch->vertices )
								) )
							if( NS_FAILURE( ns_model_vertex_add_edge(
													curr_vertex,
													_nzm_branch_curr_model_vertex( curr_branch ),
													NULL,
													NULL
													),
													error ) )
								goto _NS_NZM_PARSE_FILE_EXIT;

						/* Add an entry into this branches vertex list. */
						if( NS_FAILURE( ns_list_push_back( &curr_branch->vertices, curr_vertex ), error ) )
							goto _NS_NZM_PARSE_FILE_EXIT;

						curr_branch->iter = ns_list_rev_begin( &curr_branch->vertices );
						}

					--count;
					}
				else
					{
					/* NOTE: For every branch there are N-1 diameters following
						the coordinates. */
					if( ! reading_diams )
						{
						reading_diams = NS_TRUE;

						if( ns_stack_is_empty( &branches ) )
							{
							error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
							goto _NS_NZM_PARSE_FILE_EXIT;
							}

						curr_branch = ( NzmBranch* )ns_stack_peek( &branches );

						if( ns_list_size( &curr_branch->vertices ) < 2 )
							{
							error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
							goto _NS_NZM_PARSE_FILE_EXIT;
							}

						curr_branch->iter = ns_list_begin( &curr_branch->vertices );
						count             = ( nsuint )ns_list_size( &curr_branch->vertices ) - 1;

						curr_branch->num_cyls = ( nsint )count;

						/* NOTE: The line we just read has a diameter so parse it now! */
						if( NS_FAILURE( _ns_nzm_parse_float( ptr, &diameter ), error ) )
							goto _NS_NZM_PARSE_FILE_EXIT;

						ns_model_vertex_set_radius(
							_nzm_branch_curr_model_vertex( curr_branch ),
							diameter / 2.0f
							);

						curr_branch->iter = ns_list_iter_next( curr_branch->iter );

						--count;
						}
					else
						{
						if( ns_stack_is_empty( &branches ) )
							{
							error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
							goto _NS_NZM_PARSE_FILE_EXIT;
							}

						curr_branch = ( NzmBranch* )ns_stack_peek( &branches );

						ns_assert(
							ns_list_iter_equal(
								curr_branch->iter,
								ns_list_rev_begin( &curr_branch->vertices )
								)
							);

						/* Finished reading the current branch! */
						_ns_nzm_set_branch_cylinders( curr_branch );
						ns_assert( _ns_nzm_validate_branch( curr_branch ) );

						/* Attach branches to parents when their child count
							goes to zero. */
						_NS_NZM_RECURSIVE_CONNECT:
						if( 0 == curr_branch->num_children )
							{
							/* NOTE: Temporarily set the stack destructor to NULL so the
								'curr_branch' structure is not deleted. */
							finalize_func = ns_stack_get_finalize_func( &branches );
							ns_stack_set_finalize_func( &branches, NULL );

							ns_stack_pop( &branches );

							ns_stack_set_finalize_func( &branches, finalize_func );

							/* Attach to parent branch, if it has one. */
							if( 0 < curr_branch->parent_cyl )
								{
								if( ns_stack_is_empty( &branches ) )
									{
									_nzm_branch_delete( curr_branch );

									error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
									goto _NS_NZM_PARSE_FILE_EXIT;
									}

								parent_branch = ( NzmBranch* )ns_stack_peek( &branches );

								if( NS_FAILURE( _ns_nzm_attach_branch_to_parent(
														model,
														curr_branch,
														parent_branch
														),
														error ) )
									{
									_nzm_branch_delete( curr_branch );
									goto _NS_NZM_PARSE_FILE_EXIT;
									}
								}
							else
								{
								ns_assert( -1 == curr_branch->parent_cyl );

								/* This branch is the root of a sub-tree of the whole model.
									Temporarily set it as an origin. Later if theres a soma
									they will all become children of the soma vertex. */
								if( NS_FAILURE( ns_model_add_origin(
														model,
														_nzm_branch_model_vertex( ns_list_begin( &curr_branch->vertices ) ),
														NULL
														),
														error ) )
									{
									_nzm_branch_delete( curr_branch );
									goto _NS_NZM_PARSE_FILE_EXIT;
									}
								}

							/* Dont need it anymore since its been attached, so delete it. */
							_nzm_branch_delete( curr_branch );

							/* Is this dendrite completely connected? */
							if( ! ns_stack_is_empty( &branches ) )
								{
								/* Decrement child count since we just attached one. */
								curr_branch = ( NzmBranch* )ns_stack_peek( &branches );

								if( 0 == curr_branch->num_children )
									{
									error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
									goto _NS_NZM_PARSE_FILE_EXIT;
									}

								--curr_branch->num_children;
								goto _NS_NZM_RECURSIVE_CONNECT;
								}
							}

						section = _NS_NZM_NO_SECTION;
						}
					}
				break;

			case _NS_NZM_SECTION_SOMA:
				if( 0 < count )
					{
					/* Read the current vector of the current contour. */
					if( NS_FAILURE( _ns_nzm_parse_xyz( ptr, &V ), error ) )
						goto _NS_NZM_PARSE_FILE_EXIT;

					V.w = 0.0f;

					if( NS_FAILURE( _ns_nzm_add_vector( &vectors, &V ), error ) )
						goto _NS_NZM_PARSE_FILE_EXIT;

					--count;
					}
				else
					{
					_ns_nzm_calc_centroid( &vectors, &V );

					if( NS_FAILURE( _ns_nzm_add_vector( &centroids, &V ), error ) )
						goto _NS_NZM_PARSE_FILE_EXIT;

					section = _NS_NZM_NO_SECTION;
					}
				break;
			}

		if( _NS_NZM_NO_SECTION == section )
			{
			if( NULL != ns_ascii_strstr( ptr, _NS_NZM_TAG_SPINE ) )
				section = _NS_NZM_SECTION_SPINE;
			if( NULL != ns_ascii_strstr( ptr, _NS_NZM_TAG_DENDRITE ) )
				{
				section       = _NS_NZM_SECTION_DENDRITE;
				reading_diams = NS_FALSE;

				ptr = ns_ascii_strstr( ptr, _NS_NZM_TAG_DENDRITE ) +
						ns_ascii_strlen( _NS_NZM_TAG_DENDRITE );

				/* Read the number of points, parent attachment cylinder,
					and number of children for this branch. */
				if( NS_FAILURE( _ns_nzm_parse_branch_info(
										ptr,
										&count,
										&parent_cyl,
										&num_children
										),
										error ) )
					goto _NS_NZM_PARSE_FILE_EXIT;

				if( count < 2 )
					{
					error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
					goto _NS_NZM_PARSE_FILE_EXIT;
					}

				if( NS_FAILURE( _ns_nzm_push_branch( &branches, num_children, parent_cyl ), error ) )
					goto _NS_NZM_PARSE_FILE_EXIT;

				ns_list_clear( &vectors );
				}
			else if( NULL != ns_ascii_strstr( ptr, _NS_NZM_TAG_SOMA ) )
				{
				section = _NS_NZM_SECTION_SOMA;

				ptr = ns_ascii_strstr( ptr, _NS_NZM_TAG_SOMA ) +
						ns_ascii_strlen( _NS_NZM_TAG_SOMA );

				/* Read the number of points in this contour. */
				if( NS_FAILURE( _ns_nzm_parse_uint( ptr, &count ), error ) )
					goto _NS_NZM_PARSE_FILE_EXIT;

				ns_list_clear( &vectors );
				}
			}/* if( no section ) */
		}/* while( lines ) */

	/* Invalid tree structure? */
	if( 0 < ns_stack_size( &branches ) )
		{
		error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		goto _NS_NZM_PARSE_FILE_EXIT;
		}

	if( 0 < ns_list_size( &centroids ) )
		if( NS_FAILURE( _ns_nzm_create_soma( model, &centroids ), error ) )
			goto _NS_NZM_PARSE_FILE_EXIT;

	_NS_NZM_PARSE_FILE_EXIT:

	ns_list_destruct( &vectors );
	ns_list_destruct( &centroids );
	ns_stack_destruct( &branches );

	return error;
	}


NS_PRIVATE NsError _ns_nzm_read_single_file
	(
	NsModel       *model,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	)
	{
	NsFile   file;
	NsError  error;


	NS_USE_VARIABLE( flags );

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_READ ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	error = _ns_nzm_parse_file( &file, model, line_num );

	ns_file_destruct( &file );

	return error;
	}


NsError ns_model_read_nzm
	(
	NsModel       *model,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	)
	{
	ns_assert( NULL != model );
	ns_assert( NULL != file_name );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( model=" NS_FMT_STRING_DOUBLE_QUOTED
		", file_name=" NS_FMT_STRING_DOUBLE_QUOTED
		", flags=" NS_FMT_ULONG
		", line_num=" NS_FMT_POINTER
		" )",
		ns_model_get_name( model ),
		file_name,
		flags,
		line_num
		);

	return _ns_nzm_read_single_file( model, file_name, flags, line_num );
	}
