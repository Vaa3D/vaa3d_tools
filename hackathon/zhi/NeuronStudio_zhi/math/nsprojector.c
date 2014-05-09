#include "nsprojector.h"
#include <ns-gl.h>

/* Avoid any circular inclusion. */
#include <math/nscircle.h>


//nsdouble F, M;


void ns_projector_init
	(
	NsProjector     *proj,
	nsdouble         window_height,
	nsdouble         near_plane_z,
	nsdouble         far_plane_z,
	nsdouble         field_of_view_degrees,
	const nsdouble  *modelview_matrix,
	const nsdouble  *projection_matrix
	)
	{
	ns_assert( NULL != proj );
	ns_assert( NULL != modelview_matrix );
	ns_assert( NULL != projection_matrix );

	NS_USE_VARIABLE( far_plane_z );

	proj->modelview_matrix  = modelview_matrix;
	proj->projection_matrix = projection_matrix;

	glGetIntegerv( GL_VIEWPORT, proj->viewport );

	/* The following code uses simple triognometry and interpolation to estimate the radius (in pixels) of a sphere
		projected onto the screen by the OpenGL rendering pipeline. */

	/*	These equations allow for a simple circle and box intersection test for selecting objects on the 3D window. */
	
	/* Given a sphere of radius 'r', the equations below determine the size of the circle that encloses the OpenGL
		rendered sphere. */

	/* Equations by Alfredo Rodriguez on June 2, 2009. */
	//F = window_height / ( ns_tan( 0.5 * NS_DEGREES_TO_RADIANS( field_of_view_degrees ) ) * 2.0 * far_plane_z );
	//M = ( far_plane_z / near_plane_z ) - 1.0;

	/* Simpler equation by Alfredo Rodriguez on June 3, 2009. */
	proj->k = window_height / ( near_plane_z * 2.0 * ns_tan( 0.5 * NS_DEGREES_TO_RADIANS( field_of_view_degrees ) ) );
	}


// Equation by Alfredo Rodriguez on June 2, 2009.
//#define _NS_PROJECT_RADIUS( proj, r, wz )\
//	( F * ( (r) * M * ( 1.0 - (wz) ) + (r) ) )

#define _NS_PROJECT_RADIUS( proj, r, wz )\
	( (proj)->k * (r) * ( 1.0 - (wz) ) )


nsboolean ns_sphered_projects_onto_drag_rect
	(
	const NsSphered    *S,
	const NsDragRect   *R,
	const NsProjector  *proj
	)
	{
	GLdouble   wx, wy, wz;
	NsCircled  C;
	//nsboolean  ret;


	if( gluProject(
			S->O.x,
			S->O.y,
			S->O.z,
			proj->modelview_matrix,
			proj->projection_matrix,
			proj->viewport,
			&wx,
			&wy,
			&wz
			) )
		{
		if( 0.0 <= wz && wz <= 1.0 )
			{
			C.O.x = wx;
			C.O.y = wy;
			C.O.y = ( ( nsdouble )proj->viewport[3] - C.O.y ) - 1.0;
			C.r   = _NS_PROJECT_RADIUS( proj, S->r, wz );

			return ns_circled_intersects_drag_rect( &C, R );

			/*if( ret )
				{
				ns_println(
					"(%d,%d %d) intersected [%d,%d %d,%d]",
					C.O.x, C.O.y, C.r,
					R->start_x, R->start_y, R->end_x, R->end_y
					);
				}

			return ret;*/
			}
		}

	return NS_FALSE;
	}
