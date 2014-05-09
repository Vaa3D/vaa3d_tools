#ifndef ROTATE_H
#define ROTATE_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmath.h>
#include <math/nsvector.h>

NS_DECLS_BEGIN

/* Returns a unit vector at an 'angle' ( in the Z-axis )with
	respect to the line defined by the endpoints. */
NS_IMPEXP void get_z_rotated_vector_with_respect_to_line
	(
	nsfloat      xstart,
	nsfloat      ystart,
   nsfloat      xend,
	nsfloat      yend,
	nsfloat      angle,
	NsVector2f  *V
	);

NS_DECLS_END

#endif
