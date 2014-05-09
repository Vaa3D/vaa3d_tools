#ifndef __NS_MATH_PROJECTOR_H__
#define __NS_MATH_PROJECTOR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmath.h>
#include <math/nsspheredecls.h>
#include <math/nsdragrectdecls.h>

NS_DECLS_BEGIN

typedef struct _NsProjector
	{
	nsdouble         k;
   nsint            viewport[4];
	const nsdouble  *modelview_matrix;
	const nsdouble  *projection_matrix;
	}
	NsProjector;


NS_IMPEXP void ns_projector_init
	(
	NsProjector     *proj,
	nsdouble         window_height,
	nsdouble         near_plane_z,
	nsdouble         far_plane_z,
	nsdouble         field_of_view_degrees,
	const nsdouble  *modelview_matrix,
	const nsdouble  *projection_matrix
	);


NS_IMPEXP nsboolean ns_sphered_projects_onto_drag_rect
	(
	const NsSphered    *S,
	const NsDragRect   *R,
	const NsProjector  *proj
	);

NS_DECLS_END

#endif/* __NS_MATH_PROJECTOR_H__ */
