#ifndef __NS_MODEL_MODEL_TRANSFORM_H__
#define __NS_MODEL_MODEL_TRANSFORM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <math/nsmatrix.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

/* NOTE: Using a line since it has a start and end position.
	i.e. just using one list instead of two lists of NsVector3f. */
typedef struct _NsModelTransform
	{
	NsList        vertices;
	NsByteArray   lines;
	NsVector3f    C;
	nsint         which;
	NsMatrix44f*  ( *rotate_func )( NsMatrix44f*, nsfloat );
	NsVector3f    P1;
	nsfloat       angle;
	NsVector3f    R;
	nsboolean     rotated;
	nsboolean     init;
	}
	NsModelTransform;




/* Translate all selected vertices by the amount 'T'. i.e. for
	all the vertices V, apply V->position += T. */

/* Initializes the data structure. */
NS_IMPEXP NsError ns_model_translate_selected_vertices_begin
	(
	NsModelTransform  *xfrm,
	NsModel           *model
	);

/* Applies translation. */
NS_IMPEXP void ns_model_translate_selected_vertices_apply
	(
	NsModelTransform  *xfrm,
	const NsVector3f  *T
	);

/* Pass false for 'confirm' to restore original positions.
	Sets the 'list' with vertex auto iteration numbers. */
NS_IMPEXP NsError ns_model_translate_selected_vertices_end
	(
	NsModelTransform  *xfrm,
	nsboolean          confirm,
	NsList            *list
	);




/* Rotates the selected vertices by the specified angle, in radians,
	around the axis defined by the given plane.

	NS_XY: Rotate around Z
	NS_ZY: Rotate around X
	NS_XZ: Rotate around Y

	If 'center' is true then a rotation center is found as the center
	of mass of the selected vertices. Otherwise the vertices just rotate
	around the origin 0,0,0. */

/* Initializes the data structure. */
NS_IMPEXP NsError ns_model_rotate_selected_vertices_begin
	(
	NsModelTransform  *xfrm,
	NsModel           *model,
	nsint              which
	);

/* Applies rotation. 'P' is some point around the center. */
NS_IMPEXP void ns_model_rotate_selected_vertices_apply
	(
	NsModelTransform  *xfrm,
	const NsVector3f  *P
	);

/* Pass false for 'confirm' to restore original positions. */
NS_IMPEXP NsError ns_model_rotate_selected_vertices_end
	(
	NsModelTransform  *xfrm,
	nsboolean          confirm,
	NsList            *list
	);


NS_IMPEXP nssize ns_model_transform_size( const NsModelTransform *xfrm );

NS_IMPEXP void ns_model_transform_center( const NsModelTransform *xfrm, NsVector3f *C );
NS_IMPEXP void ns_model_transform_radius( const NsModelTransform *xfrm, NsVector3f *R );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_TRANSFORM_H__ */
