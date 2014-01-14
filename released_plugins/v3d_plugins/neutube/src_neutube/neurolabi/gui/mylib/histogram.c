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
*  Histogram Data Abstraction and Array Statistics Routines                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  December 2008                                                                 *
*                                                                                         *
*  (c) December 20, '09, Dr. Gene Myers and Howard Hughes Medical Institute               *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

#include "utilities.h"
#include "array.h"
#include "histogram.h"



#define SIZEOF(x) ((int) sizeof(x))


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM SPACE MANAGEMENT ROUTINES AND PRIMARY GENERATOR                           *
 *                                                                                      *
 ****************************************************************************************/

// Awk-generated (manager.awk) Array memory management

typedef struct
  { Value_Kind kind;    //  Exactly the same as a Histogram save the extra field for mean.
    Value      binsize;
    Value      offset;
    int        nbins;
    Size_Type  total;
    Size_Type *counts;
    double     mean;    //  histogram mean
  } Histofull;

static Value_Kind type2kind[] = { UVAL, UVAL, UVAL, UVAL, IVAL, IVAL, IVAL, IVAL, FVAL, FVAL };

static inline int histofull_nsize(Histofull *h)
{ return (SIZEOF(Size_Type)*h->nbins); }


typedef struct __Histofull
  { struct __Histofull *next;
    struct __Histofull *prev;
    int                 refcnt;
    int                 nsize;
    Histofull           histofull;
  } _Histofull;

static _Histofull *Free_Histofull_List = NULL;
static _Histofull *Use_Histofull_List  = NULL;

static pthread_mutex_t Histofull_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Histofull_Offset = sizeof(_Histofull)-sizeof(Histofull);
static int Histofull_Inuse  = 0;

int Histogram_Refcount(Histogram *histogram)
{ _Histofull *object = (_Histofull *) (((char *) histogram) - Histofull_Offset);
  return (object->refcnt);
}

static inline int allocate_histofull_counts(Histofull *histofull, int nsize, char *routine)
{ _Histofull *object = (_Histofull *) (((char *) histofull) - Histofull_Offset);
  if (object->nsize < nsize)
    { void *x = Guarded_Realloc(histofull->counts,(size_t) nsize,routine);
      if (x == NULL) return (1);
      histofull->counts = x;
      object->nsize = nsize;
    }
  return (0);
}

static inline int sizeof_histofull_counts(Histofull *histofull)
{ _Histofull *object = (_Histofull *) (((char *) histofull) - Histofull_Offset);
  return (object->nsize);
}

static inline void kill_histofull(Histofull *histofull);

static inline Histofull *new_histofull(int nsize, char *routine)
{ _Histofull *object;
  Histofull  *histofull;

  pthread_mutex_lock(&Histofull_Mutex);
  if (Free_Histofull_List == NULL)
    { object = (_Histofull *) Guarded_Realloc(NULL,sizeof(_Histofull),routine);
      if (object == NULL) return (NULL);
      histofull = &(object->histofull);
      object->nsize = 0;
      histofull->counts = NULL;
    }
  else
    { object = Free_Histofull_List;
      Free_Histofull_List = object->next;
      histofull = &(object->histofull);
    }
  Histofull_Inuse += 1;
  object->refcnt = 1;
  if (Use_Histofull_List != NULL)
    Use_Histofull_List->prev = object;
  object->next = Use_Histofull_List;
  object->prev = NULL;
  Use_Histofull_List = object;
  pthread_mutex_unlock(&Histofull_Mutex);
  if (allocate_histofull_counts(histofull,nsize,routine))
    { kill_histofull(histofull);
      return (NULL);
    }
  return (histofull);
}

static inline Histofull *copy_histofull(Histofull *histofull)
{ Histofull *copy = new_histofull(histofull_nsize(histofull),"Copy_Histogram");
  void *_counts = copy->counts;
  *copy = *histofull;
  copy->counts = _counts;
  if (histofull->counts != NULL)
    memcpy(copy->counts,histofull->counts,(size_t) histofull_nsize(histofull));
  return (copy);
}

Histogram *Copy_Histogram(Histogram *histogram)
{ return ((Histogram *) copy_histofull(((Histofull *) histogram))); }

static inline int pack_histofull(Histofull *histofull)
{ _Histofull *object  = (_Histofull *) (((char *) histofull) - Histofull_Offset);
  if (object->nsize > histofull_nsize(histofull))
    { int ns = histofull_nsize(histofull);
      if (ns != 0)
        { void *x = Guarded_Realloc(histofull->counts,(size_t) ns,"Pack_Histofull");
          if (x == NULL) return (1);
          histofull->counts = x;
        }
      else
        { free(histofull->counts);
          histofull->counts = NULL;
        }
      object->nsize = ns;
    }
  return (0);
}

Histogram *Pack_Histogram(Histogram *histogram)
{ if (pack_histofull(((Histofull *) histogram))) return (NULL);
  return (histogram);
}

Histogram *Inc_Histogram(Histogram *histogram)
{ _Histofull *object  = (_Histofull *) (((char *) histogram) - Histofull_Offset);
  pthread_mutex_lock(&Histofull_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Histofull_Mutex);
  return (histogram);
}

static inline void free_histofull(Histofull *histofull)
{ _Histofull *object  = (_Histofull *) (((char *) histofull) - Histofull_Offset);
  pthread_mutex_lock(&Histofull_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Histofull_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Histogram\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Histofull_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Histofull_List;
  Free_Histofull_List = object;
  Histofull_Inuse -= 1;
  pthread_mutex_unlock(&Histofull_Mutex);
}

void Free_Histogram(Histogram *histogram)
{ free_histofull(((Histofull *) histogram)); }

static inline void kill_histofull(Histofull *histofull)
{ _Histofull *object  = (_Histofull *) (((char *) histofull) - Histofull_Offset);
  pthread_mutex_lock(&Histofull_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Histofull_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Histogram\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Histofull_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Histofull_Inuse -= 1;
  pthread_mutex_unlock(&Histofull_Mutex);
  if (histofull->counts != NULL)
    free(histofull->counts);
  free(((char *) histofull) - Histofull_Offset);
}

void Kill_Histogram(Histogram *histogram)
{ kill_histofull(((Histofull *) histogram)); }

static inline void reset_histofull()
{ _Histofull *object;
  Histofull  *histofull;
  pthread_mutex_lock(&Histofull_Mutex);
  while (Free_Histofull_List != NULL)
    { object = Free_Histofull_List;
      Free_Histofull_List = object->next;
      histofull = &(object->histofull);
      if (histofull->counts != NULL)
        free(histofull->counts);
      free(object);
    }
  pthread_mutex_unlock(&Histofull_Mutex);
}

void Reset_Histogram()
{ reset_histofull(); }

int Histogram_Usage()
{ return (Histofull_Inuse); }

void Histogram_List(void (*handler)(Histogram *))
{ _Histofull *a, *b;
  for (a = Use_Histofull_List; a != NULL; a = b)
    { b = a->next;
      handler((Histogram *) &(a->histofull));
    }
}

static inline Histofull *read_histofull(FILE *input)
{ char name[9];
  Histofull *obj;
  Histofull read;
  fread(name,9,1,input);
  if (strncmp(name,"Histogram",9) != 0)
    return (NULL);
  obj = new_histofull(0,"Read_Histogram");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Histofull),1,input) == 0) goto error;
  obj->counts = read.counts;
  if (histofull_nsize(obj) != 0)
    { if (allocate_histofull_counts(obj,histofull_nsize(obj),"Read_Histogram")) goto error;
      if (fread(obj->counts,(size_t) histofull_nsize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_histofull(obj);
  return (NULL);
}

Histogram *Read_Histogram(FILE *input)
{ return ((Histogram *) read_histofull(input)); }

static inline void write_histofull(Histofull *histofull, FILE *output)
{ fwrite("Histogram",9,1,output);
  fwrite(histofull,sizeof(Histofull),1,output);
  if (histofull_nsize(histofull) != 0)
    fwrite(histofull->counts,(size_t) histofull_nsize(histofull),1,output);
}

void Write_Histogram(Histogram *histogram, FILE *output)
{ write_histofull(((Histofull *) histogram),output); }

Histogram *G(Make_Histogram)(Value_Kind kind, int nbins, Value binsize, Value offset)
{ Histofull *h     = new_histofull(nbins*SIZEOF(Size_Type),"Histogram_Array");
  Size_Type *count = h->counts;
  Indx_Type  p;

  h->kind    = kind;
  h->binsize = binsize;
  h->offset  = offset;
  h->nbins   = nbins;

  h->mean    = DBL_MIN;
  h->total   = 0;
  for (p = 0; p < nbins; p++)
    count[p] = 0;

  return ((Histogram *) h);
}

Histogram *Empty_Histogram(Histogram *h)
{ int        nbins = h->nbins;
  Size_Type *count = h->counts;
  Indx_Type  p;

  for (p = 0; p < nbins; p++)
    count[p] = 0;
  h->total = 0;
  ((Histofull *) h)->mean = DBL_MIN;
  return (h);
}

  /*  Generate a histogram of array a with nbins of width binsize where the smallest bin's
      lower boundary is offset (see data descriptor comments in .h file).  The type of values   
      given for binsize and offset should be congruent with the type of a.  When nbins or 
      binsize or both are zero then the histogram bins are set up as follows based on the 
      range [min,max] of values in a: 

         nbins = 0 & binsize = 0:
            bins are of length *1* and cover the range of values
              in a starting at *floor(min)*

         nbins = 0 & binsize != 0:
            bins are of length *binsize* and cover the range of values
              in a starting at *floor(min)*

         nbins != 0 & binsize = 0:
            The bin size is the smallest number of the form [1,2,5]*10^a for which nbins
            of this size cover the range [min,max].  The bins start at the first multiple
            of the bin size <= min and nbins is adjusted downwards so that the binning
            just covers the required range [min,max].

         nbins != 0 & binsize != 0
            The implied bining is used as specified and any values not in the implied range
            are not added to the histogram, i.e. the total count of the histogram can be less
            then the size of a.
  */

#define FILL_BINS_F( EXPR, ADVANCE )	\
  if (clip)				\
    for (p = 0; p < size; p++)		\
      { int i = (int) ((EXPR-o)/b);	\
        if (0 <= i && i < nbins)	\
          count[i] += 1;		\
        ADVANCE				\
      }					\
  else if (b == 1)			\
    if (o < 0)				\
      for (p = 0; p < size; p++)	\
        { count[(int) (EXPR - o)] += 1;	\
          ADVANCE			\
        }				\
    else				\
      { count -= (int) o;		\
        for (p = 0; p < size; p++)	\
          { count[(int) EXPR] += 1;	\
            ADVANCE			\
          }				\
      }					\
  else if (o == 0)			\
    for (p = 0; p < size; p++)		\
      { count[(int) (EXPR/b)] += 1;	\
        ADVANCE				\
      }					\
  else					\
    for (p = 0; p < size; p++)		\
      { count[(int) ((EXPR-o)/b)] += 1;	\
        ADVANCE				\
      }

#define FILL_BINS_D( EXPR, ADVANCE )	\
  if (clip)				\
    for (p = 0; p < size; p++)		\
      { int i = (int) ((EXPR-o)/b);	\
        if (0 <= i && i < nbins)	\
          count[i] += 1;		\
        ADVANCE				\
      }					\
  else if (b == 1)			\
    { count -= o;			\
      for (p = 0; p < size; p++)	\
        { count[EXPR] += 1;		\
          ADVANCE			\
        }				\
    }					\
  else if (o == 0)			\
    for (p = 0; p < size; p++)		\
      { count[EXPR/b] += 1;		\
        ADVANCE				\
      }					\
  else					\
    for (p = 0; p < size; p++)		\
      { count[(EXPR-o)/b] += 1;		\
        ADVANCE				\
      }

#define FILL_BINS_FREG				\
  if (clip)					\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          { int i = (int) ((v[p]-o)/b);		\
            if (0 <= i && i < nbins)		\
              count[i] += 1;			\
          }					\
      }						\
  else if (b == 1)				\
    if (o < 0)					\
      for (k = 0; k < len; k += 2)		\
        { vr = raster[k];			\
          wr = raster[k+1];			\
          for (p = vr; p <= wr; p++)		\
            count[(int) (v[p] - o)] += 1;	\
        }					\
    else					\
      { count -= (int) o;			\
        for (k = 0; k < len; k += 2)		\
          { vr = raster[k];			\
            wr = raster[k+1];			\
            for (p = vr; p <= wr; p++)		\
              count[(int) v[p]] += 1;		\
          }					\
      }						\
  else if (o == 0)				\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          count[(int) (v[p]/b)] += 1;		\
      }						\
  else						\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          count[(int) ((v[p]-o)/b)] += 1;	\
      }

#define FILL_BINS_DREG				\
  if (clip)					\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          { int i = (int) ((v[p]-o)/b);		\
            if (0 <= i && i < nbins)		\
              count[i] += 1;			\
          }					\
      }						\
  else if (b == 1)				\
    { count -= o;				\
      for (k = 0; k < len; k += 2)		\
        { vr = raster[k];			\
          wr = raster[k+1];			\
          for (p = vr; p <= wr; p++)		\
            count[v[p]] += 1;		 	\
        }					\
    }						\
  else if (o == 0)				\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          count[v[p]/b] += 1;			\
      }						\
  else						\
    for (k = 0; k < len; k += 2)		\
      { vr = raster[k];				\
        wr = raster[k+1];			\
        for (p = vr; p <= wr; p++)		\
          count[(v[p]-o)/b] += 1;		\
      }


Histogram *G(Histogram_Array)(AForm *form, int nbins, Value binsize, Value offset)
{ static uint64  MinOff_uval[]   = {     0,       0 };
  static uint64  FullSpan_uval[] = { 0x100, 0x10000, 0, 0 };
  static int64   MinOff_ival[]   = { 0, 0, 0, 0,  0xff,  0xffff };
  static int64   FullSpan_ival[] = { 0, 0, 0, 0, 0x100, 0x10000, 0, 0, };
  static float64 MinOff_fval[]   = { 0 };
  static float64 FullSpan_fval[] = { 0 };

  Array        *a     = AForm_Array(form);
  boolean       clip  = 0;
  Range_Bundle  rng;

  switch (type2kind[a->type]) {
      case UVAL:
        if (nbins == 0)
          { uint64 rti;

            Array_Range(&rng,form);
            if (binsize.uval == 0)
              binsize.uval = 1;
              offset.uval = (rng.minval.uval / binsize.uval) * binsize.uval;
            rti = (uint64) ((rng.maxval.uval - offset.uval) / binsize.uval);
            if (offset.uval + ((uint64) nbins)*binsize.uval <= rng.maxval.uval)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.uval == 0)
          { double bwide;

            Array_Range(&rng,form);
            bwide = (rng.maxval.uval - rng.minval.uval) / (1.*nbins);
            if (bwide == 0.)
              binsize.uval = 1;
            else
              { uint64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.uval = x;
              }
                offset.uval = (rng.minval.uval / binsize.uval) * binsize.uval;
            nbins = (int) ((rng.maxval.uval - offset.uval) / binsize.uval);
            if (offset.uval + ((uint64) nbins)*binsize.uval <= rng.maxval.uval)
              nbins += 1;
          } 
        else
          { if (FullSpan_uval[a->type] == 0 || offset.uval > MinOff_uval[a->type] ||
                offset.uval + ((uint64) nbins)*binsize.uval < FullSpan_uval[a->type])
              { Array_Range(&rng,form);
                clip = (offset.uval > rng.minval.uval ||
                        offset.uval + binsize.uval * ((uint64) nbins) <= rng.maxval.uval);
              }
          }
        break;
      case IVAL:
        if (nbins == 0)
          { uint64 rti;

            Array_Range(&rng,form);
            if (binsize.ival == 0)
              binsize.ival = 1;
              if (rng.minval.ival < 0)
                offset.ival = ((rng.minval.ival+1) / binsize.ival - 1) * binsize.ival;
              else
                offset.ival = (rng.minval.ival / binsize.ival) * binsize.ival;
            rti = (uint64) ((rng.maxval.ival - offset.ival) / binsize.ival);
            if (offset.ival + ((int64) nbins)*binsize.ival <= rng.maxval.ival)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.ival == 0)
          { double bwide;

            Array_Range(&rng,form);
            bwide = (rng.maxval.ival - rng.minval.ival) / (1.*nbins);
            if (bwide == 0.)
              binsize.ival = 1;
            else
              { int64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.ival = x;
              }
              if (rng.minval.ival < 0)
                offset.ival = ((rng.minval.ival+1) / binsize.ival - 1) * binsize.ival;
              else
            nbins = (int) ((rng.maxval.ival - offset.ival) / binsize.ival);
            if (offset.ival + ((int64) nbins)*binsize.ival <= rng.maxval.ival)
              nbins += 1;
          } 
        else
          { if (FullSpan_ival[a->type] == 0 || offset.ival > MinOff_ival[a->type] ||
                offset.ival + ((int64) nbins)*binsize.ival < FullSpan_ival[a->type])
              { Array_Range(&rng,form);
                clip = (offset.ival > rng.minval.ival ||
                        offset.ival + binsize.ival * ((int64) nbins) <= rng.maxval.ival);
              }
          }
        break;
      case FVAL:
        if (nbins == 0)
          { uint64 rti;

            Array_Range(&rng,form);
            if (binsize.fval == 0)
              binsize.fval = 1;
              offset.fval = floor(rng.minval.fval / binsize.fval) * binsize.fval;
            rti = (uint64) ((rng.maxval.fval - offset.fval) / binsize.fval);
            if (offset.fval + ((float64) nbins)*binsize.fval <= rng.maxval.fval)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.fval == 0)
          { double bwide;

            Array_Range(&rng,form);
            bwide = (rng.maxval.fval - rng.minval.fval) / (1.*nbins);
            if (bwide == 0.)
              binsize.fval = 1;
            else
              { double x = pow(10.,floor(log10(bwide)));
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.fval = x;
              }
              offset.fval = floor(rng.minval.fval / binsize.fval) * binsize.fval;
            nbins = (int) ((rng.maxval.fval - offset.fval) / binsize.fval);
            if (offset.fval + ((float64) nbins)*binsize.fval <= rng.maxval.fval)
              nbins += 1;
          } 
        else
          { if (FullSpan_fval[a->type] == 0 || offset.fval > MinOff_fval[a->type] ||
                offset.fval + ((float64) nbins)*binsize.fval < FullSpan_fval[a->type])
              { Array_Range(&rng,form);
                clip = (offset.fval > rng.minval.fval ||
                        offset.fval + binsize.fval * ((float64) nbins) <= rng.maxval.fval);
              }
          }
        break;
  }

  { Histofull *h     = new_histofull(nbins*SIZEOF(Size_Type),"Histogram_Array");
    Size_Type *count = h->counts;
    Size_Type  size  = AForm_Size(form);
    Indx_Type  p;

    h->kind    = type2kind[a->type];
    h->binsize = binsize;
    h->offset  = offset;
    h->nbins   = nbins;

    for (p = 0; p < nbins; p++)
      count[p] = 0;

    switch (a->type) {
        case UINT8_TYPE:
          { uint8 *v = AUINT8(a);
            uint8  o = (uint8) offset.uval;
            uint8  b = (uint8) binsize.uval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case UINT16_TYPE:
          { uint16 *v = AUINT16(a);
            uint16  o = (uint16) offset.uval;
            uint16  b = (uint16) binsize.uval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case UINT32_TYPE:
          { uint32 *v = AUINT32(a);
            uint32  o = (uint32) offset.uval;
            uint32  b = (uint32) binsize.uval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case UINT64_TYPE:
          { uint64 *v = AUINT64(a);
            uint64  o = (uint64) offset.uval;
            uint64  b = (uint64) binsize.uval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case INT8_TYPE:
          { int8 *v = AINT8(a);
            int8  o = (int8) offset.ival;
            int8  b = (int8) binsize.ival;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case INT16_TYPE:
          { int16 *v = AINT16(a);
            int16  o = (int16) offset.ival;
            int16  b = (int16) binsize.ival;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case INT32_TYPE:
          { int32 *v = AINT32(a);
            int32  o = (int32) offset.ival;
            int32  b = (int32) binsize.ival;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case INT64_TYPE:
          { int64 *v = AINT64(a);
            int64  o = (int64) offset.ival;
            int64  b = (int64) binsize.ival;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_D( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_D( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case FLOAT32_TYPE:
          { float32 *v = AFLOAT32(a);
            float32  o = (float32) offset.fval;
            float32  b = (float32) binsize.fval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_F( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_F( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_F( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
        case FLOAT64_TYPE:
          { float64 *v = AFLOAT64(a);
            float64  o = (float64) offset.fval;
            float64  b = (float64) binsize.fval;
              switch (AForm_Class(form))
              { case FRAME_CLASS:
                  if (Frame_Within_Array(form))
                    { Offs_Type *off = Frame_Offsets(form);
                      v += Frame_Index(form);
                        FILL_BINS_F( v[off[p]], )
                      break;
                    }
                  else
                    v = Frame_Values(form);
                case ARRAY_CLASS:
                    FILL_BINS_F( v[p], )
                  break;
                case SLICE_CLASS:
                  { Indx_Type e = Set_Slice_To_First(form);
                      FILL_BINS_F( v[e], e = Next_Slice_Index(form); )
                    break;
                  }
              }
            break;
	  }
    }
    if (clip)
      { size = 0;
        for (p = 0; p < h->nbins; p++)
          size += count[p];
      }

    h->total = size;
    h->mean  = DBL_MIN;
    return ((Histogram *) h);
  }
}

Histogram *Histagain_Array(Histogram *h, AForm *form, boolean clip)
{ Value      offset  = h->offset;
  Value      binsize = h->binsize;
  Size_Type  size    = AForm_Size(form);
  Size_Type *count   = h->counts;
  int        nbins   = h->nbins;
  Array     *a       = AForm_Array(form);
  Indx_Type  p;

  switch (a->type) {
      case UINT8_TYPE:
        { uint8 *v = AUINT8(a);
          uint8  o = (uint8) offset.uval;
          uint8  b = (uint8) binsize.uval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case UINT16_TYPE:
        { uint16 *v = AUINT16(a);
          uint16  o = (uint16) offset.uval;
          uint16  b = (uint16) binsize.uval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case UINT32_TYPE:
        { uint32 *v = AUINT32(a);
          uint32  o = (uint32) offset.uval;
          uint32  b = (uint32) binsize.uval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case UINT64_TYPE:
        { uint64 *v = AUINT64(a);
          uint64  o = (uint64) offset.uval;
          uint64  b = (uint64) binsize.uval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case INT8_TYPE:
        { int8 *v = AINT8(a);
          int8  o = (int8) offset.ival;
          int8  b = (int8) binsize.ival;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case INT16_TYPE:
        { int16 *v = AINT16(a);
          int16  o = (int16) offset.ival;
          int16  b = (int16) binsize.ival;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case INT32_TYPE:
        { int32 *v = AINT32(a);
          int32  o = (int32) offset.ival;
          int32  b = (int32) binsize.ival;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case INT64_TYPE:
        { int64 *v = AINT64(a);
          int64  o = (int64) offset.ival;
          int64  b = (int64) binsize.ival;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_D( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_D( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_D( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case FLOAT32_TYPE:
        { float32 *v = AFLOAT32(a);
          float32  o = (float32) offset.fval;
          float32  b = (float32) binsize.fval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_F( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_F( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_F( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
      case FLOAT64_TYPE:
        { float64 *v = AFLOAT64(a);
          float64  o = (float64) offset.fval;
          float64  b = (float64) binsize.fval;
            switch (AForm_Class(form))
            { case FRAME_CLASS:
                if (Frame_Within_Array(form))
                  { Offs_Type *off = Frame_Offsets(form);
                    v += Frame_Index(form);
                      FILL_BINS_F( v[off[p]], )
                    break;
                  }
                else
                  v = Frame_Values(form);
              case ARRAY_CLASS:
                  FILL_BINS_F( v[p], )
                break;
              case SLICE_CLASS:
                { Indx_Type e = Set_Slice_To_First(form);
                    FILL_BINS_F( v[e], e = Next_Slice_Index(form); )
                  break;
                }
            }
          break;
        }
  }

  size = 0;
  for (p = 0; p < h->nbins; p++)
    size += count[p];
  h->total = size;
  ((Histofull *) h)->mean = DBL_MIN;

  return (h);
}


Histogram *G(Histogram_Region)(Array *form, Region *reg, int nbins, Value binsize, Value offset)
{ static uint64  MinOff_uval[]   = {     0,       0 };
  static uint64  FullSpan_uval[] = { 0x100, 0x10000, 0, 0 };
  static int64   MinOff_ival[]   = { 0, 0, 0, 0,  0xff,  0xffff };
  static int64   FullSpan_ival[] = { 0, 0, 0, 0, 0x100, 0x10000, 0, 0, };
  static float64 MinOff_fval[]   = { 0 };
  static float64 FullSpan_fval[] = { 0 };

  Array        *a     = AForm_Array(form);
  boolean       clip  = 0;
  Range_Bundle  rng;

  switch (type2kind[a->type]) {
      case UVAL:
        if (nbins == 0)
          { uint64 rti;

            Region_Range(&rng,form,reg);
            if (binsize.uval == 0)
              binsize.uval = 1;
              offset.uval = (rng.minval.uval / binsize.uval) * binsize.uval;
            rti = (uint64) ((rng.maxval.uval - offset.uval) / binsize.uval);
            if (offset.uval + ((uint64) nbins)*binsize.uval <= rng.maxval.uval)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Region)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.uval == 0)
          { double bwide;

            Region_Range(&rng,form,reg);
            bwide = (rng.maxval.uval - rng.minval.uval) / (1.*nbins);
            if (bwide == 0.)
              binsize.uval = 1;
            else
              { uint64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.uval = x;
              }
                offset.uval = (rng.minval.uval / binsize.uval) * binsize.uval;
            nbins = (int) ((rng.maxval.uval - offset.uval) / binsize.uval);
            if (offset.uval + ((uint64) nbins)*binsize.uval <= rng.maxval.uval)
              nbins += 1;
          } 
        else
          { if (FullSpan_uval[a->type] == 0 || offset.uval > MinOff_uval[a->type] ||
                offset.uval + ((uint64) nbins)*binsize.uval < FullSpan_uval[a->type])
              { Region_Range(&rng,form,reg);
                clip = (offset.uval > rng.minval.uval ||
                        offset.uval + binsize.uval * ((uint64) nbins) <= rng.maxval.uval);
              }
          }
        break;
      case IVAL:
        if (nbins == 0)
          { uint64 rti;

            Region_Range(&rng,form,reg);
            if (binsize.ival == 0)
              binsize.ival = 1;
              if (rng.minval.ival < 0)
                offset.ival = ((rng.minval.ival+1) / binsize.ival - 1) * binsize.ival;
              else
                offset.ival = (rng.minval.ival / binsize.ival) * binsize.ival;
            rti = (uint64) ((rng.maxval.ival - offset.ival) / binsize.ival);
            if (offset.ival + ((int64) nbins)*binsize.ival <= rng.maxval.ival)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Region)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.ival == 0)
          { double bwide;

            Region_Range(&rng,form,reg);
            bwide = (rng.maxval.ival - rng.minval.ival) / (1.*nbins);
            if (bwide == 0.)
              binsize.ival = 1;
            else
              { int64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.ival = x;
              }
              if (rng.minval.ival < 0)
                offset.ival = ((rng.minval.ival+1) / binsize.ival - 1) * binsize.ival;
              else
            nbins = (int) ((rng.maxval.ival - offset.ival) / binsize.ival);
            if (offset.ival + ((int64) nbins)*binsize.ival <= rng.maxval.ival)
              nbins += 1;
          } 
        else
          { if (FullSpan_ival[a->type] == 0 || offset.ival > MinOff_ival[a->type] ||
                offset.ival + ((int64) nbins)*binsize.ival < FullSpan_ival[a->type])
              { Region_Range(&rng,form,reg);
                clip = (offset.ival > rng.minval.ival ||
                        offset.ival + binsize.ival * ((int64) nbins) <= rng.maxval.ival);
              }
          }
        break;
      case FVAL:
        if (nbins == 0)
          { uint64 rti;

            Region_Range(&rng,form,reg);
            if (binsize.fval == 0)
              binsize.fval = 1;
              offset.fval = floor(rng.minval.fval / binsize.fval) * binsize.fval;
            rti = (uint64) ((rng.maxval.fval - offset.fval) / binsize.fval);
            if (offset.fval + ((float64) nbins)*binsize.fval <= rng.maxval.fval)
              rti += 1;
            if (rti > 0x7FFFFFFFull)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Region)\n");
                exit (1);
              }
            nbins = (int) rti;
          }
        else if (binsize.fval == 0)
          { double bwide;

            Region_Range(&rng,form,reg);
            bwide = (rng.maxval.fval - rng.minval.fval) / (1.*nbins);
            if (bwide == 0.)
              binsize.fval = 1;
            else
              { double x = pow(10.,floor(log10(bwide)));
                if (x < bwide)
                  { if (2*x < bwide)
                      { if (5*x < bwide)
                          x = 10*x;
                        else
                          x = 5*x;
                      }
                    else
                      x = 2*x;
                  }
                binsize.fval = x;
              }
              offset.fval = floor(rng.minval.fval / binsize.fval) * binsize.fval;
            nbins = (int) ((rng.maxval.fval - offset.fval) / binsize.fval);
            if (offset.fval + ((float64) nbins)*binsize.fval <= rng.maxval.fval)
              nbins += 1;
          } 
        else
          { if (FullSpan_fval[a->type] == 0 || offset.fval > MinOff_fval[a->type] ||
                offset.fval + ((float64) nbins)*binsize.fval < FullSpan_fval[a->type])
              { Region_Range(&rng,form,reg);
                clip = (offset.fval > rng.minval.fval ||
                        offset.fval + binsize.fval * ((float64) nbins) <= rng.maxval.fval);
              }
          }
        break;
  }

  { Histofull *h     = new_histofull(nbins*SIZEOF(Size_Type),"Histogram_Array");
    Size_Type *count = h->counts;
    Size_Type  size  = AForm_Size(form);
    Indx_Type  p;

    h->kind    = type2kind[a->type];
    h->binsize = binsize;
    h->offset  = offset;
    h->nbins   = nbins;

    for (p = 0; p < nbins; p++)
      count[p] = 0;

    switch (a->type) {
        case UINT8_TYPE:
          { uint8 *v = AUINT8(a);
            uint8  o = (uint8) offset.uval;
            uint8  b = (uint8) binsize.uval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case UINT16_TYPE:
          { uint16 *v = AUINT16(a);
            uint16  o = (uint16) offset.uval;
            uint16  b = (uint16) binsize.uval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case UINT32_TYPE:
          { uint32 *v = AUINT32(a);
            uint32  o = (uint32) offset.uval;
            uint32  b = (uint32) binsize.uval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case UINT64_TYPE:
          { uint64 *v = AUINT64(a);
            uint64  o = (uint64) offset.uval;
            uint64  b = (uint64) binsize.uval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case INT8_TYPE:
          { int8 *v = AINT8(a);
            int8  o = (int8) offset.ival;
            int8  b = (int8) binsize.ival;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case INT16_TYPE:
          { int16 *v = AINT16(a);
            int16  o = (int16) offset.ival;
            int16  b = (int16) binsize.ival;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case INT32_TYPE:
          { int32 *v = AINT32(a);
            int32  o = (int32) offset.ival;
            int32  b = (int32) binsize.ival;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case INT64_TYPE:
          { int64 *v = AINT64(a);
            int64  o = (int64) offset.ival;
            int64  b = (int64) binsize.ival;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_DREG
            break;
	  }
        case FLOAT32_TYPE:
          { float32 *v = AFLOAT32(a);
            float32  o = (float32) offset.fval;
            float32  b = (float32) binsize.fval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_FREG
            break;
	  }
        case FLOAT64_TYPE:
          { float64 *v = AFLOAT64(a);
            float64  o = (float64) offset.fval;
            float64  b = (float64) binsize.fval;
              Indx_Type *raster = reg->raster;
              Size_Type  len    = reg->rastlen;
              Indx_Type  vr, wr, k;
                FILL_BINS_FREG
            break;
	  }
    }
      { size = 0;
        for (p = 0; p < h->nbins; p++)
          size += count[p];
      }

    h->total = size;
    h->mean  = DBL_MIN;
    return ((Histogram *) h);
  }
}

Histogram *Histagain_Region(Histogram *h, Array *form, Region *reg, boolean clip)
{ Value      offset  = h->offset;
  Value      binsize = h->binsize;
  Size_Type  size    = AForm_Size(form);
  Size_Type *count   = h->counts;
  int        nbins   = h->nbins;
  Array     *a       = AForm_Array(form);
  Indx_Type  p;

  switch (a->type) {
      case UINT8_TYPE:
        { uint8 *v = AUINT8(a);
          uint8  o = (uint8) offset.uval;
          uint8  b = (uint8) binsize.uval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case UINT16_TYPE:
        { uint16 *v = AUINT16(a);
          uint16  o = (uint16) offset.uval;
          uint16  b = (uint16) binsize.uval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case UINT32_TYPE:
        { uint32 *v = AUINT32(a);
          uint32  o = (uint32) offset.uval;
          uint32  b = (uint32) binsize.uval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case UINT64_TYPE:
        { uint64 *v = AUINT64(a);
          uint64  o = (uint64) offset.uval;
          uint64  b = (uint64) binsize.uval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case INT8_TYPE:
        { int8 *v = AINT8(a);
          int8  o = (int8) offset.ival;
          int8  b = (int8) binsize.ival;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case INT16_TYPE:
        { int16 *v = AINT16(a);
          int16  o = (int16) offset.ival;
          int16  b = (int16) binsize.ival;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case INT32_TYPE:
        { int32 *v = AINT32(a);
          int32  o = (int32) offset.ival;
          int32  b = (int32) binsize.ival;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case INT64_TYPE:
        { int64 *v = AINT64(a);
          int64  o = (int64) offset.ival;
          int64  b = (int64) binsize.ival;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_DREG
          break;
        }
      case FLOAT32_TYPE:
        { float32 *v = AFLOAT32(a);
          float32  o = (float32) offset.fval;
          float32  b = (float32) binsize.fval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_FREG
          break;
        }
      case FLOAT64_TYPE:
        { float64 *v = AFLOAT64(a);
          float64  o = (float64) offset.fval;
          float64  b = (float64) binsize.fval;
            Indx_Type *raster = reg->raster;
            Size_Type  len    = reg->rastlen;
            Indx_Type  vr, wr, k;
              FILL_BINS_FREG
          break;
        }
  }

  size = 0;
  for (p = 0; p < h->nbins; p++)
    size += count[p];
  h->total = size;
  ((Histofull *) h)->mean = DBL_MIN;

  return (h);
}


Histogram *G(Histogram_P_Vertex)(Array *a, Partition *w, int cb,
                                 int nbins, Value binsize, Value offset)
{ Region    *c;
  Histogram *h;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Histogram_P_Vertex)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,0,1);
  h = Histogram_Region(a,c,nbins,binsize,offset);
  Free_Region(c);
  return (h);
}

Histogram *G(Histogram_Level_Set)(Array *a, Level_Tree *t, Level_Set *r,
                                  int nbins, Value binsize, Value offset)
{ Region    *c;
  Histogram *h;
  c = Record_Level_Set(t,r,0,1);
  h = Histogram_Region(a,c,nbins,binsize,offset);
  Free_Region(c);
  return (h);
}

Histogram *Histagain_P_Vertex(Histogram *R(M(h)), Array *a, Partition *w, int cb, boolean clip)
{ Region    *c;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Histagain_P_Vertex)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,0,1);
  Histagain_Region(h,a,c,clip);
  Free_Region(c);
  return (h);
}

Histogram *Histagain_Level_Set(Histogram *R(M(h)), Array *a, Level_Tree *t, Level_Set *r,
                               boolean clip)
{ Region    *c;
  c = Record_Level_Set(t,r,0,1);
  Histagain_Region(h,a,c,clip);
  Free_Region(c);
  return (h);
}

  /*  Generate a histogram based on the histogram h consisting of the bins in the interval
        [min,max).  min and max need not be between 0 and h->nbins but it must be that min < max.
        If min < 0 or max > h->nbins then g's domain will be expanded as necessary to cover
        the implied range of [Bin2Value(min),Bin2Value(max)].
  */

Histogram *G(Histogram_Slice)(Histogram *h, int min, int max)
{ Histofull *g;
  int        nbins;
  Value      off;
  int        i;

  switch (h->kind) {
      case UVAL:
        off.uval = h->offset.uval + h->binsize.uval * ((uint64) min);
        break;
      case IVAL:
        off.ival = h->offset.ival + h->binsize.ival * ((int64) min);
        break;
      case FVAL:
        off.fval = h->offset.fval + h->binsize.fval * ((float64) min);
        break;
  }

  nbins = max-min;
  if (nbins <= 0)
    { fprintf(stderr,"Requested bin interval is empty (Histogram_Slice)");
      exit (1);
    }

  g = new_histofull(nbins*SIZEOF(Size_Type),"Histogram_Slice");

  g->kind    = h->kind;
  g->binsize = h->binsize;
  g->offset  = off;
  g->nbins   = nbins;
  for (i = min; i < max; i++)
    if (i < 0 || i >= h->nbins)
      g->counts[i-min] = 0;
    else
      g->counts[i-min] = h->counts[i];

  g->mean = DBL_MIN;

  return ((Histogram *) g);
}


/****************************************************************************************
 *                                                                                      *
 *  BIN INDICES, DOMAIN VALUES, AND PERCENTILES                                         *
 *                                                                                      *
 ****************************************************************************************/

  /*  Routines to map between bin indices, domain values, and percentiles:

        Bin2Value: offset + b*binsize.
        Value2Bin: max_i s.t. Bin2Value(i) <= v

        Bin2Percentile: sum_(j>=i) count[j] / total
        Percentile2Bin: max_i s.t. Bin2Percentile(i) >= fraction

        Value2Percentile: Bin2Percentile(j) - (v-Value2Bin(j))*count[j]/total,
                            for j = Value2Bin(v)
        Percentile2Value: max_v s.t. Value2Percentile(v) >= fraction

      The bin index parameters do not need to be between 0 and h->nbins-1 and the bin index
      returned by Value2Bin may not be either depending on the value of v.  The fraction
      parameter however must be between 0 and 1, and values and bin indices returned by
      the percentile routines are always in range.
  */

Value Bin2Value(Histogram *h, int b)
{ Value v;
  switch (h->kind) {
      case UVAL:
        v.uval = h->offset.uval + h->binsize.uval * ((uint64) b);
        break;
      case IVAL:
        v.ival = h->offset.ival + h->binsize.ival * ((int64) b);
        break;
      case FVAL:
        v.fval = h->offset.fval + h->binsize.fval * ((float64) b);
        break;
  }
  return (v);
}

int Value2Bin(Histogram *h, Value v)
{ int bck;
  switch (h->kind) {
      case UVAL:
        { uint64 o = h->offset.uval;
          uint64 b = h->binsize.uval;
          if (v.uval < o)
            bck = - (int) ((o - v.uval) / b);
          else
            bck = (int) ((v.uval - o) / b);
          break;
        }
      case IVAL:
        { int64 o = h->offset.ival;
          int64 b = h->binsize.ival;
            bck = (int) ((v.ival - o) / b);
          break;
        }
      case FVAL:
        { float64 o = h->offset.fval;
          float64 b = h->binsize.fval;
            bck = (int) ((v.fval - o) / b);
          break;
        }
      default:
        bck = 0;
        break;
  }
  return (bck);
}

double Bin2Percentile(Histogram *h, int b)
{ Size_Type *count;
  int64      sum;
  int        i;

  count = h->counts;
  sum   = 0;
  for (i = b; i < h->nbins; i++)
    sum += count[i];
  return ((1.*sum)/h->total);
}

int Percentile2Bin(Histogram *h, double fraction)
{ Size_Type *count;
  int64      cthr, sum;
  int        i;

  cthr  = (int64) (h->total * fraction);
  count = h->counts;
  sum   = 0;
  if (cthr <= 0)
    return (h->nbins);
  for (i = h->nbins-1; i > 0; i--)
    { sum += count[i];
      if (sum >= cthr) break;
    }
  return (i);
}

double Value2Percentile(Histogram *h, Value v)
{ int b    = Value2Bin(h,v);
  switch (h->kind) {
      case UVAL:
        return (Bin2Percentile(h,b) - 
                   (((int64) (v.uval-Bin2Value(h,b).uval))*h->counts[b])/h->total);
      case IVAL:
        return (Bin2Percentile(h,b) - 
                   (((int64) (v.ival-Bin2Value(h,b).ival))*h->counts[b])/h->total);
      case FVAL:
        return (Bin2Percentile(h,b) - 
                   (((float64) (v.fval-Bin2Value(h,b).fval))*h->counts[b])/h->total);
  }
  return (0.);
}

Value Percentile2Value(Histogram *h, double fraction)
{ Size_Type *count;
  int64      cthr, sum;
  int        i;
  Value      v;

  cthr  = (int64) (h->total * fraction);
  count = h->counts;
  sum   = 0;
  if (cthr <= 0)
    return (Bin2Value(h,h->nbins));
  if (cthr >= h->total)
    return (Bin2Value(h,0));
  for (i = h->nbins-1; i > 0; i--)
    { sum += count[i];
      if (sum >= cthr) break;
    }
  switch (h->kind) {
      case UVAL:
        v.uval = h->offset.uval + h->binsize.uval * ((uint64) i) +
                (uint64) (h->binsize.uval * (1.*sum-cthr)/count[i]);
        break;
      case IVAL:
        v.ival = h->offset.ival + h->binsize.ival * ((int64) i) +
                (int64) (h->binsize.ival * (1.*sum-cthr)/count[i]);
        break;
      case FVAL:
        v.fval = h->offset.fval + h->binsize.fval * ((float64) i) +
                (float64) (h->binsize.fval * (1.*sum-cthr)/count[i]);
        break;
  }
  return (v);
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM STATISTICS                                                                *
 *                                                                                      *
 ****************************************************************************************/

static void set_histogram_mean(Histofull *h)
{ Size_Type *count;
  int        i;
  double     sum, b, u;

  count = h->counts;
  
  switch (h->kind) {
      case UVAL:
        b = (double) h->binsize.uval;
          u = h->offset.uval + .5*b - .5;
        break;
      case IVAL:
        b = (double) h->binsize.ival;
          u = h->offset.ival + .5*b - .5;
        break;
      case FVAL:
        b = (double) h->binsize.fval;
          u = h->offset.fval + .5*b;
        break;
      default:
        b = u = 0.;
        break;
  }

  sum  = 0.;
  for (i = 0; i < h->nbins; i++)
    { sum  += count[i] * u;
      u    += b;
    }

  h->mean  = sum / h->total;
}

double Histogram_Mean(Histogram *h)
{ if (((Histofull *) h)->mean == DBL_MIN)
    set_histogram_mean(((Histofull *) h));
  return (((Histofull *) h)->mean);
}

double Histogram_Variance(Histogram *h)
{ int        i;
  double     sum, b, u;
  Size_Type *count;

  if (((Histofull *) h)->mean == DBL_MIN)
    set_histogram_mean(((Histofull *) h));
  
  count = h->counts;

  switch (h->kind) {
      case UVAL:
        b = (double) h->binsize.uval;
          u = (h->offset.uval + .5*b - .5) - ((Histofull *) h)->mean;
        break;
      case IVAL:
        b = (double) h->binsize.ival;
          u = (h->offset.ival + .5*b - .5) - ((Histofull *) h)->mean;
        break;
      case FVAL:
        b = (double) h->binsize.fval;
          u = (h->offset.fval + .5*b) - ((Histofull *) h)->mean;
        break;
      default:
        u = b = 0.;
        break;
  }

  sum = 0.;
  for (i = 0; i < h->nbins; i++)
    { sum += count[i] * u * u;
      u += b;
    }
  return (sum/h->total);
}

double Histogram_Sigma(Histogram *h)
{ return (sqrt(Histogram_Variance(h))); }

double Histogram_Central_Moment(Histogram *h, int n)
{ int        i;
  double     sum, b, u;
  Size_Type *count;

  if (((Histofull *) h)->mean == DBL_MIN)
    set_histogram_mean(((Histofull *) h));
  
  count = h->counts;

  switch (h->kind) {
      case UVAL:
        b = (double) h->binsize.uval;
          u = (h->offset.uval + .5*b - .5) - ((Histofull *) h)->mean;
        break;
      case IVAL:
        b = (double) h->binsize.ival;
          u = (h->offset.ival + .5*b - .5) - ((Histofull *) h)->mean;
        break;
      case FVAL:
        b = (double) h->binsize.fval;
          u = (h->offset.fval + .5*b) - ((Histofull *) h)->mean;
        break;
      default:
        u = b = 0.;
        break;
  }

  sum = 0.;
  for (i = 0; i < h->nbins; i++)
    { sum += count[i] * pow(u,n);
      u   += b;
    }
  return (sum/h->total);
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM ENTROPY                                                                   *
 *                                                                                      *
 ****************************************************************************************/

   /*  Assuming the histogram h is a discrete probaility distribution as defined by the
         choice of bin size, Histogram_Entropy returns - sum_b p(b) log_2 p(b) where
         p(b) is counts[b]/total for each bin b.  Cross_Entropy is - sum_b p(b) log_2 q(b)
         where q(b) is the distribution for g.  The histograms h and g must have the same
         bin size and while their offsets can be different, the difference must be a
         multiple of the bin size.  Relative_Entropy is sum_b p(b) log_2 p(b)/q(b);
   */

static double  etable[100001];
static double  loge5, loge10, loge15;
static boolean firstentropy = 1;

static pthread_mutex_t Entropy_Mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void mylog2_table()
{ int i;

#ifdef _MSC_VER

  double logfac = 1./log(2.);

#define log2(e) (log(e)*logfac);

#endif

  for (i = 0; i <= 100000; i++)
    etable[i] = log2(i/100000.);
  loge5  = log2(1.e-5);
  loge10 = 2*loge5;
  loge15 = loge5+loge10;
}

static inline double mylog2(double p)
{ if (p >= 1e-5)
    return (etable[(int) (p*1.e5)]);
  else if (p >= 1e-10)
    return (loge5 + etable[(int) (p*1.e10)]);
  else if (p >= 1e-15)
    return (loge10 + etable[(int) (p*1.e15)]);
  else
    return (loge15 + etable[(int) (p*1.e20)]);
}

double Histogram_Entropy(Histogram *h)
{ Size_Type *count = h->counts;
  double     normal = 1./h->total;
  double     entropy;
  int        i;

  pthread_mutex_lock(&Entropy_Mutex);
  if (firstentropy)
    { firstentropy = 0;
      mylog2_table();
    }
  pthread_mutex_unlock(&Entropy_Mutex);

  entropy = 0.;
  for (i = 0; i < h->nbins; i++)
    { double p = count[i] * normal;
      if (p > 1.e-20)
        entropy -= p*mylog2(p);
    }
  return (entropy);
}

double Histogram_Cross_Entropy(Histogram *h, Histogram *g)
{ Size_Type *hcount = h->counts;
  Size_Type *gcount = g->counts;
  double     hnormal = 1./h->total;
  double     gnormal = 1./g->total;
  double     entropy;
  int64      disp;
  int        i, j;

  switch (h->kind) {
      case UVAL:
        { uint64 delt;
          if (h->binsize.uval != g->binsize.uval)
            { fprintf(stderr,"Histogram do not have same bin size (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          if (h->offset.uval >= g->offset.uval)
            disp = (int64) (delt = h->offset.uval - g->offset.uval);
          else
            disp = - (int64) (delt = g->offset.uval - h->offset.uval);
          if (delt % h->binsize.uval != 0)
            { fprintf(stderr,"Histogram bin offsets not in synch (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          disp /= h->binsize.uval;
          break;
        }
      case IVAL:
        { int64 delt;
          if (h->binsize.ival != g->binsize.ival)
            { fprintf(stderr,"Histogram do not have same bin size (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          disp = delt = h->offset.ival - g->offset.ival;
          if (delt % h->binsize.ival != 0)
            { fprintf(stderr,"Histogram bin offsets not in synch (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          disp /= h->binsize.ival;
          break;
        }
      case FVAL:
        { float64 delt;
          if (h->binsize.fval != g->binsize.fval)
            { fprintf(stderr,"Histogram do not have same bin size (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          delt = (h->offset.fval - g->offset.fval) / h->binsize.fval;
          disp = (int64) delt;
          if (fabs(disp - delt) > 1.e-10)
            { fprintf(stderr,"Histogram bin offsets not in synch (Histogram_Cross_Entropy)\n");
              exit (1);
            }
          break;
        }
      default:
        disp = 0;
        break;
    }

  pthread_mutex_lock(&Entropy_Mutex);
  if (firstentropy)
    { firstentropy = 0;
      mylog2_table();
    }
  pthread_mutex_unlock(&Entropy_Mutex);

  entropy = 0.;
  for (i = 0, j = (int) disp; i < h->nbins; i++, j++)
    { double p = hcount[i] * hnormal;
      double q;
      if (0 <= j && j < g->nbins)
        { q = gcount[j] * gnormal;
          if (q < 1.e-20)
            q = 1.e-20;
        }
      else
        q = 1.e-20;
      entropy -= p*mylog2(q);
    }

  return (entropy);
}

double Histogram_Relative_Entropy(Histogram *h, Histogram *g)
{ return (Histogram_Cross_Entropy(h,g) - Histogram_Entropy(h)); }


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM THRESHOLDS                                                                *
 *                                                                                      *
 ****************************************************************************************/

  /*  Compute the Otsu threshold value for an image based on its histogram h: this value is
        only to the resolution of the bin size of the histogram, so a bin index b is
        returned, the implication being that everyting >= Bin2Value(b) is considered
        foreground.
  */

int Otsu_Threshold(Histogram *h)
{ int        i, t;
  Size_Type  c, pden, tden;
  double     psum, tsum;
  double     var, max;
  double     b, u;
  Size_Type *count;

  if (((Histofull *) h)->mean == DBL_MIN)
    set_histogram_mean(((Histofull *) h));

  count = h->counts;

  switch (h->kind) {
      case UVAL:
        b = (double) h->binsize.uval;
          u = h->offset.uval + .5*b - .5;
        break;
      case IVAL:
        b = (double) h->binsize.ival;
          u = h->offset.ival + .5*b - .5;
        break;
      case FVAL:
        b = (double) h->binsize.fval;
          u = h->offset.fval + .5*b;
        break;
      default:
        u = b = 0.;
        break;
  }

  tden = h->total;
  tsum = tden * ((Histofull *) h)->mean;

  pden = 0;
  psum = 0.;
  max  = 0.;
  t    = 0;
  for (i = 0; i < h->nbins-1; i++)
    { c = count[i];
      pden += c;
      psum += c * u;
      tden -= c;
      u    += b;
      var = psum/pden - (tsum-psum)/tden;
      var = (pden*var)*(tden*var);
      if (var > max)
        { max = var;
          t   = i;
        }
    }

  return (t+1);
}

  /*  Similarly, slope threshold returns the inflection point (to the nearest bin boundary)
        of the histogram relative to a line from the maximum bin to the larget non-zero bin.
  */

int Triangle_Threshold(Histogram *h)
{ int        i;
  int        low, hgh;
  Size_Type  c, max;
  Size_Type *count;
  double     slope;

  count = h->counts;

  low = hgh = 0;
  max = count[0];
  for (i = 1; i < h->nbins; i++)
    { c = count[i];
      if (c > max)
        { max = count[i];
          low = i;
        }
      if (c > 0)
        hgh = i;
    }
  hgh = hgh+1;

  slope = (1.*max) / (hgh-low);
  max   = 0;
  for (i = low+1; i < hgh; i++)
    { c = (Size_Type) ((hgh - i) * slope) - count[i];
      if (c > max)
        { max = c;
          low = i;
        }
    }

  return (low+1);
}

int Intermeans_Threshold(Histogram *h)
{ int        i, t, n;
  int        low, hgh;
  Size_Type  sum1, size1;
  Size_Type  sumt, sizet;
  Size_Type *count;
  double     mean1, mean2;

  count = h->counts;

  sumt  = 0;
  sizet = 0;
  low   = -1;
  hgh   = -1;
  for (i = 0; i < h->nbins; i++)
    { Size_Type c = count[i];
      if (c > 0)
        { if (low < 0)
            low = i;
          hgh = i;
          sizet += c;
          sumt  += c * i;
        }
    }

  t = (low+hgh)/2;
  n = -1;
  while (t != n)
    { n = t;

      sum1  = 0;
      size1 = 0;
      for (i = low; i < t; i++)
        { sum1  += count[i] * i;
          size1 += count[i];
        }
      mean1 = (double) ((1.*sum1) / size1);
      mean2 = (double) ((1. * ( sumt - sum1 )) / (sizet - size1));
      t = (int) (.5 * (mean1 + mean2));
    }
  
  return (t);
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM DISPLAY                                                                   *
 *                                                                                      *
 ****************************************************************************************/

  /*  Print an ascii display of histogram h on FILE output indented by indent spaces.
      The parameter flag is the bitwise or of the defined constants below which determine
      what is displayed and how it is displayed.  If binsize is not 0 then the histogram
      will be displayed in bins of the given size, with the bin boundaries being multiples
      of binsize (the counts of spanning bins in the underlying histogram are interpolated).

        BIN_COUNT            0x01   //  show bin counts
        CUMULATIVE_COUNT     0x02   //  show cumulative counts
        CUMULATIVE_PERCENT   0x04   //  show cumulative per cent of the total
        ASCENDING_HGRAM      0x08   //  display in ascending order (descending by default)
          CLIP_HGRAM_HIGH    0x10   //  do not display any 0 count bins at the top
          CLIP_HGRAM_LOW     0x20   //  do not display any 0 count bins at the bottom
        CLIP_HGRAM           0x30   //  do not display any 0 count bins at either extreme
        BIN_MIDDLE           0x40   //  display the mid-value of a bin as opposed to its range
  */

void Print_Histogram(Histogram *h, FILE *output, int indent, int flags, Value binsize)
{ Size_Type sum, pre, *count;
  double    total;
  int       i, j, top, bot, inc;
  int       rwidth, dwidth, lwidth;
  int       bflag, cflag, pflag, mflag;
  int       exp;

  count = h->counts;

  if (flags == 0)
    flags = BIN_COUNT;

  if ((flags & CLIP_HGRAM_HIGH) != 0)
    { for (top = h->nbins-1; top >= 0; top--)
        if (count[top] != 0)
          break;
    }
  else
    top = h->nbins-1;

  if ((flags & CLIP_HGRAM_LOW) != 0)
    { for (bot = 0; bot < h->nbins; bot++)
        if (count[bot] != 0)
          break;
    }
  else
    bot = 0;

  if (top < bot)
    { fprintf(output,"%*sEmpty histogram!\n",indent,"");
      return;
    }

  bflag = ((flags & BIN_COUNT) != 0);
  cflag = ((flags & CUMULATIVE_COUNT) != 0);
  pflag = ((flags & CUMULATIVE_PERCENT) != 0);
  mflag = ((flags & BIN_MIDDLE) != 0);

  if ((flags & ASCENDING_HGRAM) == 0)
    inc = -1;
  else
    inc = 1;

  total  = (double) h->total;
  dwidth = ((int) floor(log10(total))) + 1;
  total  = 100./total;

  switch (h->kind) {

      case UVAL:
        { uint64 b  = h->binsize.uval;
          uint64 o  = h->offset.uval;
          uint64 f  = h->offset.uval;

          uint64 u  = o + ((uint64) bot)*b;
          uint64 v  = o + ((uint64) (top+1))*b;

          uint64 B  = b;
          if (binsize.uval != 0)
            B = binsize.uval;

            bot = (int) (u/B);
            top = (int) ((v-1)/B+1);
          u = ((uint64) bot)*B;
          v = ((uint64) top)*B;

          if (v != 0)
            rwidth = ((int) floor(log10(fabs((double) v)))) + 1;
          else
            rwidth = 1;

          sum = 0;
          if (inc < 0)
            u = ((uint64) (top-1)) * B;
          else
            u = ((uint64) bot) * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (B == 1)
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u+(B-1)/2);
              else
                  if (B == 1)
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*llu - %*llu:",indent,"",rwidth,u,rwidth,u+(B-1));

              while (f > u)                            
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u)                       
                { j += 1;
                  f += b;
                }

              pre = sum;

              if (f < u && j >= 0 && j < h->nbins)      
                sum -= (Size_Type) (count[j] * ((u - f)/(1.*b)));
               
              while (f + b <= v)                        
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v && j >= 0 && j < h->nbins)      
                sum += (Size_Type) (count[j] * ((v - f)/(1.*b)));

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }

      case IVAL:
        { int64 b  = h->binsize.ival;
          int64 o  = h->offset.ival;
          int64 f  = h->offset.ival;

          int64 u  = o + ((int64) bot)*b;
          int64 v  = o + ((int64) (top+1))*b;

          int64 B  = b;
          if (binsize.ival != 0)
            B = binsize.ival;

            if (u < 0)
              bot = (int) ((u+1)/B-1);
            else
              bot = (int) (u/B);
            if (v < 0)
              top = (int) (v/B);
            else
              top = (int) ((v-1)/B+1);
          u = ((int64) bot)*B;
          v = ((int64) top)*B;

          if (v != 0)
            rwidth = ((int) floor(log10(fabs((double) v)))) + 1;
          else
            rwidth = 1;
            if (u < 0)
              lwidth = ((int) floor(log10((double) -u))) + 2;
            else
              lwidth = ((int) floor(log10((double) u))) + 1;
            if (lwidth > rwidth)
              rwidth = lwidth;

          sum = 0;
          if (inc < 0)
            u = ((int64) (top-1)) * B;
          else
            u = ((int64) bot) * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (B == 1)
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u+(B-1)/2);
              else
                  if (B == 1)
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*lld - %*lld:",indent,"",rwidth,u,rwidth,u+(B-1));

              while (f > u)                            
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u)                       
                { j += 1;
                  f += b;
                }

              pre = sum;

              if (f < u && j >= 0 && j < h->nbins)      
                sum -= (Size_Type) (count[j] * ((u - f)/(1.*b)));
               
              while (f + b <= v)                        
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v && j >= 0 && j < h->nbins)      
                sum += (Size_Type) (count[j] * ((v - f)/(1.*b)));

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }

      case FVAL:
        { float64 b  = h->binsize.fval;
          float64 o  = h->offset.fval;
          float64 f  = h->offset.fval;

          float64 u  = o + ((float64) bot)*b;
          float64 v  = o + ((float64) (top+1))*b;
          float64 eps = b*1e-8;             

          float64 B  = b;
          if (binsize.fval != 0)
            B = binsize.fval;

            bot = (int) floor((u+eps)/B);
            top = (int) ceil((v-eps)/B);
          u = ((float64) bot)*B;
          v = ((float64) top)*B;

          if (v != 0)
            rwidth = ((int) floor(log10(fabs((double) v)))) + 1;
          else
            rwidth = 1;
            if (u < 0)
              { lwidth = ((int) floor(log10((double) -u))) + 1;
                if (lwidth > rwidth)
                  rwidth = lwidth;
              }
            lwidth = (int) floor(log10(B));
            if (rwidth > 9 && lwidth > 4)
              { exp    = 1;
                rwidth = (rwidth-lwidth)+5;
                lwidth = rwidth-6;
                if (u < 0) rwidth += 1;
              }
            else if (lwidth < -9 && rwidth < -4)
              { exp = 1;
                rwidth = 5+(rwidth-lwidth);
                lwidth = rwidth-6;
                if (u < 0) rwidth += 1;
              }
            else
              { exp = 0;
                if (lwidth > 0)
                  lwidth = 0;
                else
                  { int ten, w;
                    lwidth = -lwidth;
                    ten = 1;
                    for (w = 0; w < lwidth; w++)
                      ten *= 10;
                    for (w = 0; w < 8; w++)
                      { if (ten*B - ((int) (ten*B)) < .01)
                          break;
                        ten *= 10;
                      }
                    lwidth += w;
                  }
                if (rwidth <= 0)
                  { rwidth = lwidth+2;
                    if (u < 0) rwidth += 1;
                  }
                else
                  { if (u < 0)
                      { if (ceil(log10((double) -u)) >= rwidth)
                          rwidth += 1;
                      }
                    if (lwidth > 0)
                      rwidth += lwidth + 1;
                  }
              }

          sum = 0;
          if (inc < 0)
            u = ((float64) (top-1)) * B;
          else
            u = ((float64) bot) * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (exp)
                    fprintf(output,"%*s%*.*e:",indent,"",rwidth,lwidth,u+B/2.);
                  else if (B == 1)
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u);
                  else
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u+B/2.);
              else
                  if (exp)
                    fprintf(output,"%*s%*.*e - %*.*e:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);
                  else
                    fprintf(output,"%*s%*.*f - %*.*f:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);

              while (f > u+eps)                        
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u+eps)                   
                { j += 1;
                  f += b;
                }

              pre = sum;

              if (f < u-eps && j >= 0 && j < h->nbins)  
                sum -= (Size_Type) (count[j] * ((u - f)/(1.*b)));
               
              while (f + b <= v+eps)                    
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v-eps && j >= 0 && j < h->nbins)  
                sum += (Size_Type) (count[j] * ((v - f)/(1.*b)));

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }
  }
}
