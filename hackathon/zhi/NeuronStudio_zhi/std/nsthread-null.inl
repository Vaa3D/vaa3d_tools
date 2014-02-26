
NS_PRIVATE NsError ____ns_thread_run( NsThread *thread, NsThreadFunc func, nspointer args )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( thread );
	NS_USE_VARIABLE( func );
	NS_USE_VARIABLE( args );

	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_thread_get_priority( const NsThread *thread, NsThreadPriority *priority )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( thread );
	NS_USE_VARIABLE( priority );

	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_thread_set_priority( NsThread *thread, NsThreadPriority priority )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( thread );
	NS_USE_VARIABLE( priority );

	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_thread_self( NsThread *thread )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( thread );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


nsulong ns_thread_id( void )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );
	return NS_THREAD_INVALID_ID;
	}


void ns_thread_sleep( nssize milliseconds )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );
	NS_USE_VARIABLE( milliseconds );
	}


void ns_thread_yield( void )
	{  ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );  }


nsulong ns_process_id( void )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );
	return NS_PROCESS_INVALID_ID;
	}
