#include "nsmodel-transform.h"


NS_PRIVATE void _ns_model_transform_calc_center_and_radius( NsModelTransform *xfrm )
	{
   nslistiter     curr;
	nsmodelvertex  V;
   nsfloat        r;
   nsfloat        sum;
	NsVector3f     D;
	NsVector3f     P;


	/* Do a WEIGHTED average on the radii of the vertices,
		i.e. a center of mass. */

	ns_vector3f_zero( &xfrm->C );

   sum = 0.0f;

	NS_LIST_FOREACH( &xfrm->vertices, curr )
      {
		V = ns_list_iter_get_object( curr );

		r = ns_model_vertex_get_radius( V );
		ns_model_vertex_get_position( V, &P );

      sum += r;

      ns_vector3f_cmpd_add( &xfrm->C, ns_vector3f_cmpd_scale( &P, r ) );
      }

   ns_vector3f_cmpd_scale( &xfrm->C, 1.0f / sum );

	ns_vector3f_zero( &xfrm->R );

	NS_LIST_FOREACH( &xfrm->vertices, curr )
      {
		V = ns_list_iter_get_object( curr );

		r = ns_model_vertex_get_radius( V );
		ns_model_vertex_get_position( V, &P );

		D.x = NS_POW2( ( P.x ) - ( xfrm->C.x ) ) + NS_POW2( r );
		D.y = NS_POW2( ( P.y ) - ( xfrm->C.y ) ) + NS_POW2( r );
		D.z = NS_POW2( ( P.z ) - ( xfrm->C.z ) ) + NS_POW2( r );

		if( D.x > xfrm->R.x )xfrm->R.x = D.x;
		if( D.y > xfrm->R.y )xfrm->R.y = D.y;
		if( D.z > xfrm->R.z )xfrm->R.z = D.z;
      }

	xfrm->R.x = ns_sqrtf( xfrm->R.x );
	xfrm->R.y = ns_sqrtf( xfrm->R.y );
	xfrm->R.z = ns_sqrtf( xfrm->R.z );
	}


NS_PRIVATE void _ns_model_transform_destruct( NsModelTransform *xfrm )
	{
	ns_list_destruct( &xfrm->vertices );
	ns_byte_array_destruct( &xfrm->lines );
	}


NS_PRIVATE NsError _ns_model_transform_construct( NsModelTransform *xfrm, NsModel *model )
	{
	nslistiter   V;
	NsVector3f   P;
	NsLine3f    *L;
	NsError      error;


	xfrm->init = NS_FALSE;

	ns_list_construct( &xfrm->vertices, NULL );
	ns_verify( NS_SUCCESS( ns_byte_array_construct( &xfrm->lines, 0 ), error ) );

	if( NS_FAILURE( ns_model_buffer_selected_vertices_ex( model, &xfrm->vertices ), error ) )
		{
		_ns_model_transform_destruct( xfrm );
		return error;
		}

	if( NS_FAILURE(
			ns_byte_array_resize(
				&xfrm->lines,
				ns_list_size( &xfrm->vertices ) * sizeof( NsLine3f )
				),
			error ) )
		{
		_ns_model_transform_destruct( xfrm );
		return error;
		}

	L = NULL;

	if( 0 < ns_byte_array_size( &xfrm->lines ) )
		L = ( NsLine3f* )ns_byte_array_begin( &xfrm->lines );

	NS_LIST_FOREACH( &xfrm->vertices, V )
		{
		ns_model_vertex_get_position( ns_list_iter_get_object( V ), &P );

		L->P1 = P;
		L->P2 = P;

		++L;
		}

	xfrm->init = NS_TRUE;

	_ns_model_transform_calc_center_and_radius( xfrm );

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_model_transform_end
	(
	NsModelTransform  *xfrm,
	nsboolean          confirm,
	NsList            *list
	)
	{
	nslistiter      curr;
	nsmodelvertex   V;
	NsLine3f       *L;
	NsError         error;


	if( ! xfrm->init )
		return ns_no_error();

	if( confirm )
		{
		NS_LIST_FOREACH( &xfrm->vertices, curr )
			{
			V = ns_list_iter_get_object( curr );

			if( ! ns_model_vertex_is_manually_traced( V ) )
				ns_model_vertex_mark_manually_traced_recursive( V, NS_TRUE );

			if( NS_FAILURE( ns_model_auto_iter_list_add_by_vertex( list, V ), error ) )
				return error;
			}
		}
	else
		{
		L = NULL;

		if( 0 < ns_byte_array_size( &xfrm->lines ) )
			L = ( NsLine3f* )ns_byte_array_begin( &xfrm->lines );

		NS_LIST_FOREACH( &xfrm->vertices, curr )
			{
			ns_model_vertex_set_position( ns_list_iter_get_object( curr ), &L->P1 );
			++L;
			}
		}

	_ns_model_transform_destruct( xfrm );
	return ns_no_error();
	}




NsError ns_model_translate_selected_vertices_begin
	(
	NsModelTransform  *xfrm,
	NsModel           *model
	)
	{
	NsError error;

	ns_assert( NULL != xfrm );
	ns_assert( NULL != model );

	if( NS_FAILURE( _ns_model_transform_construct( xfrm, model ), error ) )
		return error;

	xfrm->P1 = xfrm->C;

	return ns_no_error();
	}


void ns_model_translate_selected_vertices_apply
	(
	NsModelTransform  *xfrm,
	const NsVector3f  *T
	)
	{
	NsLine3f    *L;
	nslistiter   V;


	ns_assert( NULL != xfrm );
	ns_assert( NULL != T );

	if( ! xfrm->init )
		return;

	L = NULL;

	if( 0 < ns_byte_array_size( &xfrm->lines ) )
		{
		L = ( NsLine3f* )ns_byte_array_begin( &xfrm->lines );
		ns_vector3f_add( &xfrm->C, &xfrm->P1, T );
		}

	NS_LIST_FOREACH( &xfrm->vertices, V )
		{
		ns_vector3f_add( &L->P2, &L->P1, T );

		ns_model_vertex_set_position( ns_list_iter_get_object( V ), &L->P2 );
		++L;
		}
	}


NsError ns_model_translate_selected_vertices_end
	(
	NsModelTransform  *xfrm,
	nsboolean          confirm,
	NsList            *list
	)
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != list );
	
	return _ns_model_transform_end( xfrm, confirm, list );
	}




NsError ns_model_rotate_selected_vertices_begin
	(
	NsModelTransform  *xfrm,
	NsModel           *model,
	nsint              which
	)
	{
	NsError error;

	ns_assert( NULL != xfrm );
	ns_assert( NULL != model );

	if( NS_FAILURE( _ns_model_transform_construct( xfrm, model ), error ) )
		return error;

	switch( which )
		{
		case NS_XY:
			xfrm->rotate_func = ns_matrix44f_rotation_z;
			break;

		case NS_ZY:
			xfrm->rotate_func = ns_matrix44f_rotation_x;
			break;

		case NS_XZ:
			xfrm->rotate_func = ns_matrix44f_rotation_y;
			break;

		default:
			ns_assert_not_reached();
		}

	xfrm->which   = which;
	xfrm->P1      = xfrm->C;
	xfrm->rotated = NS_FALSE;
	xfrm->angle   = 0.0f;

	return ns_no_error();
	}


NS_PRIVATE nsfloat _ns_model_rotate_selected_vertices_delta
	(
	NsModelTransform  *xfrm,
	const NsVector3f  *P2
	)
	{
	NsVector3f  V1, V2;
	nsfloat     angle1, angle2, delta;


	delta = 0.0f;

	angle1 = angle2 = 0.0f;

	if( xfrm->rotated )
		{
		ns_vector3f_sub( &V1, &xfrm->P1, &xfrm->C );
		ns_vector3f_sub( &V2, P2, &xfrm->C );

		switch( xfrm->which )
			{
			case NS_XY:
				angle1 = ns_atan2f( V1.y, V1.x );
				angle2 = ns_atan2f( V2.y, V2.x );
				break;

			case NS_ZY:
				angle1 = ns_atan2f( V1.z, V1.y );
				angle2 = ns_atan2f( V2.z, V2.y );
				break;

			case NS_XZ:
				angle1 = ns_atan2f( V1.x, V1.z );
				angle2 = ns_atan2f( V2.x, V2.z );
				break;

			default:
				ns_assert_not_reached();
			}

		delta = angle2 - angle1;
		}

	xfrm->P1      = *P2;
	xfrm->rotated = NS_TRUE;

	return delta;
	}


void ns_model_rotate_selected_vertices_apply
	(
	NsModelTransform  *xfrm,
	const NsVector3f  *P
	)
	{
	NsLine3f    *L;
	nslistiter   V;
	NsVector3f   P1;
	NsMatrix44f  M;


	ns_assert( NULL != xfrm );
	ns_assert( NULL != P );

	if( ! xfrm->init )
		return;

	L = NULL;

	if( 0 < ns_byte_array_size( &xfrm->lines ) )
		{
		L = ( NsLine3f* )ns_byte_array_begin( &xfrm->lines );

		xfrm->angle += _ns_model_rotate_selected_vertices_delta( xfrm, P );
		( xfrm->rotate_func )( &M, xfrm->angle );
		}

	NS_LIST_FOREACH( &xfrm->vertices, V )
		{
		/* NOTE: To establish a rotation center, first translate
			the starting point. */
		P1 = L->P1;
		ns_vector3f_cmpd_sub( &P1, &xfrm->C );
		ns_matrix44f_mul_vector3f( &L->P2, &M, &P1 );
		ns_vector3f_cmpd_add( &L->P2, &xfrm->C );

		ns_model_vertex_set_position( ns_list_iter_get_object( V ), &L->P2 );
		++L;
		}
	}


NsError ns_model_rotate_selected_vertices_end
	(
	NsModelTransform  *xfrm,
	nsboolean          confirm,
	NsList            *list
	)
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != list );
	
	return _ns_model_transform_end( xfrm, confirm, list );
	}




nssize ns_model_transform_size( const NsModelTransform *xfrm )
	{
	ns_assert( NULL != xfrm );
	return ns_list_size( &xfrm->vertices );
	}


void ns_model_transform_center( const NsModelTransform *xfrm, NsVector3f *C )
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != C );
	
	*C = xfrm->C;
	}


void ns_model_transform_radius( const NsModelTransform *xfrm, NsVector3f *R )
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != R );

	*R = xfrm->R;
	}
