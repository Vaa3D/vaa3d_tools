#ifndef __NS_STD_BYTE_ARRAY_H__
#define __NS_STD_BYTE_ARRAY_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nscookie.h>

NS_DECLS_BEGIN

typedef struct _NsByteArray
	{
	nssize      size;
   nsuint8    *bytes;
	#ifdef NS_DEBUG
	nsboolean   cookie;
	#endif
	}
	NsByteArray;


/* If size is zero then the array is made NULL and no error occurs. */
NS_IMPEXP NsError ns_byte_array_construct( NsByteArray *array, nssize size );
NS_IMPEXP void ns_byte_array_destruct( NsByteArray *array );

NS_IMPEXP nssize ns_byte_array_size( const NsByteArray *array );

NS_IMPEXP void ns_byte_array_clear( NsByteArray *array );

/* IMPORTANT: The 'array' will free the 'buffer' when it
	is cleared or destroyed! */
NS_IMPEXP void ns_byte_array_buffer( NsByteArray *array, nspointer buffer, nssize bytes );

/* Clears the array but does NOT free the pixels. */
NS_IMPEXP void ns_byte_array_release( NsByteArray *array );

/* If size is zero then the array is made NULL and no error occurs. */
NS_IMPEXP NsError ns_byte_array_resize( NsByteArray *array, nssize size );

NS_IMPEXP void ns_byte_array_memset( NsByteArray *array, nsuint8 value );
NS_IMPEXP void ns_byte_array_memzero( NsByteArray *array );

NS_IMPEXP void ns_byte_array_memcpy( NsByteArray *array, nsconstpointer buffer, nssize bytes );


#ifdef NS_DEBUG

#define ns_byte_array_validate( array )\
	____ns_byte_array_validate( (array), NS_MODULE )

NS_IMPEXP void ____ns_byte_array_validate( const NsByteArray *array, const nschar *module );

NS_IMPEXP nsuint8* ns_byte_array_begin( const NsByteArray *array );

#endif


NS_IMPEXP nsuint8* ns_byte_array_end( const NsByteArray *array );
NS_IMPEXP nsuint8* ns_byte_array_at( const NsByteArray *array, nssize index );

#define ns_byte_array_get_at( array, index )\
   ( ( array )->bytes[ ( index ) ] )

#define ns_byte_array_set_at( array, index, value )\
   ( ( array )->bytes[ ( index ) ] = ( value ) )

NS_DECLS_END

#endif/* __NS_STD_BYTE_ARRAY_H__ */
