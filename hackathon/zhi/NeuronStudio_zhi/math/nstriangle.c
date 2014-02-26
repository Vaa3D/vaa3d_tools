#include "nstriangle.h"

/* Avoid any circular inclusion. */
#include <math/nsvector.h>


nsdouble ns_triangle3f_surface_area( const NsTriangle3f *T )
	{
	NsVector3f AB, AC, X;

	/* Compute the cross product. */
	ns_vector3f_sub( &AB, &T->B, &T->A ); // AB = B - A
	ns_vector3f_sub( &AC, &T->C, &T->A ); // AC = C - A

	ns_vector3f_cross( &X, &AB, &AC ); // X = AB x AC

	/* The area of a triangle is 1/2 the magnitude of
		the cross product. */
	return .5 * ( nsdouble )ns_vector3f_mag( &X );
	}
