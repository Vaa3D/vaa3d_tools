#ifndef __NS_MATH_LINE_H__
#define __NS_MATH_LINE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <math/nslinedecls.h>
#include <math/nsraydecls.h>
#include <math/nsaabboxdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsLine3d* ns_line3d
	(
	NsLine3d  *L,
	nsdouble   x1,
	nsdouble   y1,
	nsdouble   z1,
	nsdouble   x2,
	nsdouble   y2,
	nsdouble   z2
	);

NS_IMPEXP NsLine3f* ns_line3f
	(
	NsLine3f  *L,
	nsfloat    x1,
	nsfloat    y1,
	nsfloat    z1,
	nsfloat    x2,
	nsfloat    y2,
	nsfloat    z2
	);

/* NOTE: The rays origin is set to line->P1 and the rays direction
	vector is normalized. */
NS_IMPEXP NsRay3d* ns_line3d_to_ray( const NsLine3d *line, NsRay3d *ray );


/*	Calculate the line segment PaPb that is the shortest route between
	two lines L1=P1P2 and L2=P3P4. Calculate also the values of mua and mub where
	Pa = P1 + mua (P2 - P1)
	Pb = P3 + mub (P4 - P3)

	Return false if no solution exists.

	NOTE: 'mua' and/or 'mub' can be negative if not necessary.
*/
NS_IMPEXP nsboolean ns_line3d_intersects_line
	(
	const NsLine3d  *L1,
	const NsLine3d  *L2,
	NsLine3d        *isect_line,
	nsdouble        *mua,
	nsdouble        *mub
	);


/* Finds the shortest line segment 'L3' between the given line
	segments 'L1' and 'L2'. Some implementations will return a
	boolean indicating whether or not one could be found (i.e.
	the endpoints of 'L3' fall with the segments 'L1' and 'L2'.

	Sets 'L3' as:
	L3.P1 is a point on the line defined by the L1 segment.
	L3.P2 is a point on the line defined by the L2 segment.
*/
NS_IMPEXP void ns_line3d_find_shortest_between
	(
	const NsLine3d  *L1,
	const NsLine3d  *L2,
	NsLine3d        *L3
	);


/* Runs a 3D Bresenham line drawing algorithm for the passed
	line 'L'.. Calls the passed 'raster_func' as raster points
	are generated. */
NS_IMPEXP void ns_line3i_raster
	(
	const NsLine3i  *L,
	void             ( *raster_func )( const NsVector3i*, nspointer ),
	nspointer        user_data
	);


NS_IMPEXP nsdouble ns_line3d_length( const NsLine3d *L );
NS_IMPEXP nsfloat ns_line3f_length( const NsLine3f *L );

NS_DECLS_END

#endif/* __NS_MATH_LINE_H__ */
