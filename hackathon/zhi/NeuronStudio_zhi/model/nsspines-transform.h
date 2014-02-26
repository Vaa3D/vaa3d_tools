#ifndef __NS_MODEL_SPINES_TRANSFORM_H__
#define __NS_MODEL_SPINES_TRANSFORM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <math/nsmatrix.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>

NS_DECLS_BEGIN

/* NOTE: Using a line since it has a start and end position.
	i.e. just using one list instead of two lists of NsVector3f. */
typedef struct _NsSpinesTransform
	{
	NsList        spines;
	NsByteArray   entries;
	NsVector3f    C;
	nsint         which;
	NsMatrix44f*  ( *rotate_func )( NsMatrix44f*, nsfloat );
	NsVector3f    P1;
	nsfloat       angle;
	NsVector3f    R;
	nsboolean     rotated;
	nsboolean     init;
	}
	NsSpinesTransform;




/* Translate all selected spines by the amount 'T'. i.e. for
	all the spines V, apply V->position += T. */

/* Initializes the data structure. */
NS_IMPEXP NsError ns_spines_translate_selected_begin
	(
	NsSpinesTransform  *xfrm,
	NsModel            *model
	);

/* Applies translation. */
NS_IMPEXP void ns_spines_translate_selected_apply
	(
	NsSpinesTransform  *xfrm,
	const NsVector3f   *T
	);

/* Pass false for 'confirm' to restore original positions. */
NS_IMPEXP void ns_spines_translate_selected_end
	(
	NsSpinesTransform  *xfrm,
	nsboolean           confirm
	);




/* Rotates the selected spines by the specified angle, in radians,
	around the axis defined by the given plane.

	NS_XY: Rotate around Z
	NS_ZY: Rotate around X
	NS_XZ: Rotate around Y

	If 'center' is true then a rotation center is found as the center
	of mass of the selected spines. Otherwise the spines just rotate
	around the origin 0,0,0. */

/* Initializes the data structure. */
NS_IMPEXP NsError ns_spines_rotate_selected_begin
	(
	NsSpinesTransform  *xfrm,
	NsModel            *model,
	nsint               which
	);

/* Applies rotation. 'P' is some point around the center. */
NS_IMPEXP void ns_spines_rotate_selected_apply
	(
	NsSpinesTransform  *xfrm,
	const NsVector3f   *P
	);

/* Pass false for 'confirm' to restore original positions. */
NS_IMPEXP void ns_spines_rotate_selected_end
	(
	NsSpinesTransform  *xfrm,
	nsboolean           confirm
	);


NS_IMPEXP nssize ns_spines_transform_size( const NsSpinesTransform *xfrm );

NS_IMPEXP void ns_spines_transform_center( const NsSpinesTransform *xfrm, NsVector3f *C );
NS_IMPEXP void ns_spines_transform_radius( const NsSpinesTransform *xfrm, NsVector3f *R );


/* Finds the spine with position 'P', that is the furthest from the point 'O'. */
NS_IMPEXP void ns_spines_transform_furthest( const NsSpinesTransform *xfrm, const NsVector3f *O, NsVector3f *P );

NS_DECLS_END

#endif/* __NS_MODEL_SPINES_TRANSFORM_H__ */
