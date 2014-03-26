/*****************************************************************************************\
*                                                                                         *
*  Contour Data Abstractions                                                              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
\*****************************************************************************************/

#ifndef _CONTOUR_LIB

#define _CONTOUR_LIB

#include <stdio.h>
#include "image_lib.h"

/* 2D (Outer) Contour abstraction    */

typedef struct
  { int  length;          // Length of the contour 
    int  boundary;        // Does contour touch the image boundary?
    int  width;           // Width of image from which contour was derived
    int  iscon4;          // Is this of a 4- or 8-connected region?
    int *tour;            // Pixel coords defining a 4- or 8-connected tour of the contour
  } Contour;

//  Produce a contour of the 4- or 8-connected region (determined by iscon4) of pixels
//    that include pixel and have value >=, =, or <= level (determined by cmprsn).

typedef enum { LE, EQ, GE } Comparator;

Contour *Trace_Region(Image *image, int pixel, Comparator cmprsn, int level, int iscon4);

//  Produce outer-contours of a level set or a watershed partition

#include "level_set.h"
#include "water_shed.h"

Contour *Trace_Level_Set(Level_Set *r);
Contour *Trace_Watershed(Watershed_2D *w, int cb);

//  Storage management as per convention

Contour *Copy_Contour(Contour *trace);
void     Pack_Contour(Contour *trace);
void     Free_Contour(Contour *trace);
void     Kill_Contour(Contour *trace);
void     Reset_Contour();
int      Contour_Usage();

//  Extent of a contour in pixel coordinates: a return bundle and not an object
//    (i.e. a pointer to a static copy within the producing routine that is
//          *reused* on every invocation).

typedef struct
  { int   min_x;    
    int   max_x;
    int   min_y;
    int   max_y;
  } Contour_Extent;

//  Return a "raster_scan" encoding of a 2D contour.  An array x[0..(*rpairs)-1]
//    such that Union_{i=0,2,4,...} [x[i],x[i+1]) is the interior of the
//    contour.  Provides a fast way to fill or scan every pixel in a contour.
//    Yaster does the same thing except along y-scan lines and pixels are encodes
//    as (x)*height+(y) as opposed to (y)*width+(x).

int            *Raster_Scan(Contour *trace, int *rpairs);
int            *Yaster_Scan(Contour *trace, int *rpairs, int height);

//  Obvious

int             Contour_Area(Contour *trace);
int             Contour_Center_Of_Mass(Contour *trace);
int             Contour_Bright_Center_Of_Mass(Contour *trace, int level);
Contour_Extent *Contour_Get_Extent(Contour *trace);

/* Surfaces of level sets abstraction */

typedef struct
  { int x;      // TBD
  } Surface;

//  TBD

#endif
