/********************************************************************
*   mc.h    :   Marching Cubes poligon generation Routines.
*               This version avoids the saddle cases holes by:
*               1) It doesn't consider A==!(A)
*               2) It assumes that all positive points inside
*                  one voxel are part of the same volume. In
*                  other words volumes separated by less than
*                  one voxel are connected. This includes the
*                  pure diagonal or 0001-0100.
*********************************************************************/

#ifndef MC_H
#define MC_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmemory.h>

typedef struct
{
    float   x;
    float   y;
    float   z;
} MCPOINT;

typedef struct
{
    MCPOINT p1;
    MCPOINT p2;
    MCPOINT p3;
} MCPOLY;

typedef struct
{
    int     precount;
    int     postcount;
    float   efficiency;
    MCPOLY* polys;
} MCSTRIP;


/*********************************************************
*   MCGetStrip():
*   This is the main routine. It returns a list of the
*   polygons found between the two layers of data.
*   It is up the caller to ns_free the returned structure
*   by calling MCFreeStrip() once done with it.
*
*       
*               e-----f    e,f,g,h are points in data2
*              /|    /| 
*             a-----b |  a,b,c,d are points in data1
*             | h---|-g
*             |/    |/
*             d-----c  
*
**********************************************************/
MCSTRIP* MCGetStrip(unsigned char* data1, unsigned char* data2,
                    int nx, int ny, int threshold,
                    float x0,float y0,float z0,
                    float xstep, float ystep, float zstep);


void MCFreeStrip(MCSTRIP* strip);


#endif

