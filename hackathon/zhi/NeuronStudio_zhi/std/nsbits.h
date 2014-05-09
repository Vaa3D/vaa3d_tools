#ifndef __NS_STD_BITS_H__
#define __NS_STD_BITS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmemory.h>

NS_DECLS_BEGIN

#define _ns_bit_buf_mask( bit )\
	( 1 << ( (bit) & 7 ) )

#define _ns_bit_buf_func( buf, bit, func )\
	( *( (buf) + ( (bit) >> 3 ) ) func )


#define ns_bit_buf_set( buf, bit )\
	( _ns_bit_buf_func( (buf), (bit), |= _ns_bit_buf_mask( (bit) ) ) )

#define ns_bit_buf_clear( buf, bit )\
	( _ns_bit_buf_func( (buf), (bit), &= ~_ns_bit_buf_mask( (bit) ) ) )

#define ns_bit_buf_is_set( buf, bit )\
	( 0 != _ns_bit_buf_func( (buf), (bit), & _ns_bit_buf_mask( (bit) ) ) )

#define ns_bit_buf_is_clear( buf, bit )\
	( ! ns_bit_buf_is_set( (buf), (bit) ) )

#define ns_bit_buf_toggle( buf, bit )\
	( _ns_bit_buf_func( (buf), (bit), ^= _ns_bit_buf_mask( (bit) ) ) )


NS_IMPEXP void ns_bit_buf_set_all( nsuint8 *buf, nssize num_bits );
NS_IMPEXP void ns_bit_buf_clear_all( nsuint8 *buf, nssize num_bits );
NS_IMPEXP void ns_bit_buf_toggle_all( nsuint8 *buf, nssize num_bits );

NS_DECLS_END

#endif/* __NS_STD_BITS_H__ */
