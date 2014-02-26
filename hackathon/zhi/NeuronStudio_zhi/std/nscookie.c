#include "nscookie.h"


#define _NS_COOKIE  12345


void ns_cookie_set( nspointer buffer, nssize size )
	{
	if( NULL != buffer )
		*( NS_OFFSET_POINTER( nscookie, buffer, size ) ) = _NS_COOKIE;
	}


nsboolean ns_cookie_valid( nsconstpointer buffer, nssize size )
	{
	if( NULL != buffer && _NS_COOKIE != *( NS_OFFSET_POINTER( const nscookie, buffer, size ) ) )
		return NS_FALSE;

	return NS_TRUE;
	}
