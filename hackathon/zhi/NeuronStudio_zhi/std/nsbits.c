#include "nsbits.h"


void ns_bit_buf_set_all( nsuint8 *buf, nssize num_bits )
	{
	ns_assert( NULL != buf );
	ns_memset( buf, 0xFF, NS_BITS_TO_BYTES( num_bits ) );
	}


void ns_bit_buf_clear_all( nsuint8 *buf, nssize num_bits )
	{
	ns_assert( NULL != buf );
	ns_memzero( buf, NS_BITS_TO_BYTES( num_bits ) );
	}


void ns_bit_buf_toggle_all( nsuint8 *buf, nssize num_bits )
	{
	nssize i;

	ns_assert( NULL != buf );

	for( i = 0; i < num_bits; ++i )
		ns_bit_buf_toggle( buf, i );
	}
