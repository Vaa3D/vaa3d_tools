#include "nsmemory.h"


nspointer ns_malloc( nssize bytes )
	{  return _ns_malloc( bytes );  }


nspointer ns_calloc( nssize n, nssize size )
	{
	nspointer mem_block = ns_malloc( n * size );

	if( NULL != mem_block )
		ns_memzero( mem_block, n * size );

	return mem_block;
	}


nspointer ns_realloc( nspointer mem_block, nssize bytes )
	{  return _ns_realloc( mem_block, bytes );  }


void ns_free( nspointer mem_block )
	{  _ns_free( mem_block );  }


nspointer ns_memset( nspointer dest, nsint8 c, nssize count )
	{  return _ns_memset( dest, c, count );  }


nspointer ns_memzero( nspointer dest, nssize count )
	{  return ns_memset( dest, 0, count );  }


nspointer ns_memcpy( nspointer dest, nsconstpointer src, nssize count )
	{  return _ns_memcpy( dest, src, count );  }


nspointer ns_memmove( nspointer dest, nsconstpointer src, nssize count )
	{  return _ns_memmove( dest, src, count );  }
