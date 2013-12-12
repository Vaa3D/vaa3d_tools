/*****************************************************************************************\
*                                                                                         *
*  Drawing Level Sets, Contours, and Basic Shapes                                         *
*                                                                                         *
*  Author  :  Gene Myers                                                                  *
*  Date    :  June 2007                                                                   *
*  Last Mod:  Aug 2008                                                                    *
*                                                                                         *
\*****************************************************************************************/

#ifndef _DRAW_LIB

#define _DRAW_LIB

#include <stdio.h>
#include "image_lib.h"
#include "water_shed.h"

/* A Paint_Brush specifies how to modify each pixel while performing any of the draw operations
   below.  Its consists of [0,1] values for each primary color and a specification of how to
   use these values coded in "op".  For non-COLOR images, the red value is taken as the one to
   draw with.  For COLOR images, a negative color value signals that that channel should not be
   painted.  The "op" directive controls painting as follows:

   SET: each pixel value is set to the draw value.
   ADD: each pixel value has the draw value added to it,
           and the pixel is clipped if it over-flows.
   SUB: each pixel value has the draw value subtracted from it,
           and the pixel is clipped if it under-flows.
   MUL: each pixel is multiplied by the draw value.
   MAX: each pixel is set to the maximum of its current value and the draw value.
   MIN: each pixel is set to the minimum of its current value and the draw value.

   In the case of GREY, GREY16, and COLOR images the draw value is scaled to the range of the
   image, i.e.  [0,255] and [0,65535], save for the MUL directive where it remains a real value
   between 0 and 1
*/

typedef enum { SET = 0, ADD = 1, SUB = 2, MUL = 3, MAX = 4, MIN = 5 } Pixel_Op;

typedef struct
  { Pixel_Op op;
    float    red;
    float    green;
    float    blue;
  } Paint_Brush;

#include "contour_lib.h"
#include "level_set.h"
#include "water_shed.h"

/* Drawing routines for contours.  */

void Draw_Contour_Outline(Contour *trace, Paint_Brush *brush, Image *canvas);
void Draw_Contour_Interior(Contour *trace, Paint_Brush *brush, Image *canvas);
void Draw_Contour_Exterior(Contour *trace, Paint_Brush *brush, Image *canvas);

/* The routines below are wrappers that compute the outer contour of the level
   set or watershed and then paint the desired region.  Note carefully that if
   a level set or watershed region has holes then these routines do not draw
   the region precisely but fill all the holes.                                */

void Draw_Level_Set_Outline(Level_Set *r, Paint_Brush *brush, Image *canvas);
void Draw_Level_Set_Interior(Level_Set *r, Paint_Brush *brush, Image *canvas);
void Draw_Level_Set_Exterior(Level_Set *r, Paint_Brush *brush, Image *canvas);

void Draw_Watershed_Outline(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas);
void Draw_Watershed_Interior(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas);
void Draw_Watershed_Exterior(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas);

/* Draw_Region flood fills the 4- or 8-connected region (determined by iscon4) of the
   image "source" of pixels that include pixel seed and have value >=, =, or <= level
   (determined by cmprsn).  The drawing is performed on image canvas which must have
   the same dimensions as source.                                                     */

void Draw_Region(Paint_Brush *brush, Image *canvas, Image *source,
                 int seed, Comparator cmprsn, int level, int iscon4);

/* The routines accuractely draw catchment basins and level sets by using
   the flood-fill routine, Draw_Region, above.                               */

void Draw_Watershed(Watershed_2D *shed, int cp, Paint_Brush *brush, Image *canvas);
void Draw_Level_Set(Level_Set *r, Paint_Brush *brush, Image *canvas);

/* Draw simple objects */

void Draw_Point(int x, int y, Paint_Brush *brush, Image *canvas);
void Draw_Cross(int x, int y, int length, Paint_Brush *brush, Image *canvas);
void Draw_Rectangle(int x, int y, int w, int h, Paint_Brush *brush, Image *canvas);
void Draw_Circle(int x, int y, int radius, Paint_Brush *brush, Image *canvas);
void Draw_Line(int x0, int y0, int x1, int y1, Paint_Brush *brush, Image *canvas);

/* Draw an entire canvas */

void Draw_Image(Paint_Brush *brush, Image *canvas);

#endif
