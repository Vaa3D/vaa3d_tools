/*****************************************************************************************\
*                                                                                         *
*  Image Filters                                                                          *
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
#include "image_filters.h"


/*********** 2D AND 3D FILTERS AND CONVOLUTION *********************/

#undef DEBUG_FILTER

static void print_filter(float32 *a, int w, int h)
{ int x, y;

  printf("\nFilter:\n");
  for (y = 0; y < w; y++)
    for (x = 0; x < w; x++)
      printf("  (%2d,%2d): %g\n",x,y,a[w*y+x]);
}

Image *Gaussian_2D_Filter(double sigma)
{ int      r = 3*sigma;
  int      w = 2*r + 1;
  Image   *filter = Make_Image(FLOAT32,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y;
    double sum;

    sigma = 2 * sigma * sigma;
    sum   = 0;
    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        { double v    = exp(-((x-r)*(x-r) + (y-r)*(y-r)) / sigma);
          a[w*y + x]  = v;
          sum        += v;
        }
    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        a[w*y + x] /= sum;
  }

#ifdef DEBUG_FILTER
  print_filter(a,w,w);
#endif

  return (filter);
}

Image *Mexican_Hat_2D_Filter(double sigma)
{ int      r = 3*sigma;
  int      w = 2*r + 1;
  Image   *filter = Make_Image(FLOAT32,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y;
    double sum;

    sigma = 2 * sigma * sigma;
    sum   = 0;
    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        { double r2   = (x-r)*(x-r) + (y-r)*(y-r);
          double v    = (sigma - r2) * exp( -r2 / sigma);
          a[w*y + x]  = v;
          sum        += fabs(v);
        }
    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        a[w*y + x] /= sum;
  }

#ifdef DEBUG_FILTER
  print_filter(a,w,w);
#endif

  return (filter);
}

Image *Average_Round_2D_Filter(int r)
{ int      w = 2*r + 1;
  Image   *filter = Make_Image(FLOAT32,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y, z;
    double f;

    z = r;
    f = 0.;
    for (x = 1; x <= r; x++)
      { while (z*z + x*x > r*r)
          z -= 1;
        f += 2.*z + 1.;
      }
    f = 1. / ( 2.*f + (2.*r+1.) );

    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        a[w*y + x]  = 0.;

    z = r;
    for (x = 0; x <= r; x++)
      { while (z*z + x*x > r*r)
          z -= 1;
        for (y = 0; y <= z; y++)  
          { a[w*(r-y) + (r-x)] = f;
            a[w*(r-y) + (r+x)] = f;
            a[w*(r+y) + (r-x)] = f;
            a[w*(r+y) + (r+x)] = f;
          }
      }
  }

#ifdef DEBUG_FILTER
  print_filter(a,w,w);
#endif

  return (filter);
}

Image *Average_Square_2D_Filter(int r)
{ int      w = 2*r + 1;
  Image   *filter = Make_Image(FLOAT32,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y;
    double f = 1./(w*w);

    for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        a[w*y + x]  = f;
  }

#ifdef DEBUG_FILTER
  print_filter(a,w,w);
#endif

  return (filter);
}

Image *Convolve_Image(Image *filter, Image *image, int in_place)
{ int fw = filter->width;
  int fh = filter->height;
  int fx = fw/2;
  int fy = fh/2;
  int iw = image->width;
  int ih = image->height;
  int ic = fy*iw + fx;

  float32 *iarray, *farray, *buffer;

  double    sum;
  float32  *fa, *bo, *bt;
  float32  *ia, *io;
  int       x, y, p;
  int      xt, yt;
  int       u, v;
  int      ut, vt;
  int      out;

  if (filter->kind != FLOAT32)
    { fprintf(stderr,"Filter is not an image of type FLOAT32 (Filter_Image)\n");
      exit (1);
    }

  image  = Translate_Image(image,FLOAT32,in_place);
  buffer = (float32 *) Guarded_Malloc(sizeof(float32)*iw*(fy+1),"Filter_Image");
  farray = (float32 *) (filter->array);
  iarray = (float32 *) (image->array);

  yt = ih-(fh-fy);
  xt = iw-(fw-fx);
  bo = buffer;
  bt = buffer + iw*(fy+1);
  for (y = 0; y < ih; y++)
    { out = (y < fy || yt < y);
      for (x = 0; x < iw; x++)
        { sum = 0.;
          fa  = farray;
          io  = iarray + ((y*iw + x) - ic);

          if (out || x < fx || xt < x)
            { vt  = y + (fh-fy);
              ut  = x + (fw-fx);
              for (v = y-fy; v < vt; v++)
                { ia = io;
                  for (u = x-fx; u < ut; u++)
                    { if (0 <= v && v < ih && 0 <= u && u < iw)
                        sum += (*fa) * (*ia);
                      fa += 1;
                      ia += 1;
                    }
                  io += iw;
                }
            }
   
          else
            { for (v = 0; v < fh; v++)
                { ia = io;
                  for (u = 0; u < fw; u++)
                    sum += (*fa++) * (*ia++);
                  io += iw;
                }
            }
          *bo++ = sum;
        }
      if (bo >= bt)
        bo = buffer;
      if (y >= fy)
        { ia = iarray + (y-fy)*iw;
          for (x = 0; x < iw; x++)
            *ia++ = bo[x];
        }
    }

  ia = iarray + (y-fy)*iw;
  for (y -= fy; y < ih; y++)
    { for (x = 0; x < iw; x++)
        *ia++ = *bo++;
      if (bo >= bt)
        bo = buffer;
    }

  return (image);
}

Stack *Gaussian_3D_Filter(double sigma)
{ int      r = 3*sigma;
  int      w = 2*r + 1;
  int      w2 = w*w;
  Stack   *filter = Make_Stack(FLOAT32,w,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y, z;
    double sum;

    sigma = 2 * sigma * sigma;
    sum   = 0;
    for (z = 0; z < w; z++)
     for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        { double v           = exp(-((x-r)*(x-r) + (y-r)*(y-r) + (z-r)*(z-r)) / sigma);
          a[w2*z + w*y + x]  = v;
          sum               += v;
        }
    for (z = w2*w-1; z >= 0; z--)
      a[z] /= sum;
  }

#ifdef DEBUG_FILTER
  print_filter_3d(a,w,w,w);
#endif

  return (filter);
}

Stack *Mexican_Hat_3D_Filter(double sigma)
{ int      r = 3*sigma;
  int      w = 2*r + 1;
  int      w2 = w*w;
  Stack   *filter = Make_Stack(FLOAT32,w,w,w);
  float32 *a = (float32 *) filter->array;

  { int    x, y, z;
    double sum;

    sigma = 2 * sigma * sigma;
    sum   = 0;
    for (z = 0; z < w; z++)
     for (y = 0; y < w; y++)
      for (x = 0; x < w; x++)
        { double r2          = (x-r)*(x-r) + (y-r)*(y-r) + (z-r)*(z-r);
          double v           = (sigma - r2) * exp( -r2 / sigma);
          a[w2*z + w*y + x]  = v;
          sum               += fabs(v);
        }
    for (z = w2*w-1; z >= 0; z--)
      a[z] /= sum;
  }

#ifdef DEBUG_FILTER
  print_filter_3d(a,w,w,w);
#endif

  return (filter);
}

Stack *Average_Square_3D_Filter(int r)
{ int      w = 2*r + 1;
  int      w3 = w*w*w;
  Stack   *filter = Make_Stack(FLOAT32,w,w,w);
  float32 *a = (float32 *) filter->array;

  { int    z;
    double f = 1./w3;

    for (z = w3-1; z >= 0; z--)
      a[z] = f;
  }

#ifdef DEBUG_FILTER
  print_filter_3d(a,w,w,w);
#endif

  return (filter);
}
