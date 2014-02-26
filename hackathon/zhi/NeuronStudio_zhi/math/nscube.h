#ifndef __NS_MATH_CUBE_H__
#define __NS_MATH_CUBE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <math/nscubedecls.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP void ns_cubei_zero( NsCubei *C );

NS_IMPEXP nsint ns_cubei_width( const NsCubei *C );
NS_IMPEXP nsint ns_cubei_height( const NsCubei *C );
NS_IMPEXP nsint ns_cubei_length( const NsCubei *C );

NS_DECLS_END

#endif/* __NS_MATH_CUBE_H__ */
