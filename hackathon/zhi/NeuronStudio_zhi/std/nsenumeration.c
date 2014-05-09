#include "nsenumeration.h"


void ns_enumeration_construct( NsEnumeration *e )
	{
	ns_assert( NULL != e );

	e->has_more_elements = NULL;
	e->next_element      = NULL;
	}


void ns_enumeration_destruct( NsEnumeration *e )
	{
	ns_assert( NULL != e );

	e->has_more_elements = NULL;
	e->next_element      = NULL;
	}


nsboolean ns_enumeration_has_more_elements( const NsEnumeration *e )
	{
	ns_assert( NULL != e );
	ns_assert( NULL != e->has_more_elements );

	return ( e->has_more_elements )( e );
	}


nspointer ns_enumeration_next_element( NsEnumeration *e )
	{
	ns_assert( NULL != e );
	ns_assert( NULL != e->next_element );

	return ( e->next_element )( e );
	}


nspointer ns_enumeration_get_user_data( const NsEnumeration *e )
	{
	ns_assert( NULL != e );
	return ( nspointer )( e->user_data );
	}


void ns_enumeration_set_user_data( NsEnumeration *e, nspointer user_data )
	{
	ns_assert( NULL != e );
	e->user_data = user_data;
	}
