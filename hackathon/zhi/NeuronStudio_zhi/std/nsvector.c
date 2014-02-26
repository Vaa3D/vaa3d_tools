#include "nsvector.h"


/* NOTE on class NsVector pointers:

	first: Points to the start of the vector and/or the first actual object
			 in the vector.
	last : Points just past the end of the last actual object in the vector.
	end  : Points just past the end of the vector.
*/


NS_PRIVATE void _ns_vector_destruct_all
	(
	NsVector      *vector,
	nsvectoriter   first,
	nsvectoriter   last
	)
	{
	ns_assert( first <= last );

	if( NULL != vector->finalize_func )
		for( ; ns_vector_iter_not_equal( first, last ); first = ns_vector_iter_next( first ) )
			( vector->finalize_func )( ns_vector_iter_get_object( first ) );
	}


NS_PRIVATE void _ns_vector_shift_left_all
	(
	nsvectoriter  to,
	nsvectoriter  first,
	nsvectoriter  last
	)
	{
	ns_assert( first <= last );
	ns_assert( to <= first );

	while( ns_vector_iter_not_equal( first, last ) )
		{
		ns_vector_iter_set_object( to, ns_vector_iter_get_object( first ) );

		first = ns_vector_iter_next( first );
		to    = ns_vector_iter_next( to );
		}
	}


/* NOTE: Copy backwards! */
NS_PRIVATE void _ns_vector_shift_right_all
	(
	nsvectoriter  to,
	nsvectoriter  first,
	nsvectoriter  last
	)
	{
	ns_assert( first <= last );
	ns_assert( last <= to );

	while( ns_vector_iter_not_equal( first, last ) )
		{
		ns_vector_iter_set_object(
			to,
			ns_vector_iter_get_object( ns_vector_iter_prev( last ) )
			);

		last = ns_vector_iter_prev( last );
		to   = ns_vector_iter_prev( to );
		}
	}


NsError ns_vector_construct
	(
	NsVector        *vector,
	nssize           n,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != vector );

	vector->first         = NULL;
	vector->last          = NULL;
	vector->term          = NULL;
	vector->finalize_func = finalize_func;

	return ns_vector_reserve( vector, n );
	}


void ns_vector_destruct( NsVector *vector )
	{
   ns_assert( NULL != vector );
   ns_vector_clear( vector );
	}


void ns_vector_clear( NsVector *vector )
	{
	ns_assert( NULL != vector );

	ns_vector_erase_all( vector, vector->first, vector->last );
	ns_delete( vector->first );

	vector->first = NULL;
	vector->last  = NULL;
	vector->term  = NULL;
	}


NsError ns_vector_reserve( NsVector *vector, nssize n )
	{
	nssize     size;
	nspointer  temp;


	ns_assert( NULL != vector );

	if( ns_vector_capacity( vector ) < n )
		{
		size = ns_vector_size( vector );

		temp = ns_realloc(
					vector->first,
					n * sizeof( nspointer )
					);

		if( NULL == temp )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

		vector->first = temp;
		vector->last  = vector->first + size;
		vector->term  = vector->first + n;
		}

	return ns_no_error();
	}


nssize ns_vector_size( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	ns_assert( vector->first <= vector->last );

	return ( NULL != vector->first ) ? ( nssize )( vector->last - vector->first ) : 0;
	}


nssize ns_vector_capacity( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	ns_assert( vector->first <= vector->term );

	return ( NULL != vector->first ) ? ( nssize )( vector->term - vector->first ) : 0;
	}


nsboolean ns_vector_is_empty( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	return 0 == ns_vector_size( vector );
	}


NsError ns_vector_resize( NsVector *vector, nssize n )
	{
	ns_assert( NULL != vector );

	if( n < ns_vector_size( vector ) )
		ns_vector_erase_all( vector, ns_vector_at( vector, n ), ns_vector_end( vector ) );
	else if( ns_vector_capacity( vector ) < n )
		return ns_vector_reserve( vector, n );

	return ns_no_error();
	}


nsvectoriter ns_vector_begin( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	return vector->first;
	}


nsvectoriter ns_vector_end( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	return vector->last;
	}


nsvectorreviter ns_vector_rev_begin( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	return ( NULL != vector->last ) ? vector->last - 1 : NULL;
	}


nsvectorreviter ns_vector_rev_end( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	return ( NULL != vector->first ) ? vector->first - 1 : NULL;
	}


nsvectoriter ns_vector_at( const NsVector *vector, nssize index )
	{
	ns_assert( NULL != vector );
	ns_assert( index < ns_vector_size( vector ) );

	return vector->first + index;
	}


nspointer ns_vector_peek_front( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	ns_assert( ! ns_vector_is_empty( vector ) );

	return ns_vector_iter_get_object( ns_vector_begin( vector ) );
	}


nspointer ns_vector_peek_back( const NsVector *vector )
	{
	ns_assert( NULL != vector );
	ns_assert( ! ns_vector_is_empty( vector ) );

	return ns_vector_rev_iter_get_object( ns_vector_rev_begin( vector ) );
	}


NsError ns_vector_push_back( NsVector *vector, nspointer object )
	{
	ns_assert( NULL != vector );
	return ns_vector_insert( vector, ns_vector_end( vector ), object );
	}


void ns_vector_pop_back( NsVector *vector )
	{
	ns_assert( NULL != vector );
	ns_assert( ! ns_vector_is_empty( vector ) );

	ns_vector_erase( vector, ns_vector_rev_begin( vector ) );
	}


NsError ns_vector_insert( NsVector *vector, nsvectoriter I, nspointer object )
	{
	nssize   index;
	nssize   capacity;
	NsError  error;


	ns_assert( NULL != vector );
	ns_assert( vector->first <= I && I <= vector->last );

	/* NOTE: Because of the call to ns_vector_reserve(), the iterator
		'I' could become invalid if the block of memory that holds the
		vector was moved. Therefore simply save the index and reset the
		iterator later. */
	index    = ( nssize )( I - vector->first );
	capacity = ns_vector_capacity( vector );

	if( ns_vector_size( vector ) == capacity )
		{
		capacity += capacity / 2 + 1;

		if( NS_FAILURE( ns_vector_reserve( vector, capacity ), error ) )
			return error;
		}

	I = vector->first + index;

	_ns_vector_shift_right_all( vector->last, I, vector->last );
	ns_vector_iter_set_object( I, object );

	vector->last += 1;

	ns_assert( vector->first <= vector->last && vector->last <= vector->term );
	return ns_no_error();
	}


void ns_vector_erase( NsVector *vector, nsvectoriter I )
	{
	ns_assert( NULL != vector );
	ns_vector_erase_all( vector, I, ns_vector_iter_next( I ) );
	}


void ns_vector_erase_all( NsVector *vector, nsvectoriter first, nsvectoriter last )
	{
	ns_assert( NULL != vector );

	ns_assert( first <= last );
	ns_assert( vector->first <= first && first <= vector->last );
	ns_assert( vector->first <= last  && last  <= vector->last );

	_ns_vector_destruct_all( vector, first, last );
	_ns_vector_shift_left_all( first, last, vector->last );
	
	vector->last -= ( nssize )( last - first );

	ns_assert( vector->first <= vector->last && vector->last <= vector->term );
	}


nspointer ns_vector_get_object( const NsVector *vector, nssize index )
	{
	ns_assert( NULL != vector );
	ns_assert( index < ns_vector_size( vector ) );

	return ns_vector_iter_get_object( ns_vector_at( vector, index ) );
	}


void ns_vector_set_object( NsVector *vector, nssize index, nspointer object )
	{
	ns_assert( NULL != vector );
	ns_assert( index < ns_vector_size( vector ) );

	ns_vector_iter_set_object( ns_vector_at( vector, index ), object );
	}

