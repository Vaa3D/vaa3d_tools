
#define _NS_DIR_INVALID_HANDLE  -1


NS_PRIVATE NsError ____ns_dir_open( NsDir *dir )
	{
	NsError error;

   ns_assert( NULL != dir );

	if( NS_FAILURE( _ns_os_windows_find_first( &dir->handle, &dir->fd ), error ) )
		return error;

	dir->is_open  = NS_TRUE;
	dir->at_first = NS_TRUE;

   return ns_no_error();
	}


NS_PRIVATE void ____ns_dir_close( NsDir *dir )
	{
   ns_assert( NULL != dir );

   if( dir->is_open && _NS_DIR_INVALID_HANDLE != dir->handle )
      {
      _ns_os_windows_find_close( dir->handle, dir->fd );

      dir->handle   = _NS_DIR_INVALID_HANDLE;
      dir->is_open  = NS_FALSE;
      dir->at_end   = NS_FALSE;
		dir->at_first = NS_FALSE;
      }
	}


NS_PRIVATE NsError ____ns_dir_rewind( NsDir *dir )
	{
   ns_assert( NULL != dir );
   ns_assert( dir->is_open && _NS_DIR_INVALID_HANDLE != dir->handle );

   ns_dir_close( dir );
   return ns_dir_open( dir );
	}


NS_PRIVATE const nschar* ____ns_dir_read( NsDir *dir )
	{
   ns_assert( NULL != dir );
   ns_assert( dir->is_open && _NS_DIR_INVALID_HANDLE != dir->handle );

   if( dir->at_end )
      return NULL;

	if( dir->at_first )
		{
		dir->at_first = NS_FALSE;
		return _ns_os_windows_find_name( dir->fd );
		}

	if( ! _ns_os_windows_find_next( dir->handle, dir->fd ) )
		dir->at_end = NS_TRUE;

   return dir->at_end ? NULL : _ns_os_windows_find_name( dir->fd );
	}
