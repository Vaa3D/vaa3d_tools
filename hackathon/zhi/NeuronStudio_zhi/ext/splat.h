/****************************************************
    SPLAT.H :   OpenGL Splat rendering routines.

    Copyright (c) Alfredo Rodriguez, 2005
    Last Revision Date: 9/20/2005

*****************************************************/
#ifndef SPLAT_H
#define SPLAT_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmemory.h>
#include <std/nsmath.h>

NS_DECLS_BEGIN

NS_IMPEXP void SplatBegin( void );
NS_IMPEXP void SplatEnd( void );

NS_IMPEXP void SplatRender( nsfloat coord[3], nsfloat rgba[4] );

NS_IMPEXP void SplatSetSize( nsfloat s );

NS_DECLS_END

#endif
