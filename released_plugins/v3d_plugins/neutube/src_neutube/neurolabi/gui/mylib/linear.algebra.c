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
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*           Dec 2008 -- Refined TDG's and cubic splines to Decompose/Solve paradigm       *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "utilities.h"
#include "linear.algebra.h"

#define SIZEOF(x) ((int) sizeof(x))

#define TINY 1.0e-20

static inline double pivot_check(double pivot)
{ if (pivot == 0.)
    fprintf(stderr,"Singular Matrix!\n");
  else if (fabs(pivot) <= TINY)
    { fprintf(stderr,"Warning: pivot magnitude is less than %g, setting to this value\n",TINY);
      if (pivot < 0.)
        pivot = -TINY;
      else
        pivot = TINY;
    }
  return (pivot);
}


/****************************************************************************************
 *                                                                                      *
 *  TRI-BAND SYSTEM AND CUBIC SPLINE SOLVER                                             *
 *                                                                                      *
 ****************************************************************************************/

//  Triband_Decompose effectively precomputes the inverse of the tridiagonal system [ a, b, c].
//    It does so in a private storage area it shares with Triband_Solve which solves the system
//    [ a, b, c ] x = v returning the answer in v.  Triband_Decompose does not affect the values of
//    a, b, and c.  Triband_Solve can be called repeatedly with different vectors v to solve the
//    same system with different constraints over and over again.  The routines handle the
//    circular case where a[0] != 0 or c[n-1] != 0 albeit a little less efficiently.

//  For the non-circular case, the Thompson algorithm is used where [ a, b, c ] is turned into
//    an upper triangular system.  We unconventionally split this algorithm into the bits that
//    don't depend on v and store those in the precomputation step.  The operation count of 7n
//    for the solution phase is as good as that with a custom LU-decompose.

//  To solve the circular case, we use the Sherman-Morrison formula (Recipes, pg 73).  This
//    involves tweaking the original matrix a bit before decomposing it as a non-circular
//    matrix and then solving two systems with this modified matrix and combining them.  The
//    nice thing is that one system does not depend on v and can be solved in the decomposition
//    phase.  The operation count for the solve stage ends up being an impressive 10n.

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Triband_Decompose, non-circular   .21ms
//      Triband_Decompose, circular       .28ms
//      Triband_Solve, non-circular       .12ms
//      Triband_Solve, circular           .14ms

typedef struct
  { double   *decom;
    Dimn_Type size;
    int       circular;
    int       width;
  } Bractor;

static inline int bractor_dsize(Bractor *f)
{ if (f->width == 3)
    return (f->size*4*SIZEOF(double));
  else if (f->width == 5)
    return ((f->size*7+8)*SIZEOF(double));
  else
    return (f->size*2*SIZEOF(double));
}


typedef struct __Bractor
  { struct __Bractor *next;
    struct __Bractor *prev;
    int               refcnt;
    int               dsize;
    Bractor           bractor;
  } _Bractor;

static _Bractor *Free_Bractor_List = NULL;
static _Bractor *Use_Bractor_List  = NULL;

static pthread_mutex_t Bractor_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Bractor_Offset = sizeof(_Bractor)-sizeof(Bractor);
static int Bractor_Inuse  = 0;

int Band_Factor_Refcount(Band_Factor *band_factor)
{ _Bractor *object = (_Bractor *) (((char *) band_factor) - Bractor_Offset);
  return (object->refcnt);
}

static inline int allocate_bractor_decom(Bractor *bractor, int dsize, char *routine)
{ _Bractor *object = (_Bractor *) (((char *) bractor) - Bractor_Offset);
  if (object->dsize < dsize)
    { void *x = Guarded_Realloc(bractor->decom,(size_t) dsize,routine);
      if (x == NULL) return (1);
      bractor->decom = x;
      object->dsize = dsize;
    }
  return (0);
}

static inline int sizeof_bractor_decom(Bractor *bractor)
{ _Bractor *object = (_Bractor *) (((char *) bractor) - Bractor_Offset);
  return (object->dsize);
}

static inline void kill_bractor(Bractor *bractor);

static inline Bractor *new_bractor(int dsize, char *routine)
{ _Bractor *object;
  Bractor  *bractor;

  pthread_mutex_lock(&Bractor_Mutex);
  if (Free_Bractor_List == NULL)
    { object = (_Bractor *) Guarded_Realloc(NULL,sizeof(_Bractor),routine);
      if (object == NULL) return (NULL);
      bractor = &(object->bractor);
      object->dsize = 0;
      bractor->decom = NULL;
    }
  else
    { object = Free_Bractor_List;
      Free_Bractor_List = object->next;
      bractor = &(object->bractor);
    }
  Bractor_Inuse += 1;
  object->refcnt = 1;
  if (Use_Bractor_List != NULL)
    Use_Bractor_List->prev = object;
  object->next = Use_Bractor_List;
  object->prev = NULL;
  Use_Bractor_List = object;
  pthread_mutex_unlock(&Bractor_Mutex);
  if (allocate_bractor_decom(bractor,dsize,routine))
    { kill_bractor(bractor);
      return (NULL);
    }
  return (bractor);
}

static inline Bractor *copy_bractor(Bractor *bractor)
{ Bractor *copy = new_bractor(bractor_dsize(bractor),"Copy_Band_Factor");
  void *_decom = copy->decom;
  *copy = *bractor;
  copy->decom = _decom;
  if (bractor->decom != NULL)
    memcpy(copy->decom,bractor->decom,(size_t) bractor_dsize(bractor));
  return (copy);
}

Band_Factor *Copy_Band_Factor(Band_Factor *band_factor)
{ return ((Band_Factor *) copy_bractor(((Bractor *) band_factor))); }

static inline int pack_bractor(Bractor *bractor)
{ _Bractor *object  = (_Bractor *) (((char *) bractor) - Bractor_Offset);
  if (object->dsize > bractor_dsize(bractor))
    { int ns = bractor_dsize(bractor);
      if (ns != 0)
        { void *x = Guarded_Realloc(bractor->decom,(size_t) ns,"Pack_Bractor");
          if (x == NULL) return (1);
          bractor->decom = x;
        }
      else
        { free(bractor->decom);
          bractor->decom = NULL;
        }
      object->dsize = ns;
    }
  return (0);
}

Band_Factor *Pack_Band_Factor(Band_Factor *band_factor)
{ if (pack_bractor(((Bractor *) band_factor))) return (NULL);
  return (band_factor);
}

Band_Factor *Inc_Band_Factor(Band_Factor *band_factor)
{ _Bractor *object  = (_Bractor *) (((char *) band_factor) - Bractor_Offset);
  pthread_mutex_lock(&Bractor_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Bractor_Mutex);
  return (band_factor);
}

static inline void free_bractor(Bractor *bractor)
{ _Bractor *object  = (_Bractor *) (((char *) bractor) - Bractor_Offset);
  pthread_mutex_lock(&Bractor_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Bractor_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Band_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Bractor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Bractor_List;
  Free_Bractor_List = object;
  Bractor_Inuse -= 1;
  pthread_mutex_unlock(&Bractor_Mutex);
}

void Free_Band_Factor(Band_Factor *band_factor)
{ free_bractor(((Bractor *) band_factor)); }

static inline void kill_bractor(Bractor *bractor)
{ _Bractor *object  = (_Bractor *) (((char *) bractor) - Bractor_Offset);
  pthread_mutex_lock(&Bractor_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Bractor_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Band_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Bractor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Bractor_Inuse -= 1;
  pthread_mutex_unlock(&Bractor_Mutex);
  if (bractor->decom != NULL)
    free(bractor->decom);
  free(((char *) bractor) - Bractor_Offset);
}

void Kill_Band_Factor(Band_Factor *band_factor)
{ kill_bractor(((Bractor *) band_factor)); }

static inline void reset_bractor()
{ _Bractor *object;
  Bractor  *bractor;
  pthread_mutex_lock(&Bractor_Mutex);
  while (Free_Bractor_List != NULL)
    { object = Free_Bractor_List;
      Free_Bractor_List = object->next;
      bractor = &(object->bractor);
      if (bractor->decom != NULL)
        free(bractor->decom);
      free(object);
    }
  pthread_mutex_unlock(&Bractor_Mutex);
}

void Reset_Band_Factor()
{ reset_bractor(); }

int Band_Factor_Usage()
{ return (Bractor_Inuse); }

void Band_Factor_List(void (*handler)(Band_Factor *))
{ _Bractor *a, *b;
  for (a = Use_Bractor_List; a != NULL; a = b)
    { b = a->next;
      handler((Band_Factor *) &(a->bractor));
    }
}

static inline Bractor *read_bractor(FILE *input)
{ char name[11];
  Bractor *obj;
  Bractor read;
  fread(name,11,1,input);
  if (strncmp(name,"Band_Factor",11) != 0)
    return (NULL);
  obj = new_bractor(0,"Read_Band_Factor");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Bractor),1,input) == 0) goto error;
  obj->decom = read.decom;
  if (bractor_dsize(obj) != 0)
    { if (allocate_bractor_decom(obj,bractor_dsize(obj),"Read_Band_Factor")) goto error;
      if (fread(obj->decom,(size_t) bractor_dsize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_bractor(obj);
  return (NULL);
}

Band_Factor *Read_Band_Factor(FILE *input)
{ return ((Band_Factor *) read_bractor(input)); }

static inline void write_bractor(Bractor *bractor, FILE *output)
{ fwrite("Band_Factor",11,1,output);
  fwrite(bractor,sizeof(Bractor),1,output);
  if (bractor_dsize(bractor) != 0)
    fwrite(bractor->decom,(size_t) bractor_dsize(bractor),1,output);
}

void Write_Band_Factor(Band_Factor *band_factor, FILE *output)
{ write_bractor(((Bractor *) band_factor),output); }

Band_Factor *Triband_Decompose(Double_Matrix *tri_vector)
{ Dimn_Type n;
  double   *a, *b, *c;

  double *ap, *bp, *cp;
  double  b0,  bn;

  int      circular;
  Bractor *factor;

  if (tri_vector->ndims != 2 || tri_vector->dims[1] != 3 || tri_vector->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix is not a 3 x n double array (Triband_Decompose)\n");
      exit (1);
    }

  n = tri_vector->dims[0];

  if (n < 2)
    { fprintf(stderr,"Matrix has second dimension less than 2! (Triband_Decompose)\n");
      exit (1);
    }

  a = AFLOAT64(tri_vector);
  b = a + n;
  c = b + n;

  factor = new_bractor(n*4*SIZEOF(double),"Triband_Decompose");

  factor->size     = n;
  factor->width    = 3;
  factor->circular = circular = (a[0] != 0. || c[n-1] != 0.);

  ap = factor->decom;
  bp = ap + n;
  cp = bp + n;

  // For circular case, let g = [ -b0 ..0.. cn ]^T and h = [ 1 ..0.. -a0/b0 ]
  //   Let A' = A - g * h which by the choice of g and h is non-cyclic

  if (circular)
    { b0 = b[0];     //  Save the elements of b that are about to be trashed
      bn = b[n-1];

      if (b0 == 0.)
        return (NULL);

      b[0]   += b0;
      b[n-1] += (a[0]*c[n-1])/b0;
    }
  else
    b0 = bn = 0.;

  { double    wm1, ai, bi;  //  prepare the inverse of non-circular A
    Dimn_Type i;            //    (sneaky: a[0] and c[n-1] values play no part in forming a result
                            //     even though we didn't zero them in the circular case)
    wm1 = 0.;
    for (i = 0; i < n; i++)
      { ap[i] = ai = -a[i];
  
        bi = pivot_check(b[i] + ai*wm1);
        if (bi == 0.)
          { if (circular)
              { b[0]   = b0;   //  Restore the elements of b that got trashed in the circular case
                b[n-1] = bn;
              }
            return (NULL);
          }
  
        bp[i] = bi = 1./bi;
        cp[i] = wm1 = c[i] * bi;
      }
  }

  if (circular)      //  for circular case, solve system A' * z = g (take advantage of 0's)

    { double *z = cp + n;

      { double    zm1;
        Dimn_Type i;

        z[0] = zm1 = -.5;            //  sneaky: know g[0] * bp[0] = -b0 * (1./2*b0) = -.5
        for (i = 1; i < n-1; i++)
          z[i] = zm1 *= ap[i]*bp[i];
        z[n-1] = zm1 = (c[n-1] + ap[n-1]*zm1) * bp[n-1];
  
        for (i = n-1; i-- > 0; )
          z[i] = zm1 = z[i] - cp[i]*zm1;
      }

      b[0]   = b0;     //  Restore the elements of b that got trashed
      b[n-1] = bn;
    }

  return ((Band_Factor *) factor);
}

Double_Vector *Triband_Solve(Double_Vector *R(M(values)), Band_Factor *f)
{ Dimn_Type n;
  double   *v;
  double   *ap, *bp, *cp;
  Bractor  *factor = (Bractor *) f;

  if (values->ndims != 1 || values->type != FLOAT64_TYPE)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (Triband_Solve)\n");
      exit (1);
    }
  if (factor->width != 3)
    { fprintf(stderr,"Band_Factor is not for a tri-diagonal system (Triband_Solve)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (factor->size != n)
    { fprintf(stderr,"Vector length does not match Band_Factor length (Triband_Solve)\n");
      exit (1);
    }

  ap = factor->decom;
  bp = ap + n;
  cp = bp + n;

  { double    vm1;             //  solve core system
    Dimn_Type i;

    vm1 = 0.;
    for (i = 0; i < n; i++)
      v[i] = vm1 = (v[i] + ap[i]*vm1) * bp[i];
  
    for (i = n-1; i-- > 0; )
      v[i] = vm1 = v[i] - cp[i]*vm1;
  }

  if (factor->circular)       //  for circular case, v += ( h*v / (1 + h*z) ) z

    { double *z = cp + n;

      { double    hn, gm;
        Dimn_Type i;

        hn = 2.*ap[0]*bp[0];  // sneaky: h[n-1] = -a0/b0 = 2.*(-a0)*(1/(2*b0)) = 2.*ap[0]*bp[0]
        gm = (v[0] + hn*v[n-1]) / (1. + z[0] + hn*z[n-1]);

        for (i = 0; i < n; i++)
          v[i] -= gm * z[i];
      }
    }

  return (values);
}


/*
    Given control points v1, v2, ... vn, compute the Bezier displacements for cubic
    splines between them such that (a) the points are interpolated, and (b) 1st and
    2nd order continuous.  If the curve is to begin at v1 and end at vn then these
    are the solution [d] to the tridiagonal system at left, and if the curve is to be
    closed then they are the solution to the system at right.

       | 2 1        | d1     v2-v1         | 4 1     1 |  d1     v2-vn
       | 1 4 1      | d2     v3-v1         | 1 4 1     |  d2     v3-v1
       |   1 4 1    | d3     v4-v2         |   1 4 1   |  d3     v4-v2
       |     .....  |      =               |     ..... |      =
       |      1 4 1 | dn-1   vn-vn-2       |     1 4 1 | dn-1   vn-vn-2
       |        1 2 | dn     vn-vn-1       | 1     1 4 | dn     v1-vn-1

    We use a tailored version of the TDG routines to return the desired displacements
    in the array v.  The required decompose step is cached and reused with each call if
    n and circular are unchanged.  Op counts are 5n for the non-circular case and 7n for
    the circular case
*/

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Spline_Decompose, non-circular, decompose   .18ms
//      Spline_Decompose, circular, decompose       .26ms
//      Spline_Slopes, non-circular, solve          .11ms
//      Spline_Slopes, circular, solve              .13ms

Band_Factor *G(Spline_Decompose)(Dimn_Type n, boolean closed)
{ double   *w, *u, b, wm1;
  Bractor  *factor;
  Dimn_Type i;

  factor = new_bractor(n*2*SIZEOF(double),"Spline_Decompose");

  factor->size     = n;
  factor->width    = 1;
  factor->circular = closed;

  w = factor->decom;

  if (closed)
    b = 3.;
  else
    b = 2.;

  w[0] = wm1 = 1./b;            //  pre-compute that portion of the inversion that is
  for (i = 1; i < n-1; i++)     //    invariant for the given value of n
    w[i] = wm1 = 1. / (4.-wm1);
  w[n-1] = 1. / (b - wm1);

  if (closed)
    { u = w + n;

      u[0] = wm1 = w[0];   //  solve system with g = [ 1 0... 1 ] as v, take advantage of 0's
      for (i = 1; i < n-1; i++)
        u[i] = wm1 *= -w[i];
      u[n-1] = wm1 = (1. - wm1) * w[n-1];

      for (i = n-1; i-- > 0; )
        u[i] = wm1 = u[i] - w[i]*wm1;
    }

  return ((Band_Factor *) factor);
}

Double_Vector *Spline_Slopes(Double_Vector *R(M(values)), Band_Factor *f)
{ Bractor   *factor = (Bractor *) f;
  Dimn_Type  i, n;
  double    *v, *w, *u, vm1, v0, gm;

  if (values->ndims != 1 || values->type != FLOAT64_TYPE)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (Spline_Slopes)\n");
      exit (1);
    }
  if (factor->width != 1)
    { fprintf(stderr,"Band_Factor is not for a cubic spline system (Spline_Slopes)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (factor->size != n)
    { fprintf(stderr,"Vector length does not match Band_Factor length (Spline_Slopes)\n");
      exit (1);
    }

  w = factor->decom;

  v0 = vm1 = v[0];
  if (factor->circular)
    v[0] = v[1] - v[n-1];
  else
    v[0] = v[1] - vm1;
  for (i = 1; i < n-1; i++)
    { double vi = v[i];
      v[i] = v[i+1] - vm1;
      vm1  = vi;
    }
  if (factor->circular)
    v[n-1] = v0-vm1;
  else
    v[n-1] -= vm1;

  vm1 = 0.;
  for (i = 0; i < n; i++)
    v[i] = vm1 = (v[i] - vm1) * w[i];

  for (i = n-1; i-- > 0; )
    vm1 = (v[i] -= w[i]*vm1);

  if (factor->circular)
    { u = w + n;

      gm = (v[0] + v[n-1]) / (1. + u[0]  + u[n-1]);   // h.v / (1 + h.u)

      for (i = 0; i < n; i++)
        v[i] -= gm * u[i];
    }

  return (values);
}


/****************************************************************************************
 *                                                                                      *
 *  PENTA-BAND SYSTEM SOLVER                                                            *
 *                                                                                      *
 ****************************************************************************************/

//  Pentaband_Decompose effectively precomputes the inverse of the penta-diagonal system
//    [ a, b, c, d, e].  It does so in a private storage area it shares with Pentaband_Solve
//    which solves the system [ a, b, c, d, e ] x = v returning the answer in v.
//    Pentaband_Decompose does not affect the values of a, b, c, d, and e.  Pentaband_Solve
//    can be called repeatedly with different vectors v to solve the same system with different
//    constraints over and over again.  The routines handle the circular case albeit a little
//    less efficiently.

//  For the non-circular case, the Thompson algorithm is used where [ a, b, c, d, e ] is turned into
//    an upper triangular system.  We unconventionally split this algorithm into the bits that
//    don't depend on v and store those in the precomputation step.  The operation count of 11n
//    for the solution phase is as good as that with a custom LU-decompose.

//  To solve the circular case, we use the Woodbury formula (Recipes, pg 73) that is a
//    generalization of the Sherman-Morrison formula.  Its use involves tweaking the original
//    matrix a bit before decomposing it as a non-circular  matrix and then solving three systems
//    with this modified matrix and taking a linear combination thereof where the coefficents
//    depend on the two systems that do not depend on v.  As for the tridiagonal case, the nice
//    thing is that only one system depends on v and the other two can be solved in the
//    decomposition phase.  The operation count for the solve stage ends up being an
//    impressive 16n.

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Pentaband_Decompose, non-circular   .25ms
//      Pentaband_Decompose, circular       .52ms
//      Pentaband_Solve, non-circular       .17ms
//      Pentaband_Solve, circular           .23ms

Band_Factor *G(Pentaband_Decompose)(Double_Matrix *penta_vector)
{ Dimn_Type  n;
  double    *a, *b, *c, *d, *e;

  double *ap, *bp, *cp, *dp, *ep;
  double  h0m, h0n, h1n, p0, p1;
  double  c0, c1, cm, cn, dm, bn;

  int      circular;
  Bractor *factor;

  if (penta_vector->ndims != 2 || penta_vector->dims[1] != 5 || penta_vector->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix is not a 5 x n double array (Pentaband_Decompose)\n");
      exit (1);
    }

  n = penta_vector->dims[0];

  if (n < 3)
    { fprintf(stderr,"Matrix has second dimension less than 3! (Pentaband_Decompose)\n");
      exit (1);
    }

  a = AFLOAT64(penta_vector);
  b = a + n;
  c = b + n;
  d = c + n;
  e = d + n;

  factor = new_bractor((n*7+8)*SIZEOF(double),"Pentaband_Decompose");

  factor->size     = n;
  factor->width    = 5;
  factor->circular = circular = (a[0] != 0. || a[1] != 0. || b[0] != 0. ||
                                 d[n-1] != 0. || e[n-1] != 0. || e[n-2] != 0.);
  ap = factor->decom;
  bp = ap + n;
  cp = bp + n;
  dp = cp + n;
  ep = dp + n;

  //  For the circular case, let G = [ -p0   0 0... en-2 dn-1 ]^T and H = [ 1 0 0... a0/-p0 b0/-p0 ]
  //                                 [   0 -p1 0...    0 en-1 ]           [ 0 1 0...    0   a1/-p1 ]
  //    Let A' = A - G * H which by the choice of G and H is a non-circular matrix

  if (circular)
    { c0 = c[0];     //  Save the elements of b, c, and d that are about to be trashed
      c1 = c[1];
      cm = c[n-2];
      cn = c[n-1];
      dm = d[n-2];
      bn = b[n-1];

      p0 = c0;
      p1 = c1 - b[1]*(d[0]/c0);   // 2nd pivot of the unalterred matrix, should != 0 if invertable

      if (p0 == 0 || p1 == 0)
        return (NULL);

      h0m = a[0]/p0;
      h0n = b[0]/p0;
      h1n = a[1]/p1;

      c[0]   += p0;
      c[1]   += p1;
      c[n-1] += h0n*d[n-1] + h1n*e[n-1];
      c[n-2] += h0m*e[n-2];
      d[n-2] += h0n*e[n-2];
      b[n-1] += h0m*d[n-1];
    }
  else
    c0 = c1 = cm = cn = dm = bn = p0 = p1 = h0m = h0n = h1n = 0.;  //  Keeps compiler quiet

  { double    dm2, em2;      //  prepare the inverse of non-circular A
    double    dm1, em1;
    double    ai, bi, ci;
    Dimn_Type i;

    em1 = em2 = 0.;
    dm1 = dm2 = 0.;
    for (i = 0; i < n; i++)
      { ap[i] = ai = a[i];
  
        bp[i] = bi = b[i] - ai*dm2; 
        ci = pivot_check(c[i] - (ai*em2 + bi*dm1));
        if (ci == 0.)
          { if (circular)
              { c[0]   = c0;     //  Restore the elements of b, c, and d that got
                c[1]   = c1;     //    trashed in the circular case
                c[n-2] = cm;
                c[n-1] = cn;
                d[n-2] = dm;
                b[n-1] = bn;
              }
            return (NULL);
          }
  
        em2 = em1;
        dm2 = dm1;
  
        cp[i] = ci = 1./ci;
        dp[i] = dm1 = (d[i] - bi*em1) * ci;
        ep[i] = em1 = e[i] * ci;
      }
  }
  
  if (circular)   //  Prepare for solving by computing and storing: z1, z2, and pv s.t.
                  //    A * zi = gi and pv = (I + H*Z)^-1 * H where Z = [ z1 z2 ]
    { double *z1 = ep + n;
      double *z2 = z1 + n;
      double *pv = z2 + n;

      { double    ui, um1, um2;          //  Solve A * z1 = u1 and A * z2 = u2
        double    wi, wm1, wm2;
        Dimn_Type i;

        z1[0] = um2 = -.5;      //  sneaky: know g1[0] * cp[0] = -c0 * (1./2*c0) = -.5
        z2[0] = wm2 =  .0;
        z1[1] = um1 =  .5*bp[1]*cp[1];
        z2[1] = wm1 = -p1*cp[1];
        for (i = 2; i < n-2; i++)
          { z1[i] = ui = - (ap[i]*um2 + bp[i]*um1) * cp[i];
            um2  = um1;
            um1  = ui;
            z2[i] = wi = - (ap[i]*wm2 + bp[i]*wm1) * cp[i];
            wm2  = wm1;
            wm1  = wi;
          }
        z1[n-2] = ui  = (e[n-2] - (ap[n-2]*um2 + bp[n-2]*um1)) * cp[n-2];
        z2[n-2] = wi  = (       - (ap[n-2]*wm2 + bp[n-2]*wm1)) * cp[n-2];
        z1[n-1] = um1 = (d[n-1] - (ap[n-1]*um1 + bp[n-1]*ui )) * cp[n-1];
        z2[n-1] = wm1 = (e[n-1] - (ap[n-1]*wm1 + bp[n-1]*wi )) * cp[n-1];

        um2 = wm2 = 0.;
        for (i = n-1; i-- > 0; )
          { z1[i] = ui = z1[i] - (dp[i]*um1 + ep[i]*um2);
            um2  = um1;
            um1  = ui;
            z2[i] = wi = z2[i] - (dp[i]*wm1 + ep[i]*wm2);
            wm2  = wm1;
            wm1  = wi;
          }
      }

      { double p11, p12, p21, p22;
        double fac, piv;
        double q11, q12, q21, q22;

        p11 = 1. + z1[0] - (h0m*z1[n-2] + h0n*z1[n-1]);   //  [ p11 p12 ] = P = I + H * Z
        p12 =      z2[0] - (h0m*z2[n-2] + h0n*z2[n-1]);   //  [ p21 p22 ]
        p21 =      z1[1] -                h1n*z1[n-1];
        p22 = 1. + z2[1] -                h1n*z2[n-1];

        fac = -p21/p11;             //  Q = P^-1  (does this need pivoting?)
        piv = p22 + p12*fac;
        q21 = fac / piv; 
        q22 = 1. / piv;
        q11 = (1. - p12*q21) / p11;
        q12 = -p12*q22 / p11;

        pv[0] = q11;                //  [ pv0 pv1  ...0...  -pv2 -pv3 ]  = Q * H = PV
        pv[1] = q12;                //  [ pv4 pv5  ...0...  -pv6 -pv7 ]
        pv[2] = q11*h0m;
        pv[3] = q11*h0n + q12*h1n;
        pv[4] = q21;
        pv[5] = q22;
        pv[6] = q21*h0m;
        pv[7] = q21*h0n + q22*h1n;
      }

      c[0]   = c0;     //  Restore the elements of b, c, and d that got trashed
      c[1]   = c1;
      c[n-2] = cm;
      c[n-1] = cn;
      d[n-2] = dm;
      b[n-1] = bn;
    }

  return ((Band_Factor *) factor);
}

Double_Vector *Pentaband_Solve(Double_Vector *R(M(values)), Band_Factor *f)
{ double    *v, *ap, *bp, *cp, *dp, *ep;
  double    vm1, vm2, vi;
  Dimn_Type i, n;
  Bractor  *factor = (Bractor *) f;

  if (values->ndims != 1 || values->type != FLOAT64_TYPE)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (Pentaband_Solve)\n");
      exit (1);
    }
  if (factor->width != 5)
    { fprintf(stderr,"Band_Factor is not for a penta-diagonal system (Pentaband_Solve)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (factor->size != n)
    { fprintf(stderr,"Vector length does not match last system decomposition (Pentaband_Solve)\n");
      exit (1);
    }

  ap = factor->decom;
  bp = ap + n;
  cp = bp + n;
  dp = cp + n;
  ep = dp + n;

  vm1  = vm2  = 0.;
  for (i = 0; i < n; i++)
    { v[i] = vi = (v[i] - (ap[i]*vm2 + bp[i]*vm1)) * cp[i];
      vm2  = vm1;
      vm1  = vi;
    }

  vm2 = 0.;
  for (i = n-1; i-- > 0; )
    { v[i] = vi = v[i] - (dp[i]*vm1 + ep[i]*vm2);
      vm2  = vm1;
      vm1  = vi;
    }

  if (factor->circular)        //  v += Z * (PV * v);
    { double *z1 = ep + n;
      double *z2 = z1 + n;
      double *pv = z2 + n;

      double f1 = (pv[0]*v[0] + pv[1]*v[1]) - (pv[2]*v[n-2] + pv[3]*v[n-1]);
      double f2 = (pv[4]*v[0] + pv[5]*v[1]) - (pv[6]*v[n-2] + pv[7]*v[n-1]);
      int    i;

      for (i = 0; i < n; i++)
        v[i] = v[i] - (f1*z1[i] + f2*z2[i]);
    }

  return (values);
}


/****************************************************************************************
 *                                                                                      *
 *  LU-FACTORIZATION SYSTEM SOLVER                                                      *
 *                                                                                      *
 ****************************************************************************************/

//  NB: perm vector has a hidden double vector of the same size tacked on to its
//        end as it is a needed working vector for solving the LU system.

static inline int lu_factor_psize(LU_Factor *f)
{ return (f->lu_mat->dims[0]*(SIZEOF(int)+SIZEOF(double))); }


typedef struct __LU_Factor
  { struct __LU_Factor *next;
    struct __LU_Factor *prev;
    int                 refcnt;
    int                 psize;
    LU_Factor           lu_factor;
  } _LU_Factor;

static _LU_Factor *Free_LU_Factor_List = NULL;
static _LU_Factor *Use_LU_Factor_List  = NULL;

static pthread_mutex_t LU_Factor_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int LU_Factor_Offset = sizeof(_LU_Factor)-sizeof(LU_Factor);
static int LU_Factor_Inuse  = 0;

int LU_Factor_Refcount(LU_Factor *lu_factor)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  return (object->refcnt);
}

static inline int allocate_lu_factor_perm(LU_Factor *lu_factor, int psize, char *routine)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (object->psize < psize)
    { void *x = Guarded_Realloc(lu_factor->perm,(size_t) psize,routine);
      if (x == NULL) return (1);
      lu_factor->perm = x;
      object->psize = psize;
    }
  return (0);
}

static inline int sizeof_lu_factor_perm(LU_Factor *lu_factor)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  return (object->psize);
}

static inline void kill_lu_factor(LU_Factor *lu_factor);

static inline LU_Factor *new_lu_factor(int psize, char *routine)
{ _LU_Factor *object;
  LU_Factor  *lu_factor;

  pthread_mutex_lock(&LU_Factor_Mutex);
  if (Free_LU_Factor_List == NULL)
    { object = (_LU_Factor *) Guarded_Realloc(NULL,sizeof(_LU_Factor),routine);
      if (object == NULL) return (NULL);
      lu_factor = &(object->lu_factor);
      object->psize = 0;
      lu_factor->perm = NULL;
    }
  else
    { object = Free_LU_Factor_List;
      Free_LU_Factor_List = object->next;
      lu_factor = &(object->lu_factor);
    }
  LU_Factor_Inuse += 1;
  object->refcnt = 1;
  if (Use_LU_Factor_List != NULL)
    Use_LU_Factor_List->prev = object;
  object->next = Use_LU_Factor_List;
  object->prev = NULL;
  Use_LU_Factor_List = object;
  pthread_mutex_unlock(&LU_Factor_Mutex);
  lu_factor->lu_mat = NULL;
  if (allocate_lu_factor_perm(lu_factor,psize,routine))
    { kill_lu_factor(lu_factor);
      return (NULL);
    }
  return (lu_factor);
}

static inline LU_Factor *copy_lu_factor(LU_Factor *lu_factor)
{ LU_Factor *copy = new_lu_factor(lu_factor_psize(lu_factor),"Copy_LU_Factor");
  void *_perm = copy->perm;
  *copy = *lu_factor;
  if (lu_factor->lu_mat != NULL)
    copy->lu_mat = Copy_Array(lu_factor->lu_mat);
  copy->perm = _perm;
  if (lu_factor->perm != NULL)
    memcpy(copy->perm,lu_factor->perm,(size_t) lu_factor_psize(lu_factor));
  return (copy);
}

LU_Factor *Copy_LU_Factor(LU_Factor *lu_factor)
{ return ((LU_Factor *) copy_lu_factor(lu_factor)); }

static inline int pack_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (lu_factor->lu_mat != NULL)
    if (Pack_Array(lu_factor->lu_mat) == NULL) return (1);
  if (object->psize > lu_factor_psize(lu_factor))
    { int ns = lu_factor_psize(lu_factor);
      if (ns != 0)
        { void *x = Guarded_Realloc(lu_factor->perm,(size_t) ns,"Pack_LU_Factor");
          if (x == NULL) return (1);
          lu_factor->perm = x;
        }
      else
        { free(lu_factor->perm);
          lu_factor->perm = NULL;
        }
      object->psize = ns;
    }
  return (0);
}

LU_Factor *Pack_LU_Factor(LU_Factor *lu_factor)
{ if (pack_lu_factor(lu_factor)) return (NULL);
  return (lu_factor);
}

LU_Factor *Inc_LU_Factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  pthread_mutex_lock(&LU_Factor_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&LU_Factor_Mutex);
  return (lu_factor);
}

static inline void free_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  pthread_mutex_lock(&LU_Factor_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&LU_Factor_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released LU_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_LU_Factor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_LU_Factor_List;
  Free_LU_Factor_List = object;
  LU_Factor_Inuse -= 1;
  pthread_mutex_unlock(&LU_Factor_Mutex);
  if (lu_factor->lu_mat != NULL)
    { Free_Array(lu_factor->lu_mat);
      lu_factor->lu_mat = NULL;
    }
}

void Free_LU_Factor(LU_Factor *lu_factor)
{ free_lu_factor(lu_factor); }

static inline void kill_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  pthread_mutex_lock(&LU_Factor_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&LU_Factor_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released LU_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_LU_Factor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  LU_Factor_Inuse -= 1;
  pthread_mutex_unlock(&LU_Factor_Mutex);
  if (lu_factor->perm != NULL)
    free(lu_factor->perm);
  if (lu_factor->lu_mat != NULL)
    Kill_Array(lu_factor->lu_mat);
  free(((char *) lu_factor) - LU_Factor_Offset);
}

void Kill_LU_Factor(LU_Factor *lu_factor)
{ kill_lu_factor(lu_factor); }

static inline void reset_lu_factor()
{ _LU_Factor *object;
  LU_Factor  *lu_factor;
  pthread_mutex_lock(&LU_Factor_Mutex);
  while (Free_LU_Factor_List != NULL)
    { object = Free_LU_Factor_List;
      Free_LU_Factor_List = object->next;
      lu_factor = &(object->lu_factor);
      if (lu_factor->perm != NULL)
        free(lu_factor->perm);
      free(object);
    }
  pthread_mutex_unlock(&LU_Factor_Mutex);
}

void Reset_LU_Factor()
{ reset_lu_factor(); }

int LU_Factor_Usage()
{ return (LU_Factor_Inuse); }

void LU_Factor_List(void (*handler)(LU_Factor *))
{ _LU_Factor *a, *b;
  for (a = Use_LU_Factor_List; a != NULL; a = b)
    { b = a->next;
      handler((LU_Factor *) &(a->lu_factor));
    }
}

static inline LU_Factor *read_lu_factor(FILE *input)
{ char name[9];
  LU_Factor *obj;
  LU_Factor read;
  fread(name,9,1,input);
  if (strncmp(name,"LU_Factor",9) != 0)
    return (NULL);
  obj = new_lu_factor(0,"Read_LU_Factor");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(LU_Factor),1,input) == 0) goto error;
  if (read.lu_mat != NULL)
    { obj->lu_mat = Read_Array(input);
      if (obj->lu_mat == NULL) goto error;
    }
  else
    obj->lu_mat = NULL;
  obj->perm = read.perm;
  if (lu_factor_psize(obj) != 0)
    { if (allocate_lu_factor_perm(obj,lu_factor_psize(obj),"Read_LU_Factor")) goto error;
      if (fread(obj->perm,(size_t) lu_factor_psize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_lu_factor(obj);
  return (NULL);
}

LU_Factor *Read_LU_Factor(FILE *input)
{ return ((LU_Factor *) read_lu_factor(input)); }

static inline void write_lu_factor(LU_Factor *lu_factor, FILE *output)
{ fwrite("LU_Factor",9,1,output);
  fwrite(lu_factor,sizeof(LU_Factor),1,output);
  if (lu_factor->lu_mat != NULL)
    Write_Array(lu_factor->lu_mat,output);
  if (lu_factor_psize(lu_factor) != 0)
    fwrite(lu_factor->perm,(size_t) lu_factor_psize(lu_factor),1,output);
}

void Write_LU_Factor(LU_Factor *lu_factor, FILE *output)
{ write_lu_factor(lu_factor,output); }

//  m is a square double matrix where the row index moves the fastest.
//    LU_Decompose takes M and produces an LU factorization of m that
//    can then be used to rapidly solve the system for given right hand sides
//    and to compute m's determinant.  The return value is NULL if the matrix
//    is nonsingular.  If the matrix appears unstable (had to use a very nearly
//    zero pivot) then the integer pointed at by stable will be zero, and
//    non-zero otherwise.  m is subsumed and effectively destroyed by the routine.

LU_Factor *G(LU_Decompose)(Double_Matrix *S(m), boolean *O(stable))
{ LU_Factor      *factor;
  Dimn_Type       n, i, j;
  int            *p, sign;
  double         *v;
  double         *Avec[1001], **a;

  if (m->ndims != 2 || m->type != FLOAT64_TYPE || m->dims[0] != m->dims[1])
    { fprintf(stderr,"Matrix is not a square 2D double array (LU_Decompose)\n");
      exit (1);
    }
  if (Array_Refcount(m) != 1)
    { fprintf(stderr,"Matrix m is not subsumable, has a reference count > 1 (LU_Decompose)\n");
      exit (1);
    }

  n = m->dims[0];
  if (n > 1000)
    a = (double **) Guarded_Malloc(sizeof(double *)*((size_t) n),"LU_Decompose");
  else
    a = Avec;

  factor = new_lu_factor(n*(SIZEOF(int)+SIZEOF(double)),"LU_Decompose");
  p = factor->perm;
  v = (double *) (p+n);

  p[0] = 0;
  a[0] = AFLOAT64(m);
  for (i = 1; i < n; i++)
    { a[i] = a[i-1] + n;
      p[i] = i;
    }

  *stable = 1;
  sign    = 1;
  for (i = 0; i < n; i++)  // Find the scale factors for each row in v.
    { double b, f, *r;

      r = a[i];
      b = 0.;
      for (j = 0; j < n; j++)
        { f = fabs(r[j]);
          if (f > b)
            b = f;
        }
      if (b == 0.0)
        { Kill_Array(m);
          Kill_LU_Factor(factor);
          if (n > 1000)
            free(a);
          return (NULL);
        }
      v[i] = 1./b;
    }

  for (j = 0; j < n; j++)      //  For each column
    { double    b, s, *r;
      Dimn_Type k, w;

      for (i = 0; i < j; i++)    // Determine U
        { r = a[i];
          s = r[j];
          for (k = 0; k < i; k++)
            s -= r[k]*a[k][j];
          r[j] = s;
        }

      b = -1.;
      w = j;
      for (i = j; i < n; i++)      // Determine L without dividing by pivot, in order to
        { r = a[i];                //   determine who the pivot should be.
          s = r[j];
          for (k = 0; k < j; k++)
            s -= r[k]*a[k][j];
          r[j] = s;

          s = v[i]*fabs(s);        // Update best pivot seen thus far
          if (s > b)
            { b = s;
              w = i;
            }
	}

      if (w != j)                  // Pivot if necessary
        { r    = a[w];
          a[w] = a[j];
          a[j] = r;
          k    = p[w];
          p[w] = p[j];
          p[j] = k;
          sign = -sign;
          v[w] = v[j];
        }

      if (fabs(a[j][j]) < TINY)    // Complete column of L by dividing by selected pivot
        { if (a[j][j] < 0.)
            a[j][j] = -TINY;
          else
            a[j][j] = TINY;
          *stable = 0;
        }
      b = 1./a[j][j];
      for (i = j+1; i < n; i++)
        a[i][j] *= b;
    }

#ifdef DEBUG_LU
  { Dimn_Type i, j;

    printf("\nLU Decomposition\n");
    for (i = 0; i < n; i++)
      { printf("  %2d: ",p[i]);
        for (j = 0; j < n; j++)
          printf(" %8g",a[i][j]);
        printf("\n");
      }
  }
#endif

  if (n > 1000)
    free(a);

  factor->sign   = sign;
  factor->lu_mat = m;
  return (factor);
}

void Show_LU_Product(FILE *file, LU_Factor *f)
{ Dimn_Type n, i, j, k;
  int      *p;
  double    u, **a, *d;

  n = f->lu_mat->ndims;
  d = AFLOAT64(f->lu_mat);
  p = f->perm;
  a = (double **) (p+n);

  for (i = 0; i < n; i++)
    a[i] = d + p[i]*n;

  fprintf(file,"\nLU Product:\n");
  for (i = 0; i < n; i++)
    { for (j = 0; j < i; j++)
        { u = 0.;
          for (k = 0; k <= j; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      for (j = i; j < n; j++)
        { u = a[i][j];
          for (k = 0; k < i; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      fprintf(file,"\n");
    }
}


//  Given rhs vector b and LU-factorization f, solve the system of equations
//    and return the result in b.
//  To invert a given the LU-decomposition, simply call LU_Solve with
//    b = [ 0^k-1 1 0^n-k] to get the k'th column of the inverse matrix.

Double_Vector *LU_Solve(Double_Vector *R(M(bv)), LU_Factor *f)
{ double   *x;
  Dimn_Type n, i, j;
  int      *p;
  double   *a, *b, s, *r;

  if (bv->ndims != 1 || bv->type != FLOAT64_TYPE || bv->dims[0] != f->lu_mat->dims[0])
    { fprintf(stderr,"B-vector is not a double 1D array of same size as matrix (LU_Solve)\n");
      exit (1);
    }

  n = f->lu_mat->dims[0];
  a = AFLOAT64(f->lu_mat);
  p = f->perm;
  b = AFLOAT64(bv);
  x = (double *) (p+n);

  for (i = 0; i < n; i++)
    { r = a + p[i]*n;
      s = b[p[i]];
      for (j = 0; j < i; j++)
        s -= r[j] * x[j];
      x[i] = s;
    }

  for (i = n; i-- > 0; )
    { r = a + p[i]*n;
      s = x[i]; 
      for (j = i+1; j < n; j++)
        s -= r[j] * b[j];
      b[i] = s/r[i];
    }

  return (bv);
}
  
//  Set a square matrix to the identity matrix of that size

Double_Matrix *Set_Matrix2Identity(Double_Matrix *R(M(m)))
{ Dimn_Type      i, j, n;
  Indx_Type      p;
  double        *a;
 
  if (m->ndims != 2 || m->type != FLOAT64_TYPE || m->dims[0] != m->dims[1])
    { fprintf(stderr,"Matrix is not a square 2D double array (SetMatrix2Identity)\n");
      exit (1);
    }

  a = AFLOAT64(m);
  n = m->dims[0];

  p = 0;
  for (j = 0; j < n; j++)                 //  Set g to the identity matrix
    for (i = 0; i < n; i++)
      if (i == j)
        a[p++] = 1.;
      else
        a[p++] = 0.;

  return (m);
}

//  Generate a square identity matrix of size n

Double_Matrix *G(Identity_Matrix)(Dimn_Type n)
{ Dimn_Type dims[2];

  dims[0] = dims[1] = n;
  return (Set_Matrix2Identity(Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,dims)));
}

//  Transpose a matrix g in-place and as a convenience return a pointer to it

Double_Matrix *Transpose_Matrix(Double_Matrix *R(M(g)))
{ Dimn_Type n, m;
  double   *a;

  if (g->ndims != 2 || g->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix is not a 2D double array (Transpose_Matrix)\n");
      exit (1);
    }

  n = g->dims[0];
  m = g->dims[1];
  a = AFLOAT64(g);
 
  if (n == m)
    { Indx_Type p, q;
      Dimn_Type i, j;

      p = 0;
      for (j = 0; j < n; j++)                 //  Transpose the result
        { q = j;
          for (i = 0; i < j; i++)
            { double x = a[p];
              a[p++] = a[q];
              a[q] = x;
              q += n;
            }
          p += (n-j);
        }
    }
  else
    { Size_Type base = m*n-1;
      Indx_Type q, p, r;
      double    x, y;

      for (p = 0; p <= base; p++)

        { q = (m*p) % base;         //  Determine if p is the smallest element of an
          if (q == p) continue;        //    exchange cycle of length > 1
          while (q > p)
            q = (m*q) % base;
          if (q < p) continue;

          y = a[q];                 //  Yes?  Do the exchanges
          do
            { r = (m*q) % base;
              x = a[r];
              a[r] = y;
              y = x;
              q = r;
            }
          while (q != p);
        }

      g->dims[0] = m;
      g->dims[1] = n;
    }

  return (g);
}

Array *Set_Matrix2Product(Array *R(M(c)), Array *a, boolean ta, Array *b, boolean tb)
{ int d0, d1;
 
  if (a->ndims != 2 || a->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix a is not a 2D double array (Set_Matrix2Product)\n");
      exit (1);
    }
  if (b->ndims != 2 || b->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix b is not a 2D double array (Set_Matrix2Product)\n");
      exit (1);
    }
  if (b->ndims != 2 || b->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix B is not a 2D double array (Set_Matrix2Product)\n");
      exit (1);
    }

  d0 = (tb != 0);
  d1 = (ta == 0);
  if (a->dims[1-d1] != b->dims[1-d0])
    { fprintf(stderr,"Inner product dimensions for a and b not equal");
      fprintf(stderr," (Set_Matrix2Product Multiply)\n");
      exit (1);
    }
  if (c->dims[0] != b->dims[d0] || c->dims[1] != a->dims[d1])
    { fprintf(stderr,"Matrix c doesn't have the shape of the product of a & b");
      fprintf(stderr," (Set_Matrix2Product Multiply)\n");
      exit (1);
    }

  { double    *x = AFLOAT64(a);
    double    *y = AFLOAT64(b);
    double    *z = AFLOAT64(c);

    double    s;
    Indx_Type p, q, t, r;

    if (tb)
      if (ta)
        { Size_Type a0 = a->dims[0];
          Size_Type a1 = a->dims[1];
          Size_Type as = a->size;
          Size_Type bs = b->size;
          double   *buf, Buf[1000];

          if (a1 > 1000)
            buf = (double *) Guarded_Malloc(sizeof(double)*((size_t) a1),"Set_Matrix2Product");
          else
            buf = Buf;

          for (q = 0; q < a0; q++)
            { r = 0;
              for (p = q; p < as; p += a0)
                buf[r++] = x[p];
              p = 0;
              while (p < bs)
                { s = 0.;
                  for (r = 0; r < a1; r++)
                    s += buf[r] * y[p++];
                  *z++ = s;
                }
            }

          if (a1 > 1000)
            free(buf);
        }
      else
        { Size_Type a0 = a->dims[0];
          Size_Type as = a->size;
          Size_Type bs = b->size;

          for (q = 0; q < as; q = t)
            { p = 0;
              t = q+a0;
              while (p < bs)
                { s = 0.;
                  for (r = q; r < t; r++)
                    s += x[r] * y[p++];
                  *z++ = s;
                }
            }
        }
    else
      { Size_Type b0 = b->dims[0];
        Size_Type b1 = b->dims[1];
        Size_Type bs = b->size;
        Size_Type cs = c->size;
        double   *buf, Buf[1000];

        if (b1 > 1000)
          buf = (double *) Guarded_Malloc(sizeof(double)*((size_t) b1),"Set_Matrix2Product");
        else
          buf = Buf;

        if (ta)
          Transpose_Matrix(a);
        for (q = 0; q < b0; q++)
          { r = 0;
            for (p = q; p < bs; p += b0)
              buf[r++] = y[p];
            t = 0;
            for (p = q; p < cs; p += b0)
              { s = 0.;
                for (r = 0; r < b1; r++)
                  s += x[t++] * buf[r];
                z[p] = s;
              }
          }
        if (ta)
          Transpose_Matrix(a);

        if (b1 > 1000)
          free(buf);
      }
  }

  return (c);
}


Array *G(Matrix_Multiply)(Array *a, boolean ta, Array *b, boolean tb)
{ Dimn_Type dims[2];

  if (a->ndims != 2 || a->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix a is not a 2D double array (Matrix_Multiply)\n");
      exit (1);
    }
  if (b->ndims != 2 || b->type != FLOAT64_TYPE)
    { fprintf(stderr,"Matrix b is not a 2D double array (Matrix_Multiply)\n");
      exit (1);
    }

  dims[0] = b->dims[tb!=0];
  dims[1] = a->dims[ta==0];
  return (Set_Matrix2Product(Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,dims),a,ta,b,tb));
}


//  Generate the right inverse of the matrix that gave rise to the LU factorization f.
//    That is for matrix A, return matrix A^-1 s.t. A * A^-1 = I.  If transpose is non-zero
//    then the transpose of the right inverse is returned.

Double_Matrix *LU_Invert(LU_Factor *f, boolean transpose)
{ Dimn_Type      n, i;
  Double_Matrix *g;
  Array_Bundle   gb, *gp = &gb;

  n = f->lu_mat->dims[0];

  g = Identity_Matrix(n);

  for (i = 0; i < n; i++)                 //  Find the inverse of each column in the
    { gb = *g;                            //    corresponding *row* (i.e. the transpose)
      LU_Solve(Get_Array_Plane(gp,i),f);
    }

  if (!transpose)
    Transpose_Matrix(g);

  return (g);
}

//  Given an LU-factorization f, return the value of the determinant of the
//    original matrix.

double LU_Determinant(LU_Factor *f)
{ Dimn_Type i, n;
  int      *p;
  double   *a, det;

  n = f->lu_mat->dims[0];
  a = AFLOAT64(f->lu_mat);
  p = f->perm;

  det = f->sign;
  for (i = 0; i < n; i++)
    det *= a[p[i]*n+i];
  return (det);
}


/****************************************************************************************
 *                                                                                      *
 *  ORTHOGONAL BASIS                                                                    *
 *                                                                                      *
 ****************************************************************************************/

//  Make an orthogonal rotation matrix that spans the same space as basis.
//    Use a "stable" version of Gram-Schmidt.  NR says should use SVD, but that's
//    a lot more code, so be cautious about stability of output matrix.

Double_Matrix *Orthogonalize_Matrix(Double_Matrix *R(M(basis)))
{ Dimn_Type n;
  Indx_Type n2;
  double    Magvec[1001], *mag;

  if (basis->ndims != 2 || basis->type != FLOAT64_TYPE || basis->dims[0] != basis->dims[1])
    { fprintf(stderr,"basis matrix is not a square double 2D array (Orthogonalize_Array)\n");
      exit (1);
    }

  n   = basis->dims[0];
  n2  = basis->size;
  if (n > 1000)
    mag = (double *) Guarded_Malloc(sizeof(double)*((size_t) n),"Orthogonaize_Matrix");
  else
    mag = Magvec;

  { float64  *v = (float64 *) (basis->data);
    Indx_Type j, h;
    Dimn_Type i, k;
    double    x, b;

    for (j = 0; j < n2; j += n)
      { for (h = i = 0; h < j; i++, h += n)
          { b = 0;                   // v_j = v_j - proj(v_i) v_j  (= (v_i . v_j) / (v_i . v_i) v_i
            for (k = 0; k < n; k++)
              b += v[h+k] * v[j+k];
            b /= mag[i];
            for (k = 0; k < n; k++)
              x = v[j+k] -= b * v[h+k];
          }

        b = 0;                      // v_j = v_j / ||v_j||
        for (k = 0; k < n; k++)
          { x  = v[j+k];
            b += x*x;
          }
        mag[i] = 1./b;
        b = 1./sqrt(mag[i]);
        for (k = 0; k < n; k++)
          v[j+k] *= b;
      }
  }

  if (n > 1000)
    free(mag);

  return (basis);
}
