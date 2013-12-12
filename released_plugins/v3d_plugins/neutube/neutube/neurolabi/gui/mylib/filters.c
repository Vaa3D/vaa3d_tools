#ifdef _MSC_VER

#pragma warning( disable:4996 )

#ifndef __cplusplus
#define inline __inline
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

//  WINDOW pthreads "LIBRARY"

  //  Mutex macros

typedef SRWLOCK pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER RTL_SRWLOCK_INIT

#define pthread_mutex_lock(m)  AcquireSRWLockExclusive(m)

#define pthread_mutex_unlock(m) ReleaseSRWLockExclusive(m)

  //  Condition variable macros

typedef CONDITION_VARIABLE pthread_cond_t;

#define PTHREAD_COND_INITIALIZER RTL_CONDITION_VARIABLE_INIT

#define pthread_cond_signal(c) WakeConditionVariable(c)

#define pthread_cond_broadcast(c) WakeAllConditionVariable(c)

#define pthread_cond_wait(c,m) SleepConditionVariableSRW(c,m,INFINITE,0)

  //  Simple thread support

typedef struct
  { HANDLE *handle;
    void   *(*fct)(void *);
    void   *arg;
    void   *retval;
    int     id;
  } Mythread;

typedef Mythread *pthread_t;

static DWORD WINAPI MyStart(void *arg)
{ Mythread *tv = (Mythread *) arg;

  tv->retval = tv->fct(tv->arg);
  return (0);
}

static int pthread_create(pthread_t *thread, void *attr,
                          void *(*fct)(void *), void *arg)
{ Mythread *tv;
  if (attr != NULL)
    { fprintf(stderr,"Do not support thread attributes\n");
      exit (1);
    }
  tv = (Mythread *) malloc(sizeof(Mythread));
  if (tv == NULL)
    { fprintf(stderr,"pthread_create: Out of memory.\n");
      exit (1);
    };
  tv->fct    = fct;
  tv->arg    = arg;
  tv->handle = CreateThread(NULL,0,MyStart,tv,0,&tv->id);
  if (tv->handle == NULL)
    return (EAGAIN);
  else
    return (0);
}

static int pthread_join(pthread_t t, void **ret)
{ Mythread *tv = (Mythread *) t;

  WaitForSingleObject(tv->handle,INFINITE);
  if (ret != NULL)
    *ret = tv->retval;

  CloseHandle(tv->handle);
  free(tv);
  return (0);
}

typedef int pthread_id;

static pthread_id pthread_tag()
{ return (GetCurrentThreadId()); }

static int pthread_is_this(pthread_id id)
{ return (GetCurrentThreadId() == id); }

#else   //  Small extension to pthreads!

#include <pthread.h>

typedef pthread_t pthread_id;

#define pthread_tag() pthread_self()

static inline int pthread_is_this(pthread_id id)
{ return (pthread_equal(pthread_self(),id)); }

#endif

/*****************************************************************************************\
*                                                                                         *
*  Image Filters                                                                          *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
#  Mod   :  Jan. 2009 -- Generalized to n-dimensional arrays.  Added commutative filters  *
*                          and special ones such as Average, etc.  Added gradient,        *
*                          curvature, Laplacian, flow fields                              *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <stdint.h>
#include <float.h>

#define UINT8_MIN  0
#define UINT16_MIN 0
#define UINT32_MIN 0
#define UINT64_MIN 0

#include "mylib.h"
#include "utilities.h"
#include "array.h"
#include "water.shed.h"
#include "linear.algebra.h"
#include "connectivity.h"
#include "filters.h"

#define BND_ZERO    0
#define BND_REFLECT 1
#define BND_WRAP    2
#define BND_EXTEND  3
#define BND_INVERT  4

extern int Boundary_Case_8qm5;


static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

static int check_filter(Array *filter, char *routine)
{ int span;

  if (filter->type != FLOAT64_TYPE)
    { fprintf(stderr,"Filter is not an array of type FLOAT64 (%s)\n",routine);
      exit (1);
    }
  if (filter->ndims != 1)
    { fprintf(stderr,"Filter is not one dimensional (%s)\n",routine);
      exit (1);
    }

  span = filter->dims[0];
  if (span%2 == 0)
    { fprintf(stderr,"Filter does not have odd length (%s)\n",routine);
      exit (1);
    }

  return (span);
}

/****************************************************************************************
 *                                                                                      *
 *  CONVOLUTION TEMPLATES                                                               *
 *                                                                                      *
 ****************************************************************************************/

static double PI  = 3.141592653589793;

static Double_Vector *G(gaussian_vector)(double sigma, int r, boolean normalize)
{ int       w = 2*r + 1;
  Dimn_Type d[1];
  Array    *filter;

  d[0] = w;
  filter = Make_Array(PLAIN_KIND,FLOAT64_TYPE,1,d);

  { float64 *a = AFLOAT64(filter);
    int      p;
    double   o, s, v, S;

    o = 2. * sigma * sigma;
    s = 1./sqrt(PI*o);
    S = 0.;
    for (p = 0; p < w; p++)
      { v    = (p-r);
        v    = s * exp(-v*v/o);
        a[p] = v;
        S   += v;
      }

    if (normalize)
      for (p = 0; p < w; p++)
        a[p] /= S;
  }

  return (filter);
}

Double_Vector *G(Gaussian_Filter)(double sigma, int radius)
{ return (gaussian_vector(sigma,radius,1)); }

static Double_Vector *G(gaussian_2nd_derivative)(double sigma, int r, boolean normalize)
{ int    w = 2*r + 1;
  Array *filter;

  filter = gaussian_vector(sigma,r,0);    

  { float64 *a = AFLOAT64(filter);
    int      p;
    double   o, v, S;

    o = sigma * sigma;
    S = 0.;
    for (p = 0; p < w; p++)
      { v     = p-r;
        v     = 1.-v*v/o;
        a[p] *= v;
        S    += a[p];
      }

    if (normalize)
      { S = -(S/w); 
        for (p = 0; p < w; p++)
          a[p] += S;
      }
  }

  return (filter);
}

static Double_Vector *G(box_vector)(int radius, int span)
{ int       w = 2*span + 1;
  Dimn_Type d[1];
  Array    *filter;

  d[0]   = w;
  filter = Make_Array(PLAIN_KIND,FLOAT64_TYPE,1,d);

  { float64 *a = AFLOAT64(filter);
    int      p;
    double   s = 1./(2*radius+1);

    for (p = 0; p < w; p++)
      a[p] = 0;
    for (p = span-radius; p <= span+radius; p++)
      a[p] = s;
  }

  return (filter);
}

Double_Vector *G(Box_Filter)(int radius)
{ return (box_vector(radius,radius)); }

Double_Array *G(Filter_Power)(Double_Vector *F(filter), int n)
{ Dimn_Type     dims[1];
  Coordinate   *fdims;
  Double_Array *fpowr;
  int           span;

  check_filter(filter,"Filter_Power");

  dims[0] = n;
  fdims = Make_Array(PLAIN_KIND,DIMN_TYPE,1,dims);
  span  = filter->dims[0];

  { int        i;
    Dimn_Type *f = ADIMN(fdims);

    for (i = 0; i < n; i++)
      f[i] = span;
  }

  fpowr = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,fdims);

  { float64  *a = AFLOAT64(fpowr);
    float64  *f = AFLOAT64(filter);
    Size_Type off, outr;
    Dimn_Type w;
    Indx_Type h, p, q;
    int       d;

    for (p = 0; p < fpowr->size; p++)
     a[p] = 1.;

    off = 1;
    for (d = 0; d < n; d++)
      { outr = off*span;
        for (h = 0; h < fpowr->size; h += outr)
          for (p = h+off; p-- > h; )
            for (w = 0, q = p; w < span; w++, q += off)
              a[q] *= f[w];
        off = outr;
      }
  }

  Free_Array(filter);
  return (fpowr);
}

Double_Array *G(Filter_Product)(Double_Array *F(filter1), Double_Array *F(filter2))
{ Dimn_Type     dims[1];
  Coordinate   *fdims;
  Double_Array *fprod;

  check_filter(filter1,"Filter_Product");
  check_filter(filter2,"Filter_Product");

  dims[0] = filter1->ndims + filter2->ndims;
  fdims = Make_Array(PLAIN_KIND,DIMN_TYPE,1,dims);

  { int        i;
    Dimn_Type *f = ADIMN(fdims);

    for (i = 0; i < filter2->ndims; i++)
      f[i] = filter2->dims[i];
    f += filter2->ndims;
    for (i = 0; i < filter1->ndims; i++)
      f[i] = filter1->dims[i];

  }

  fprod = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,fdims);

  { float64  *a = AFLOAT64(fprod);
    float64  *f = AFLOAT64(filter1);
    float64  *g = AFLOAT64(filter2);
    Indx_Type p, q, s;
    double    x;

    s = 0;
    for (q = 0; q < filter1->size; q++)
      { x = f[q];
        for (p = 0; p < filter2->size; p++)
          a[s++] = x*g[p];
      }
  }

  Free_Array(filter1);
  Free_Array(filter2);
  return (fprod);
}

Double_Vector *G(Filter_Convolution)(Double_Vector *F(filter1), Double_Vector *F(filter2))
{ Double_Array *fcon;
  int           n;

  check_filter(filter1,"Filter_Convolution");
  check_filter(filter2,"Filter_Convolution");

  n    = (int) (filter1->size + filter2->size - 1);
  fcon = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(n));

  { float64  *a = AFLOAT64(fcon);
    float64  *f = AFLOAT64(filter1);
    float64  *g = AFLOAT64(filter2);
    Indx_Type p, q;

    for (p = 0; p < n; p++)
      a[p] = 0.;

    for (p = 0; p < filter2->size; p++)
      for (q = 0; q < filter1->size; q++)
        a[p+q] += f[q]*g[p];
  }

  Free_Array(filter1);
  Free_Array(filter2);
  return (fcon);
}

Double_Vector *G(LOG_Filter)(int n, double sigma, int radius)
{ Array     *full;
  Slice     *slice;
  float64   *vals;
  double     sig2, sum;
  Dimn_Type *c;
  Indx_Type  p;
  int        d;

  full  = Filter_Power(gaussian_vector(sigma,radius,0),n);
  sig2  = 1. / (sigma*sigma);
  vals  = AFLOAT64(full);
  slice = Make_Slice(full,Idx2CoordA(full,0),Idx2CoordA(full,full->size-1));

  Set_Slice_To_First(slice);
  c = ADIMN(Slice_Coordinate(slice));
  sum = 0.;
  for (p = 0; p < full->size; p++)
    { double mag = 0.;
      for (d = 0; d < n; d++)
        { int x = c[d] - radius;
          mag += x*x;
        }
      vals[p] *= (n - mag * sig2);
      sum     += vals[p];
      Next_Slice_Index(slice);
    }

  sum = (-sum/full->size);
  for (p = 0; p < full->size; p++)
    vals[p] += sum;

  Kill_Slice(slice);

  return (full);
}

Double_Vector *G(ALOG_Filter)(int n, double *sigma, int *radius)
{ int        i;
  Array     *full;
  Slice     *slice;
  float64   *vals, sum;
  Dimn_Type *c;
  Indx_Type  p;
  int        d;

  full = NULL;
  for (i = 0; i < n; i++)
    if (i == 0)
      full = gaussian_vector(sigma[i],radius[i],0);
    else
      full = Filter_Product(gaussian_vector(sigma[i],radius[i],0),full);

  vals  = AFLOAT64(full);
  slice = Make_Slice(full,Idx2CoordA(full,0),Idx2CoordA(full,full->size-1));

  Set_Slice_To_First(slice);
  c = ADIMN(Slice_Coordinate(slice));
  sum = 0.;
  for (p = 0; p < full->size; p++)
    { double mag = 0.;
      for (d = 0; d < n; d++)
        { double m = (((int) c[d]) - radius[d]) / sigma[d];
          mag += m*m;
        }
      vals[p] *= (n - mag);
      sum     += vals[p];
      Next_Slice_Index(slice);
    }

  sum = (-sum/full->size);
  for (p = 0; p < full->size; p++)
    vals[p] += sum;

  Kill_Slice(slice);

  return (full);
}

Double_Vector *G(DOG_Filter)(int n, double sigma1, double sigma2, int radius)
{ Array *full1, *full2;

  full1 = Filter_Power(gaussian_vector(sigma1,radius,0),n);
  full2 = Filter_Power(gaussian_vector(sigma2,radius,0),n);

  Array_Op_Array(full1,SUB_OP,full2);
  Kill_Array(full2);

  return (full1);
}

Double_Vector *G(ADOG_Filter)(int n, double *sigma1, double *sigma2, int *radius)
{ int    i;
  Array *full1, *full2;

  full1 = full2 = NULL;
  for (i = 0; i < n; i++)
    if (i == 0)
      { full1 = gaussian_vector(sigma1[i],radius[i],0);
        full2 = gaussian_vector(sigma2[i],radius[i],0);
      }
    else
      { full1 = Filter_Product(gaussian_vector(sigma1[i],radius[i],0),full1);
        full2 = Filter_Product(gaussian_vector(sigma2[i],radius[i],0),full2);
      }

  Array_Op_Array(full1,SUB_OP,full2);
  Kill_Array(full2);

  return (full1);
}

Double_Vector *G(DOB_Filter)(int n, int radius1, int radius2)
{ int    r;
  Array *full1, *full2;

  if (radius1 < radius2)
    r = radius2;
  else
    r = radius1;

  full1 = Filter_Power(box_vector(radius1,r),n);
  full2 = Filter_Power(box_vector(radius2,r),n);

  Array_Op_Array(full1,SUB_OP,full2);
  Kill_Array(full2);

  return (full1);
}

Double_Vector *G(ADOB_Filter)(int n, int *radius1, int *radius2)
{ int    r, i;
  Array *full1, *full2;

  full1 = full2 = NULL;
  for (i = 0; i < n; i++)
    { if (radius1[i] < radius2[i])
        r = radius2[i];
      else
        r = radius1[i];
      if (i == 0)
        { full1 = box_vector(radius1[i],r);
          full2 = box_vector(radius2[i],r);
        }
      else
        { full1 = Filter_Product(box_vector(radius1[i],r),full1);
          full2 = Filter_Product(box_vector(radius2[i],r),full2);
        }
    }

  Array_Op_Array(full1,SUB_OP,full2);
  Kill_Array(full2);

  return (full1);
}

Double_Matrix *G(Gabor_Filter)(double aspect, double sigma, double wavelen, double phase,
                               double orient, int radius1, int radius2)
{ Dimn_Type d[2];
  Array    *filter;

  d[0] = 2*radius1+1;
  d[1] = 2*radius2+1;
  filter = Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,d);

  { float64  *a = AFLOAT64(filter);
    Dimn_Type p;
    int       x, y;
    double    xp, yp;
    double    cos0, sin0;

    sigma   = 2.*sigma*sigma;
    wavelen = 2.*PI/wavelen;
    aspect  = aspect * aspect;
    orient  = orient * PI / 180.;
    phase   = phase  * PI / 180.;

    cos0 = cos(orient);
    sin0 = sin(orient);

    p = 0;
    for (y = -radius2; y <= radius2; y++)
      for (x = -radius1; x <= radius1; x++)
        { xp = x*cos0 + y*sin0;
          yp = y*cos0 - x*sin0;
          a[p++] = exp( -(xp*xp + aspect*yp*yp)/sigma ) * cos( wavelen * xp + phase );
        }
  }

  return (filter);
}


/****************************************************************************************
 *                                                                                      *
 *  SEPERABLE CONVOLUTION AND OTHER SPECIAL FILTERS                                     *
 *                                                                                      *
 ****************************************************************************************/


  Array *Filter_Dimension(Array *R(M(image)), Double_Vector *F(filter), int d) 
  { int           span, width;
    float64      *fdata;                             
    float64      *line_buffer, Line_Buffer[2048];

    span  = check_filter(filter,"Filter_Dimension");    
    fdata = AFLOAT64(filter);                        

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Filter_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Filter_Dimension");
        }
      else
        { line_buffer = Line_Buffer; 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Filter_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (uint8) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (uint16) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (uint32) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (uint64) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (int8) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (int16) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (int32) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (int64) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (float32) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }


                        { float64 s;

                          bdata = buffer - radr;    //  Inner product of filter and current window
                          for (q = p; q < n; q += off)
                            { s = 0.;
                              for (k = 0; k < span; k++)
                                s += bdata[k] * fdata[k];
                              tdata[q] = (float64) s;
                              bdata   += 1;
                            }
                        }
          
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    Free_Array(filter);  
    return (image);
  }

  Array *Average_Dimension(Array *R(M(image)), int radius, int d)               
  { int           span, width;
    float64       invs;                              
    float64      *line_buffer, Line_Buffer[2048];

    span =  2*radius+1;                              
    invs  = 1. / span;                               

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Average_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Average_Dimension");
        }
      else
        { line_buffer = Line_Buffer; 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Average_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *adata;		
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint8 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (uint8) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (uint8) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *adata;		
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint16 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (uint16) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (uint16) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *adata;		
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (uint32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (uint32) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *adata;		
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (uint64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (uint64) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *adata;		
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int8 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (int8) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (int8) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *adata;		
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int16 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (int16) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (int16) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *adata;		
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (int32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (int32) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *adata;		
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (int64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (int64) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *adata;		
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (float32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (float32) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *adata;		
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = (float64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = (float64) (s * invs);
                            }
                        }
          
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    return (image);
  }

  Array *Sum_Dimension(Array *R(M(image)), int radius, int d)               
  { int           span, width;
    float64      *line_buffer, Line_Buffer[2048];

    span =  2*radius+1;                              

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Sum_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Sum_Dimension");
        }
      else
        { line_buffer = Line_Buffer; 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Sum_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *adata;		
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint8 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *adata;		
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint16 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *adata;		
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *adata;		
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *adata;		
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int8 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *adata;		
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int16 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *adata;		
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *adata;		
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *adata;		
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float32 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *adata;		
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float64 s;

                          s = buffer[0];               //  Incremenatlly compute sum of els to get
                          for (k = 1; k <= radr; k++)  //    average in O(1) per element
                            s += buffer[k] + buffer[-k];
                          tdata[p] = s;
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { s += (*bdata++);
                              s -= (*adata++);
                              tdata[q] = s;
                            }
                        }
        
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    return (image);
  }

  Array *Square_Dimension(Array *R(M(image)), int radius, int d)               
  { int           span, width;
    float64       invs;                              
    float64      *line_buffer, Line_Buffer[2048];

    span =  2*radius+1;                              
    invs  = 1. / span;                               

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Square_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Square_Dimension");
        }
      else
        { line_buffer = Line_Buffer; 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Square_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *adata;		
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (uint8) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (uint8) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *adata;		
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (uint16) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (uint16) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *adata;		
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (uint32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (uint32) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *adata;		
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (uint64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (uint64) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *adata;		
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (int8) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (int8) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *adata;		
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (int16) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (int16) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *adata;		
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (int32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (int32) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *adata;		
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (int64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (int64) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *adata;		
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (float32) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (float32) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *adata;		
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { double  x, s;
         
                          x = (double) buffer[0];
                          s = x*x;                    //  Incrementally compute sum of els and sum
                          for (k = 1; k <= radr; k++) //    of squares of els to get variance in
                            { x = (double) buffer[k]; //    O(1) per element
                              s  += x*x;
                              x = (double) buffer[-k];
                              s  += x*x;
                            }
                          tdata[p] = (float64) (s * invs);
          
                          adata = buffer - radr;
                          bdata = buffer + (radr+1);
                          for (q = p+off; q < n; q += off)
                            { x   = (double) (*bdata++);
                              s  += x*x;
                              x   = (double) (*adata++);
                              s  -= x*x;
                              tdata[q] = (float64) (s * invs);
                            }
                        }
        
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    return (image);
  }

  Array *Min_Dimension(Array *R(M(image)), int radius, int d)               
  { int           span, width;
    float64      *line_buffer, Line_Buffer[2048];
    float64      *heck_ramp, Heck_Ramp[2048];        

    span =  2*radius+1;                              

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Min_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Min_Dimension");
          heck_ramp   = line_buffer + width;        
        }
      else
        { line_buffer = Line_Buffer; 
          heck_ramp   = Heck_Ramp;                 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Min_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);
              uint8  *ramp   = ((uint8 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *adata;		
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint8      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint8 a = *adata++;
                              uint8 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);
              uint16  *ramp   = ((uint16 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *adata;		
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint16      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint16 a = *adata++;
                              uint16 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);
              uint32  *ramp   = ((uint32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *adata;		
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint32 a = *adata++;
                              uint32 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);
              uint64  *ramp   = ((uint64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *adata;		
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint64 a = *adata++;
                              uint64 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);
              int8  *ramp   = ((int8 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *adata;		
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int8      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int8 a = *adata++;
                              int8 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);
              int16  *ramp   = ((int16 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *adata;		
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int16      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int16 a = *adata++;
                              int16 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);
              int32  *ramp   = ((int32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *adata;		
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int32 a = *adata++;
                              int32 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);
              int64  *ramp   = ((int64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *adata;		
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int64 a = *adata++;
                              int64 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);
              float32  *ramp   = ((float32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *adata;		
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { float32 a = *adata++;
                              float32 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);
              float64  *ramp   = ((float64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *adata;		
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] > max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] > max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { float64 a = *adata++;
                              float64 b = *bdata++;
                              if (a > b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    return (image);
  }

  Array *Max_Dimension(Array *R(M(image)), int radius, int d)               
  { int           span, width;
    float64      *line_buffer, Line_Buffer[2048];
    float64      *heck_ramp, Heck_Ramp[2048];        

    span =  2*radius+1;                              

    if (d < 0 || d >= image->ndims)
      { fprintf(stderr,"Dimension is out of bounds (Max_Dimension)\n");
        exit (1);
      }

    { Dimn_Type radr, dim;
      Size_Type area, off, outr;
      Indx_Type h, p, n;

      width = image->dims[d] + span;
      if (width > 2048)
        { line_buffer =
                (float64 *) Guarded_Malloc(sizeof(float64)*2*((size_t) width),"Max_Dimension");
          heck_ramp   = line_buffer + width;        
        }
      else
        { line_buffer = Line_Buffer; 
          heck_ramp   = Heck_Ramp;                 
        }
  
      radr = (span >> 1);
      if (radr >= image->dims[d])
        { fprintf(stderr,"Radius of filter (%d) wider than dimension of array",radr);
          fprintf(stderr," (%d) (Max_Dimension)\n",image->dims[d]);
          exit (1);
        }

      area = image->size;

      off = 1;
      for (h = 0; h < d; h++)
        off *= image->dims[h];
      dim    = image->dims[d];
      outr   = off*dim;

      switch (image->type) {
          case UINT8_TYPE:
            { uint8  *tdata  = AUINT8(image);
              uint8  *buffer = ((uint8 *) line_buffer) + radr;
              uint8  *muffer = buffer + (dim-1);
              uint8  *ramp   = ((uint8 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint8      *adata;		
                      uint8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint8 b0 = buffer[0];
                              uint8 m0 = muffer[0];
                              uint8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint8      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint8 a = *adata++;
                              uint8 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16  *tdata  = AUINT16(image);
              uint16  *buffer = ((uint16 *) line_buffer) + radr;
              uint16  *muffer = buffer + (dim-1);
              uint16  *ramp   = ((uint16 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint16      *adata;		
                      uint16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint16 b0 = buffer[0];
                              uint16 m0 = muffer[0];
                              uint16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint16      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint16 a = *adata++;
                              uint16 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32  *tdata  = AUINT32(image);
              uint32  *buffer = ((uint32 *) line_buffer) + radr;
              uint32  *muffer = buffer + (dim-1);
              uint32  *ramp   = ((uint32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint32      *adata;		
                      uint32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint32 b0 = buffer[0];
                              uint32 m0 = muffer[0];
                              uint32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint32 a = *adata++;
                              uint32 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64  *tdata  = AUINT64(image);
              uint64  *buffer = ((uint64 *) line_buffer) + radr;
              uint64  *muffer = buffer + (dim-1);
              uint64  *ramp   = ((uint64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      uint64      *adata;		
                      uint64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { uint64 b0 = buffer[0];
                              uint64 m0 = muffer[0];
                              uint64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = UINT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = UINT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = UINT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = UINT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { uint64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { uint64 a = *adata++;
                              uint64 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8  *tdata  = AINT8(image);
              int8  *buffer = ((int8 *) line_buffer) + radr;
              int8  *muffer = buffer + (dim-1);
              int8  *ramp   = ((int8 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int8      *adata;		
                      int8      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int8 b0 = buffer[0];
                              int8 m0 = muffer[0];
                              int8 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT8_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT8_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT8_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT8_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int8      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int8 a = *adata++;
                              int8 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16  *tdata  = AINT16(image);
              int16  *buffer = ((int16 *) line_buffer) + radr;
              int16  *muffer = buffer + (dim-1);
              int16  *ramp   = ((int16 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int16      *adata;		
                      int16      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int16 b0 = buffer[0];
                              int16 m0 = muffer[0];
                              int16 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT16_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT16_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT16_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT16_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int16      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int16 a = *adata++;
                              int16 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32  *tdata  = AINT32(image);
              int32  *buffer = ((int32 *) line_buffer) + radr;
              int32  *muffer = buffer + (dim-1);
              int32  *ramp   = ((int32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int32      *adata;		
                      int32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int32 b0 = buffer[0];
                              int32 m0 = muffer[0];
                              int32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT32_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT32_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT32_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT32_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int32 a = *adata++;
                              int32 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64  *tdata  = AINT64(image);
              int64  *buffer = ((int64 *) line_buffer) + radr;
              int64  *muffer = buffer + (dim-1);
              int64  *ramp   = ((int64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      int64      *adata;		
                      int64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { int64 b0 = buffer[0];
                              int64 m0 = muffer[0];
                              int64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = INT64_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = INT64_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = INT64_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = INT64_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { int64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { int64 a = *adata++;
                              int64 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32  *tdata  = AFLOAT32(image);
              float32  *buffer = ((float32 *) line_buffer) + radr;
              float32  *muffer = buffer + (dim-1);
              float32  *ramp   = ((float32 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float32      *adata;		
                      float32      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float32 b0 = buffer[0];
                              float32 m0 = muffer[0];
                              float32 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = FLT_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = FLT_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = FLT_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = FLT_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float32      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { float32 a = *adata++;
                              float32 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64  *tdata  = AFLOAT64(image);
              float64  *buffer = ((float64 *) line_buffer) + radr;
              float64  *muffer = buffer + (dim-1);
              float64  *ramp   = ((float64 *) heck_ramp) + radr;   

              if (Boundary_Case_8qm5 == BND_ZERO)
                { int k;
                  for (k = 1; k <= radr; k++)
                    { buffer[-k] = 0;
                      muffer[k]  = 0;
                    } 
                }

              for (h = 0; h < area; h = n)
                { n = h+outr;
                  for (p = h+off; p-- > h; )
                    { Indx_Type q;
                      int       k;
                      float64      *adata;		
                      float64      *bdata;
          
                      bdata = buffer;
                      for (q = p; q < n; q += off)
                        *bdata++ = tdata[q];
          
                      switch (Boundary_Case_8qm5)
                        { case BND_REFLECT:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[k];
                                muffer[k]  = muffer[-k];
                              } 
                            break;
                          case BND_WRAP:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = muffer[1-k];
                                muffer[k]  = buffer[k-1];
                              }
                            break;
                          case BND_EXTEND:
                            for (k = 1; k <= radr; k++)
                              { buffer[-k] = buffer[0];
                                muffer[k]  = muffer[0];
                              }
                            break;
                          case BND_INVERT:
                            { float64 b0 = buffer[0];
                              float64 m0 = muffer[0];
                              float64 bk, mk;
                              for (k = 1; k <= radr; k++)  //  Care must be taken to avoid overflow
                                { bk = buffer[k];
                                  if (b0 > bk)
                                    { bk = b0 + (b0-bk);
                                      // if (bk < b0) bk = DBL_MAX;
                                    }
                                  else
                                    { bk = b0 - (bk-b0);
                                      // if (bk > b0) bk = DBL_MIN;
                                    }
                                  buffer[-k] = bk;

                                  mk = muffer[-k];
                                  if (m0 > mk)
                                    { mk = m0 + (m0-mk);
                                      // if (mk < m0) mk = DBL_MAX;
                                    }
                                  else
                                    { mk = m0 - (mk-m0);
                                      // if (mk > m0) mk = DBL_MIN;
                                    }
                                  muffer[k] = mk;
                                } 
                              break;
                            }
                        }

        
                        { float64      max;             //  Use van-Heck ramp algorithm to get
                          Dimn_Type g, j, u;         //  max/min in O(1) per element
        
                          adata = ramp   - radr;
                          bdata = buffer - radr;
                          g     = dim+(2*radr-1);
                          for (u = g - (dim-2)%span; 1; u -= span)
                            { adata[g] = max = bdata[g];
                              for (j = g; j-- > u; )
                                if (bdata[j] < max)          
                                  adata[j] = max;
                                else
                                  adata[j] = max = bdata[j];
                              max = bdata[u];
                              for (j = u+1; j <= g; j++)
                                if (bdata[j] < max)          
                                  bdata[j] = max;
                                else
                                  max = bdata[j];
                              g = u-1;
                              if (u < span) break;
                            }
        
                          bdata = buffer + radr;
                          for (q = p; q < n; q += off)
                            { float64 a = *adata++;
                              float64 b = *bdata++;
                              if (a < b)              
                                tdata[q] = b;
                              else
                                tdata[q] = a;
                            }
                        }
        
                    }
                }
              break;
            }
      }
    }

    if (width > 2048)
      free(line_buffer);

    return (image);
  }


Array *Filter_Array(Array *R(M(image)), Double_Vector *F(filter))              
{ int    d;

    for (d = 0; d < image->ndims; d++)
         Filter_Dimension(image,Inc_Array(filter),d);
    Free_Array(filter);     

  return (image);
}

Array *Filter_Array_List(Array *R(M(image)), Double_Vector **F(filter))     
{ int    d;

    for (d = 0; d < image->ndims; d++)
         Filter_Dimension(image,filter[d],d);

  return (image);
}

Array *Average_Array(Array *R(M(image)), int radius)                            
{ int    d;

    for (d = 0; d < image->ndims; d++)
        Average_Dimension(image,radius,d);

  return (image);
}

Array *Sum_Array(Array *R(M(image)), int radius)                            
{ int    d;

    for (d = 0; d < image->ndims; d++)
        Sum_Dimension(image,radius,d);

  return (image);
}

Array *Variance_Array(Array *R(M(image)), int radius)                            
{ int    d;
  Array *mean;   

    { mean = Copy_Array(image);

      Average_Dimension(mean,radius,0);
      Square_Dimension(image,radius,0);
      for (d = 1; d < image->ndims; d++)
        { Average_Dimension(mean,radius,d);
          Average_Dimension(image,radius,d);
        }

      switch (image->type) {
          case UINT8_TYPE:
            { Indx_Type k;
              uint8  *v = AUINT8(image);
              uint8  *m = AUINT8(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case UINT16_TYPE:
            { Indx_Type k;
              uint16  *v = AUINT16(image);
              uint16  *m = AUINT16(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case UINT32_TYPE:
            { Indx_Type k;
              uint32  *v = AUINT32(image);
              uint32  *m = AUINT32(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case UINT64_TYPE:
            { Indx_Type k;
              uint64  *v = AUINT64(image);
              uint64  *m = AUINT64(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case INT8_TYPE:
            { Indx_Type k;
              int8  *v = AINT8(image);
              int8  *m = AINT8(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case INT16_TYPE:
            { Indx_Type k;
              int16  *v = AINT16(image);
              int16  *m = AINT16(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case INT32_TYPE:
            { Indx_Type k;
              int32  *v = AINT32(image);
              int32  *m = AINT32(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case INT64_TYPE:
            { Indx_Type k;
              int64  *v = AINT64(image);
              int64  *m = AINT64(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case FLOAT32_TYPE:
            { Indx_Type k;
              float32  *v = AFLOAT32(image);
              float32  *m = AFLOAT32(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
          case FLOAT64_TYPE:
            { Indx_Type k;
              float64  *v = AFLOAT64(image);
              float64  *m = AFLOAT64(mean);
              for (k = 0; k < image->size; k++)
                v[k] -= m[k] * m[k];
              break;
            }
      }

      Kill_Array(mean);
    }

  return (image);
}

Array *Min_Array(Array *R(M(image)), int radius)                            
{ int    d;

    for (d = 0; d < image->ndims; d++)
        Min_Dimension(image,radius,d);

  return (image);
}

Array *Max_Array(Array *R(M(image)), int radius)                            
{ int    d;

    for (d = 0; d < image->ndims; d++)
        Max_Dimension(image,radius,d);

  return (image);
}

Array *G(LOG_Array)(Array *R(M(img)), double sigma, int radius)
{ Double_Vector *mex, *gas;
  Float_Array   *rez, *lap;
  int            i, j, n;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"LOG will be meaningless as image is unsigned (LOG_Array)\n");
      fflush(stdout);
    }

  mex = gaussian_2nd_derivative(sigma,radius,1);
  gas = gaussian_vector(sigma,radius,1);

  n   = img->ndims-1;
  rez = Copy_Array(img);
  Array_Op_Scalar(img,SET_OP,INT32_TYPE,VALI(0));
  for (i = 0; i < n; i++)
    { lap = Filter_Dimension(Copy_Array(rez),Inc_Array(mex),i);
      for (j = i+1; j < n; j++)
        Filter_Dimension(lap,Inc_Array(gas),j);
      Array_Op_Array(img,ADD_OP,lap);
      Filter_Dimension(rez,Inc_Array(gas),i);
      if (i < n-1)
        Free_Array(lap);
      else
        Kill_Array(lap);
    }
  Filter_Dimension(rez,Inc_Array(mex),n);
  Array_Op_Array(img,ADD_OP,rez);
  Kill_Array(rez);
  Kill_Array(gas);
  Kill_Array(mex);
  return (img);
}

Array *G(ALOG_Array)(Array *R(M(img)), double *sigma, int *radius)
{ Double_Vector *mex, *gas;
  Float_Array   *rez, *lap;
  int            i, j, n;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"LOG will be meaningless as image is unsigned (LOG_Array)\n");
      fflush(stdout);
    }

  n   = img->ndims-1;
  rez = Copy_Array(img);
  Array_Op_Scalar(img,SET_OP,INT32_TYPE,VALI(0));
  for (i = 0; i < n; i++)
    { gas = gaussian_vector(sigma[i],radius[i],1);
      mex = gaussian_2nd_derivative(sigma[i],radius[i],1);

      lap = Filter_Dimension(Copy_Array(rez),Inc_Array(mex),i);
      for (j = i+1; j < n; j++)
        Filter_Dimension(lap,Inc_Array(gas),j);
      Array_Op_Array(img,ADD_OP,lap);
      Filter_Dimension(rez,Inc_Array(gas),i);
      if (i < n-1)
        { Free_Array(lap);
          Free_Array(gas);
        }
      else
        { Kill_Array(lap);
          Kill_Array(gas);
        }
      Free_Array(mex);
    }
  mex = gaussian_2nd_derivative(sigma[n],radius[n],1);
  Filter_Dimension(rez,Inc_Array(mex),n);
  Array_Op_Array(img,ADD_OP,rez);
  Kill_Array(mex);
  Kill_Array(rez);
  return (img);
}

Array *G(DOG_Array)(Array *R(M(img)), double sigma1, double sigma2, int radius)
{ Double_Vector *gas1, *gas2;
  Float_Array   *tmp;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"DOG will be meaningless as image is unsigned (DOG_Array)\n");
      fflush(stdout);
    }

  gas1 = gaussian_vector(sigma1,radius,1);
  gas2 = gaussian_vector(sigma2,radius,1);

  tmp  = Filter_Array(Copy_Array(img),gas2);
  img  = Filter_Array(img,gas1);
  Array_Op_Array(img,SUB_OP,tmp);

  Kill_Array(tmp);
  return (img);
}

Array *G(ADOG_Array)(Array *R(M(img)), double *sigma1, double *sigma2, int *radius)
{ Double_Vector *gas1, *gas2;
  Float_Array   *tmp;
  int            i;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"DOG will be meaningless as image is unsigned (ADOG_Array)\n");
      fflush(stdout);
    }

  tmp = Copy_Array(img);
  for (i = 0; i < img->ndims; i++)
    { gas1 = gaussian_vector(sigma1[i],radius[i],1);
      gas2 = gaussian_vector(sigma2[i],radius[i],1);

      tmp  = Filter_Array(tmp,gas2);
      img  = Filter_Array(img,gas1);
    }
  Array_Op_Array(img,SUB_OP,tmp);

  Kill_Array(tmp);
  return (img);
}

Array *G(DOB_Array)(Array *R(M(img)), int radius1, int radius2)
{ Float_Array *tmp;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"DOB will be meaningless as image is unsigned (DOB)\n");
      fflush(stdout);
    }

  tmp = Average_Array(Copy_Array(img),radius2);
  img = Average_Array(img,radius1);
  Array_Op_Array(img,SUB_OP,tmp);
  Kill_Array(tmp);
  return (img);
}

Array *G(ADOB_Array)(Array *R(M(img)), int *radius1, int *radius2)
{ Float_Array *tmp;
  int          i;

  if (img->type <= UINT64_TYPE)
    { fprintf(stderr,"DOB will be meaningless as image is unsigned (ADOB_Array)\n");
      fflush(stdout);
    }

  tmp = Copy_Array(img);
  for (i = 0; i < img->ndims; i++)
    { tmp = Average_Array(tmp,radius2[i]);
      img = Average_Array(img,radius1[i]);
    }
  Array_Op_Array(img,SUB_OP,tmp);
  Kill_Array(tmp);
  return (img);
}


/****************************************************************************************
 *                                                                                      *
 * GENERAL CONVOLUTION FILTERING (N-DIMENSIONAL CONVOLUTION AT A POINT)                 *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Array      *image_ref; //  Image to be convolved
    Array      *filter;    //  Filter for convolution (owned by Convolver)
    Frame      *frame;     //  Frame for filter
  } Iconvolver;

static pthread_mutex_t Convolver_Mutex = PTHREAD_MUTEX_INITIALIZER;
static Iconvolver     *Free_Convolvers = NULL;   //  Free list for Reset

void Reset_Convolver()
{ pthread_mutex_lock(&Convolver_Mutex);
  while (Free_Convolvers != NULL)
    { Iconvolver *c = Free_Convolvers;
      Free_Convolvers = (Iconvolver *) c->image_ref;
      c->image_ref = NULL;
      Kill_Convolver(c);
    }
  pthread_mutex_unlock(&Convolver_Mutex);
}

void Kill_Convolver(Convolver *cv)
{ Iconvolver *c = (Iconvolver *) cv;
  if (c->image_ref != NULL)
    Kill_Array(c->image_ref);
  Kill_Array(c->filter);
  Kill_Frame(c->frame);
  free(c);
}

void Free_Convolver(Convolver *cv)
{ Iconvolver *c = (Iconvolver *) cv;
  Free_Array(c->image_ref);
  Free_Array(c->filter);
  Free_Frame(c->frame);

  pthread_mutex_lock(&Convolver_Mutex);
  c->image_ref = (Array *) Free_Convolvers;
  Free_Convolvers = c;
  pthread_mutex_unlock(&Convolver_Mutex);
}

Convolver *Make_Convolver(Array *I(image), Double_Array *C(filter), Coordinate *C(center))
{ Iconvolver *c;
  int         n = image->ndims;

  if (filter->type != FLOAT64_TYPE)
    { fprintf(stderr,"Filter is not an array of type FLOAT64 (Make_Convolver)\n");
      exit (1);
    }
  if (filter->ndims != image->ndims)
    { fprintf(stderr,"Filter and image do not have the same dimensionality (Make_Convolver)\n");
      exit (1);
    }

  { int32  i;
    int32 *f;

    if (center != NULL)
      { if (center->dims[0] != n)
          { fprintf(stderr,"Center coord and filter do not have the same dimensionality");
            fprintf(stderr," (Make_Convolver)\n");
            exit (1);
          }
        if (center->ndims != 1 || center->type != DIMN_TYPE)
          { fprintf(stderr,"Center is not an integer vector (Make_Convolver)\n");
            exit (1);
          }
      }
    else
      { center = Make_Array_With_Shape(PLAIN_KIND,INT32_TYPE,Coord1(image->ndims));
        f      = AINT32(center);
        for (i = 0; i < n; i++)
          f[i] = filter->dims[i]/2;
      }
  }

  pthread_mutex_lock(&Convolver_Mutex);
  if (Free_Convolvers != NULL)
    { c = Free_Convolvers;
      Free_Convolvers = (Iconvolver *) c->image_ref;
    }
  else
    c = (Iconvolver *) Guarded_Malloc(sizeof(Iconvolver),"Make_Convolver");
  pthread_mutex_unlock(&Convolver_Mutex);

  c->frame      = Make_Frame(image,AForm_Shape(filter),center);
  c->image_ref  = Inc_Array(image);
  c->filter     = filter;

  return ((Convolver *) c);
}

Double_Array *Convolver_Filter(Convolver *c)
{ return (((Iconvolver *) c)->filter); }

Coordinate *Convolver_Anchor(Convolver *c)
{ return (Frame_Anchor(((Iconvolver *) c)->frame)); }

Indx_Type Convolver_Index(Convolver *c)
{ return (Frame_Index(((Iconvolver *) c)->frame)); }

Coordinate *Convolver_Coordinate(Convolver *c)
{ return (Frame_Coordinate(((Iconvolver *) c)->frame)); }

void Place_Convolver(Convolver *M(c), Indx_Type p)
{ Place_Frame(((Iconvolver *) c)->frame,p); }

void Move_Convolver_Forward(Convolver *M(c))
{ Move_Frame_Forward(((Iconvolver *) c)->frame); }

void Move_Convolver_Backward(Convolver *M(c))
{ Move_Frame_Backward(((Iconvolver *) c)->frame); }

double Convolve(Convolver *cv)
{ Iconvolver *c = (Iconvolver *) cv;
  Array      *a = c->image_ref;
  Frame      *f = c->frame;
  float64    *w = AFLOAT64(c->filter);
  Size_Type   n = c->filter->size;
  Indx_Type   j;
  double      sum;

  sum = 0.;
  if (Frame_Within_Array(f))
    { Offs_Type *o = Frame_Offsets(f);
      switch (a->type) {
          case UINT8_TYPE:
            { uint8 *v = AUINT8(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT16_TYPE:
            { uint16 *v = AUINT16(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT32_TYPE:
            { uint32 *v = AUINT32(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT64_TYPE:
            { uint64 *v = AUINT64(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT8_TYPE:
            { int8 *v = AINT8(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT16_TYPE:
            { int16 *v = AINT16(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT32_TYPE:
            { int32 *v = AINT32(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT64_TYPE:
            { int64 *v = AINT64(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case FLOAT32_TYPE:
            { float32 *v = AFLOAT32(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case FLOAT64_TYPE:
            { float64 *v = AFLOAT64(a) + Frame_Index(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
      }
    }
  else
    { switch (a->type) {
          case UINT8_TYPE:
            { uint8 *v = (uint8 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case UINT16_TYPE:
            { uint16 *v = (uint16 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case UINT32_TYPE:
            { uint32 *v = (uint32 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case UINT64_TYPE:
            { uint64 *v = (uint64 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case INT8_TYPE:
            { int8 *v = (int8 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case INT16_TYPE:
            { int16 *v = (int16 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case INT32_TYPE:
            { int32 *v = (int32 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case INT64_TYPE:
            { int64 *v = (int64 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case FLOAT32_TYPE:
            { float32 *v = (float32 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
          case FLOAT64_TYPE:
            { float64 *v = (float64 *) Frame_Values(f);
              for (j = 0; j < n; j++)
                sum += w[j] * v[j];
              return (sum);
            }
      }
    }
  return (0.);
}

double Convolve_Segment(Convolver *cv, Indx_Type offset, Size_Type span)
{ Iconvolver *c = (Iconvolver *) cv;
  Array      *a = c->image_ref;
  Frame      *f = c->frame;
  float64    *w = AFLOAT64(c->filter);
  Size_Type   n = offset + span;
  Indx_Type   j;
  double      sum;

  sum = 0.;
  if (Frame_Within_Array(f))
    { Offs_Type *o = Frame_Offsets(f);
      switch (a->type) {
          case UINT8_TYPE:
            { uint8 *v = (uint8 *) AUINT8(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT16_TYPE:
            { uint16 *v = (uint16 *) AUINT16(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT32_TYPE:
            { uint32 *v = (uint32 *) AUINT32(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case UINT64_TYPE:
            { uint64 *v = (uint64 *) AUINT64(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT8_TYPE:
            { int8 *v = (int8 *) AINT8(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT16_TYPE:
            { int16 *v = (int16 *) AINT16(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT32_TYPE:
            { int32 *v = (int32 *) AINT32(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case INT64_TYPE:
            { int64 *v = (int64 *) AINT64(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case FLOAT32_TYPE:
            { float32 *v = (float32 *) AFLOAT32(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
          case FLOAT64_TYPE:
            { float64 *v = (float64 *) AFLOAT64(a) + Frame_Index(f);
              for (j = offset; j < n; j++)
                sum += w[j] * v[o[j]];
              return (sum);
            }
      }
    }
  else
    switch (a->type) {
        case UINT8_TYPE:
          { uint8 *v = (uint8 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case UINT16_TYPE:
          { uint16 *v = (uint16 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case UINT32_TYPE:
          { uint32 *v = (uint32 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case UINT64_TYPE:
          { uint64 *v = (uint64 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case INT8_TYPE:
          { int8 *v = (int8 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case INT16_TYPE:
          { int16 *v = (int16 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case INT32_TYPE:
          { int32 *v = (int32 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case INT64_TYPE:
          { int64 *v = (int64 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case FLOAT32_TYPE:
          { float32 *v = (float32 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
        case FLOAT64_TYPE:
          { float64 *v = (float64 *) Frame_Values(f);
            for (j = offset; j < n; j++)
              sum += w[j] * v[j];
            return (sum);
          }
    }
  return (0.);
}


/****************************************************************************************
 *                                                                                      *
 *  DERIVATIVE SCHEME ROUTINES                                                          *
 *                                                                                      *
 ****************************************************************************************/

D_Scheme _Matlab = { NULL, NULL };
D_Scheme _Sobel  = { NULL, NULL };
D_Scheme _Iso3   = { NULL, NULL };
D_Scheme _Iso5   = { NULL, NULL };
D_Scheme _Iso7   = { NULL, NULL };

D_Scheme *Matlab = &_Matlab;
D_Scheme *Sobel  = &_Sobel;
D_Scheme *Iso3   = &_Iso3;
D_Scheme *Iso5   = &_Iso5;
D_Scheme *Iso7   = &_Iso7;

void setup_schemes()
{ if (Matlab->base != NULL) return;

  Matlab->base  = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(1));
  Matlab->deriv = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));

  Sobel->base  = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));
  Sobel->deriv = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));

  Iso5->base  = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(5));
  Iso5->deriv = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(5));

  Iso7->base  = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(7));
  Iso7->deriv = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(7));

  AFLOAT64(Matlab->base)[0] = 1.;

  AFLOAT64(Matlab->deriv)[0] = -1.;
  AFLOAT64(Matlab->deriv)[1] =  0.;
  AFLOAT64(Matlab->deriv)[2] =  1.;

  AFLOAT64(Sobel->base)[0] = .25;
  AFLOAT64(Sobel->base)[1] = .50;
  AFLOAT64(Sobel->base)[2] = .25;

  AFLOAT64(Sobel->deriv)[0] = -1.;
  AFLOAT64(Sobel->deriv)[1] =  0.;
  AFLOAT64(Sobel->deriv)[2] =  1.;

  AFLOAT64(Iso3->base)[0] = .229879;
  AFLOAT64(Iso3->base)[1] = .540242;
  AFLOAT64(Iso3->base)[2] = .229879;
  
  AFLOAT64(Iso3->deriv)[0] = -.425287;
  AFLOAT64(Iso3->deriv)[1] =  .0;
  AFLOAT64(Iso3->deriv)[2] = +.425287;

  AFLOAT64(Iso5->base)[0] = .037659;
  AFLOAT64(Iso5->base)[1] = .249153;
  AFLOAT64(Iso5->base)[2] = .426375;
  AFLOAT64(Iso5->base)[3] = .249153;
  AFLOAT64(Iso5->base)[4] = .037659;
  
  AFLOAT64(Iso5->deriv)[0] = -.109604;
  AFLOAT64(Iso5->deriv)[1] = -.276691;
  AFLOAT64(Iso5->deriv)[2] =  .0;
  AFLOAT64(Iso5->deriv)[3] = +.276691;
  AFLOAT64(Iso5->deriv)[4] = +.109604;

  AFLOAT64(Iso7->base)[0] = .004711;
  AFLOAT64(Iso7->base)[1] = .069321;
  AFLOAT64(Iso7->base)[2] = .245410;
  AFLOAT64(Iso7->base)[3] = .361117;
  AFLOAT64(Iso7->base)[4] = .245410;
  AFLOAT64(Iso7->base)[5] = .069321;
  AFLOAT64(Iso7->base)[6] = .004711;
  
  AFLOAT64(Iso7->deriv)[0] = -.018708;
  AFLOAT64(Iso7->deriv)[1] = -.125376;
  AFLOAT64(Iso7->deriv)[2] = -.193357;
  AFLOAT64(Iso7->deriv)[3] =  .0;
  AFLOAT64(Iso7->deriv)[4] = +.193357;
  AFLOAT64(Iso7->deriv)[5] = +.125376;
  AFLOAT64(Iso7->deriv)[6] = +.018708;
}

D_Scheme *Make_Derivative_Scheme(Double_Vector *C(base), Double_Vector *C(deriv))
{ D_Scheme *scheme = (D_Scheme *) Guarded_Malloc(sizeof(D_Scheme),"Make_Derivative_Scheme");
  scheme->base  = base;
  scheme->deriv = deriv;
  return (scheme);
}

void Kill_Derivative_Scheme(D_Scheme *scheme)
{ if (scheme == Matlab) return;
  if (scheme == Sobel) return;
  if (scheme == Iso3) return;
  if (scheme == Iso5) return;
  if (scheme == Iso7) return;
  Kill_Array(scheme->base);
  Kill_Array(scheme->deriv);
  free(scheme);
}


/****************************************************************************************
 *                                                                                      *
 *  VECTOR ITERATOR ROUTINES                                                            *
 *                                                                                      *
 ****************************************************************************************/

  //  Vector_Iterator codes

typedef struct
  { Double_Vector *(*next)(Vector_Iterator *);  // Iterator to invoke
    void           (*kill)(Vector_Iterator *);  // Kill routine to invoke

    int              ndims;      //  Dimensions of the vector
    Double_Vector   *vector;     //  Vector bundle for return values
    Value_Iterator **mg;         //  Array of ndims iterators, one for each direction
  } VEC_It;

Double_Vector *Next_VEC(Vector_Iterator *ge)
{ VEC_It *g = (VEC_It *) ge; 
  double *v = AFLOAT64(g->vector);
  int     i;

  for (i = 0; i < g->ndims; i++)
    v[i] = NEXT_VALUE(g->mg[i]);
  return (g->vector);
}

void Kill_VEC(Vector_Iterator *ge)
{ VEC_It *g = (VEC_It *) ge; 
  int i;

  for (i = 0; i < g->ndims; i++)
    KILL_VALUE(g->mg[i]);
  Free_Array(g->vector);
  free(g);
}

static VEC_It *start_vector(int ndims, char *routine)
{ VEC_It *g = (VEC_It *) Guarded_Malloc(sizeof(VEC_It),routine);
  g->mg     = (Value_Iterator **) Guarded_Malloc(sizeof(Value_Iterator *)*((size_t) ndims),routine);
  g->next   = Next_VEC;
  g->kill   = Kill_VEC;
  g->vector = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(ndims));
  g->ndims  = ndims;
  return (g);
}


/****************************************************************************************
 *                                                                                      *
 *  MATLAB DERIVATIVE ROUTINES                                                          *
 *                                                                                      *
 ****************************************************************************************/

  //  MG_Iterator codes: Matlab derivative along a given dimension

typedef struct
  { double   (*next)(Value_Iterator *);  // Iterator to invoke
    void     (*kill)(Value_Iterator *);  // Kill routine to invoke

    AForm *form;    // Reference to form

    int        state;   // 0 if not boundary, 1 if high boundary, 2 if low boundary
    Size_Type  innr;    // PI_i=0^dim-1 form->dims[i]
    Size_Type  outr;    // innr * (form->dims[dim]-2)
    Indx_Type  idx;     // Current index in array
    Indx_Type  bound;   // Next index at which one should change state

    void      *p;       // array offset for previous pixel in direction dim
    void      *s;       // array off set for the next pixel in direction dim

  // Offsetable Frame & Slice

    Size_Type  ispan;    // PI_i=0^dim-1 array->dims[i]
    Size_Type  ospan;    // innr * (array->dims[dim]-2)
    int        in1, in2; // is Slice or Fram strictly inside array in each direction of dim

  // Slice

    Indx_Type  sdx;      // Current index in slice

  // ! Offsetable Frame

    Frame     *frame1;   // Thin left-end frame for non-offsetable Frame
    Frame     *frame2;   // Thin right-end frame for non-offsetable Frame
    Indx_Type  fdx1;     // Current index in frame1
    Indx_Type  fdx2;     // Current index in frame2

  } MG_It;

#define MGI   ((MG_It *) ge)

#define OSA(pidx)  f[pidx]
#define OPA(pidx)  b[pidx]

  static double MG_UINT8_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint8      *f = (uint8 *) MGI->s;
    uint8      *b = (uint8 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT16_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint16      *f = (uint16 *) MGI->s;
    uint16      *b = (uint16 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT32_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint32      *f = (uint32 *) MGI->s;
    uint32      *b = (uint32 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT64_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint64      *f = (uint64 *) MGI->s;
    uint64      *b = (uint64 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT8_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int8      *f = (int8 *) MGI->s;
    int8      *b = (int8 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT16_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int16      *f = (int16 *) MGI->s;
    int16      *b = (int16 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT32_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int32      *f = (int32 *) MGI->s;
    int32      *b = (int32 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT64_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int64      *f = (int64 *) MGI->s;
    int64      *b = (int64 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT32_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float32      *f = (float32 *) MGI->s;
    float32      *b = (float32 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT64_ARRAY(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float64      *f = (float64 *) MGI->s;
    float64      *b = (float64 *) MGI->p;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(p) - OPA(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OPA(p) * -.5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OPA(p-MGI->outr) - OPA(p));
            break;
          case BND_EXTEND:
            x = .5*(OPA(p+MGI->innr) - OPA(p));
            break;
          case BND_INVERT:
            x = (double) (OPA(p+MGI->innr) - OPA(p));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        switch (Boundary_Case_8qm5)
        { case BND_ZERO:
            x = OSA(p) * .5;
            break;
          case BND_REFLECT:
            x = 0.;
            break;
          case BND_WRAP:
            x = (double) (OSA(p) - OSA(p+MGI->outr));
            break;
          case BND_EXTEND:
            x = .5*(OSA(p) - OSA(p-MGI->innr));
            break;
          case BND_INVERT:
            x = (double) (OSA(p) - OSA(p-MGI->innr));
            break;
          default:
            x = 0.;
            break;
        }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }

#define OSI(pidx) ((Offs_Type *) (MGI->s))[pidx]
#define OPI(pidx) ((Offs_Type *) (MGI->p))[pidx]

  static double MG_UINT8_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint8      *v = (uint8 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT16_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint16      *v = (uint16 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT32_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint32      *v = (uint32 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT64_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint64      *v = (uint64 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT8_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int8      *v = (int8 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT16_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int16      *v = (int16 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT32_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int32      *v = (int32 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT64_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int64      *v = (int64 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT32_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float32      *v = (float32 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT64_OFFSET(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float64      *v = (float64 *) AForm_Array(MGI->form)->data;

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(v[OSI(p)] - v[OPI(p)]);
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(v[OPI(p)+MGI->ispan] - v[OPI(p)]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OPI(p)] * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OPI(p)-MGI->ospan] - v[OPI(p)]);
              break;
            case BND_EXTEND:
              x = .5*(v[OPI(p+MGI->innr)] - v[OPI(p)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(v[OSI(p)] - v[OSI(p)-MGI->ispan]);
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = v[OSI(p)] * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (v[OSI(p)] - v[OSI(p)+MGI->ospan]);
              break;
            case BND_EXTEND:
              x = .5*(v[OSI(p)] - v[OSI(p+MGI->innr)]);
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }

#define OSF(pidx) v[(pidx)+MGI->innr]   //  NOT CORRECT!!!!
#define OPF(pidx) v[(pidx)-MGI->innr]

  static double MG_UINT8_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint8      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT16_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint16      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT32_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint32      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_UINT64_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    uint64      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT8_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int8      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT16_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int16      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT32_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int32      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_INT64_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    int64      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT32_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float32      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT64_FRAME(Value_Iterator *ge)
  { Indx_Type p;
    double    x;
    float64      *v = Frame_Values(MGI->frame1);

    p = MGI->idx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSF(p) - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 1:
        x = .5*(v[MGI->fdx1++] - OPF(p));
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        return (x);
      case 2:
        x = .5*(OSF(p) - v[MGI->fdx2++]);
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        return (x);
      }
    return (0.);
  }

  static double MG_UINT8_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    uint8      *f = (uint8 *) MGI->s;
    uint8      *b = (uint8 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_UINT16_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    uint16      *f = (uint16 *) MGI->s;
    uint16      *b = (uint16 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_UINT32_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    uint32      *f = (uint32 *) MGI->s;
    uint32      *b = (uint32 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_UINT64_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    uint64      *f = (uint64 *) MGI->s;
    uint64      *b = (uint64 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_INT8_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    int8      *f = (int8 *) MGI->s;
    int8      *b = (int8 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_INT16_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    int16      *f = (int16 *) MGI->s;
    int16      *b = (int16 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_INT32_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    int32      *f = (int32 *) MGI->s;
    int32      *b = (int32 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_INT64_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    int64      *f = (int64 *) MGI->s;
    int64      *b = (int64 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT32_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    float32      *f = (float32 *) MGI->s;
    float32      *b = (float32 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }
  static double MG_FLOAT64_SLICE(Value_Iterator *ge)
  { Indx_Type p, q;
    double    x;
    float64      *f = (float64 *) MGI->s;
    float64      *b = (float64 *) MGI->p;

    p = MGI->idx;
    q = MGI->sdx;
    switch (MGI->state)
    { case 0:
        x = .5*(OSA(q) - OPA(q));
        if (++p >= MGI->bound)
          { MGI->state = 1;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 1:
        if (MGI->in1)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OPA(q) * -.5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OPA(q-MGI->ospan) - OPA(p));
              break;
            case BND_EXTEND:
              x = .5*(OPA(q+MGI->ispan) - OPA(q));
              break;
            case BND_INVERT:
              x = (double) (OPA(q+MGI->ispan) - OPA(q));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 2;
            MGI->bound += MGI->innr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      case 2:
        if (MGI->in2)
          x = .5*(OSA(q) - OPA(q));
        else
          switch (Boundary_Case_8qm5)
          { case BND_ZERO:
              x = OSA(q) * .5;
              break;
            case BND_REFLECT:
              x = 0.;
              break;
            case BND_WRAP:
              x = (double) (OSA(q) - OSA(q+MGI->ospan));
              break;
            case BND_EXTEND:
              x = .5*(OSA(q) - OSA(q-MGI->ispan));
              break;
            case BND_INVERT:
              x = (double) (OSA(q) - OSA(q-MGI->ispan));
              break;
            default:
              x = 0.;
              break;
          }
        if (++p >= MGI->bound)
          { MGI->state = 0;
            MGI->bound += MGI->outr;
          }
        MGI->idx = p;
        MGI->sdx = Next_Slice_Index(MGI->form);
        return (x);
      }
    return (0.);
  }

static void Kill_MG(Value_Iterator *ge)
{ MG_It *g = (MG_It *) ge; 
  switch (AForm_Class(g->form))
  { case FRAME_CLASS:
      if ( ! Frame_Within_Array(g->form))
        { Kill_Frame(g->frame1);
          Kill_Frame(g->frame2);
        }
      Kill_Frame(g->form);
      break;
    case SLICE_CLASS:
      Kill_Slice(g->form);
      break;
    default:
      Kill_Array((Array *) (g->form));
      break;
  }
  free(g);
}

static Value_Iterator *Matlab_Derivative(AForm *I(form), int dim)
{ static double (*aroutines[])(Value_Iterator *)
                    = { MG_UINT8_ARRAY, MG_UINT16_ARRAY, MG_UINT32_ARRAY, MG_UINT64_ARRAY,
                        MG_INT8_ARRAY,  MG_INT16_ARRAY,  MG_INT32_ARRAY,  MG_INT64_ARRAY,
                        MG_FLOAT32_ARRAY, MG_FLOAT64_ARRAY };
  static double (*iroutines[])(Value_Iterator *)
                    = { MG_UINT8_OFFSET, MG_UINT16_OFFSET, MG_UINT32_OFFSET, MG_UINT64_OFFSET,
                        MG_INT8_OFFSET,  MG_INT16_OFFSET,  MG_INT32_OFFSET,  MG_INT64_OFFSET,
                        MG_FLOAT32_OFFSET, MG_FLOAT64_OFFSET };
  static double (*froutines[])(Value_Iterator *)
                    = { MG_UINT8_FRAME, MG_UINT16_FRAME, MG_UINT32_FRAME, MG_UINT64_FRAME,
                        MG_INT8_FRAME,  MG_INT16_FRAME,  MG_INT32_FRAME,  MG_INT64_FRAME,
                        MG_FLOAT32_FRAME, MG_FLOAT64_FRAME };
  static double (*sroutines[])(Value_Iterator *)
                    = { MG_UINT8_SLICE, MG_UINT16_SLICE, MG_UINT32_SLICE, MG_UINT64_SLICE,
                        MG_INT8_SLICE,  MG_INT16_SLICE,  MG_INT32_SLICE,  MG_INT64_SLICE,
                        MG_FLOAT32_SLICE, MG_FLOAT64_SLICE };
  int       i;
  Size_Type off, iff;
  MG_It    *g;
  Array    *a = AForm_Array(form);

  g = Guarded_Malloc(sizeof(MG_It),"Matlab_Gradient");

  off = 1;
  for (i = 0; i < dim; i++)
    off *= a->dims[i];
  g->ispan = off;
  g->ospan = off*(a->dims[i] - 2);

  if (Is_Frame(form))
    { Dimn_Type *scrd = ADIMN(Frame_Shape(form));
      Dimn_Type *ccrd = ADIMN(Frame_Anchor(form));
      Offs_Type *data = Frame_Offsets(form);
      Dimn_Type *pcrd = ADIMN(Frame_Coordinate(form));

      iff = 1;
      for (i = 0; i < dim; i++)
        iff  *= scrd[i];
      g->innr = iff;
      g->outr = iff*(scrd[dim] - 2);

      if (Frame_Within_Array(form))
        { g->p = (void *) (data - iff);
          g->s = (void *) (data + iff);

          g->in1 = (ccrd[dim] > pcrd[dim]);
          g->in2 = (pcrd[dim] - ccrd[dim] + scrd[dim] < a->dims[dim]);

          g->next = iroutines[a->type]; 
        }
      else
        { Coordinate *shape = Copy_Array(Frame_Shape(form));   //  NOT CORRECT!!!!
          Coordinate *cntr  = Copy_Array(Frame_Anchor(form));

          ADIMN(shape)[dim] = 1;
          AINT32(cntr)[dim] += 1;
          g->frame2 = Make_Frame(a,shape,cntr);

          shape = Copy_Array(Frame_Shape(form));
          cntr  = Copy_Array(Frame_Anchor(form));

          ADIMN(shape)[dim] = 1;
          AINT32(cntr)[dim] = ccrd[dim] - scrd[dim];
          g->frame1 = Make_Frame(a,shape,cntr);

          g->fdx1 = g->fdx2 = 0;

          g->next = froutines[a->type];
        }

      g->form = Inc_Frame(form);
    }
  else
    { if (Is_Slice(form))
        { Dimn_Type *bcrd = ADIMN(Slice_First(form));
          Dimn_Type *ecrd = ADIMN(Slice_Last(form));

          iff = 1;
          for (i = 0; i < dim; i++)
            iff  *= (ecrd[i]-bcrd[i])+1;
          g->innr = iff;
          g->outr = iff*((ecrd[dim]-bcrd[dim]) - 1);

          g->in1 = (bcrd[dim] > 0);
          g->in2 = (ecrd[dim] < a->dims[dim]-1);
          g->sdx = Set_Slice_To_First(form);

          g->next = sroutines[a->type]; 
          g->form = Inc_Slice(form);
        }
      else
        { g->innr  = g->ispan;
          g->outr  = g->ospan;

          g->next = aroutines[a->type]; 
          g->form = Inc_Array(form);
        }

      off *= type_size[a->type];
      g->p = (void *) ( ((char *) a->data) - off);
      g->s = (void *) ( ((char *) a->data) + off);
    }
 
  g->kill = Kill_MG;

  g->idx   = 0;
  g->state = 2;
  g->bound = g->innr;

  return ((Value_Iterator *) g);
}

  //  Array-based Matlab gradient codes

static Float_Array *G(Matlab_Derivative_Array)(Array *image, int dim)
{ Array    *deriv = Convert_Array(image,image->kind,FLOAT32_TYPE,32,1.);
  float32  *fd    = AFLOAT32(deriv);
  float32  *bd;
  Size_Type area, off, corn, outr;
  double    b0, b1, b2;
  Indx_Type p, q, h, en;

  off = 1;
  for (p = 0; p < dim; p++)
    off *= image->dims[p];
  area = image->size;
  outr = off*image->dims[dim];
  corn = outr - off;

  if (off == outr)
    { fprintf(stderr,"Gradient direction has dimension = 1! (Matlab_Derivative_Array)\n");
      exit (1);
    }

  bd = fd+off;
  for (h = 0; h < area; h += outr)
    for (p = h+off; p-- > h; )
      { en = p+corn;
        b0 = fd[p];
        b1 = bd[p];
        fd[p] = (float) (b1 - b0);
        for (q = p+off; q < en; q += off)
          { b2 = bd[q];
            fd[q] = (float) (.5*(b2-b0));
            b0 = b1;
            b1 = b2;
          }
        fd[en] = (float) (b1-b0);
      }

  return (deriv);
}


/****************************************************************************************
 *                                                                                      *
 *  GENERAL DERIVATIVE ROUTINES                                                         *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { double   (*next)(Value_Iterator *);  // Iterator to invoke
    void     (*kill)(Value_Iterator *);  // Kill routine to invoke

    AForm         *form;
    Double_Vector *filter1;
    Double_Vector *filter0;
    Double_Array  *cache;

    Convolver     *lead;
    Convolver     *edge0;
    Convolver     *edge1;
    Convolver     *corner;

    Indx_Type      p;
    Size_Type      blen;
    Size_Type      width;
    Size_Type      size;

    double        *buffer;
    double        *buffer1;
    double        *buffer0;

    double        *idx;
    double        *end;

  } GG_It;

static double GG_ARRAY(Value_Iterator *ge)
{ GG_It *g = (GG_It *) ge;

  double *x = g->idx++;
  if (x == g->end)
    { double    *buf   = g->buffer;
      double    *buf0  = g->buffer0;
      double    *buf1  = g->buffer1;

      Dimn_Type  len0  = g->filter0->dims[0];
      Dimn_Type  len1  = g->filter1->dims[1];
      Size_Type  len2  = ((Iconvolver *) g->lead)->filter->size;
      Size_Type  blen  = g->blen;

      double    *fval0 = AFLOAT64(g->filter0);
      double    *fval1 = AFLOAT64(g->filter1);

      Indx_Type  p = g->p;
      Indx_Type  r, m, s;
      Indx_Type  i, j;

      if (p == g->size)
        { Place_Convolver(g->edge1,0);
          r = s = 0;
          for (j = 1; j < len1; j++)
            { for (i = 1; i < len0; i++, s += len2)
                { buf0[r++] = Convolve_Segment(g->corner,s,len2);
                  s += len2;
                }
              r = j*blen;
            }
          r = len0-1;
          for (i = 0; i < g->width; i++)
            { s = 0;
              for (j = 1; j < len1; j++)
                { buf0[r] = Convolve_Segment(g->edge1,s,len2);
                  s += len2;
                  r += blen;
                }
              r = len0+i;
              Move_Convolver_Forward(g->edge1);
            }
          p = 0;
          Place_Convolver(g->lead,0);
        }

      m = (p/g->width + len1-1) % len1;
      r = m * blen;
      Place_Convolver(g->edge0,p);
      s = 0;
      for (i = 1; i < len0; i++)
        { buf0[r++] = Convolve_Segment(g->edge0,s,len2);
          s += len2;
        }
      for (i = 0; i < g->width; i++)
        { buf0[r++] = Convolve(g->lead);
          Move_Convolver_Forward(g->lead);
        }

      r  = 0;
      m += 1;
      for (i = 0; i < blen; i++)
        { double sum = 0.;
          for (j = m; j < len1; j++)
            { sum += buf0[r] * fval1[j];
              r   += len1;
            }
          for (j = 0; j < m; j++)
            { sum += buf0[r] * fval1[j];
              r   += len1;
            }
          buf1[i] = sum;
        }

      r = 0;
      for (i = 0; i < g->width; i++)
        { double sum = 0.;
          for (j = 0; j < len0; j++)
            sum += buf1[r+j] * fval0[j];
          buf[i] = sum;
          r += 1;
        }

      g->idx  = buf;
      g->p    = p + g->width;
      x = g->idx++;
    }
  return (*x);
}

static double GG_SLICE(Value_Iterator *ge)
{ GG_It *g = (GG_It *) ge;

  double *x = g->idx++;
  if (x == g->end)
    { double    *buf   = g->buffer;
      double    *buf0  = g->buffer0;
      double    *buf1  = g->buffer1;

      Dimn_Type  len0  = g->filter0->dims[0];
      Dimn_Type  len1  = g->filter1->dims[1];
      Size_Type  len2  = ((Iconvolver *) g->lead)->filter->size;
      Size_Type  blen  = g->blen;

      double    *fval0 = AFLOAT64(g->filter0);
      double    *fval1 = AFLOAT64(g->filter1);

      Indx_Type  p = g->p;
      Indx_Type  r, m, s;
      Indx_Type  i, j;

      if (p == g->size)
        { r = Set_Slice_To_First(g->form);
          Place_Convolver(g->edge1,r);
          Place_Convolver(g->lead,r);

          r = s = 0;
          for (j = 1; j < len1; j++)
            { for (i = 1; i < len0; i++, s += len2)
                { buf0[r++] = Convolve_Segment(g->corner,s,len2);
                  s += len2;
                }
              r = j*blen;
            }
          r = len0-1;
          for (i = 0; i < g->width; i++)
            { s = 0;
              for (j = 1; j < len1; j++)
                { buf0[r] = Convolve_Segment(g->edge1,s,len2);
                  s += len2;
                  r += blen;
                }
              r = len0+i;
              Move_Convolver_Forward(g->edge1);
            }
          p = 0;
        }

      Place_Convolver(g->edge0,Slice_Index(g->form));

      r = Slice_Index(g->form);
      Place_Convolver(g->lead,r);
      Set_Slice_To_Index(g->form,r+(g->width-1));
      Next_Slice_Index(g->form);

      m = (p/g->width + len1-1) % len1;
      r = m * blen;
      s = 0;
      for (i = 1; i < len0; i++)
        { buf0[r++] = Convolve_Segment(g->edge0,s,len2);
          s += len2;
        }
      for (i = 0; i < g->width; i++)
        { buf0[r++] = Convolve(g->lead);
          Move_Convolver_Forward(g->lead);
        }

      r  = 0;
      m += 1;
      for (i = 0; i < blen; i++)
        { double sum = 0.;
          for (j = m; j < len1; j++)
            { sum += buf0[r] * fval1[j];
              r   += len1;
            }
          for (j = 0; j < m; j++)
            { sum += buf0[r] * fval1[j];
              r   += len1;
            }
          buf1[i] = sum;
        }

      r = 0;
      for (i = 0; i < g->width; i++)
        { double sum = 0.;
          for (j = 0; j < len0; j++)
            sum += buf1[r+j] * fval0[j];
          buf[i] = sum;
          r += 1;
        }

      g->idx  = buf;
      g->p    = p + g->width;
      x = g->idx++;
    }
  return (*x);
}

static double GG_FRAME(Value_Iterator *ge)
{ GG_It *g = (GG_It *) ge;

  double *x = g->idx++;
  if (x == g->end)
    { g->idx = g->buffer;
      g->p   = 0;
      x = g->idx++;
    }
  return (*x);
}

static void Kill_GG(Value_Iterator *ge)
{ GG_It    *g = (GG_It *) ge;

  if (Is_Frame(g->form))
    { Free_Frame(g->form);
      Free_Array(g->cache);
    }
  else
    { if (Is_Slice(g->form))
        Free_Slice(g->form);
      else
        Free_Array(g->form);
      Kill_Convolver(g->lead);
      Kill_Convolver(g->edge0);
      Kill_Convolver(g->edge1);
      Kill_Convolver(g->corner);
      Kill_Array(g->filter0);
      Kill_Array(g->filter1);
      free(g->buffer);
    }

  free(g);
}

Value_Iterator *General_Convolver(AForm *image, Double_Vector **F(filters))
{ GG_It       *g;
  Dimn_Type    len0, len1;
  Array       *a = AForm_Array(image);

  g = Guarded_Malloc(sizeof(GG_It),"General_Convolver");
  g->size = AForm_Size(image);

  if (Is_Slice(image))
    { g->form  = Inc_Slice(image);
      g->next  = GG_SLICE; 
      g->width = (ADIMN(Slice_Last(image))[0] - ADIMN(Slice_First(image))[0]) + 1;
    }
  else if (Is_Frame(image))
    { g->form  = Inc_Frame(image);
      g->next  = GG_FRAME;
    }
  else
    { g->form  = Inc_Array(image);
      g->next  = GG_ARRAY; 
      g->width = a->dims[0]; 
    }
  g->kill  = Kill_GG;

  if (Is_Frame(image))
    { Coordinate *new_shape  = Copy_Array(Frame_Shape(image));
      Coordinate *new_center = Copy_Array(Frame_Anchor(image));
      Coordinate *beg        = Copy_Array(Frame_Shape(image));
      Coordinate *end        = Copy_Array(Frame_Shape(image));
      Array      *echo;
      Frame      *frame;
      int i;

      for (i = 0; i < a->ndims; i++)
        { Dimn_Type x, y;
          x = filters[i]->dims[0];
          y = x/2;
          ADIMN(beg)[i] = y;
          ADIMN(end)[i] += y-1;
          ADIMN(new_shape)[i] += x;
          ADIMN(new_center)[i] += y;
        }

      frame = Make_Frame(a,new_shape,new_center);
      Place_Frame(frame,Frame_Index(image));
      echo = Make_Array_From_Frame(frame);
      Free_Frame(frame);

      echo->kind = PLAIN_KIND;
      Convert_Array_Inplace(echo,PLAIN_KIND,FLOAT64_TYPE,64,1.);

      for (i = 0; i < a->ndims; i++)
        Filter_Dimension(echo,filters[i],i);

      Clip_Array_Inplace(echo,beg,end);

      g->buffer = AFLOAT64(echo);
      g->cache  = echo;
    }

  else
    { Coordinate    *center;
      Double_Array  *filter;
      Double_Vector *vec0, *vec1;
      int            i;

      filter = Box_Filter(0);
      center = Make_Array_With_Shape(PLAIN_KIND,INT32_TYPE,Coord1(a->ndims));
      for (i = 2; i < a->ndims; i++)
        { ADIMN(center)[i] = filters[i]->dims[0]/2;
          filter = Filter_Product(filters[i],filter);
        }

      g->filter0 = Copy_Array(filters[0]);
      g->filter1 = Copy_Array(filters[1]);
      Free_Array(filters[0]);
      Free_Array(filters[1]);
  
      len0 = g->filter0->dims[0];
      len1 = g->filter1->dims[0];
  
      vec0 = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(len0-1));
      for (i = 0; i < len0-1; i++)
        AFLOAT64(vec0)[i] = 1.;
  
      vec1 = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(len1-1));
      for (i = 0; i < len1-1; i++)
        AFLOAT64(vec1)[i] = 1.;
  
      ADIMN(center)[0] = len0/2 - len0;
      ADIMN(center)[1] = len1/2 - len1;
      g->lead  = Make_Convolver(a,Copy_Array(filter),Copy_Array(center));
  
      ADIMN(center)[0] = len0/2;
      g->edge1 = Make_Convolver(a,Filter_Product(Inc_Array(vec1),Inc_Array(filter)),
                                    Copy_Array(center));
  
      ADIMN(center)[0] = len0/2 - len0;
      ADIMN(center)[1] = len1/2;
      filter = Filter_Product(vec0,filter);
      g->lead  = Make_Convolver(a,Copy_Array(filter),Copy_Array(center));
  
      ADIMN(center)[0] = len0/2;
      g->corner = Make_Convolver(a,Filter_Product(vec1,filter),center);
  
      g->blen    = g->width + (len0-1);
      g->buffer  = (double *) Guarded_Malloc(sizeof(double)*((size_t) (g->blen*(len1+1)+g->width)),
                                             "General_Convolver");
      g->buffer1 = g->buffer  + g->width;
      g->buffer0 = g->buffer1 + g->blen;

      g->end  = g->buffer + g->width;

      Place_Convolver(g->corner,0);
    }

  g->idx  = g->buffer;
  g->p    = g->size;

  return ((Value_Iterator *) g);
}

Vector_Iterator *General_Vector(int n, Value_Iterator *C(v1), ... X( Value_Iterator *C(vn) ) )
{ VEC_It *g = start_vector(n,"General_Vector");
  int     i;
  va_list ap;

  g->mg[0] = v1;
  va_start(ap,v1);
  for (i = 1; i < n; i++)
    g->mg[i] = va_arg(ap,Value_Iterator *);
  return ((Vector_Iterator *) g);
}


/****************************************************************************************
 *                                                                                      *
 *  DERIVATIVE ROUTINES BASED ON GENERAL CASE                                           *
 *                                                                                      *
 ****************************************************************************************/

Value_Iterator *Derivative(D_Scheme *scheme, AForm *I(image), int dim)
{ Value_Iterator *con;

  setup_schemes();
  if (scheme == Matlab)
    con = Matlab_Derivative(image,dim);
  else
    { int             i, ndims;
      Double_Vector **vec, *Vec[10];

      ndims = AForm_Array(image)->ndims;
      if (ndims > 10)
        vec = (Vector **) Guarded_Malloc(sizeof(Vector *)*((size_t) ndims),"Derivative");
      else
        vec = Vec;
      for (i = 0; i < ndims; i++)
        if (dim == i)
          vec[i] = Inc_Array(scheme->deriv);
        else
          vec[i] = Inc_Array(scheme->base);
      con = General_Convolver(image,vec);
      if (ndims > 10)
        free(vec);
    }
  return (con);
}

Vector_Iterator *Gradient(D_Scheme *scheme, AForm *image)
{ VEC_It *g = start_vector(AForm_Array(image)->ndims,"Gradient");
  int     i;

  if (scheme == Matlab)
    for (i = 0; i < g->ndims; i++)
      g->mg[i] = Matlab_Derivative(image,i);
  else
    for (i = 0; i < g->ndims; i++)
      g->mg[i] = Derivative(scheme,image,i);

  return ((Vector_Iterator *) g);
}

Float_Array *G(Derivative_Array)(D_Scheme *scheme, Array *image, int dim)
{ Array *deriv;
  int i;

  setup_schemes();
  if (scheme == Matlab)
    deriv = Matlab_Derivative_Array(image,dim);
  else
    { deriv = Convert_Array(image,image->kind,FLOAT32_TYPE,32,1.);
      if (scheme->base->dims[0] == 1)
          Filter_Dimension(deriv,Inc_Array(scheme->deriv),0);
      else
        for (i = 0; i < image->ndims; i++)
          if (i == dim)
            Filter_Dimension(deriv,Inc_Array(scheme->deriv),i);
          else
            Filter_Dimension(deriv,Inc_Array(scheme->base),i);
    }
  return (deriv);
}

Float_Array *G(Gradient_Array)(D_Scheme *scheme, Float_Array *image)
{ Array *g;
  int    i, n;
 
  n = image->ndims;
  g = Make_Array_With_Shape(PLAIN_KIND,FLOAT32_TYPE,AppendCoord(n,AForm_Shape(image)));
  for (i = 0; i < n; i++)
    { Array *gradi = Get_Array_Plane(g,i);
      Array_Op_Array(gradi,SET_OP,image);
      Derivative_Array(scheme,gradi,i);
    }
  return (g);
}

/****************************************************************************************
 *                                                                                      *
 *  PARTIAL DERIVATIVES                                                                 *
 *                                                                                      *
 ****************************************************************************************/

/*

static void filter_trim(Double_Vector *filter, double threshold)
{ double   *a = AFLOAT64(filter);
  Indx_Type i, j, n;
  double    s, x;

  i = 0;
  while (a[i] < threshold)
    i += 1;
  j = filter->dims[0];
  while (a[j] < threshold)
    j -= 1;

  n = 0;
  s = 0.;
  while (i <= j)
    { x = a[i++];
      s += x;
      a[n++] = x;
    }

  for (i = 0; i < n; i++)
    a[i] /= s;
  
  filter->size = filter->dims[0] = n;
}

*/

static void filter_scale(Double_Vector *filter, int ndim)
{ double   *a = AFLOAT64(filter);
  Size_Type n = filter->dims[0];
  Indx_Type i, j, k;
  double    den = 1./ndim;

  j = k = 0;
  for (i = 1; i < n; i++)
    { if (i%ndim == 0)
        { j += 1;
          k = 0;
        }
      else if (i%ndim == 1)
        { a[j] += a[i] * ((ndim-k)*den); 
          a[j+1] = a[i] * (k*den); 
        }
      else
        { a[j] += a[i] * ((ndim-k)*den); 
          a[j+1] += a[i] * (k*den); 
        }
      k += 1;
    }

  filter->size = filter->dims[0] = (Dimn_Type) (n/ndim+1);
}

Value_Iterator *Partial(D_Scheme *scheme, AForm *image, int n, int x1, ... X(int xn))
{ Double_Vector **filters, *Filters[10];
  int            *parts, Parts[10];
  Value_Iterator *it;

  va_list ap;
  int     ndims = AForm_Array(image)->ndims;
  int     i, j;

  setup_schemes();

  if (ndims > 10)
    { parts    = (int *) Guarded_Malloc(sizeof(int)*((size_t) ndims),"Partial");
      filters  = (Double_Vector **) Guarded_Malloc(sizeof(void *)*((size_t) ndims),"Partial");
    }
  else
    { parts    = Parts;
      filters  = Filters;
    }
 
  for (i = 0; i < ndims; i++)
    parts[i] = 0; 

  va_start(ap,x1);
  parts[x1] += 1;
  for (i = 1; i < n; i++)
    parts[va_arg(ap,int)] += 1;
  va_end(ap);

  for (i = 0; i < ndims; i++)
    { filters[i] = Box_Filter(0);
      for (j = 0; j < parts[i]; j++)
        filters[i] = Filter_Convolution(filters[i],Inc_Array(scheme->deriv));
      while (j++ < n)
        filters[i] = Filter_Convolution(filters[i],Inc_Array(scheme->base));

      filter_scale(filters[i],ndims);
    }

  it = General_Convolver(image,filters);

  if (ndims > 10)
    { free(filters);
      free(parts);
    }

  return (it);
}

Float_Array *G(Partial_Array)(D_Scheme *scheme, Array *image, int n, int x1, ... X(int xn))
{ int *parts, Parts[10];

  Double_Vector *filter;
  int            ndims = image->ndims;
  int            i, j;
  va_list        ap;

  Array *partial = Convert_Array(image,image->kind,FLOAT32_TYPE,32,1.);

  setup_schemes();

  if (ndims > 10)
    parts    = (int *) Guarded_Malloc(sizeof(int)*((size_t) ndims),"Partial");
  else
    parts    = Parts;
 
  for (i = 0; i < ndims; i++)
    parts[i] = 0; 

  parts[x1] += 1;
  va_start(ap,x1);
  for (i = 1; i < n; i++)
    parts[va_arg(ap,int)] += 1;
  va_end(ap);

  for (i = 0; i < ndims; i++)
    { filter = Box_Filter(0);
      for (j = 0; j < parts[i]; j++)
        filter = Filter_Convolution(filter,Inc_Array(scheme->deriv));
      while (j++ < n)
        filter = Filter_Convolution(filter,Inc_Array(scheme->base));

      filter_scale(filter,ndims);

      Filter_Dimension(partial,filter,i);
    }

  if (ndims > 10)
    free(parts);

  return (partial);
}

/****************************************************************************************
 *                                                                                      *
 *  LAPLACIAN ITERATOR                                                                  *
 *                                                                                      *
 ****************************************************************************************/

double Magnitude(Double_Vector *v)
{ Indx_Type p;
  double    s, d;
  double   *f = AFLOAT64(v);

  s = 0.;
  for (p = 0; p < v->size; p++)
    { d = f[p];
      s += d*d;
    }
  return (s);
}

typedef struct
  { double         (*next)(Value_Iterator *);  // Iterator to invoke
    void           (*kill)(Value_Iterator *);  // Kill routine to invoke
    Vector_Iterator *vec;
  } Mag_It;

static double Mag_Next(Value_Iterator *it)
{ Mag_It *g = (Mag_It *) it;
  return (Magnitude(NEXT_VECTOR(g->vec)));
}

static void Mag_Kill(Value_Iterator *it)
{ Mag_It *g = (Mag_It *) it;
  KILL_VECTOR(g->vec);
  free(g);
}

Value_Iterator *Magnitude_Iterator(Vector_Iterator *it)
{ Mag_It *g;
  g = (Mag_It *) Guarded_Malloc(sizeof(Mag_It),"Magnitude_Iterator");
  g->vec = it;
  g->next = Mag_Next;
  g->kill = Mag_Kill;
  return ((Value_Iterator *) g);
}

typedef struct
  { double   (*next)(Value_Iterator *);  // Iterator to invoke
    void     (*kill)(Value_Iterator *);  // Kill routine to invoke

    int              ndims;
    Value_Iterator **part;
  } LP_It;

static void Kill_LP(Value_Iterator *ge)
{ LP_It *g = (LP_It *) ge;
  int    i;

  for (i = 0; i < g->ndims; i++)
    KILL_VALUE(g->part[i]);
}

static double Next_LP(Value_Iterator *ge)
{ LP_It *g = (LP_It *) ge;
  int    i;
  double sum;

  sum = 0.;
  for (i = 0; i < g->ndims; i++)
    sum += NEXT_VALUE(g->part[i]);
  return (sum);
}

Value_Iterator *Laplacian(D_Scheme *scheme, AForm *I(image))
{ LP_It *g;
  int    i;

  g        = (LP_It *) Guarded_Malloc(sizeof(LP_It),"Laplacian");
  g->ndims = AForm_Array(image)->ndims;
  g->part  = (Value_Iterator **) Guarded_Malloc(sizeof(void *)*((size_t) g->ndims),"Laplacian");
  for (i = 0; i < g->ndims; i++)
    g->part[i] = Partial(scheme,image,2,i,i);
  g->next = Next_LP;
  g->kill = Kill_LP;
  return ((Value_Iterator *) g);
}

Float_Array *G(Laplacian_Array)(D_Scheme *scheme, Array *image)
{ Array *laplace, *partial;
  int    i;

  laplace = Convert_Array(image,image->kind,FLOAT32_TYPE,32,1.);
  Partial_Array(scheme,laplace,2,0,0);
  if (image->ndims > 1)
    { partial = Make_Array(PLAIN_KIND,FLOAT32_TYPE,image->ndims,image->dims);
      for (i = 1; i < image->ndims; i++)
        { Array_Op_Array(partial,SET_OP,image);
          Partial_Array(scheme,partial,2,i,i);
          Array_Op_Array(laplace,ADD_OP,partial);
        }
      Kill_Array(partial);
    }
  return (laplace);
}


/****************************************************************************************
 *                                                                                      *
 *  CURVATURE, and FLOW_FIELD                                                           *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { double   (*next)(Value_Iterator *);  // Iterator to invoke
    void     (*kill)(Value_Iterator *);  // Kill routine to invoke

    Value_Iterator *fxx;
    Value_Iterator *fxy;
    Value_Iterator *fyy;
    Value_Iterator *fx;
    Value_Iterator *fy;
  } LC_It;

static void Kill_LC_2(Value_Iterator *ge)
{ LC_It *g = (LC_It *) ge;

  KILL_VALUE(g->fxx);
  KILL_VALUE(g->fxy);
  KILL_VALUE(g->fyy);
  KILL_VALUE(g->fx);
  KILL_VALUE(g->fy);
}

static double Next_LC_2(Value_Iterator *ge)
{ LC_It *g = (LC_It *) ge;

  double fxx = NEXT_VALUE(g->fxx);
  double fxy = NEXT_VALUE(g->fxy);
  double fyy = NEXT_VALUE(g->fyy);
  double fx  = NEXT_VALUE(g->fx);
  double fy  = NEXT_VALUE(g->fy);
  double fx2 = fx*fx;
  double fy2 = fy*fy;

  return (fyy*fx2 - 2.*fxy*fx*fy + fxx*fy2) / pow(fx2 + fy2,1.5);
}

Value_Iterator *G(Levelset_Curvature)(D_Scheme *scheme, Array *image)
{ LC_It *g = (LC_It *) Guarded_Malloc(sizeof(LC_It),"LevelSet_Curvature");
  g->fxx  = Partial(scheme,image,2,0,0);
  g->fxy  = Partial(scheme,image,2,0,1);
  g->fyy  = Partial(scheme,image,2,1,1);
  g->fx   = Derivative(scheme,image,0);
  g->fy   = Derivative(scheme,image,1);
  g->next = Next_LC_2;
  g->kill = Kill_LC_2;
  return ((Value_Iterator *) g);
}

Float_Array *G(Levelset_Curvature_Array)(Array *image, Float_Array *grad)
{ Array    *curv;
  int       ndims;
  Size_Type area;

  ndims = image->ndims;
  area  = image->size;
  curv  = Make_Array(PLAIN_KIND,FLOAT32_TYPE,ndims,image->dims);

/*
  if (ndims == 2)
    { D_Scheme *scheme;
      Array *grad = Gradient_Array(scheme,image);
      Array *curv = Partial_Array(scheme,image,2,0,1);
      Array_Op_Array(curv,MUL_OP,Get_Array_Plane(grad,0));
      Array_Op_Array(curv,MUL_OP,Get_Array_Plane(grad,1));
      Array_Op_Array(grad,MUL_OP,grad);
      Array *part = Partial_Array(scheme,image,2,0,0);
      Array_Op_Array(part,MUL_OP,Get_Array_Plane(grad,0));
      Array_Op_Array(curv,SUB_OP,part);
      Array *part = Partial_Array(scheme,image,2,1,1);
      Array_Op_Array(part,MUL_OP,Get_Array_Plane(grad,1));
      Array_Op_Array(curv,SUB_OP,part);
      Array_Op_Array(Get_Plane_Array(grad,0),ADD_OP,Get_Plane_Array(grad,1));
      Array_Op_Scalar(Get_Plane_Array(grad,0),POW_OP,FVAL,VALF(1.5));
      Array_Op_Array(curv,DIV_OP,Get_Plane_Array(grad,0));
      Kill_Array(grad);
      Kill_Array(part);
    } 
*/

  if (ndims == 2)
    { Dimn_Type width   = image->dims[0];
      Dimn_Type height  = image->dims[1];
      Dimn_Type cwidth  = width-1;
      Dimn_Type cheight = height-1;
  
      float32 *fx  = AFLOAT32(grad);
      float32 *fy  = fx + area;
      float32 *crv = AFLOAT32(curv);

      float32 *fxpx = fx + 1;
      float32 *fxmx = fx - 1;
      float32 *fypx = fy + 1;
      float32 *fymx = fy - 1;
      float32 *fypy = fy + width;
      float32 *fymy = fy - width;
  
      Indx_Type p;
      Dimn_Type i, j;
      double    cx, cy, cx2, cy2, fxx, fxy, fyy;
  
      p = 0;
      for (j = 0; j < height; j++)
        for (i = 0; i < width; i++, p++)

          { cx  = fx[p];
            cy  = fy[p];
            cx2 = cx*cx;
            cy2 = cy*cy;

            if (i == 0)
              { fxx = cx - fxpx[p];
                fxy = cy - fypx[p];
              }
            else if (i == cwidth)
              { fxx = fxmx[p] - cx;
                fxy = fymx[p] - cy;
              }
            else
              { fxx = .5 * (fxmx[p] - fxpx[p]);
                fxy = .5 * (fymx[p] - fypx[p]);
              }

            if (j == 0)
              fyy = cy - fypy[p];
            else if (j == cheight)
              fyy = fymy[p] - cy;
            else
              fyy = .5 * (fymy[p] - fypy[p]);

            crv[p] = (float) ((fyy*cx2 - 2.*fxy*cx*cy + fxx*cy2) / pow(cx2 + cy2,1.5));
          }
    }

  else if (ndims == 3)
    { Dimn_Type  width  = image->dims[0];
      Dimn_Type height  = image->dims[1];
      Dimn_Type depth   = image->dims[1];
      Dimn_Type cwidth  = width-1;
      Dimn_Type cheight = height-1;
      Dimn_Type cdepth  = depth-1;
  
      float32 *fx  = AFLOAT32(grad);
      float32 *fy  = fx + area;
      float32 *fz  = fy + area;
      float32 *crv = AFLOAT32(curv);

      float32 *fyp = fy + width;
      float32 *fym = fy - width;
      float32 *fzp = fz + width;
      float32 *fzm = fz - width;
      float32 *fzo = fz + ((Size_Type) width)*height;
      float32 *fzn = fz - ((Size_Type) width)*height;
  
      Indx_Type p;
      Dimn_Type i, j, k;
      double    cx, cy, cz, cx2, cy2, cz2, fxx, fyy, fzz, fxy, fxz, fyz;
  
      p = 0;
      for (k = 0; k < depth; k++)
       for (j = 0; j < height; j++)
        for (i = 0; i < width; i++, p++)

          { cx  = fx[p];
            cy  = fy[p];
            cz  = fz[p];
            cx2 = cx*cx;
            cy2 = cy*cy;
            cz2 = cz*cz;

            if (i == 0)
              { fxx = cx - fx[p+1];
                fxy = cy - fy[p+1];
                fxz = cz - fz[p+1];
              }
            else if (i == cwidth)
              { fxx = fx[p-1] - cx;
                fxy = fy[p-1] - cy;
                fxz = fz[p-1] - cz;
              }
            else
              { fxx = .5 * (fx[p-1] - fx[p+1]);
                fxy = .5 * (fy[p-1] - fy[p+1]);
                fxz = .5 * (fz[p-1] - fz[p+1]);
              }

            if (j == 0)
              { fyy = cy - fyp[p];
                fyz = cy - fzp[p];
              }
            else if (j == cheight)
              { fyy = fym[p] - cy;
                fyz = fzm[p] - cy;
              }
            else
              { fyy = .5 * (fym[p] - fyp[p]);
                fyz = .5 * (fzm[p] - fzp[p]);
              }

            if (k == 0)
              fzz = cz - fzo[p];
            else if (k == cdepth)
              fzz = fzn[p] - cz;
            else
              fzz = .5 * (fzn[p] - fzo[p]);

            crv[p] =  (float) ((      ( cx2*(fyy*fzz-fyz*fyz)   + cy2*(fxx*fzz-fxz*fxz)
                                                                + cz2*(fxx*fyy-fxy*fxy)
                                      )
                                 - 2.*( cx*cy*(fxy*fzz-fxz*fyz) + cx*cz*(fxy*fyz-fxz*fyy)
                                                                + cy*cz*(fxx*fyz-fxy*fxz)
                                      )
                               ) / pow(cx2 + cy2 + cz2,2.0));
          }
    }

  else  // ndims > 3 -- hold on to your hat!
    { Double_Matrix  *H, *R, *P;
      LU_Factor      *lu;
      float32        *f = AFLOAT32(grad);
      float32        *crv = AFLOAT32(curv);
      Dimn_Type      *dims = image->dims;
      Indx_Type       p;

      H = Identity_Matrix(ndims);
      R = Identity_Matrix(ndims);
  
      for (p = 0; p < area; p++)
        {
          { float32  *g, *h;
            float64  *s;
            Indx_Type q, o, r;
            Indx_Type i, n;
            int       d, e;
  
            q = p;                      //  Compute lower half of the symmetric Hessian
            s = AFLOAT64(H);
            o = 1;
            for (d = 0; d < ndims; d++)
              { n = dims[d];
                i = q%n;
                q = q/n;
                g = f + o;
                h = f - o;
                o = o*n;
  
                if (i == 0)
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = f[r] - g[r];
                else if (i == n-1)
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = h[r] - f[r];
                else
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = .5 * (h[r] - g[r]);
                s += ndims;
              }
          }
  
          { float64  *s, *t;        //  Fill in the symmetric half of the Hessian
            int       d, e, w;

            s = t = (float64 *) (H->data);
            for (d = 0; d < ndims; d++)
              { for (w = 0, e = 0; e < d; e++, w += ndims)
                  t[w] = s[e];
                s += ndims;
                t += 1;
              }
          }
  
          { float64  *r, v, fd; //  The Gaussian curvature is the determinant of the Hessian
            int       d, e;     //    along an orthonormal set of vectors perpendicular to the
            int       u, w;     //    Jacobian
            Indx_Type n;
            boolean   stable;

            w = 0;
            r = AFLOAT64(R);
            for (e = (int) (R->size-ndims), n = p; e < R->size; e++, n += area)
              { r[e] = f[n];
                if (r[e] >= r[w])
                  w = e;
              }
            for (d = 0; d < R->size - ndims; d++)
              r[e] = 0;
            u = 0;
            for (d = 0; d < ndims-1; d++)
              { if (u == w) u += 1;
                r[d*ndims+u] = 1.;
              }

            Orthogonalize_Matrix(R);

            R->dims[1] = ndims-1;
            R->size   -= ndims;
 
            P = Array_Multiply(R,H);

            for (d = 0; d < ndims; d++)
              for (e = 0; e < ndims; e++)
                { v = r[d*ndims+e];
                  r[d*ndims+e] = r[e*ndims+d];
                  r[e*ndims+d] = v;
                }

            R->dims[1] = ndims;
            R->dims[0] = ndims-1;

            lu = LU_Decompose(Array_Multiply(P,R),&stable);
            
            v = 0.;
            n = p;
            for (d = 0; d < ndims; d++)
              { fd = f[n];
                v  += fd*fd;
                n  += area;
              }

            crv[p] = (float) (LU_Determinant(lu) / v);

            Free_LU_Factor(lu);
            Free_Array(P);
          }
        }
    }

  return (curv);
}

Float_Array *G(Surface_Curvature_Array)(Array *image, Float_Array *grad)
{ Array    *curv;
  int       ndims;
  Size_Type area;

  ndims = image->ndims;
  area  = image->size;
  curv  = Make_Array(PLAIN_KIND,FLOAT32_TYPE,ndims,image->dims);

  if (ndims == 2)
    { Dimn_Type width   = image->dims[0];
      Dimn_Type height  = image->dims[1];
      Dimn_Type cwidth  = width-1;
      Dimn_Type cheight = height-1;
  
      float32 *fx  = AFLOAT32(grad);
      float32 *fy  = fx + area;
      float32 *crv = AFLOAT32(curv);

      float32 *fxpx = fx + 1;
      float32 *fxmx = fx - 1;
      float32 *fypx = fy + 1;
      float32 *fymx = fy - 1;
      float32 *fypy = fy + width;
      float32 *fymy = fy - width;
  
      Indx_Type p;
      Dimn_Type i, j;
      double    cx, cy, cx2, cy2, fxx, fxy, fyy;
  
      p = 0;
      for (j = 0; j < height; j++)
        for (i = 0; i < width; i++, p++)

          { cx  = fx[p];
            cy  = fy[p];
            cx2 = cx*cx;
            cy2 = cy*cy;

            if (i == 0)
              { fxx = cx - fxpx[p];
                fxy = cy - fypx[p];
              }
            else if (i == cwidth)
              { fxx = fxmx[p] - cx;
                fxy = fymx[p] - cy;
              }
            else
              { fxx = .5 * (fxmx[p] - fxpx[p]);
                fxy = .5 * (fymx[p] - fypx[p]);
              }

            if (j == 0)
              fyy = cy - fypy[p];
            else if (j == cheight)
              fyy = fymy[p] - cy;
            else
              fyy = .5 * (fymy[p] - fypy[p]);

            crv[p] = (float) ((fxx*fyy - fxy*fxy) / pow(1. + cx2 + cy2,1.5));
          }
    }

  else if (ndims == 3)
    { Dimn_Type width   = image->dims[0];
      Dimn_Type height  = image->dims[1];
      Dimn_Type depth   = image->dims[1];
      Dimn_Type cwidth  = width-1;
      Dimn_Type cheight = height-1;
      Dimn_Type cdepth  = depth-1;
  
      float32 *fx  = AFLOAT32(grad);
      float32 *fy  = fx + area;
      float32 *fz  = fy + area;
      float32 *crv = AFLOAT32(curv);

      float32 *fyp = fy + width;
      float32 *fym = fy - width;
      float32 *fzp = fz + width;
      float32 *fzm = fz - width;
      float32 *fzo = fz + ((Size_Type) width)*height;
      float32 *fzn = fz - ((Size_Type) width)*height;
  
      Indx_Type p;
      Dimn_Type i, j, k;
      double    cx, cy, cz, cx2, cy2, cz2, fxx, fyy, fzz, fxy, fxz, fyz;
  
      p = 0;
      for (k = 0; k < depth; k++)
       for (j = 0; j < height; j++)
        for (i = 0; i < width; i++, p++)

          { cx  = fx[p];
            cy  = fy[p];
            cz  = fz[p];
            cx2 = cx*cx;
            cy2 = cy*cy;
            cz2 = cz*cz;

            if (i == 0)
              { fxx = cx - fx[p+1];
                fxy = cy - fy[p+1];
                fxz = cz - fz[p+1];
              }
            else if (i == cwidth)
              { fxx = fx[p-1] - cx;
                fxy = fy[p-1] - cy;
                fxz = fz[p-1] - cz;
              }
            else
              { fxx = .5 * (fx[p-1] - fx[p+1]);
                fxy = .5 * (fy[p-1] - fy[p+1]);
                fxz = .5 * (fz[p-1] - fz[p+1]);
              }

            if (j == 0)
              { fyy = cy - fyp[p];
                fyz = cy - fzp[p];
              }
            else if (j == cheight)
              { fyy = fym[p] - cy;
                fyz = fzm[p] - cy;
              }
            else
              { fyy = .5 * (fym[p] - fyp[p]);
                fyz = .5 * (fzm[p] - fzp[p]);
              }

            if (k == 0)
              fzz = cz - fzo[p];
            else if (k == cdepth)
              fzz = fzn[p] - cz;
            else
              fzz = .5 * (fzn[p] - fzo[p]);

            crv[p] = (float)
                       ((fxx*fyy*fzz + 2.*fxy*fxz*fyz - (fxx*fyz*fyz + fyy*fxz*fxz + fzz*fxy*fxy))
                         / pow(1. + cx2 + cy2 + cz2,2.0));
          }
    }

  else  // ndims > 3 -- hold on to your hat!
    { Double_Matrix  *H;
      LU_Factor      *lu;
      float32        *f    = AFLOAT32(grad);
      float32        *crv  = AFLOAT32(curv);
      float64         N, den;
      Dimn_Type      *dims = image->dims;
      Indx_Type       p;
      boolean         stable;

      H = Identity_Matrix(ndims);
      N = (ndims+1.)/2.;
  
      for (p = 0; p < area; p++)
        {
          { float32  *g, *h;
            float64  *s;
            Indx_Type q, o, r;
            Indx_Type i, n;
            int       d, e;
  
            q = p;                      //  Compute lower half of the symmetric Hessian
            s = (float64 *) (H->data);
            o = 1;
            for (d = 0; d < ndims; d++)
              { n = dims[d];
                i = q%n;
                q = q/n;
                g = f + o;
                h = f - o;
                o = o*n;
  
                if (i == 0)
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = f[r] - g[r];
                else if (i == n-1)
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = h[r] - f[r];
                else
                  for (e = 0, r = p; e <= d; e++, r += area)
                    s[e] = .5 * (h[r] - g[r]);
                s += ndims;
              }
          }
  
          { float64  *s, *t, fd;       //  Fill in the symmetric half of the Hessian
            int       d, e, w;         //    & compute 1 + Laplacian
            Indx_Type n;

            den = 1.;
            s = t = (float64 *) (H->data);
            for (d = 0, n = p; d < ndims; d++, n += area)
              { for (w = 0, e = 0; e < d; e++, w += ndims)
                  t[w] = s[e];
                s += ndims;
                t += 1;
                fd   = f[n];
                den += fd*fd;
              }
          }
  
          lu = LU_Decompose(Inc_Array(H),&stable);
          crv[p] = (float) (LU_Determinant(lu) / pow(den,N));
          Free_LU_Factor(lu);
        }

      Free_Array(H);
    }

  return (curv);
}

Float_Array *G(Gradient_Flow_Field_Array)(Array *image, double mu)
{ Array *normalized;
  Array *grad_stack, *force_stack;

  if (image->type != FLOAT32_TYPE)
    normalized = Convert_Array(image,PLAIN_KIND,FLOAT32_TYPE,32,1.);
  else
    normalized = Copy_Array(image);
  Scale_Array_To_Range(normalized,VALF(0.),VALF(1.));

  grad_stack = Gradient_Array(Sobel,normalized);

  Free_Array(normalized);

  force_stack = Copy_Array(grad_stack);

  { float32      *u, *du, *fu, *mag;
    Array        *lap, *gs2;
    Array_Bundle *plane;
    double        maxdel;
    int           d, ndims;
    Indx_Type     area, p;

    area   = image->size;
    ndims  = image->ndims;
    maxdel = 1.;
    while (maxdel > .001)
      { maxdel = 0.;

        // gs2 = Vector_Magnitude(grad_stack);
        gs2 = Copy_Array(grad_stack);   //   Not correct, place holder !!!
        fu  = AFLOAT32(grad_stack);
        mag = AFLOAT32(gs2);

        for (d = 0; d < ndims; d++)
          { plane = Get_Array_Plane(force_stack,d);
            lap   = Laplacian_Array(Matlab,plane);

            u  = AFLOAT32(plane);
            du = AFLOAT32(lap);

            for (p = 0; p < area; p++)
              { double del;

                del  = mu * du[p] + mag[p] * (u[p] - fu[p]);
                u[p] = (float) (u[p] + del);
                del  = fabs(del);
                if (del > maxdel)
                  maxdel = del;
              }

            fu += area;

            Free_Array(lap);
          }

        Free_Array(gs2);
      }
  }

  Free_Array(grad_stack);

  return (force_stack);
}


/*****************************************************************************************\
*                                                                                         *
*  Linear time distance transforms                                                        *
*     Implementation of method in "Distance transforms of sampled functions," by          *
*         Pedro F. Felzenszwalb and Daniel P. Hutenlocher                                 *
*                                                                                         *
*  Author:  Fuhui Long, rewrite and refinement by Gene Myers                              *
*  Date  :  Dec 2009                                                                      *
*                                                                                         *
\*****************************************************************************************/


//  Compute the distance transform of 1d vector a[0 to size by step] with respect to
//     squared Euclidean distance.  The result is computed in place within a.

static void dt_squared(float32 *a, Size_Type size, Size_Type step, uint32 *label)
{ Indx_Type *v, V[2049];
  double    *z, *b, Z[2049], B[2049];
  uint32    *l, *il, L[2049];

  Dimn_Type k, n;
  Indx_Type q, p;
  double    s, t;

  n = (Dimn_Type) (size/step);
  if (n > 2048)
    { z = (double *) Guarded_Malloc((sizeof(uint32)+sizeof(Indx_Type)+2*sizeof(double)) *
                                    ((size_t) (n+1)), "Distance_Transform");
      b = z + (n+1);
      v = (Indx_Type *) (b + (n+1));
      l = (uint32 *) (v + (n+1));
    }
  else
    { z = Z;
      b = B;
      v = V;
      l = L;
    }

  if (label == NULL)
    il = (uint32 *) a;
  else
    il = label;

  k    = 0;
  v[0] = 0;
  b[0] = a[0];
  l[0] = il[0];
  z[0] = 0.;                        //  unlike F&H we limit the domain to [0,infinity]
  t    = 0.;
  for (p = step, q = 1; q < n; q++, p += step) 
    { while (1)
        { s = (double) q;
          s = ( (a[p] + s*s) - (b[k] + t*t) ) / (2.*(s-t));
          if (z[k] < s) break;
          if (k == 0)             //  unlike F&H we limit the domain to [0,infinity]
            { if (s < 0.) s = 0.;
              goto add;
            }
          t = (double) v[--k];
        }
      k     += 1;
    add:
      v[k]   = q;
      b[k]   = a[p];
      l[k]   = il[p];
      z[k]   = s;
      t      = (double) q;
    }

  s = n;               //  we optimize F&H by removing curves not covering [0,n]
  while (z[k] >= s)
    k -= 1;

  z += 1;
  z[k] = s;           //  add a cap to the envelope

  k = 0;
  if (label == NULL)
    for (q = 0, p = 0; q < n; q++, p += step)
      { s = (double) q;
        while (z[k] < s)
          k += 1;
        t = s - v[k];
        a[p] = (float) (t*t + b[k]);
      }
   else
    for (q = 0, p = 0; q < n; q++, p += step)
      { s = (double) q;
        while (z[k] < s)
          k += 1;
        t = s - v[k];
        label[p] = l[k];
        a[p] = (float) (t*t + b[k]);
      }

  if (n > 2048)
    free(z);
}

//  Compute the distance transform of 1d vector a[0 to size by step] with respect to
//     Manhattan/linear distance.  The result is computed in place within a.  If square
//     is non-zero then the transform is squared (useful for optimizing the first pass
//     in computing the Euclidean transform of a binary matrix).

static void dt_linear(float32 *a, Size_Type size, Size_Type step, uint32 *label)
{ Indx_Type p;
  double    t;

  if (label == NULL)

    { t = a[0];
      for (p = step; p < size; p += step) 
        { t = t+1.;
          if (t < a[p])
            a[p] = (float) t;
          else
            t = a[p];
        }

      t = a[size-step];
      for (p = size-step; p > 0; ) 
        { p -= step;
          t  = t+1.;
          if (t < a[p])
            a[p] = (float) t;
          else
            t = a[p];
        }
    }
  else

    { t = a[0];
      for (p = step; p < size; p += step) 
        { t = t+1.;
          if (t < a[p])
            { a[p] = (float) t;
              label[p] = label[p-step];
            }
          else
            t = a[p];
        }

      t = a[size-step];
      for (p = size-step; p > 0; ) 
        { p -= step;
          t  = t+1.;
          if (t < a[p])
            { a[p] = (float) t;
              label[p] = label[p+step];
            }
          else
            t = a[p];
        }
    }
}

// Squared_Distance generate a floating point array of the same shape
//   as image that is the squared Euclidean distrance transform of the underlying
//   image, i.e. R[p] = min_q (I[q] + ||p-q||_2^2).

Float_Array *Squared_Distance(APart *image)
{ Float_Array *g;
  float32     *a;
  Size_Type    s, t, n, m;
  Indx_Type    p, q;
  int          i;

  g = Make_Array_With_Shape(PLAIN_KIND,FLOAT32_TYPE,AForm_Shape(image));

  Array_Op_Array(g,SET_OP,image);

  n = g->size;
  a = AFLOAT32(g);
  s = 1;
  for (i = 0; i < g->ndims; i++)
    { t = s*g->dims[i];
      for (p = 0; p < n; p += t)
        { m = p+s;
          for (q = p; q < m; q++)
            dt_squared(a+q,t,s,NULL);
        }
      s = t;
    }

  return (g);
}

// Manhattan_Distance generate a floating point array of the same shape
//   as image that is the squared Euclidean distrance transform of the underlying
//   image, i.e. R[p] = min_q (I[q] + ||p-q||_1).

Float_Array *Manhattan_Distance(APart *image)
{ Float_Array *g;
  float32     *a;
  Size_Type    s, t, n, m;
  Indx_Type    p, q;
  int          i;

  g = Make_Array_With_Shape(PLAIN_KIND,FLOAT32_TYPE,AForm_Shape(image));

  Array_Op_Array(g,SET_OP,image);

  n = g->size;
  a = AFLOAT32(g);
  s = 1;
  for (i = 0; i < g->ndims; i++)
    { t = s*g->dims[i];
      for (p = 0; p < n; p += t)
        { m = p+s;
          for (q = p; q < m; q++)
            dt_linear(a+q,t,s,NULL);
        }
      s = t;
    }

  return (g);
}

// Generate the Euclidean distance transform of the given image interpreting non-zero
//   pixels as foreground and zero pixels as background.  If boundary is non-zero then
//   it is assumed that all foreground pixels on the boundary of the image are 1 unit
//   away from a background pixel, otherwise the distance is from the nearest background
//   pixel *within* the image.

Float_Array *Binary_Euclidean_Distance(APart *image, int boundary)
{ Float_Array *g;
  float32     *a, *b;
  Size_Type    s, t, n, m;
  Indx_Type    p, q;
  int          i;
  double       x;

  g = Make_Array_With_Shape(PLAIN_KIND,FLOAT32_TYPE,AForm_Shape(image));

  Array_Op_Array(g,SET_OP,image);

  x = 1.;
  for (i = 0; i < g->ndims; i++)   //  x is effectively infinitely large for the 1st pass
    x += g->dims[i];

  n = g->size;
  a = AFLOAT32(g);
  for (p = 0; p < n; p++)
    if (a[p] != 0.)
      a[p] = (float) x;

  s = 1;
  t = g->dims[0];
  if (boundary)
    { b = a + (t-1);
      for (p = 0; p < n; p += t)
        { if (a[p] != 0.)
            a[p] = 1.;
          if (b[p] != 0.)
            b[p] = 1.;
        }
    }
  for (p = 0; p < n; p += t)
    dt_linear(a+p,t,s,NULL);

  for (p = 0; p < n; p++)
    { x = a[p];
      a[p] = (float) (x*x);
    }

  s = t;
  for (i = 1; i < g->ndims; i++)
    { t = s*g->dims[i];
      if (boundary)
        { b = a + (t-1);
          for (p = 0; p < n; p += t)
            { m = p+s;
              for (q = p; q < m; q++)
                { if (a[q] != 0.)
                    a[q] = 1.;
                  if (b[q] != 0.)
                    b[q] = 1.;
                }
            }
        }
      for (p = 0; p < n; p += t)
        { m = p+s;
          for (q = p; q < m; q++)
            dt_squared(a+q,t,s,NULL);
        }
      s = t;
    }

  for (p = 0; p < n; p++)
    a[p] = (float) sqrt(a[p]);

  return (g);
}


/*****************************************************************************************\
*                                                                                         *
*  Distance-based VORONOI PARTITION, DELAUNEY TRIANGULATION, and GEODESIC LABELING        *
*                                                                                         *
\*****************************************************************************************/

  //  Make label and distance working arrays of shape 'shape' and return in pointers plab & pdst.
  //    Initialize label to 0 (unassigned) and distance to a number large enough to effectively
  //    be infinity for dt_linear.

static void make_distance_arrays(Coordinate *shape, Label_Array **plab, Float_Array **pdst)
{ Float_Array *dst = Make_Array_With_Shape(PLAIN_KIND,FLOAT32_TYPE,Inc_Array(shape));
  Label_Array *lab = Make_Array_With_Shape(PLAIN_KIND,UINT32_TYPE,shape);

  float32 *d = AFLOAT32(dst);
  uint32  *l = AUINT32(lab);

  Size_Type n = dst->size;
  double    x;
  Indx_Type p;

  x = 1.;
  for (p = 0; p < dst->ndims; p++)   //  x is effectively infinitely large for the 1st pass
    x += dst->dims[p];

  for (p = 0; p < n; p++)
    { d[p] = (float) x;
      l[p] = 0;
    }

  *plab = lab;
  *pdst = dst;
}

  //  Compute the squared distance transform and implied labeling with dt_linear & dt_squared

static void label_by_distance(Label_Array *lab, Float_Array *dst)
{ float32   *d = AFLOAT32(dst);
  uint32    *l = AUINT32(lab);
  Size_Type  n = dst->size;

  Size_Type  s, t, m;
  Indx_Type  p, q;
  int        i;

  s = 1;
  t = dst->dims[0];
  for (p = 0; p < n; p += t)
    dt_linear(d+p,t,s,l+p);

  for (p = 0; p < n; p++)
    { double x = d[p];
      d[p] = (float) (x*x);
    }

  s = t;
  for (i = 1; i < dst->ndims; i++)
    { t = s*dst->dims[i];
      for (p = 0; p < n; p += t)
        { m = p+s;
          for (q = p; q < m; q++)
            dt_squared(d+q,t,s,l+q);
        }
      s = t;
    }
}

  //  Establish each voronoi point i as background with label i-1 and then compute
  //    the distance transform with labels

static void voronoi(int nseed, Indx_Type *seeds, Coordinate *shape,
                    Label_Array **plab, Float_Array **pdst)
{ Float_Array   *dst;
  Label_Array   *lab;

  make_distance_arrays(shape,&lab,&dst);

  { float32   *d = AFLOAT32(dst);
    uint32    *l = AUINT32(lab);
    int        i;

    for (i = 0; i < nseed; i++)
      { d[seeds[i]] = 0.;
        l[seeds[i]] = i+1;
      }
  }

  label_by_distance(lab,dst);

  *pdst = dst;
  *plab = lab;
}

  //  Compute voronoi, free distance array and return label array

Label_Array *Voronoi_Labeling(int nseed, Indx_Type *seeds, Coordinate *shape)
{ Label_Array *lab;
  Float_Array *dst;

  voronoi(nseed,seeds,shape,&lab,&dst);
  Free_Array(dst);
  return (lab);
}

  //  Convert the floating distance array in place to a UINT16 pixel array where
  //      values are max(2^16-1,2*sqrt(v));

static void convert_distances(Float_Array *dst)
{ float32  *d = AFLOAT32(dst);
  uint16   *e = AUINT16(dst);
  Size_Type n = dst->size;
  double    max = 0xFFFF;
  Indx_Type p;

  for (p = 0; p < n; p++)
    { double x = 2.*sqrt(d[p]);
      if (x > max)
        x = max;
      e[p] = (uint16) x;
    }

  dst->type  = UINT16_TYPE;
  dst->scale = 16;
}

  //  Compute and return Delauney triangulation in the form of a watershed object

Partition *Delauney_Partition(int nseed, Indx_Type *seeds, Coordinate *shape)
{ Label_Array   *lab;
  Float_Array   *dst;
  Partition     *shed;

  voronoi(nseed,seeds,shape,&lab,&dst);
  convert_distances(dst);
  shed = Make_Partition(dst,lab,nseed,0,0);
  Set_Partition_Labels(shed,NULL);
  Free_Array(dst);
  Free_Array(lab);
  return (shed);
}

  //  Globals and utility routine to flood fill label all the foreground pixels, labelling
  //    each iscon2n-connection region with a unique integer and setting their distance
  //    values to 0.

typedef struct
  { uint32  *label;
    float32 *distn;
    uint32   number;
  } GeoArg;

#define GEO(a) ((GeoArg *) (a))

static boolean GeoTest(Indx_Type p, void *arg)   //  non-zero pixels are foreground
{ return (GEO(arg)->label[p]); }

static int GeoBump(Size_Type cnt, void *arg)
{ GEO(arg)->number += 1;
  (void) cnt;
  return (1);
}

static void GeoMark(Indx_Type p, void *arg)
{ GEO(arg)->label[p] = GEO(arg)->number;
  GEO(arg)->distn[p] = 0.;
}

  // geodesic is just like voronoi, with the difference in how pixels are set up

static void geodesic(APart *image, boolean iscon2n, int *nlabels,
                     Label_Array **plab, Float_Array **pdst)
{ Float_Array *dst;
  Label_Array *lab;
  GeoArg       arg;
  Size_Type      n;
  Indx_Type      e, p;
  Array         *a = AForm_Array(image);

  make_distance_arrays(AForm_Shape(image),&lab,&dst);
  
  n = AForm_Size(image);
  switch (a->type) {
      case UINT8_TYPE:
        { uint8   *v = AUINT8(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case UINT16_TYPE:
        { uint16   *v = AUINT16(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case UINT32_TYPE:
        { uint32   *v = AUINT32(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case UINT64_TYPE:
        { uint64   *v = AUINT64(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case INT8_TYPE:
        { int8   *v = AINT8(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case INT16_TYPE:
        { int16   *v = AINT16(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case INT32_TYPE:
        { int32   *v = AINT32(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case INT64_TYPE:
        { int64   *v = AINT64(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case FLOAT32_TYPE:
        { float32   *v = AFLOAT32(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
      case FLOAT64_TYPE:
        { float64   *v = AFLOAT64(a);
          uint32 *l = AUINT32(lab);

          if (Is_Slice(image))
            { e = Set_Slice_To_First(image);
              for (p = 0; p < n; p++)
                { l[p] = (v[e] != 0);
                  e = Next_Slice_Index(image);
                }
            }
          else
            for (p = 0; p < n; p++)
              l[p] = (v[p] != 0);
          break;
        }
  }

  arg.number = 0;                //  use flood fill routine to find and label iscon2n-connected
  arg.label  = AUINT32(lab);     //    regions.
  arg.distn  = AFLOAT32(dst);
  Flood_All(lab,0,iscon2n,&arg,GeoTest,NULL,NULL,&arg,GeoBump,&arg,GeoMark);

  label_by_distance(lab,dst);

  *nlabels = arg.number;
  *pdst = dst;
  *plab = lab;
}

  //  Compute the geodesic labeling

Label_Array *Geodesic_Labeling(APart *image, boolean iscon2n, int *nregions)
{ Label_Array *lab;
  Float_Array *dst;

  geodesic(image,iscon2n,nregions,&lab,&dst);
  Free_Array(dst);
  return (lab);
}

  //  Compute the geodesic labeling and region graph in the form of a watershed object

Partition *Geodesic_Partition(APart *image, boolean iscon2n)
{ Label_Array *lab;
  Float_Array *dst;
  Partition     *shed;
  int            nregions;

  geodesic(image,iscon2n,&nregions,&lab,&dst);
  convert_distances(dst);

  shed = Make_Partition(dst,lab,nregions,iscon2n,0);
  Set_Partition_APart(shed,image);
  Set_Partition_Labels(shed,NULL);   //  Free ref Watershed_Graph created
  Free_Array(dst);

  Free_Array(dst);
  Free_Array(lab);
  return (shed);
}
