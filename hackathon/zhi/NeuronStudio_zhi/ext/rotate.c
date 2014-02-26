#include "rotate.h"


void get_z_rotated_vector_with_respect_to_line
	(
	nsfloat      xstart,
	nsfloat      ystart,
   nsfloat      xend,
	nsfloat      yend,
	nsfloat      angle,
	NsVector2f  *V
	)
	{
	nsfloat dx, dy, mag, ang;

	dx = xend - xstart;
	dy = yend - ystart;

	mag = ns_sqrtf( dx * dx + dy * dy );
	ang = ns_acosf( dx / mag );

	ang = ang + ( nsfloat )NS_DEGREES_TO_RADIANS( ( nsdouble )angle );

	V->x = ns_cosf( ang );
	V->y = ns_sinf( ang );

	/* This is to account for the y-axis being reversed in
		voxel space. */
	if( dy < 0.0f )
		V->y = -V->y;
	}
