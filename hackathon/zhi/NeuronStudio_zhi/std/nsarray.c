#include "nsarray.h"


NsError ns_array_construct( NsArray *array, nssize size )
   {
	ns_assert( NULL != array );

	array->size    = 0;
	array->objects = NULL;

	return ns_array_resize( array, size );
   }


void ns_array_destruct( NsArray *array )
   {
	ns_assert( NULL != array );
	ns_array_clear( array );
	}


nssize ns_array_size( const NsArray *array )
	{
	ns_assert( NULL != array );
	return array->size;
	}


void ns_array_clear( NsArray *array )
	{
	ns_assert( NULL != array );
	ns_array_resize( array, 0 );
	}


NsError ns_array_resize( NsArray *array, nssize size )
	{
   nspointer *temp;

   ns_assert( NULL != array );

	if( size == ns_array_size( array ) )
		return ns_no_error();

	if( 0 == size )
		{
		ns_delete( array->objects );
		temp = NULL;
		}
	else
		{
		temp = _ns_realloc( array->objects, sizeof( nspointer ) * size );

	   if( NULL == temp )
		   return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

   array->objects = temp;
	array->size    = size;

   return ns_no_error();
	}


void ns_array_memzero( NsArray *array )
	{
   ns_assert( NULL != array );
   ns_assert( NULL != array->objects );

   ns_memzero( array->objects, sizeof( nspointer ) * ns_array_size( array ) );
	}


nsarrayreviter ns_array_rev_begin( const NsArray *array )
	{
	nsarrayreviter I;

	ns_assert( NULL != array );
	ns_assert( NULL != array->objects );
	ns_assert( 0 < array->size );

	I = array->objects + ( array->size - 1 );
	return I;
	}


nsarrayreviter ns_array_rev_end( const NsArray *array )
	{
	nsarrayreviter I;

	ns_assert( NULL != array );
	ns_assert( NULL != array->objects );

	I = array->objects - 1;
	return I;
	}


nsarrayiter ns_array_at_ex( const NsArray *array, nssize index )
	{
   ns_assert( NULL != array );
   ns_assert( NULL != array->objects );
   ns_assert( index < ns_array_size( array ) );

   return array->objects + index;
	}
