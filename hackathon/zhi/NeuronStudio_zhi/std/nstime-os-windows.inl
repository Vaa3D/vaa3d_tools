
nstimer ns_timer( void )
   {  return _ns_os_windows_timer();  }


nsdouble ns_difftimer( nstimer stop, nstimer start )
	{  return _ns_os_windows_difftimer( stop, start );  }
