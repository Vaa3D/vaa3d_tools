#include "nsspines-transform.h"


NS_PRIVATE void _ns_spines_transform_calc_center_and_radius( NsSpinesTransform *xfrm )
	{
   nslistiter     curr;
	nsspine        S;
   nsfloat        r;
   nsfloat        sum;
	NsVector3f     D;
	NsVector3f     P;


	/* Do a WEIGHTED average on the head diameter of the spines,
		i.e. a center of mass. */

	ns_vector3f_zero( &xfrm->C );

   sum = 0.0f;

	NS_LIST_FOREACH( &xfrm->spines, curr )
      {
		S = ns_list_iter_get_object( curr );

		r = ( nsfloat )ns_spine_get_head_diameter( S );
		ns_spine_get_position( S, &P );

      sum += r;

      ns_vector3f_cmpd_add( &xfrm->C, ns_vector3f_cmpd_scale( &P, r ) );
      }

   ns_vector3f_cmpd_scale( &xfrm->C, 1.0f / sum );

	ns_vector3f_zero( &xfrm->R );

	NS_LIST_FOREACH( &xfrm->spines, curr )
      {
		S = ns_list_iter_get_object( curr );

		r = ( nsfloat )ns_spine_get_head_diameter( S );
		ns_spine_get_position( S, &P );

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


NS_PRIVATE void _ns_spines_transform_destruct( NsSpinesTransform *xfrm )
	{
	ns_list_destruct( &xfrm->spines );
	ns_byte_array_destruct( &xfrm->entries );
	}


typedef struct _NsSpinesTransformEntry
	{
	NsLine3f    LP;
	NsLine3f    LA;
	NsAABBox3d  B;
	}
	NsSpinesTransformEntry;


NS_PRIVATE NsError _ns_spines_transform_construct( NsSpinesTransform *xfrm, NsModel *model )
	{
	nslistiter               curr;
	nsspine                  S;
	NsVector3f               P, A;
	NsSpinesTransformEntry  *E;
	NsError                  error;


	xfrm->init = NS_FALSE;

	ns_list_construct( &xfrm->spines, NULL );
	ns_verify( NS_SUCCESS( ns_byte_array_construct( &xfrm->entries, 0 ), error ) );

	if( NS_FAILURE( ns_model_buffer_selected_spines_ex( model, &xfrm->spines ), error ) )
		{
		_ns_spines_transform_destruct( xfrm );
		return error;
		}

	if( NS_FAILURE(
			ns_byte_array_resize(
				&xfrm->entries,
				ns_list_size( &xfrm->spines ) * sizeof( NsSpinesTransformEntry )
				),
			error ) )
		{
		_ns_spines_transform_destruct( xfrm );
		return error;
		}

	E = NULL;

	if( 0 < ns_byte_array_size( &xfrm->entries ) )
		E = ( NsSpinesTransformEntry* )ns_byte_array_begin( &xfrm->entries );

	NS_LIST_FOREACH( &xfrm->spines, curr )
		{
		S = ns_list_iter_get_object( curr );

		ns_spine_get_position( S, &P );
		ns_spine_get_attach_point( S, &A );
		ns_spine_get_bounding_box( S, &E->B );

		E->LP.P1 = P;
		E->LP.P2 = P;
		E->LA.P1 = A;
		E->LA.P2 = A;

		++E;
		}

	xfrm->init = NS_TRUE;

	_ns_spines_transform_calc_center_and_radius( xfrm );

	return ns_no_error();
	}


NS_PRIVATE void _ns_spines_transform_end
	(
	NsSpinesTransform  *xfrm,
	nsboolean           confirm
	)
	{
	nslistiter                     curr;
	nsspine                        S;
	const NsSpinesTransformEntry  *E;


	if( ! xfrm->init )
		return;

	if( ! confirm )
		{
		E = NULL;

		if( 0 < ns_byte_array_size( &xfrm->entries ) )
			E = ( const NsSpinesTransformEntry* )ns_byte_array_begin( &xfrm->entries );

		NS_LIST_FOREACH( &xfrm->spines, curr )
			{
			S = ns_list_iter_get_object( curr );

			ns_spine_set_position( S, &E->LP.P1 );
			//ns_spine_set_attach_point( S, &E->LA.P1 );
			ns_spine_set_bounding_box( S, &E->B );
			ns_model_spine_calc_xyplane_angle( S );

			++E;
			}
		}

	_ns_spines_transform_destruct( xfrm );
	}


NsError ns_spines_translate_selected_begin
	(
	NsSpinesTransform  *xfrm,
	NsModel            *model
	)
	{
	NsError error;

	ns_assert( NULL != xfrm );
	ns_assert( NULL != model );

	if( NS_FAILURE( _ns_spines_transform_construct( xfrm, model ), error ) )
		return error;

	xfrm->P1 = xfrm->C;

	return ns_no_error();
	}


void ns_spines_translate_selected_apply
	(
	NsSpinesTransform  *xfrm,
	const NsVector3f   *Tf
	)
	{
	NsSpinesTransformEntry  *E;
	nslistiter               curr;
	nsspine                  S;
	NsVector3d               Td;
	NsAABBox3d               B;


	ns_assert( NULL != xfrm );
	ns_assert( NULL != Tf );

	if( ! xfrm->init )
		return;

	ns_vector3f_to_3d( Tf, &Td );

	E = NULL;

	if( 0 < ns_byte_array_size( &xfrm->entries ) )
		{
		E = ( NsSpinesTransformEntry* )ns_byte_array_begin( &xfrm->entries );
		ns_vector3f_add( &xfrm->C, &xfrm->P1, Tf );
		}

	NS_LIST_FOREACH( &xfrm->spines, curr )
		{
		ns_vector3f_add( &E->LP.P2, &E->LP.P1, Tf );
		ns_vector3f_add( &E->LA.P2, &E->LA.P1, Tf );

		B = E->B;
		ns_vector3d_add( &B.O, &E->B.O, &Td );

		S = ns_list_iter_get_object( curr );

		ns_spine_set_position( S, &E->LP.P2 );
		//ns_spine_set_attach_point( S, &E->LA.P2 );
		ns_spine_set_bounding_box( S, &B );
		ns_model_spine_calc_xyplane_angle( S );

		++E;
		}
	}


void ns_spines_translate_selected_end
	(
	NsSpinesTransform  *xfrm,
	nsboolean           confirm
	)
	{
	ns_assert( NULL != xfrm );
	_ns_spines_transform_end( xfrm, confirm );
	}


NsError ns_spines_rotate_selected_begin
	(
	NsSpinesTransform  *xfrm,
	NsModel            *model,
	nsint               which
	)
	{
	NsError error;

	ns_assert( NULL != xfrm );
	ns_assert( NULL != model );

	if( NS_FAILURE( _ns_spines_transform_construct( xfrm, model ), error ) )
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


NS_PRIVATE nsfloat _ns_spines_rotate_selected_delta
	(
	NsSpinesTransform  *xfrm,
	const NsVector3f   *P2
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


void ns_spines_rotate_selected_apply
	(
	NsSpinesTransform  *xfrm,
	const NsVector3f   *P
	)
	{
	NsSpinesTransformEntry  *E;
	nslistiter               curr;
	nsspine                  S;
	NsVector3f               LPP1, LAP1;
	NsVector3f               O1;
	NsVector3f               O2;
	NsMatrix44f              M;
	NsAABBox3d               B;


	ns_assert( NULL != xfrm );
	ns_assert( NULL != P );

	if( ! xfrm->init )
		return;

	E = NULL;

	if( 0 < ns_byte_array_size( &xfrm->entries ) )
		{
		E = ( NsSpinesTransformEntry* )ns_byte_array_begin( &xfrm->entries );

		xfrm->angle += _ns_spines_rotate_selected_delta( xfrm, P );
		( xfrm->rotate_func )( &M, xfrm->angle );
		}

	NS_LIST_FOREACH( &xfrm->spines, curr )
		{
		S = ns_list_iter_get_object( curr );

		LPP1 = E->LP.P1;
		ns_vector3f_cmpd_sub( &LPP1, &xfrm->C );
		ns_matrix44f_mul_vector3f( &E->LP.P2, &M, &LPP1 );
		ns_vector3f_cmpd_add( &E->LP.P2, &xfrm->C );

		LAP1 = E->LA.P1;
		ns_vector3f_cmpd_sub( &LAP1, &xfrm->C );
		ns_matrix44f_mul_vector3f( &E->LA.P2, &M, &LAP1 );
		ns_vector3f_cmpd_add( &E->LA.P2, &xfrm->C );

		ns_vector3d_to_3f( &E->B.O, &O1 );
		ns_vector3f_cmpd_sub( &O1, &xfrm->C );
		ns_matrix44f_mul_vector3f( &O2, &M, &O1 );
		ns_vector3f_cmpd_add( &O2, &xfrm->C );

		B = E->B;
		ns_vector3f_to_3d( &O2, &B.O );

		ns_spine_set_position( S, &E->LP.P2 );
		//ns_spine_set_attach_point( S, &E->LA.P2 );
		ns_spine_set_bounding_box( S, &B );
		ns_model_spine_calc_xyplane_angle( S );

		++E;
		}
	}


void ns_spines_rotate_selected_end
	(
	NsSpinesTransform  *xfrm,
	nsboolean           confirm
	)
	{
	ns_assert( NULL != xfrm );
	_ns_spines_transform_end( xfrm, confirm );
	}




nssize ns_spines_transform_size( const NsSpinesTransform *xfrm )
	{
	ns_assert( NULL != xfrm );
	return ns_list_size( &xfrm->spines );
	}


void ns_spines_transform_center( const NsSpinesTransform *xfrm, NsVector3f *C )
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != C );
	
	*C = xfrm->C;
	}


void ns_spines_transform_radius( const NsSpinesTransform *xfrm, NsVector3f *R )
	{
	ns_assert( NULL != xfrm );
	ns_assert( NULL != R );

	*R = xfrm->R;
	}


void ns_spines_transform_furthest( const NsSpinesTransform *xfrm, const NsVector3f *O, NsVector3f *P )
	{
	nslistiter  S;
	NsVector3f  curr_position, max_position;
	nsfloat     curr_distance, max_distance;


	ns_assert( NULL != xfrm );
	ns_assert( NULL != O );
	ns_assert( NULL != P );

	max_position = *P = *O;
	max_distance = 0.0f;

	NS_LIST_FOREACH( &xfrm->spines, S )
		{
		ns_spine_get_position( ns_list_iter_get_object( S ), &curr_position );

		curr_distance =
			NS_DISTANCE_SQUARED(
				curr_position.x, curr_position.y, curr_position.z,
				O->x, O->y, O->z
				);

		if( max_distance < curr_distance )
			{
			max_distance = curr_distance;
			max_position = curr_position;
			}
		}

	*P = max_position;
	}
