#ifndef __NS_RENDER_2D_OS_WINDOWS_H__
#define __NS_RENDER_2D_OS_WINDOWS_H__

#include <nsconfigure.h>

#include <windows.h>

#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsascii.h>
#include <math/nsvector.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nscolor.h>
#include <image/nssampler.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>
#include <render/nsrender.h>

NS_DECLS_BEGIN

#define _NS_RENDER2D_TO_CLIENT_SPACE( state, V, C )\
	(V).x = (V).x - (C).x;\
	(V).y = (V).y - (C).y;\
	ns_vector2f_cmpd_scale( &(V), (state)->constants.zoom )


NS_IMPEXP void ns_render2d_pre_images( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_images( NsRenderState *state );

NS_IMPEXP void ns_render2d_image_xy
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_image_zy
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_image_xz
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_texts( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_texts( NsRenderState *state );

NS_IMPEXP void ns_render2d_text_xy
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_text_zy
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_text_xz
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_borders( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_borders( NsRenderState *state );

NS_IMPEXP void ns_render2d_border_xy
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_border_zy
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_border_xz
	(
	const NsVector3i  *V1,
	const NsVector3i  *V2,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_lines( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_lines( NsRenderState *state );

NS_IMPEXP void ns_render2d_line_xy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_line_zy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_line_xz
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_points( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_points( NsRenderState *state );

NS_IMPEXP void ns_render2d_point_xy
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_point_zy
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_point_xz
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_spheres( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_spheres( NsRenderState *state );

NS_IMPEXP void ns_render2d_sphere_xy
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_sphere_zy
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_sphere_xz
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_aabboxes( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_aabboxes( NsRenderState *state );

NS_IMPEXP void ns_render2d_aabbox_xy
	(
	const NsAABBox3d  *B,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_aabbox_zy
	(
	const NsAABBox3d  *B,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_aabbox_xz
	(
	const NsAABBox3d  *B,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_frustums( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_frustums( NsRenderState *state );

NS_IMPEXP void ns_render2d_frustum_xy
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_frustum_zy
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_frustum_xz
	(
	const NsVector3f  *P1,
	nsfloat            radius1,
	const NsVector3f  *P2,
	nsfloat            radius2,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_triangles( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_triangles( NsRenderState *state );

NS_IMPEXP void ns_render2d_triangle_xy
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_triangle_zy
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_triangle_xz
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_ellipses( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_ellipses( NsRenderState *state );

NS_IMPEXP void ns_render2d_ellipse_xy
	(
	const NsVector3f  *V,
	const NsVector3f  *R,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_ellipse_zy
	(
	const NsVector3f  *V,
	const NsVector3f  *R,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_ellipse_xz
	(
	const NsVector3f  *V,
	const NsVector3f  *R,
	NsRenderState     *state
	);


NS_IMPEXP void ns_render2d_pre_rectangles( NsRenderState *state );
NS_IMPEXP void ns_render2d_post_rectangles( NsRenderState *state );

NS_IMPEXP void ns_render2d_rectangle_xy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_rectangle_zy
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_IMPEXP void ns_render2d_rectangle_xz
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	);

NS_DECLS_END

#endif/* __NS_RENDER_2D_OS_WINDOWS_H__ */
