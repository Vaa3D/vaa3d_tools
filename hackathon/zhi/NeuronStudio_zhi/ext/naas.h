/**************************************************************

    Creates a non axis aligned set of voxels centered at zero
    by applying a shear to an Axis-Aligned set.


***************************************************************/
#ifndef NAAS_H
#define NAAS_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmemory.h>
#include <math/nsvector.h>

NS_DECLS_BEGIN

/* 'dim' are the radii in pixels of the box.
	call get_non_axis_aligned_set( 3 r's, 3 r's, 0, ... );

	'count' is filled with the number of points
	in the allocated array that is returned. If
	a memory failure occurs then NULL is returned.
*/
NS_IMPEXP NsVector3i* get_non_axis_aligned_set
	(
	nsint        dim1,
	nsint        dim2,
	nsint        dim3,
	nsfloat      xdir,
	nsfloat      ydir,
	nsfloat      zdir,
	nssize      *count,
	NsVector3i  *corners
	);

NS_DECLS_END

#endif
