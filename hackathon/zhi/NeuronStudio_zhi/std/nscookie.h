#ifndef __NS_STD_COOKIE_H__
#define __NS_STD_COOKIE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>

NS_DECLS_BEGIN

typedef nsuint16 nscookie;

/* Set a magic value in a buffer to detect overruns. The buffer
	can be NULL in which case obviously nothing is set. 
	IMPORTANT: The buffer must be have 'size' + sizeof( nscookie )
	many bytes if not NULL. */
NS_IMPEXP void ns_cookie_set( nspointer buffer, nssize size );

/* Check the magic value. If the buffer is NULL then 'true' is returned.
	Returns 'false' if the value has been corrupted, i.e. a buffer overrun
	occurred at some time. */
NS_IMPEXP nsboolean ns_cookie_valid( nsconstpointer buffer, nssize size );

NS_DECLS_END

#endif/* __NS_STD_COOKIE_H__ */
