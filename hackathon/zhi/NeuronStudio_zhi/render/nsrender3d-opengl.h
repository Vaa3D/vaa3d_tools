#ifndef __NS_RENDER_3D_OPENGL_H__
#define __NS_RENDER_3D_OPENGL_H__

#include <nsconfigure.h>
#include <ns-gl.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmath.h>
#include <math/nsvector.h>
#include <math/nsmatrix.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nscolor.h>
#include <image/nssampler.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>
#include <render/nsrender.h>

NS_DECLS_BEGIN

NS_IMPEXP void ns_render3d_pre_images( NsRenderState *state );

NS_IMPEXP void ns_render3d_image
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_images( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_texts( NsRenderState *state );

NS_IMPEXP void ns_render3d_text
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_texts( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_borders( NsRenderState *state );

NS_IMPEXP void ns_render3d_border
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_borders( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_lines( NsRenderState *state );

NS_IMPEXP void ns_render3d_line
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_lines( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_points( NsRenderState *state );

NS_IMPEXP void ns_render3d_point
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_points( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_spheres( NsRenderState *state );

NS_IMPEXP void ns_render3d_sphere
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_spheres( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_aabboxes( NsRenderState *state );

NS_IMPEXP void ns_render3d_aabbox
	(
	const NsAABBox3d  *B,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_aabboxes( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_frustums( NsRenderState *state );

NS_IMPEXP void ns_render3d_frustum
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_frustums( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_triangles( NsRenderState *state );

NS_IMPEXP void ns_render3d_triangle
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_triangles( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_ellipses( NsRenderState *state );

NS_IMPEXP void ns_render3d_ellipse
	(
	const NsVector3f  *V,
	const NsVector3f  *R,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_ellipses( NsRenderState *state );


NS_IMPEXP void ns_render3d_pre_rectangles( NsRenderState *state );

NS_IMPEXP void ns_render3d_rectangle
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render3d_post_rectangles( NsRenderState *state );

NS_DECLS_END

#endif/* __NS_RENDER_3D_OPENGL_H__ */
