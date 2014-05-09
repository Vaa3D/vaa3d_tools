#ifndef __NS_STD_MUTEX_H__
#define __NS_STD_MUTEX_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsascii.h>
#include <std/nsutil.h>
#include <std/nslog.h>

NS_DECLS_BEGIN

#define NS_MUTEX_NAME_SIZE  64

typedef struct _NsMutex
	{
   #ifdef NS_OS_WINDOWS
   nspointer  handle;
   #endif
	nsboolean  is_created;
	#ifdef NS_DEBUG
	nsulong    owner_thread_id;
	nschar     name[ NS_MUTEX_NAME_SIZE ]; 
	#endif
	nsulong    _pad0;
	}
	NsMutex;


/* Actual creation/destruction of the platform-dependent mutex
	implementation is delayed until create/destroy are called. */
NS_IMPEXP void ns_mutex_construct( NsMutex *mutex );
NS_IMPEXP void ns_mutex_destruct( NsMutex *mutex );

NS_IMPEXP nsboolean ns_mutex_is_created( NsMutex *mutex );

/* Sets 'is_init' to true if the virtual call succeeds. */
NS_IMPEXP NsError ns_mutex_create( NsMutex *mutex );

/* Only makes the virtual call if 'is_init' is true. */
NS_IMPEXP NsError ns_mutex_destroy( NsMutex *mutex );

NS_IMPEXP NsError ns_mutex_lock( NsMutex *mutex );

NS_IMPEXP NsError ns_mutex_try_lock( NsMutex *mutex, nsboolean *did_lock );

NS_IMPEXP NsError ns_mutex_unlock( NsMutex *mutex );


#ifdef NS_DEBUG
	#define ns_mutex_get_name( mutex )\
		( (mutex)->name )

	#define ns_mutex_set_name( mutex, _name )\
		( ns_ascii_strncpy( (mutex)->name, (_name), NS_MUTEX_NAME_SIZE ) )
#else
	#define ns_mutex_get_name( mutex )\
		NULL

	#define ns_mutex_set_name( mutex, _name )
#endif/* NS_DEBUG */


/* See nsmacros.h for definition of NS_STATIC_MUTEX_DECLARE. */
#define NS_STATIC_MUTEX_IMPLEMENT( smutex )\
nspointer smutex( void )\
	{\
	NS_PRIVATE NsMutex _##smutex;\
	return &_##smutex;\
	}

NS_DECLS_END

#endif/* __NS_STD_MUTEX_H__ */
