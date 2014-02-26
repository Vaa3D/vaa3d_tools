

#define _NS_THREAD_INVALID_HANDLE  0


typedef struct _NsThreadRun
	{
	NsThreadFunc  func;
	nspointer     args;
	nsboolean     wait;
	NsError       error;
 	}
	NsThreadRun;


/* Wrapper for user-defined thread callback so that
	ns_thread_add() and ns_thread_remove() can be called
	in the proper thread. */
NS_PRIVATE void _ns_thread_func( NsThreadRun *run )
	{
	NsThreadFunc  func;
	nspointer     args;


	func = run->func;
	args = run->args;

	run->wait = NS_FALSE;

	/* IMPORTANT: Assuming that both 'func' and 'args' point to addresses
		that remain valid long enough to be called here. The callback function
		is probably no worry but the arguments could be invalid if placed on
		the stack. */
	( func )( args );

	_ns_os_windows_thread_end();
	}


NS_PRIVATE NsError ____ns_thread_run( NsThread *thread, NsThreadFunc func, nspointer args )
	{
	NsThreadRun  run;
	NsError      error;


   ns_assert( NULL != thread );
   ns_assert( NULL != func );

	run.func  = func;
	run.args  = args;
	run.wait  = NS_TRUE;
	run.error = ns_no_error();

   if( NS_FAILURE( _ns_os_windows_thread_begin( &thread->handle, _ns_thread_func, &run ), error ) )
		return error;

	/* Halt this thread until the new thread completes initialization. */
	while( run.wait )
		ns_thread_yield();

	return run.error;
	}


NS_PRIVATE NsError ____ns_thread_get_priority( const NsThread *thread, NsThreadPriority *priority )
	{
	ns_assert( NULL != thread );
	ns_assert( _NS_THREAD_INVALID_HANDLE != thread->handle );
	ns_assert( NULL != priority );

	return _ns_os_windows_thread_get_priority( thread->handle, priority );
	}


NS_PRIVATE NsError ____ns_thread_set_priority( NsThread *thread, NsThreadPriority priority )
	{
   ns_assert( NULL != thread );
	ns_assert( _NS_THREAD_INVALID_HANDLE != thread->handle );

   return _ns_os_windows_thread_set_priority( thread->handle, ( nsenum )priority );
	}


NS_PRIVATE NsError ____ns_thread_self( NsThread *thread )
	{
	ns_assert( NULL != thread );

	thread->handle = _ns_os_windows_thread_self();
	return ns_no_error();
	}


nsulong ns_thread_id( void )
	{  return _ns_os_windows_thread_id();  }


void ns_thread_sleep( nssize milliseconds )
	{  _ns_os_windows_thread_sleep( milliseconds );  }


void ns_thread_yield( void )
	{  _ns_os_windows_thread_sleep( 0 );  }


nsulong ns_process_id( void )
	{  return _ns_os_windows_process_id();  }
