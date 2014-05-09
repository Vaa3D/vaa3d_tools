#ifndef CIRCLES_H
#define CIRCLES_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmemory.h>
#include <std/nsmath.h>
#include <math/nsvector.h>
#include <ns-gl.h>


/* These only need to be called once at program startup (init)
	and program shutdown (finalize). */
NS_IMPEXP void CirclesInit( void );
NS_IMPEXP void CirclesFinalize( void );

NS_IMPEXP void CirclesBegin( nsint complexity, nssize line_width );
NS_IMPEXP void CirclesEnd( void );

/* NOTE: 'rgba' is assumed to be an array of 4 unsigned char's. */
NS_IMPEXP void CircleRender( const NsVector3d *P, nsfloat radius, const nsuchar *rgba );

#endif
