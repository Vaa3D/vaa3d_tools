/*****************************************************************************************\
*                                                                                         *
*  Drawing Level Sets, Regions, and Basic Shapes                                          *
*                                                                                         *
*  Author  :  Gene Myers                                                                  *
*  Date    :  June 2007                                                                   *
*  Last Mod:  Aug 2008                                                                    *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _DRAW_LIB

#define _DRAW_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "mylib.h"
#include "array.h"
#include "region.h"
#include "level.set.h"
#include "water.shed.h"

typedef enum
  { SET_PIX = 0,
    ADD_PIX = 1,
    SUB_PIX = 2,
    MUL_PIX = 3,
    DIV_PIX = 4,
    MIN_PIX = 5,
    MAX_PIX = 6
  } Drawer;

typedef struct
  { Drawer op;
    Value  val;
  } Plain_Bundle;

typedef struct
  { Drawer op;
    Value  real;
    Value  imag;
  } Complex_Bundle;

typedef struct
  { Drawer op;
    Value  red;
    Value  green;
    Value  blue;
    Value  alpha;
  } Color_Bundle;

typedef void Brush_Bundle;

void Draw_Region         (Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Outline (Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Exterior(Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Holes   (Array *M(canvas), Brush_Bundle *brush, Region *trace);

void Draw_Level_Set         (Array *M(canvas), Brush_Bundle *brush,
                             Level_Tree *t, Level_Set *r, int share);
void Draw_Level_Set_Outline (Array *M(canvas), Brush_Bundle *brush,
                             Level_Tree *t, Level_Set *r, int share);
void Draw_Level_Set_Exterior(Array *M(canvas), Brush_Bundle *brush,
                             Level_Tree *t, Level_Set *r, int share);
void Draw_Level_Set_Holes   (Array *M(canvas), Brush_Bundle *brush,
                             Level_Tree *t, Level_Set *r, int share);

void Draw_P_Vertex         (Array *M(canvas), Brush_Bundle *brush, Partition *p, int v, int share);
void Draw_P_Vertex_Outline (Array *M(canvas), Brush_Bundle *brush, Partition *p, int v, int share);
void Draw_P_Vertex_Exterior(Array *M(canvas), Brush_Bundle *brush, Partition *p, int v, int share);
void Draw_P_Vertex_Holes   (Array *M(canvas), Brush_Bundle *brush, Partition *p, int v, int share);

void Draw_Region_Image   (Array *M(canvas), Array *image, Region *trace);
void Draw_Level_Set_Image(Array *M(canvas), Array *image, Level_Tree *t, Level_Set *r, int share);
void Draw_P_Vertex_Image (Array *M(canvas), Array *image, Partition *p, int b, int share);

void Draw_Floodfill(Array *M(canvas), Brush_Bundle *brush,
                    APart *source, int share, boolean iscon2n,
                    Indx_Type seed, void *arg, boolean (*text)(Indx_Type p, void *arg));
void Draw_Basic(Array *M(canvas), Brush_Bundle *brush, APart *source, int share,
                boolean iscon2n, Indx_Type seed, Comparator cmprsn, Value level);

void Draw_Point(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(point));
void Draw_Cross(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius);
void Draw_Rectangle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(corner1),
                                                           Coordinate *F(corner2));
void Draw_Circle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius);
void Draw_Line(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(beg), Coordinate *F(end));

void Draw_Image(Array *M(image), Brush_Bundle *brush);

#ifdef __cplusplus
}
#endif

#endif
