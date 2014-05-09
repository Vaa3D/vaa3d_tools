#include "nscube.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


void ns_cubei_zero( NsCubei *C )
	{
	ns_assert( NULL != C );

	ns_vector3i_zero( &C->C1 );
	ns_vector3i_zero( &C->C2 );
	}


nsint ns_cubei_width( const NsCubei *C )
	{
	ns_assert( NULL != C );
	return NS_ABS( C->C2.x - C->C1.x ) + 1;
	}


nsint ns_cubei_height( const NsCubei *C )
	{
	ns_assert( NULL != C );
	return NS_ABS( C->C2.y - C->C1.y ) + 1;
	}


nsint ns_cubei_length( const NsCubei *C )
	{
	ns_assert( NULL != C );
	return NS_ABS( C->C2.z - C->C1.z ) + 1;
	}
