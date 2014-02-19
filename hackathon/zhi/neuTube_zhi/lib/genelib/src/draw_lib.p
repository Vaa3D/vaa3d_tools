/*****************************************************************************************\
*                                                                                         *
*  Drawing Level Sets, Contours, and Basic Shapes                                         *
*                                                                                         *
*  Author  :  Gene Myers                                                                  *
*  Date    :  June 2007                                                                   *
*  Last Mod:  Aug 2008                                                                    *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "utilities.h"
#include "draw_lib.h"

//  A Paint_Brush determines the setting of the following globals that are then
//    used by the various painter routines below to set pixels.

static uint8   *bytes;
static uint16  *shorts;
static float32 *reals;

static int     red;
static int     green;
static int     blue;

static float32 rred;
static float32 rgreen;
static float32 rblue;

  //  The GREY paint routines

static void PAINT_GREY_SET(int p)
{ bytes[p] = red; }

static void PAINT_GREY_ADD(int p)
{ int x = bytes[p] + red; 
  if (x > 0xFF)
    bytes[p] = 0xFF;
  else
    bytes[p] = x;
}

static void PAINT_GREY_SUB(int p)
{ int x = bytes[p] - red; 
  if (x < 0)
    bytes[p] = 0;
  else
    bytes[p] = x;
}

static void PAINT_GREY_MUL(int p)
{ bytes[p] = bytes[p] * rred; }

static void PAINT_GREY_MAX(int p)
{ int x = bytes[p]; 
  if (x < red)
    bytes[p] = red;
}

static void PAINT_GREY_MIN(int p)
{ int x = bytes[p]; 
  if (x > red)
    bytes[p] = red;
}

  //  The GREY16 paint routines

static void PAINT_GREY16_SET(int p)
{ shorts[p] = red; }

static void PAINT_GREY16_ADD(int p)
{ int x = shorts[p] + red; 
  if (x > 0xFFFF)
    shorts[p] = 0xFFFF;
  else
    shorts[p] = x;
}

static void PAINT_GREY16_SUB(int p)
{ int x = shorts[p] - red; 
  if (x < 0)
    shorts[p] = 0;
  else
    shorts[p] = x;
}

static void PAINT_GREY16_MUL(int p)
{ shorts[p] = shorts[p] * rred; }

static void PAINT_GREY16_MAX(int p)
{ int x = shorts[p]; 
  if (x < red)
    shorts[p] = red;
}

static void PAINT_GREY16_MIN(int p)
{ int x = shorts[p]; 
  if (x > red)
    shorts[p] = red;
}

  //  The COLOR paint routines

static void PAINT_COLOR_SET(int p)
{ p *= 3;
  if (red >= 0)
    bytes[p]   = red;
  if (green >= 0)
    bytes[p+1] = green;
  if (blue >= 0)
    bytes[p+2] = blue;
}

static void PAINT_COLOR_ADD(int p)
{ int x;

  p *= 3;
  if (red >= 0)
    { x = bytes[p] + red; 
      if (x > 0xFF)
        bytes[p] = 0xFF;
      else
        bytes[p] = x;
    }

  if (green >= 0)
    { x = bytes[p+1] + green; 
      if (x > 0xFF)
        bytes[p+1] = 0xFF;
      else
        bytes[p+1] = x;
    }

  if (blue >= 0)
    { x = bytes[p+2] + blue; 
      if (x > 0xFF)
        bytes[p+2] = 0xFF;
      else
        bytes[p+2] = x;
    }
}

static void PAINT_COLOR_SUB(int p)
{ int x;

  p *= 3;
  if (red >= 0)
    { x = bytes[p] - red; 
      if (x < 0)
        bytes[p] = 0;
      else
        bytes[p] = x;
    }

  if (green >= 0)
    { x = bytes[p+1] - green; 
      if (x < 0)
        bytes[p+1] = 0;
      else
        bytes[p+1] = x;
    }

  if (blue >= 0)
    { x = bytes[p+2] - blue; 
      if (x < 0)
        bytes[p+2] = 0;
      else
        bytes[p+2] = x;
    }
}

static void PAINT_COLOR_MUL(int p)
{ p *= 3;
  if (rred >= 0)
    bytes[p]   = bytes[p] * rred;
  if (rgreen >= 0)
    bytes[p+1] = bytes[p+1] * rgreen;
  if (rblue >= 0)
    bytes[p+2] = bytes[p+2] * rblue;
}

static void PAINT_COLOR_MAX(int p)
{ int x;

  p *= 3;
  if (red >= 0) 
    { x = bytes[p]; 
      if (x < red)
        bytes[p] = red;
    }

  if (green >= 0) 
   {  x = bytes[p+1]; 
      if (x < green)
        bytes[p+1] = green;
    }

  if (blue >= 0) 
    { x = bytes[p+2]; 
      if (x < blue)
        bytes[p+2] = blue;
    }
}

static void PAINT_COLOR_MIN(int p)
{ int x;

  p *= 3;
  if (red >= 0) 
    { x = bytes[p]; 
      if (x > red)
        bytes[p] = red;
    }

  if (green >= 0) 
   {  x = bytes[p+1]; 
      if (x > green)
        bytes[p+1] = green;
    }

  if (blue >= 0) 
    { x = bytes[p+2]; 
      if (x > blue)
        bytes[p+2] = blue;
    }
}

  //  The FLOAT32 paint routines

static void PAINT_FLOAT32_SET(int p)
{ reals[p] = rred; }

static void PAINT_FLOAT32_ADD(int p)
{ reals[p] += rred; }

static void PAINT_FLOAT32_SUB(int p)
{ reals[p] -= rred; }

static void PAINT_FLOAT32_MUL(int p)
{ reals[p] *= rred; }

static void PAINT_FLOAT32_MAX(int p)
{ float32 x = reals[p]; 
  if (x < rred)
    reals[p] = rred;
}

static void PAINT_FLOAT32_MIN(int p)
{ float32 x = reals[p]; 
  if (x > rred)
    reals[p] = rred;
}

static void (*Painter_Table[])(int) =
  { NULL,
    PAINT_GREY_SET, PAINT_GREY16_SET, PAINT_COLOR_SET, PAINT_FLOAT32_SET,
    PAINT_GREY_ADD, PAINT_GREY16_ADD, PAINT_COLOR_ADD, PAINT_FLOAT32_ADD,
    PAINT_GREY_SUB, PAINT_GREY16_SUB, PAINT_COLOR_SUB, PAINT_FLOAT32_SUB,
    PAINT_GREY_MUL, PAINT_GREY16_MUL, PAINT_COLOR_MUL, PAINT_FLOAT32_MUL,
    PAINT_GREY_MAX, PAINT_GREY16_MAX, PAINT_COLOR_MAX, PAINT_FLOAT32_MAX,
    PAINT_GREY_MIN, PAINT_GREY16_MIN, PAINT_COLOR_MIN, PAINT_FLOAT32_MIN
  };

static void *SETUP_PAINTER(Image *canvas, Paint_Brush *brush)
{ rred   = brush->red;
  rgreen = brush->green;
  rblue  = brush->blue;

  switch (canvas->kind)
  { case GREY:
      red   = rred * 0xFF;
      bytes = canvas->array;
      break;
    case GREY16:
      red    = rred * 0xFFFF;
      shorts = (uint16 *) canvas->array;
      break;
    case COLOR:
      red   = rred * 0xFF;
      green = rgreen * 0xFF;
      blue  = rblue * 0xFF;
      bytes = canvas->array;
      break;
    case FLOAT32:
      reals  = (float32 *) canvas->array;
      break;
  }

  return (Painter_Table[(brush->op << 2) + canvas->kind]);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR CONTOURS                                                       *
 *                                                                                      *
 ****************************************************************************************/

/* Color the pixels of a trace */

void Draw_Contour_Outline(Contour *cont, Paint_Brush *brush, Image *canvas)
{ int  i, p, len;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  len = cont->length;
  for (i = 1; i < len; i++)
    painter(cont->tour[i]);
}

/* Color the region defined by trace "contour" */

void Draw_Contour_Interior(Contour *cont, Paint_Brush *brush, Image *canvas)
{ int  *raster, ren;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  raster = Raster_Scan(cont,&ren);

  { int i, v, w, p;

    for (i = 0; i < ren; i += 2)
      { v = raster[i];
        w = raster[i+1];
        for (p = v; p < w; p++)
          painter(p);
      }
  }
}

/* Color the complement of the region defined by trace "contour" */

void Draw_Contour_Exterior(Contour *cont, Paint_Brush *brush, Image *canvas)
{ int *raster, ren;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  raster = Raster_Scan(cont,&ren);

  { int i, v, w, p;

    for (i = -1; i < ren; i += 2)
      { if (i < 0)
          v = 0;
        else
          v = raster[i];
        if (i+1 >= ren)
          w = canvas->width*canvas->height;
        else 
          w = raster[i+1];
        for (p = v; p < w; p++)
          painter(p);
      }
  }
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR LEVEL SETS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_Level_Set_Outline(Level_Set *r, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Level_Set(r);
  Draw_Contour_Outline(c,brush,canvas);
  Free_Contour(c);
}

void Draw_Level_Set_Interior(Level_Set *r, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Level_Set(r);
  Draw_Contour_Interior(c,brush,canvas);
  Free_Contour(c);
}

void Draw_Level_Set_Exterior(Level_Set *r, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Level_Set(r);
  Draw_Contour_Exterior(c,brush,canvas);
  Free_Contour(c);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR WATERHSEDS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_Watershed_Outline(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Watershed(w,cb);
  Draw_Contour_Outline(c,brush,canvas);
  Free_Contour(c);
}

void Draw_Watershed_Interior(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Watershed(w,cb);
  Draw_Contour_Interior(c,brush,canvas);
  Free_Contour(c);
}

void Draw_Watershed_Exterior(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas)
{ Contour *c;
  c = Trace_Watershed(w,cb);
  Draw_Contour_Exterior(c,brush,canvas);
  Free_Contour(c);
}

/****************************************************************************************
 *                                                                                      *
 *  GENERAL FLOOD-FILL DRAWING ROUTINE                                                  *
 *                                                                                      *
 ****************************************************************************************/

static uint8  *Value8;   //  Globals for is_(le|eq|ge) routines
static uint16 *Value16;
static int     Level;

static int is_le(int p)
{ if (Value8 != NULL)
    return (Value8[p] <= Level);
  else
    return (Value16[p] <= Level);
}

static int is_ge(int p)
{ if (Value8 != NULL)
    return (Value8[p] >= Level);
  else
    return (Value16[p] >= Level);
}

static int is_eq(int p)
{ if (Value8 != NULL)
    return (Value8[p] == Level);
  else
    return (Value16[p] == Level);
}

void Draw_Region(Paint_Brush *brush, Image *canvas, Image *source,
                 int seed, Comparator cmprsn, int level, int iscon4)
{ int  (*value)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  if (canvas->width != source->width && canvas->height != source->height)
    { fprintf(stderr,"Canvas and source do not have the same dimensions! (Draw_Region)\n");
      exit (1);
    }

  if (cmprsn == LE)        //  Select value routine and set globals for it.
    value = is_le;
  else if (cmprsn == EQ)
    value = is_eq;
  else
    value = is_ge;

  if (source->kind == GREY)
    { Value8  = source->array;
      Value16 = NULL;
    }
  else if (source->kind == GREY16)
    { Value8  = NULL;
      Value16 = (uint16 *) (source->array);
    }
  else
    { fprintf(stderr,"Image is not GREY or GREY16 (Trace_Region)\n");
      exit (1);
    }

  Level = level;

  fill_region(canvas,iscon4,seed,value,(void (*)(int))SETUP_PAINTER(canvas,brush));
}

void Draw_Watershed(Watershed_2D *w, int cb, Paint_Brush *brush, Image *canvas)
{ Image *source = w->labels;
  int    seed   = w->verts[cb].seed;

  if (canvas->kind != COLOR && brush->red < 0) return;

  if (canvas->width != source->width && canvas->height != source->height)
    { fprintf(stderr,"Canvas and source do not have the same dimensions! (Draw_Region)\n");
      exit (1);
    }

  Value8  = source->array;
  Value16 = NULL;
  Level   = Value8[seed];

  fill_region(canvas,w->iscon4,seed,is_eq,(void (*)(int))SETUP_PAINTER(canvas,brush));
}

void Draw_Level_Set(Level_Set *r, Paint_Brush *brush, Image *canvas)
{ Component_Tree *tree   = Get_Current_Component_Tree();
  Image          *image  = Get_Component_Tree_Image(tree);
  int             iscon4 = Get_Component_Tree_Connectivity(tree);

  Draw_Region(brush,canvas,image,Level_Set_Leftmost(r),GE,Level_Set_Level(r),iscon4);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR BASIC SHAPES                                                   *
 *                                                                                      *
 ****************************************************************************************/

/* Draw a w x h rectangle with lower left corner (x,y)  */

void Draw_Rectangle(int x, int y, int w, int h, Paint_Brush *brush, Image *canvas)
{ int   i, q, p, t;
  int   wide, high, cap;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  wide = canvas->width;
  high = canvas->height;

  if (x < 0)
    { w += x;
      x  = 0;
    }
  if (x + w > wide)
    w = wide-x;
  if (y < 0)
    { h += y;
      y  = 0;
    }
  if (y + h > high)
    h = high - y;

  cap = (y+h)*wide;
  for (p = y * wide + x, t = p+w; p < cap; p += wide, t += wide)
    for (q = p; q < t; q++)
      painter(q);
}

/* Reset an entire image */

void Draw_Image(Paint_Brush *brush, Image *canvas)
{ Draw_Rectangle(0,0,canvas->width,canvas->height,brush,canvas); }

/* Draw a point centered a pixel (x,w) */

void Draw_Point(int x, int y, Paint_Brush *brush, Image *canvas)
{ void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  painter(y * canvas->width + x);
}

/* Draw a cross centered a pixel (x,y) with each arm being a pixels long */

void Draw_Cross(int x, int y, int a, Paint_Brush *brush, Image *canvas)
{ int i, q, p, cap, wide;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  wide  = canvas->width;
  cap   = wide*canvas->height;

  p = y * wide + x;
  q = p - a * wide;
  for (i = -a; i <= a; i++)
    { if (q < 0 || q >= cap) continue;
      painter(q);
      q += wide;
    }
  x = x - a;
  q = p - a;
  for (i = -a; i <= a; i++)
    { if (x < 0 || x >= wide) continue;
      painter(q);
      q += 1;
      x += 1;
    }
}

/* Draw a circle centered at pixel (x,y) with radius r */

void Draw_Circle(int x, int y, int r, Paint_Brush *brush, Image *canvas)
{ int   i, a, q, p, s, r2, cap, wide;
  void (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  wide  = canvas->width;
  cap   = wide*canvas->height;
  p     = y * wide + x;
  a     = r;
  r2    = r*r;
  for (i = 0; i <= r; i++)
    { s = r2 - i*i;
      while (a*a > s)
        a -= 1;

      q = p + i*wide;
      if (q < cap)
        { s = q+a;
          if (x-a < 0)
            q += a-x;
          if (x+a >= wide)
            s -= (x+a+1)-wide;
          for (q -= a; q <= s; q++)
            painter(q);
        }

      if (i == 0) continue;

      q = p - i*wide;
      if (q >= 0)
        { s = q+a;
          if (x-a < 0)
            q += a-x;
          if (x+a >= wide)
            s -= (x+a+1)-wide;
          for (q -= a; q <= s; q++)
            painter(q);
        }
    }
}

/* Midpoint scan-conversion routine drawing a line from pixel (x0,y0) to (x1,y1).  */

void Draw_Line(int x0, int y0, int x1, int y1, Paint_Brush *brush, Image *canvas)
{ int     pix;
  int     i, d, wid;
  int     dsame, dchange;
  int     dx, dy;
  int     xinc, yinc;
  void    (*painter)(int);

  if (canvas->kind != COLOR && brush->red < 0) return;

  painter = (void (*)(int))SETUP_PAINTER(canvas,brush);

  dx = x1 - x0;
  dy = y1 - y0;
  if (dx < 0) xinc = -1; else xinc = 1;
  if (dy < 0) yinc = -1; else yinc = 1;

  wid  = canvas->width;
  pix  = wid*y0 + x0;
  wid *= yinc;

/* Case where |m| < 1 */

  if (dx*dx >= dy*dy)
    { dsame = 2*dy;
      dchange = dsame - 2*xinc*yinc*dx;
      d =  dsame*xinc - dx*yinc;
      if (dx < 0) d = -d;

#define SCAN_BODY(ifsame,increment)		\
    painter(pix);				\
    if (ifsame)					\
      d += dsame;				\
    else					\
      { d += dchange;				\
        increment;				\
      }

      if (dy >= 0)
        { for (i = x0; i != x1; i += xinc)
            { SCAN_BODY(d < 0,pix += wid)
              pix += 1;
            }
          painter(pix);
        }
      else
        { for (i = x0; i != x1; i += xinc)
            { SCAN_BODY(d >= 0,pix += wid)
              pix += 1;
            }
          painter(pix);
        }
    }

  else
    { dsame = -2*dx;
      dchange = dsame + 2*xinc*yinc*dy;
      d = dsame*yinc + dy*xinc;
      if (dy < 0) d = -d;

      if (dx >= 0)
        { for (i = y0; i != y1; i += yinc)
            { SCAN_BODY(d > 0,pix += 1)
              pix += wid;
            }
          painter(pix);
        }
      else
        { for (i = y0; i != y1; i += yinc)
            { SCAN_BODY(d <= 0,pix += 1)
              pix += wid;
            }
          painter(pix);
        }
    }
}
