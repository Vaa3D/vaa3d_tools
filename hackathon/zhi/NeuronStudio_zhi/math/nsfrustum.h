#ifndef __NS_MATH_FRUSTUM_H__
#define __NS_MATH_FRUSTUM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <math/nsfrustumdecls.h>
#include <math/nslinedecls.h>

NS_DECLS_BEGIN

/* Convert a conical frustum to a number of
	lengthwise facets(implied by lines). */
NS_IMPEXP void ns_conical_frustumd_facets
	(
	const NsConicalFrustumd  *cf,
	NsLine3d                 *lines,
	nssize                    num_facets
	);

NS_DECLS_END

#endif/* __NS_MATH_FRUSTUM_H__ */
