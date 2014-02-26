#include "nsfrustum.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>
#include <math/nspoint.h>
#include <math/nsplane.h>


void ns_conical_frustumd_facets
	(
	const NsConicalFrustumd  *cf,
	NsLine3d                 *lines,
	nssize                    num_facets
	)
	{
	NsPlaned    plane;
	NsPoint3d   P;
	NsPoint3d   V;
	NsLine3d    line;
	nssize      i;
	nsdouble    angle;
	nsdouble    delta;


	ns_assert( NULL != cf );
	ns_assert( NULL != lines );

	ns_vector3d_norm( ns_vector3d_sub( &plane.N, &cf->C2, &cf->C1 ) );

	/* Get a vector V perpendicular to N. */
	ns_planed_get_second_point( &plane, &cf->C1, &P );
	ns_vector3d_norm( ns_vector3d_sub( &V, &P, &cf->C1 ) );

	/* Create a line whose endpoints are at the right distances
		from the cylinder centers. */
	ns_vector3d_add( &line.P1, &cf->C1, ns_vector3d_scale( &P, &V, cf->r1 ) );
	ns_vector3d_add( &line.P2, &cf->C2, ns_vector3d_scale( &P, &V, cf->r2 ) );

	angle = 0.0;
	delta = NS_DEGREES_TO_RADIANS( 360.0 / num_facets );

	for( i = 0; i < num_facets; ++i )
		{
		/* Set the current line equal to the first and then rotate
			its endpoints. */
		lines[i] = line;

		ns_point3d_axis_rotate( &( lines[i].P1 ), &cf->C1, &plane.N, angle );
		ns_point3d_axis_rotate( &( lines[i].P2 ), &cf->C2, &plane.N, angle );

		angle += delta;
		}
	}
