#ifndef __NS_MATH_POINT_H__
#define __NS_MATH_POINT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmath.h>
#include <math/nspointdecls.h>
#include <math/nsvectordecls.h>
#include <math/nsaabboxdecls.h>
#include <math/nsspheredecls.h>
#include <math/nscircledecls.h>
#include <math/nslinedecls.h>
#include <math/nscubedecls.h>
#include <math/nsdragrectdecls.h>

NS_DECLS_BEGIN

#define ns_point2d ns_vector2d
#define ns_point3d ns_vector3d
#define ns_point4d ns_vector4d

#define ns_point2f ns_vector2f
#define ns_point3f ns_vector3f
#define ns_point4f ns_vector4f

#define ns_point2l ns_vector2l
#define ns_point3l ns_vector3l
#define ns_point4l ns_vector4l

#define ns_point2i ns_vector2i
#define ns_point3i ns_vector3i
#define ns_point4i ns_vector4i

#define ns_point2s ns_vector2s
#define ns_point3s ns_vector3s
#define ns_point4s ns_vector4s


NS_IMPEXP nsboolean ns_point2d_inside_aabbox( const NsPoint2d *P, const NsAABBox2d *B );
NS_IMPEXP nsboolean ns_point3d_inside_aabbox( const NsPoint3d *P, const NsAABBox3d *B );

NS_IMPEXP nsboolean ns_point3d_inside_sphere( const NsPoint3d *P, const NsSphered *S );
NS_IMPEXP nsboolean ns_point3f_inside_sphere( const NsPoint3f *P, const NsSpheref *S );

NS_IMPEXP nsboolean ns_point2d_inside_circle( const NsPoint2d *P, const NsCircled *C );
NS_IMPEXP nsboolean ns_point2f_inside_circle( const NsPoint2f *P, const NsCirclef *C );

NS_IMPEXP nsboolean ns_point3i_inside_cube( const NsPoint3i *P, const NsCubei *C );

NS_IMPEXP nsboolean ns_point2i_inside_drag_rect( const NsPoint2i *P, const NsDragRect *R );

/* R = R rotated by the angle theta about the line through
	P parallel to the axis(direction vector) A. */
NS_IMPEXP void ns_point3d_axis_rotate( NsPoint3d *R, const NsPoint3d *P, const NsVector3d *A, nsdouble theta );
NS_IMPEXP void ns_point3f_axis_rotate( NsPoint3f *R, const NsPoint3f *P, const NsVector3f *A, nsfloat theta );

/* The "project" functions returns false if the point 'P' does not project
	onto the line segment 'L'. */
NS_IMPEXP nsboolean ns_point3d_projects_onto_line( const NsPoint3d *P, const NsLine3d *L );
NS_IMPEXP nsboolean ns_point3f_projects_onto_line( const NsPoint3f *P, const NsLine3f *L );


/* Returns the 'u' value of how far along the line the point 'I' falls. */
NS_IMPEXP nsdouble ns_point3d_project_onto_line( const NsPoint3d *P, const NsLine3d *L, NsPoint3d *I );
NS_IMPEXP nsfloat ns_point3f_project_onto_line( const NsPoint3f *P, const NsLine3f *L, NsPoint3f *I );


NS_IMPEXP nsfloat ns_point3f_distance_to_line( const NsPoint3f *P, const NsLine3f *L );

NS_IMPEXP nsfloat ns_point3f_distance_to_point( const NsPoint3f *P1, const NsPoint3f *P2 );

NS_DECLS_END

#endif/* __NS_MATH_POINT_H__ */
