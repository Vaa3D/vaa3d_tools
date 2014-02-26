#include "nsthread.h"

#ifdef NS_OS_WINDOWS
	#include "nsthread-os-windows.inl"
#else
	#include "nsthread-null.inl"
#endif


void ns_thread_construct( NsThread *thread )
	{
	ns_assert( NULL != thread );
	ns_memzero( thread, sizeof( NsThread ) );
	}


void ns_thread_destruct( NsThread *thread )
	{
	ns_assert( NULL != thread );
	NS_USE_VARIABLE( thread );
	}


NsError ns_thread_run( NsThread *thread, NsThreadFunc func, nspointer args )
	{
	ns_assert( NULL != thread );
	ns_assert( NULL != func );

	return ____ns_thread_run( thread, func, args );
	}


NsError ns_thread_get_priority( const NsThread *thread, NsThreadPriority *priority )
	{
	ns_assert( NULL != thread );
	ns_assert( NULL != priority );

	return ____ns_thread_get_priority( thread, priority );
	}


NsError ns_thread_set_priority( NsThread *thread, NsThreadPriority priority )
	{
	ns_assert( NULL != thread );
	return ____ns_thread_set_priority( thread, priority );
	}


NsError ns_thread_self( NsThread *thread )
	{
	ns_assert( NULL != thread );
	return ____ns_thread_self( thread );
	}
