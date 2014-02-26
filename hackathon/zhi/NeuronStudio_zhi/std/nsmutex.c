#include "nsmutex.h"

#ifdef NS_OS_WINDOWS
	#include "nsmutex-os-windows.inl"
#else
	#include "nsmutex-null.inl"
#endif


void ns_mutex_construct( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
	ns_memzero( mutex, sizeof( NsMutex ) );
	}


void ns_mutex_destruct( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
	ns_mutex_destroy( mutex );
	}


nsboolean ns_mutex_is_created( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
	return mutex->is_created;
	}


NsError ns_mutex_create( NsMutex *mutex )
	{
	NsError error;

	ns_assert( NULL != mutex );
	ns_assert( ! mutex->is_created );

	if( NS_FAILURE( ____ns_mutex_create( mutex ), error ) )
		{
		ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " Mutex creation failed." );
		return error;
		}

	mutex->is_created = NS_TRUE;
	return ns_no_error();
	}


NsError ns_mutex_destroy( NsMutex *mutex )
	{
	NsError error;

	ns_assert( NULL != mutex );

	if( mutex->is_created )
		{
		if( NS_FAILURE( ____ns_mutex_destroy( mutex ), error ) )
			return error;

		mutex->is_created = NS_FALSE;
		}

	return ns_no_error();
	}


NsError ns_mutex_lock( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
   return ____ns_mutex_lock( mutex );
	}


NsError ns_mutex_try_lock( NsMutex *mutex, nsboolean *did_lock )
	{
	ns_assert( NULL != mutex );
	return ____ns_mutex_try_lock( mutex, did_lock );
	}


NsError ns_mutex_unlock( NsMutex *mutex )
	{
	ns_assert( NULL != mutex );
	return ____ns_mutex_unlock( mutex );
	}
