#include "nsfps.h"


NsError ns_frames_per_second_construct( NsFramesPerSecond *fps, nssize interval )
   {
	NsError error;

	ns_assert( NULL != fps );
	ns_assert( 0 < interval );

	if( NS_FAILURE( ns_byte_array_construct(
							&fps->buffer,
							interval * sizeof( nstimer )
							),
							error ) )
		return error;

	ns_byte_array_memzero( &fps->buffer );

	fps->index    = 0;
	fps->interval = interval;
	fps->times    = ( nstimer* )ns_byte_array_begin( &fps->buffer );

	return ns_no_error();
   }


void ns_frames_per_second_destruct( NsFramesPerSecond *fps )
   {
	ns_assert( NULL != fps );
	ns_byte_array_destruct( &fps->buffer );
	}


nsdouble ns_frames_per_second( NsFramesPerSecond *fps )
	{
	nstimer curr, prev;

	ns_assert( NULL != fps );
	ns_assert( fps->index < fps->interval );

	prev = fps->times[ fps->index ];
	curr = ns_timer();

	fps->times[ fps->index ] = curr;

	++(fps->index);

	if( fps->index == fps->interval )
		fps->index = 0;

	return ( nsdouble )fps->interval / ns_difftimer( curr, prev ); 
	}
