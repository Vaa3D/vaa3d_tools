#ifndef __NS_STD_TIME_H__
#define __NS_STD_TIME_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef nsintmax  nstimer;


/* High performance timer if one is available. */
NS_IMPEXP nstimer ns_timer( void );
NS_IMPEXP nsdouble ns_difftimer( nstimer stop, nstimer start );

NS_DECLS_END

#endif/* __NS_STD_TIME_H__ */
