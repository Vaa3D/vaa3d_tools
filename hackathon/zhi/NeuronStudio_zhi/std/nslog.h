#ifndef __NS_STD_LOG_H__
#define __NS_STD_LOG_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsformat.h>
#include <std/nsfile.h>
#include <std/nstime.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

NS_IMPEXP void ns_log_init( nsboolean enabled, nsboolean flush_after_entry, nssize reset_size );

NS_IMPEXP NsError ns_log_open( const nschar *path );
NS_IMPEXP nsboolean ns_log_is_open( void );

NS_IMPEXP void ns_log_close( void );


typedef enum
   {
   NS_LOG_ENTRY_UNKNOWN,
   NS_LOG_ENTRY_FUNCTION,
   NS_LOG_ENTRY_EVENT_BEGIN,
   NS_LOG_ENTRY_EVENT_STEP,
   NS_LOG_ENTRY_EVENT_END,
   NS_LOG_ENTRY_NOTE,
	NS_LOG_ENTRY_ERROR,

   _NS_LOG_ENTRY_NUM_TYPES
   }
   NsLogEntryType;


NS_IMPEXP void ns_log_enable( nsboolean enabled );
NS_IMPEXP nsboolean ns_log_is_enabled( void );

/* If echo is on, all entries are printed to the console as well. */
NS_IMPEXP void ns_log_echo( nsboolean echo );
NS_IMPEXP nsboolean ns_log_is_echo( void );

NS_IMPEXP void ns_log_entry( NsLogEntryType type, const nschar *format, ... );

NS_DECLS_END

#endif/* __NS_STD_LOG_H__ */

