#include "nsdir.h"

#ifdef NS_OS_WINDOWS
	#include "nsdir-os-windows.inl"
#else
	#include "nsdir-null.inl"
#endif


void ns_dir_construct( NsDir *dir )
	{
	ns_assert( NULL != dir );

	ns_memzero( dir, sizeof( NsDir ) );
	dir->name = NULL; /* Just to be sure. */
	}


void ns_dir_destruct( NsDir *dir )
	{
	ns_assert( NULL != dir );

   if( ns_dir_is_open( dir ) )
      ns_dir_close( dir );

	ns_delete( dir->name );
	}


nsboolean ns_dir_is_open( const NsDir *dir )
	{
   ns_assert( NULL != dir );
   return dir->is_open;
	}


NsError ns_dir_open( NsDir *dir )
	{
	NsError error;

	ns_assert( NULL != dir );

	if( NULL == dir->name )
		if( NULL == ( dir->name = ns_new_array( nschar, NS_PATH_SIZE ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_getcwd( dir->name, NS_PATH_SIZE ), error ) )
		return error;

   return ____ns_dir_open( dir );
	}


void ns_dir_close( NsDir *dir )
	{
	ns_assert( NULL != dir );

	if( NULL != dir->name )
		dir->name[0] = '\0';

	____ns_dir_close( dir );
	}


NsError ns_dir_rewind( NsDir *dir )
	{
	ns_assert( NULL != dir );
   return ____ns_dir_rewind( dir );
	}


const nschar* ns_dir_read( NsDir *dir )
	{
	ns_assert( NULL != dir );
	return ____ns_dir_read( dir );
	}


const nschar* ns_dir_name( const NsDir *dir )
	{
	ns_assert( NULL != dir );
	return dir->name;
	}
