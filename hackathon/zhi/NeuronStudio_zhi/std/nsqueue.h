#ifndef __NS_STD_QUEUE_H__
#define __NS_STD_QUEUE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nslist.h>

NS_DECLS_BEGIN

typedef struct _NsQueue
	{
   NsList  list;
	}
	NsQueue;


NS_IMPEXP void ns_queue_construct
	(
	NsQueue         *queue,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_queue_destruct( NsQueue *queue );

NS_IMPEXP NsError ns_queue_add( NsQueue *queue, nspointer object );
NS_IMPEXP void ns_queue_remove( NsQueue *queue );

#define ns_queue_is_empty( queue )\
   ( ns_list_is_empty( &( queue )->list ) )
   
#define ns_queue_size( queue )\
   ( ns_list_size( &( queue )->list ) )

#define ns_queue_clear( queue )\
	( ns_list_clear( &( queue )->list ) )

#define ns_queue_front( queue )\
   ( ns_list_peek_front( &( queue )->list ) )

#define ns_queue_get_finalize_func( queue )\
	( ns_list_get_finalize_func( &( queue )->list ) )

#define ns_queue_set_finalize_func( queue, finalize_func )\
	( ns_list_set_finalize_func( &( queue )->list, ( finalize_func ) ) )

NS_DECLS_END

#endif/* __NS_STD_QUEUE_H__ */
