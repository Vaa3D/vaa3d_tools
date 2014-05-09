#ifndef __NS_STD_THREAD_H__
#define __NS_STD_THREAD_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef void ( *NsThreadFunc )( nspointer args );

typedef enum
   {
	NS_THREAD_PRIORITY_UNKNOWN,
   NS_THREAD_PRIORITY_NORMAL,
   NS_THREAD_PRIORITY_LOW,
   NS_THREAD_PRIORITY_HIGH
   }
   NsThreadPriority;


typedef struct _NsThread
	{
	#ifdef NS_OS_WINDOWS
   nsulong  handle;
	#endif
	}
	NsThread;


NS_IMPEXP void ns_thread_construct( NsThread *thread );
NS_IMPEXP void ns_thread_destruct( NsThread *thread );

NS_IMPEXP NsError ns_thread_run( NsThread *thread, NsThreadFunc func, nspointer args );

NS_IMPEXP NsError ns_thread_get_priority( const NsThread *thread, NsThreadPriority *priority );
NS_IMPEXP NsError ns_thread_set_priority( NsThread *thread, NsThreadPriority priority );

NS_IMPEXP NsError ns_thread_self( NsThread *thread );

NS_IMPEXP nsulong ns_thread_id( void );
NS_IMPEXP void ns_thread_sleep( nssize milliseconds );
NS_IMPEXP void ns_thread_yield( void );


NS_IMPEXP nsulong ns_process_id( void );


#define NS_THREAD_INVALID_ID   0
#define NS_PROCESS_INVALID_ID  0

NS_DECLS_END

#endif/* __NS_STD_THREAD_H__ */
