#include "nsqueue.h"


void ns_queue_construct
	(
	NsQueue         *queue,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != queue );
	ns_list_construct( &queue->list, finalize_func );
	}


void ns_queue_destruct( NsQueue *queue )
	{
	ns_assert( NULL != queue );
	ns_list_destruct( &queue->list );
	}


NsError ns_queue_add( NsQueue *queue, nspointer object )
	{
	ns_assert( NULL != queue );
	return ns_list_push_back( &queue->list, object );
	}


void ns_queue_remove( NsQueue *queue )
	{
	ns_assert( NULL != queue );
	ns_assert( ! ns_queue_is_empty( queue ) );

   ns_list_pop_front( &queue->list );
	}
