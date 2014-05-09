
NS_PRIVATE NsError ____ns_mutex_create( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
	ns_assert( NULL == mutex->handle );

	return _ns_os_windows_mutex_create( &mutex->handle );
	}


NS_PRIVATE NsError ____ns_mutex_destroy( NsMutex *mutex )
	{
	NsError error;

	ns_assert( NULL != mutex );
	ns_assert( NULL != mutex->handle );

	if( NS_FAILURE( _ns_os_windows_mutex_destroy( mutex->handle ), error ) )
		return error;

	mutex->handle = NULL;
	return ns_no_error();
	}


NS_PRIVATE NsError ____ns_mutex_lock( NsMutex *mutex )
	{
	NsError error;

	ns_assert( NULL != mutex );
	ns_assert( NULL != mutex->handle );

#ifdef NS_DEBUG
	if( _ns_os_windows_thread_id() == mutex->owner_thread_id )
		{
		ns_warning(
			NS_WARNING_LEVEL_CRITICAL
			NS_MODULE
			" Recursive lock of mutex " NS_FMT_STRING ".",
			mutex->name
			);

		ns_log_entry(
			NS_LOG_ENTRY_ERROR,
			"Recursive lock "
			NS_FUNCTION
			"( mutex=" NS_FMT_STRING_DOUBLE_QUOTED ")",
			mutex->name
			);
		}
#endif/* NS_DEBUG */

	if( NS_FAILURE( _ns_os_windows_mutex_lock( mutex->handle ), error ) )
		return error;

#ifdef NS_DEBUG
	mutex->owner_thread_id = _ns_os_windows_thread_id();
#endif

	return ns_no_error();
	}


NS_PRIVATE NsError ____ns_mutex_try_lock( NsMutex *mutex, nsboolean *did_lock )
	{
	NsError error;

	ns_assert( NULL != mutex );
	ns_assert( NULL != mutex->handle );
	ns_assert( NULL != did_lock );

#ifdef NS_DEBUG
	if( _ns_os_windows_thread_id() == mutex->owner_thread_id )
		{
		ns_warning(
			NS_WARNING_LEVEL_CRITICAL
			NS_MODULE
			" Recursive lock of mutex " NS_FMT_STRING ".",
			mutex->name
			);

		ns_log_entry(
			NS_LOG_ENTRY_ERROR,
			"Recursive lock "
			NS_FUNCTION
			"( mutex=" NS_FMT_STRING_DOUBLE_QUOTED ")",
			mutex->name
			);
		}
#endif/* NS_DEBUG */

	if( NS_FAILURE( _ns_os_windows_mutex_try_lock( mutex->handle, did_lock ), error ) )
		return error;

#ifdef NS_DEBUG
	if( *did_lock )
		mutex->owner_thread_id = _ns_os_windows_thread_id();
#endif

	return ns_no_error();
	}


NS_PRIVATE NsError ____ns_mutex_unlock( NsMutex *mutex )
	{
	NsError error;

	ns_assert( NULL != mutex );
	ns_assert( NULL != mutex->handle );

	if( NS_FAILURE( _ns_os_windows_mutex_unlock( mutex->handle ), error ) )
		return error;

#ifdef NS_DEBUG
	mutex->owner_thread_id = 0;
#endif

	return ns_no_error();
	}
