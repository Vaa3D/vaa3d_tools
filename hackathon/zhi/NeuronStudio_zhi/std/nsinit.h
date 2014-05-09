#ifndef __NS_STD_INIT_H__
#define __NS_STD_INIT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmutex.h>
#include <std/nsvalue.h>
#include <std/nsprofiler.h>
#include <image/nspixels.h>

NS_DECLS_BEGIN

NS_IMPEXP NsError ns_init( nsint argc, const nschar* argv[] );
NS_IMPEXP void ns_finalize( void );

NS_DECLS_END

#endif/* __NS_STD_INIT_H__ */
