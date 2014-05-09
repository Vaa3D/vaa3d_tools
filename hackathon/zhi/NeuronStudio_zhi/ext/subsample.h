/********************************************************************
*   This program is used to create a "pipe" to subsample a stack of
*   tiff images.
*
*********************************************************************/

#ifndef SUBSAMPLE_H
#define SUBSAMPLE_H


#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmemory.h>
#include <std/nsmath.h>


NS_DECLS_BEGIN

typedef struct _SSIMAGE
	{
	nsint             xdim;
	nsint             ydim;
	nspointer         data;
	struct _SSIMAGE  *next;
	}
	SSIMAGE;


typedef struct _SUBSAMPLER
	{
	nsint       xdim;
	nsint       ydim;
	nsint       zdim;
	nsdouble    xscale;
	nsdouble    yscale;
	nsdouble    zscale;
	nsint       newxdim;
	nsint       newydim;
	nsint       newzdim;
	nsdouble    zwindow;
	nsdouble    zpoint;
	nsint       lastin;
	nsint       lastout;
	nsint       count;
	SSIMAGE    *firstimage;
	SSIMAGE    *lastimage;
	SSIMAGE     destimage;
	nsint       zend;
	}
	SUBSAMPLER;


void SubsamplerSizes
	(
	nsint      xdim,
	nsint      ydim,
	nsint      zdim,
	nsdouble   xscale,
	nsdouble   yscale,
	nsdouble   zscale,
	nsint     *new_x_dim,
	nsint     *new_y_dim,
	nsint     *new_z_dim
	);

SUBSAMPLER* InitSubsampler
	(
	SUBSAMPLER  *subsampler,
	nsint        xdim,
	nsint        ydim,
	nsint        zdim,
	nsdouble     xscale,
	nsdouble     yscale,
	nsdouble     zscale
	);

/* deallocates images, no return value */
void CleanSubsampler( SUBSAMPLER *subsampler );

/* returns one if complete, zero otherwise */
nsint SubsamplerIsComplete( SUBSAMPLER *subsampler );

/* returns index of next image required(zero based) or -1 if full */
nsint SubsamplerNextIndex( SUBSAMPLER *subsampler );

/* returns zero on succes, nonzero on error */
nsint SubsamplerAddSrcImage( SUBSAMPLER *subsampler, nsconstpointer data );
void SubsamplerAddDestImage( SUBSAMPLER *subsampler, nspointer data );

nsint SubsampleLum1ub( SUBSAMPLER *subsampler );
nsint SubsampleLum1us12( SUBSAMPLER *subsampler );
nsint SubsampleLum1us16( SUBSAMPLER *subsampler );

NS_DECLS_END

#endif
