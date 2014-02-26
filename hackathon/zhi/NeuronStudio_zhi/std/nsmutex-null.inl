
NS_PRIVATE NsError ____ns_mutex_create( NsMutex *mutex )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( mutex );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_mutex_destroy( NsMutex *mutex )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( mutex );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_mutex_lock( NsMutex *mutex )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( mutex );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_mutex_try_lock( NsMutex *mutex, nsboolean *did_lock )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( mutex );
	NS_USE_VARIABLE( did_lock );

	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NS_PRIVATE NsError ____ns_mutex_unlock( NsMutex *mutex )
	{
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " No implementation." );

	NS_USE_VARIABLE( mutex );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}
