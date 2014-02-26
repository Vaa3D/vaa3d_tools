#ifndef __NS_STD_DIR_H__
#define __NS_STD_DIR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsascii.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsDir
	{
	#ifdef NS_OS_WINDOWS
   nspointer   fd;
   nslong      handle;
   nsboolean   at_end;
	nsboolean   at_first;
	#endif
   nsboolean   is_open;
	nschar     *name;
	}
	NsDir;


NS_IMPEXP void ns_dir_construct( NsDir *dir );
NS_IMPEXP void ns_dir_destruct( NsDir *dir );

NS_IMPEXP nsboolean ns_dir_is_open( const NsDir *dir );

/* Working directory must be set before calling. */
NS_IMPEXP NsError ns_dir_open( NsDir *dir );

NS_IMPEXP void ns_dir_close( NsDir *dir );

NS_IMPEXP NsError ns_dir_rewind( NsDir *dir );

NS_IMPEXP const nschar* ns_dir_read( NsDir *dir );

/* Gives the full path of the directory as if call
	to getcwd(). */
NS_IMPEXP const nschar* ns_dir_name( const NsDir *dir );

NS_DECLS_END

#endif/* __NS_STD_DIR_H__ */
