#ifndef __NS_STD_MEMORY_H__
#define __NS_STD_MEMORY_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

#define ns_new( type )\
	ns_new_array( type, 1 )

#define ns_new0( type )\
	ns_new_array0( type, 1 )


NS_IMPEXP nspointer ns_malloc( nssize bytes );

NS_IMPEXP nspointer ns_calloc( nssize n, nssize size );

NS_IMPEXP nspointer ns_realloc( nspointer mem_block, nssize bytes );

NS_IMPEXP void ns_free( nspointer mem_block );


#define ns_new_array( type, n )\
	( ( type* )ns_malloc( (n) * sizeof( type ) ) )

#define ns_new_array0( type, n )\
	( ( type* )ns_calloc( (n), sizeof( type ) ) )


#define ns_delete( object )\
	ns_free( (object) )


NS_IMPEXP nspointer ns_memset( nspointer dest, nsint8 c, nssize count );
NS_IMPEXP nspointer ns_memzero( nspointer dest, nssize count );
NS_IMPEXP nspointer ns_memcpy( nspointer dest, nsconstpointer src, nssize count );
NS_IMPEXP nspointer ns_memmove( nspointer dest, nsconstpointer src, nssize count );

NS_DECLS_END

#endif/* __NS_STD_MEMORY_H__ */
