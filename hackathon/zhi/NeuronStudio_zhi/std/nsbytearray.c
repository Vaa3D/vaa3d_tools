#include "nsbytearray.h"


NsError ns_byte_array_construct( NsByteArray *array, nssize size )
	{
	NsError error;

	ns_assert( NULL != array );

	array->size  = 0;
	array->bytes = NULL;

	if( NS_FAILURE( ns_byte_array_resize( array, size ), error ) )
		return error;

	return ns_no_error();
	}


void ns_byte_array_destruct( NsByteArray *array )
	{
	ns_assert( NULL != array );
	ns_byte_array_clear( array );
	}


nssize ns_byte_array_size( const NsByteArray *array )
	{
	ns_assert( NULL != array );
	return array->size;
	}


void ns_byte_array_clear( NsByteArray *array )
	{
	ns_assert( NULL != array );
	ns_byte_array_resize( array, 0 );
	}


void ns_byte_array_buffer( NsByteArray *array, nspointer buffer, nssize bytes )
	{
	ns_assert( NULL != array );

	ns_byte_array_clear( array );

	array->size  = bytes;
	array->bytes = buffer;

	/* IMPORTANT: Cant check the cookie if the user passes a buffer
		since we dont know if it was set and/or if there is space. */
	#ifdef NS_DEBUG
	array->cookie = NS_FALSE;
	#endif
	}


void ns_byte_array_release( NsByteArray *array )
	{
	ns_assert( NULL != array );

	array->size  = 0;
	array->bytes = NULL;

	#ifdef NS_DEBUG
	array->cookie = NS_FALSE;
	#endif
	}


#ifdef NS_DEBUG

void ____ns_byte_array_validate( const NsByteArray *array, const nschar *module )
	{
	ns_assert( NULL != array );

	if( array->cookie && ! ns_cookie_valid( array->bytes, array->size ) )
		{
		ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " Buffer overrun detected at " NS_FMT_STRING, module );
		ns_abort();
		}
	}

#endif/* NS_DEBUG */


NsError ns_byte_array_resize( NsByteArray *array, nssize size )
	{
   nsuint8 *temp;

   ns_assert( NULL != array );

	if( size == ns_byte_array_size( array ) )
		return ns_no_error();

	#ifdef NS_DEBUG
		if( array->cookie && ! ns_cookie_valid( array->bytes, array->size ) )
			ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " Buffer overrun detected." );
	#endif

	if( 0 == size )
		{
		ns_delete( array->bytes );
		temp = NULL;
		}
	else
		{
		temp = ( nsuint8* )ns_realloc(
									array->bytes,
									size
									#ifdef NS_DEBUG
										+ sizeof( nscookie )
									#endif
									);

	   if( NULL == temp )
		   return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

   array->bytes = temp;
	array->size  = size;

	#ifdef NS_DEBUG
		ns_cookie_set( array->bytes, array->size );
		array->cookie = NS_TRUE;
	#endif

   return ns_no_error();
	}


void ns_byte_array_memset( NsByteArray *array, nsuint8 value )
	{
   ns_assert( NULL != array );
   ns_assert( NULL != array->bytes );

   ns_memset( array->bytes, value, ns_byte_array_size( array ) );
	}


void ns_byte_array_memzero( NsByteArray *array )
	{
   //ns_assert( NULL != array );
  // ns_assert( NULL != array->bytes );
   ns_memzero( array->bytes, ns_byte_array_size( array ) );
	}


void ns_byte_array_memcpy( NsByteArray *array, nsconstpointer buffer, nssize bytes )
	{
	ns_assert( NULL != array );
	ns_assert( NULL != buffer );
	ns_assert( bytes <= ns_byte_array_size( array ) );

	ns_memcpy( ns_byte_array_begin( array ), buffer, bytes );
	}



nsuint8* ns_byte_array_begin( const NsByteArray *array )
    {
   ns_assert( NULL != array );
 //  ns_assert( NULL != array->bytes );

   return array->bytes;
	}




nsuint8* ns_byte_array_end( const NsByteArray *array )
	{
   ns_assert( NULL != array );
   ns_assert( NULL != array->bytes );

   return array->bytes + ns_byte_array_size( array );
	}


nsuint8* ns_byte_array_at( const NsByteArray *array, nssize index )
	{
   ns_assert( NULL != array );
   ns_assert( NULL != array->bytes );
   ns_assert( index < ns_byte_array_size( array ) );

   return array->bytes + index;
	}
