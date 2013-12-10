/*****************************************************************************************\
*                                                                                         *
*  Contour Data Abstractions                                                              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "utilities.h"
#include "image_lib.h"
#include "level_set.h"
#include "contour_lib.h"

/****************************************************************************************
 *                                                                                      *
 *  CONTOUR ABSTRACTION: TRACE_CONTOUR, SPACE MANAGEMENT                                *
 *                                                                                      *
 ****************************************************************************************/

//  Awk-generated (manager.awk) Contour space management

static int     Contour_Length_Max = -1;
static int     Cwidth;
static int     Carea;
static uint8  *Value8;
static uint16 *Value16;

static inline int contour_tsize(Contour *contour)
{ return (sizeof(int) * contour->length); }


typedef struct __Contour
  { struct __Contour *next;
    size_t               tsize;
    Contour           contour;
  } _Contour;

static _Contour *Free_Contour_List = NULL;
static size_t    Contour_Offset, Contour_Inuse;

static inline void allocate_contour_tour(Contour *contour, size_t tsize, char *routine)
{ _Contour *object  = (_Contour *) (((char *) contour) - Contour_Offset);
  if (object->tsize < tsize)
    { if (object->tsize == 0)
        object->contour.tour = NULL;
#ifdef _MSC_VER
      object->contour.tour  = (decltype(object->contour.tour))Guarded_Realloc(object->contour.tour,tsize,routine);
#else
      object->contour.tour  = Guarded_Realloc(object->contour.tour,tsize,routine);
#endif
      object->tsize = tsize;
    }
}

static inline Contour *new_contour(size_t tsize, char *routine)
{ _Contour *object;

  if (Free_Contour_List == NULL)
    { object = (_Contour *) Guarded_Malloc(sizeof(_Contour),routine);
      Contour_Offset = ((char *) &(object->contour)) - ((char *) object);
      object->tsize = 0;
    }
  else
    { object = Free_Contour_List;
      Free_Contour_List = object->next;
    }
  Contour_Inuse += 1;
  allocate_contour_tour(&(object->contour),tsize,routine);
  return (&(object->contour));
}

static inline Contour *copy_contour(Contour *contour)
{ Contour *copy = new_contour(contour_tsize(contour),"Copy_Contour");
  Contour  temp = *copy;
  *copy = *contour;
  copy->tour = temp.tour;
  if (contour_tsize(contour) != 0)
    memcpy(copy->tour,contour->tour,contour_tsize(contour));
  return (copy);
}

Contour *Copy_Contour(Contour *contour)
{ return (copy_contour(contour)); }

static inline void pack_contour(Contour *contour)
{ _Contour *object  = (_Contour *) (((char *) contour) - Contour_Offset);
  if (object->tsize > contour_tsize(contour))
    { object->tsize = contour_tsize(contour);
      if (object->tsize != 0)
#ifdef _MSC_VER
        object->contour.tour = (decltype(object->contour.tour))Guarded_Realloc(object->contour.tour,
#else
        object->contour.tour = Guarded_Realloc(object->contour.tour,
#endif
                                               object->tsize,"Pack_Contour");
      else
        { free(object->contour.tour);
          object->tsize = 0;
        }
    }
}

void Pack_Contour(Contour *contour)
{ pack_contour(contour); }

static inline void free_contour(Contour *contour)
{ _Contour *object  = (_Contour *) (((char *) contour) - Contour_Offset);
  object->next = Free_Contour_List;
  Free_Contour_List = object;
  Contour_Inuse -= 1;
}

void Free_Contour(Contour *contour)
{ free_contour(contour); }

static inline void kill_contour(Contour *contour)
{ _Contour *object  = (_Contour *) (((char *) contour) - Contour_Offset);
  if (object->tsize != 0)
    free(contour->tour);
  free(((char *) contour) - Contour_Offset);
  Contour_Inuse -= 1;
}

void Kill_Contour(Contour *contour)
{ kill_contour(contour); }

static inline void reset_contour()
{ _Contour *object;
  while (Free_Contour_List != NULL)
    { object = Free_Contour_List;
      Free_Contour_List = object->next;
      kill_contour(&(object->contour));
      Contour_Inuse += 1;
    }
}

int Contour_Usage()
{ return (Contour_Inuse); }

void Reset_Contour()
{ reset_contour();
  Contour_Length_Max = -1;
}

/* Trace outer contour of 4- or 8-connected region that (a) contains 'pixel', and
   (b) all pixels <=, ==, or >= 'level'.  'iscon4' determines connectivity, and
   'cmprsn' the comparator for membership in the region.
*/

static inline int legal_move(int p, int d)
{ switch (d)
  { case 0:
      return (p+Cwidth < Carea);
    case 1:
      return (p % Cwidth + 1 < Cwidth);
    case 2:
      return (p >= Cwidth);
    case 3:
      return (p % Cwidth > 0);
  }
  return 0;
}

static inline int boundary_pixel(int p)
{ int q = p % Cwidth;
  return (p < Cwidth || p+Cwidth >= Carea || q == 0 || q+1 == Cwidth);
}

static inline int is_le(int p, int level)
{ if (Value8 != NULL)
    return (Value8[p] <= level);
  else
    return (Value16[p] <= level);
}

static inline int is_ge(int p, int level)
{ if (Value8 != NULL)
    return (Value8[p] >= level);
  else
    return (Value16[p] >= level);
}

static inline int is_eq(int p, int level)
{ if (Value8 != NULL)
    return (Value8[p] == level);
  else
    return (Value16[p] == level);
}

Contour *Trace_Region(Image *image, int seed, Comparator cmprsn, int level, int iscon4)
{ static int       offset[4];
  static int       firstime = 0;
  static char     *direction[] = { "S", "E", "N", "W" };

  Contour *my_cont;
  int  p, d, e, q, r;
  int  beg;
  int  len, bnd;
  int *tour;
  int (*value)(int,int);

  if (image->kind != GREY && image->kind != GREY16)
    { fprintf(stderr,"Image is not GREY or GREY16 (Trace_Region)\n");
      exit (1);
    }
  
  Cwidth  = image->width;
  Carea   = Cwidth * image->height;
  if (image->kind == GREY)
    { Value8  = image->array;
      Value16 = NULL;
    }
  else if (image->kind == GREY16)
    { Value8  = NULL;
      Value16 = (uint16 *) (image->array);
    }
  else
    { fprintf(stderr,"Image is not GREY or GREY16 (Trace_Region)\n");
      exit (1);
    }

  if (cmprsn == LE)
    value = is_le;
  else if (cmprsn == EQ)
    value = is_eq;
  else
    value = is_ge;

  if (firstime != Cwidth)
    { firstime  = Cwidth;
      offset[0] =  Cwidth;
      offset[1] =  1;
      offset[2] = -Cwidth;
      offset[3] = -1;
    }

  beg = seed;
  while (legal_move(beg,3) && value(beg+offset[3],level))
    beg += offset[3];

  len = 0;
  bnd = 0;
  p   = beg;
  d   = 0;
#ifdef DEBUG_CONTOUR
  printf("\nContour:\n  (%3d,%3d) -> %s (%d)\n",p%Cwidth+1,p/Cwidth+1,direction[d],Value8[p]);
#endif

  if (iscon4)   // 4-connected contour
    do
      { if (boundary_pixel(p))
          bnd = 1;
        q = p + offset[d];
        if (legal_move(p,d) && value(q,level))
          { e = (d+3) % 4;
            r = q + offset[e];
            if (legal_move(q,e) && value(r,level))
              { p = r;
                d = e;
                len += 2;
#ifdef DEBUG_CONTOUR
                printf("  (%3d,%3d) -> (%3d,%3d) %s\n",q%Cwidth+1,q/Cwidth+1,
                                                       r%Cwidth+1,r/Cwidth+1,direction[d]);
                fflush(stdout);
#endif
              }
            else
              { p = q;
                len += 1;
#ifdef DEBUG_CONTOUR
                printf("  (%3d,%3d) %s\n",q%Cwidth+1,q/Cwidth+1,direction[d]);
                fflush(stdout);
#endif
              }
          }
        else
          { d = (d+1) % 4;
#ifdef DEBUG_CONTOUR
            printf("  . %s\n",direction[d]);
            fflush(stdout);
#endif
          }
      }
    while (p != beg || d != 0);

  else                // 8-connected contour
    do
      { q = p + offset[d];
        if (legal_move(p,d))
          { e = (d+3) % 4;
            r = q + offset[e];
            if (legal_move(q,e) && value(r,level))
              { p = r;
                d = e;
                len += 1;
#ifdef DEBUG_CONTOUR
                printf("  (%3d,%3d) %s\n",r%Cwidth+1,r/Cwidth+1,direction[d]);
                fflush(stdout);
#endif
              }
            else if (value(q,level))
              { p = q;
                len += 1;
#ifdef DEBUG_CONTOUR
                printf("  (%3d,%3d) %s\n",q%Cwidth+1,q/Cwidth+1,direction[d]);
                fflush(stdout);
#endif
              }
            else
              { d = (d+1) % 4;
#ifdef DEBUG_CONTOUR
                printf("  . %s\n",direction[d]);
                fflush(stdout);
#endif
              }
          }
        else
          { d = (d+1) % 4;
#ifdef DEBUG_CONTOUR
            printf("  . %s\n",direction[d]);
            fflush(stdout);
#endif
          }
      }
    while (p != beg || d != 0);

  if (len+1 > Contour_Length_Max)
    Contour_Length_Max = (len+1)*1.2 + 100;
  my_cont = new_contour(Contour_Length_Max*sizeof(int),"Trace_Contour");

  my_cont->length   = len+1;
  my_cont->boundary = bnd;
  my_cont->width    = Cwidth;
  my_cont->iscon4   = iscon4;

  tour = my_cont->tour + 1;
  len  = 0;
  p = tour[-1] = beg;
  d = 0;

  if (iscon4)   // 4-connected contour
    do
      { q = p + offset[d];
        if (legal_move(p,d) && value(q,level))
          { e = (d+3) % 4;
            r = q + offset[e];
            tour[len++] = q;
            if (legal_move(q,e) && value(r,level))
              { p = r;
                d = e;
                tour[len++] = r;
              }
            else
              p = q;
          }
        else
          d = (d+1) % 4;
      }
    while (p != beg || d != 0);

  else                // 8-connected contour
    do
      { q = p + offset[d];
        if (legal_move(p,d))
          { e = (d+3) % 4;
            r = q + offset[e];
            if (legal_move(q,e) && value(r,level))
              { p = r;
                d = e;
                tour[len++] = r;
              }
            else if (value(q,level))
              { p = q;
                tour[len++] = q;
              }
            else
              d = (d+1) % 4;
          }
        else
          d = (d+1) % 4;
      }
    while (p != beg || d != 0);

  return (my_cont);
}

Contour *Trace_Level_Set(Level_Set *rgn)

{ Component_Tree *tree   = Get_Current_Component_Tree();
  Image          *image  = Get_Component_Tree_Image(tree);
  int             iscon4 = Get_Component_Tree_Connectivity(tree);

  return (Trace_Region(image,Level_Set_Leftmost(rgn),GE,Level_Set_Level(rgn),iscon4));
}

Contour *Trace_Watershed(Watershed_2D *w, int cb)

{ Image          *image  = w->labels;
  int             iscon4 = w->iscon4;
  int             pixel  = w->verts[cb].seed;

  return (Trace_Region(image,pixel,EQ,image->array[pixel],iscon4));
}


/****************************************************************************************
 *                                                                                      *
 *  CONTOUR ANALYSIS: EXTENT, CENTER OF MASS                                            *
 *                                                                                      *
 ****************************************************************************************/

Contour_Extent *Contour_Get_Extent(Contour *cont)
{ static Contour_Extent my_ext;

  int  len, wide;
  int *tour;
  int  maxx, minx;
  int  maxy, miny;
  
  wide = cont->width;
  tour = cont->tour;
  len  = cont->length-1;

  { int i, p;

    p = tour[0];
    maxx = minx = p % wide;
    maxy = miny = p;

    for (i = 1; i < len; i++)
      { p = tour[i];
        if (p > maxy)
          maxy = p;
        if (p < miny)
          miny = p;
        p = p%wide;
        if (p > maxx)
          maxx = p;
        if (p < minx)
          minx = p;
      }
  }

  my_ext.max_x = maxx;
  my_ext.min_x = minx;
  my_ext.max_y = maxy / wide;
  my_ext.min_y = miny / wide;
  return (&my_ext);
}

/* A vertical pixel boundary is the right or left edge of a conceptual square
   defining the pixel in an imaginary pixel grid.  In a raster scan one would
   fill the pixels in a row that are between a pair of pixel boundaries of two
   pixels in the row.  Given a contour, Raster_Scan, returns a list of pairs
   of pixel boundaries that when filled, fill the 4-connected region that originally
   defined the trace.  The pixel pairs are in sorted (x,y) lexicographical order.   */

static int RSORT(const void *l, const void *r)
{ int x, y;
  x = *((int *) l);
  y = *((int *) r);
  return (x-y);
}

int *Raster_Scan(Contour *trace, int *pren)
{ static int   offset[4];
  static int   firstime = 0;
  static int   max_raster = 0;
  static int  *raster = NULL;

  int  len, wide, ren;
  int *tour;
  
  wide = trace->width;
  if (firstime != wide)
    { firstime = wide;
      offset[0] =  wide;
      offset[1] =  1;
      offset[2] = -wide;
      offset[3] = -1;
    }

  tour = trace->tour + 1;
  len  = trace->length-1;

  if (len+4 > max_raster)
    { max_raster = 1.2*len + 1000;
      raster     = (int *) Guarded_Realloc(raster,sizeof(int)*max_raster,
                                           "Contour Analysis (several)");
    }

  { int  p, d, i;

    p = tour[-1];
    d = 0;
    i = 0;
    ren = 0;
    if (trace->iscon4)
      do
        { int q, r, e;
  
          q = p + offset[d];
          if (i < len && q == tour[i])
            { e = (d+3) % 4;
              r = q + offset[e];
              i += 1;
              if (i < len && r == tour[i])
                { p = r;
                  d = e;
                  i += 1;
                }
              else
                p = q;
            }
          else
            d = (d+1) % 4;
          if (d == 0)
            raster[ren++] = p;
          else if (d == 2)
            raster[ren++] = p + 1;
        }
      while (i < len || d != 0);

    else
      do
        { int q, r, e;

          q = p + offset[d];
          if (i < len)
            { e = (d+3) % 4;
              r = q + offset[e];
              if (r == tour[i])
                { p = r;
                  d = e;
                  i += 1;
                }
              else if (q == tour[i])
                { p = q;
                  i += 1;
                }
              else
                d = (d+1) % 4;
            }
          else
            d = (d+1) % 4;
          if (d == 0)
            raster[ren++] = p;
          else if (d == 2)
            raster[ren++] = p + 1;
        }
      while (i < len || d != 0);
  }

  if (ren % 2 == 1)
    { fprintf(stderr,"Gene's Contours: Raster has odd length!\n");
      exit (1);
    }

  qsort(raster,ren,sizeof(int),RSORT);

  *pren = ren;
  return (raster);
}

int *Yaster_Scan(Contour *trace, int *pren, int height)
{ static int   offset[4];
  static int   firstime = 0;
  static int   max_raster = 0;
  static int  *raster = NULL;

  int  len, wide, ren;
  int *tour;
  
  wide = trace->width;
  if (firstime != wide)
    { firstime = wide;
      offset[0] =  wide;
      offset[1] =  1;
      offset[2] = -wide;
      offset[3] = -1;
    }

  tour = trace->tour + 1;
  len  = trace->length-1;

  if (len+4 > max_raster)
    { max_raster = 1.2*len + 1000;
      raster     = (int *) Guarded_Realloc(raster,sizeof(int)*max_raster,
                                           "Contour Analysis (several)");
    }

  { int  p, d, i;

    p = tour[-1];
    d = 0;
    i = 0;
    ren = 0;
    if (trace->iscon4)
      do
        { int q, r, e;
  
          q = p + offset[d];
          if (i < len && q == tour[i])
            { e = (d+3) % 4;
              r = q + offset[e];
              i += 1;
              if (i < len && r == tour[i])
                { p = r;
                  d = e;
                  i += 1;
                }
              else
                p = q;
            }
          else
            d = (d+1) % 4;
          if (d == 3)
            raster[ren++] = (p%wide)*height + p/wide;
          else if (d == 1)
            raster[ren++] = (p%wide)*height + p/wide + 1;
        }
      while (i < len || d != 0);

    else
      do
        { int q, r, e;

          q = p + offset[d];
          if (i < len)
            { e = (d+3) % 4;
              r = q + offset[e];
              if (r == tour[i])
                { p = r;
                  d = e;
                  i += 1;
                }
              else if (q == tour[i])
                { p = q;
                  i += 1;
                }
              else
                d = (d+1) % 4;
            }
          else
            d = (d+1) % 4;
          if (d == 3)
            raster[ren++] = (p%wide)*height + p/wide;
          else if (d == 1)
            raster[ren++] = (p%wide)*height + p/wide + 1;
        }
      while (i < len || d != 0);
  }

  if (ren % 2 == 1)
    { fprintf(stderr,"Gene's Contours: Raster has odd length!\n");
      exit (1);
    }

  qsort(raster,ren,sizeof(int),RSORT);

  *pren = ren;
  return (raster);
}

/* Return the area covered by the outer contour */

int Contour_Area(Contour *cont)
{ int *raster, ren;
  int  i, area;

  raster = Raster_Scan(cont,&ren);

  area = 0;
  for (i = 0; i < ren; i += 2)
    area += raster[i+1] - raster[i];
  return (area);
}

/* Assuming all pixels have equal weight, return the pixel coordinate
   that is nearest the center of mass of the region defined by "contour" */

int Contour_Center_Of_Mass(Contour *cont)
{ int      *raster, ren, wide;

  raster = Raster_Scan(cont,&ren);
  wide   = cont->width;

  { int    i, v, w, n;
    double x, y;

    x = y = 0.;
    n = 0;
    for (i = 0; i < ren; i += 2)
      { v = raster[i];
        w = raster[i+1];
        x += (v%wide + ((w-v)-1)/2.) * (w-v);
        y += (v/wide) * (w-v);
        n += (w-v);
      }
    v = (x / n) + .5;
    w = (y / n) + .5;

    return (v + w*wide);
  }
}

/* Assuming all pixels have equal weight, return the pixel coordinate
   that is nearest the center of mass of the pixels in "contour" that
   are c or brigher.                                                     */

int Contour_Bright_Center_Of_Mass(Contour *cont, int c)
{ int      *raster, ren, wide;

  raster = Raster_Scan(cont,&ren);
  wide   = cont->width;

  { int    i, v, w, n, p;
    double x, y, z;

    x = y = 0.;
    n = 0;
    for (i = 0; i < ren; i += 2)
      { v = raster[i];
        w = raster[i+1];
        z = v/wide;
        for (p = v; p < w; p++)
          if (is_ge(p,c))
            { x += p%wide;
              y += z;
              n += 1;
            }
      }
    v = (x / n) + .5;
    w = (y / n) + .5;

    return (v + w*wide);
  }
}
