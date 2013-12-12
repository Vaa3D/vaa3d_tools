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
*  Region Data Abstractions                                                               *
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
#include "connectivity.h"
#include "array.h"
#include "level.set.h"
#include "water.shed.h"
#include "region.h"

#undef  DEBUG_CONTOUR
#undef  DEBUG_RECORD


static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

/****************************************************************************************
 *                                                                                      *
 *  2D CONTOUR ABSTRACTION: TRACE_CONTOUR, SPACE MANAGEMENT                             *
 *                                                                                      *
 ****************************************************************************************/

//  Awk-generated (manager.awk) Contour space management

typedef struct
  { int       length;          // Length of the contour
    boolean   boundary;        // Does contour touch the image boundary?
    int       dims[2];         // Width & height of image from which contour was derived
    int       iscon4;          // Is this of a 4- or 8-connected region?
    Indx_Type seed;            // Start pixel for contour (is leftmost)
    int       direct;          // Initial direction of tour;
    uint8    *tour;            // Edge direction defining outer contour of the region
  } Ctour;

static inline int ctour_tsize(Ctour *contour)
{ return (contour->length); }


typedef struct __Ctour
  { struct __Ctour *next;
    struct __Ctour *prev;
    int             refcnt;
    int             tsize;
    Ctour           ctour;
  } _Ctour;

static _Ctour *Free_Ctour_List = NULL;
static _Ctour *Use_Ctour_List  = NULL;

static pthread_mutex_t Ctour_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Ctour_Offset = sizeof(_Ctour)-sizeof(Ctour);
static int Ctour_Inuse  = 0;

int Contour_Refcount(Contour *contour)
{ _Ctour *object = (_Ctour *) (((char *) contour) - Ctour_Offset);
  return (object->refcnt);
}

static inline int allocate_ctour_tour(Ctour *ctour, int tsize, char *routine)
{ _Ctour *object = (_Ctour *) (((char *) ctour) - Ctour_Offset);
  if (object->tsize < tsize)
    { void *x = Guarded_Realloc(ctour->tour,(size_t) tsize,routine);
      if (x == NULL) return (1);
      ctour->tour = x;
      object->tsize = tsize;
    }
  return (0);
}

static inline int sizeof_ctour_tour(Ctour *ctour)
{ _Ctour *object = (_Ctour *) (((char *) ctour) - Ctour_Offset);
  return (object->tsize);
}

static inline void kill_ctour(Ctour *ctour);

static inline Ctour *new_ctour(int tsize, char *routine)
{ _Ctour *object;
  Ctour  *ctour;

  pthread_mutex_lock(&Ctour_Mutex);
  if (Free_Ctour_List == NULL)
    { object = (_Ctour *) Guarded_Realloc(NULL,sizeof(_Ctour),routine);
      if (object == NULL) return (NULL);
      ctour = &(object->ctour);
      object->tsize = 0;
      ctour->tour = NULL;
    }
  else
    { object = Free_Ctour_List;
      Free_Ctour_List = object->next;
      ctour = &(object->ctour);
    }
  Ctour_Inuse += 1;
  object->refcnt = 1;
  if (Use_Ctour_List != NULL)
    Use_Ctour_List->prev = object;
  object->next = Use_Ctour_List;
  object->prev = NULL;
  Use_Ctour_List = object;
  pthread_mutex_unlock(&Ctour_Mutex);
  if (allocate_ctour_tour(ctour,tsize,routine))
    { kill_ctour(ctour);
      return (NULL);
    }
  return (ctour);
}

static inline Ctour *copy_ctour(Ctour *ctour)
{ Ctour *copy = new_ctour(ctour_tsize(ctour),"Copy_Contour");
  void *_tour = copy->tour;
  *copy = *ctour;
  copy->tour = _tour;
  if (ctour->tour != NULL)
    memcpy(copy->tour,ctour->tour,(size_t) ctour_tsize(ctour));
  return (copy);
}

Contour *Copy_Contour(Contour *contour)
{ return ((Contour *) copy_ctour(((Ctour *) contour))); }

static inline int pack_ctour(Ctour *ctour)
{ _Ctour *object  = (_Ctour *) (((char *) ctour) - Ctour_Offset);
  if (object->tsize > ctour_tsize(ctour))
    { int ns = ctour_tsize(ctour);
      if (ns != 0)
        { void *x = Guarded_Realloc(ctour->tour,(size_t) ns,"Pack_Ctour");
          if (x == NULL) return (1);
          ctour->tour = x;
        }
      else
        { free(ctour->tour);
          ctour->tour = NULL;
        }
      object->tsize = ns;
    }
  return (0);
}

Contour *Pack_Contour(Contour *contour)
{ if (pack_ctour(((Ctour *) contour))) return (NULL);
  return (contour);
}

Contour *Inc_Contour(Contour *contour)
{ _Ctour *object  = (_Ctour *) (((char *) contour) - Ctour_Offset);
  pthread_mutex_lock(&Ctour_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Ctour_Mutex);
  return (contour);
}

static inline void free_ctour(Ctour *ctour)
{ _Ctour *object  = (_Ctour *) (((char *) ctour) - Ctour_Offset);
  pthread_mutex_lock(&Ctour_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Ctour_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Contour\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Ctour_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Ctour_List;
  Free_Ctour_List = object;
  Ctour_Inuse -= 1;
  pthread_mutex_unlock(&Ctour_Mutex);
}

void Free_Contour(Contour *contour)
{ free_ctour(((Ctour *) contour)); }

static inline void kill_ctour(Ctour *ctour)
{ _Ctour *object  = (_Ctour *) (((char *) ctour) - Ctour_Offset);
  pthread_mutex_lock(&Ctour_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Ctour_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Contour\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Ctour_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Ctour_Inuse -= 1;
  pthread_mutex_unlock(&Ctour_Mutex);
  if (ctour->tour != NULL)
    free(ctour->tour);
  free(((char *) ctour) - Ctour_Offset);
}

void Kill_Contour(Contour *contour)
{ kill_ctour(((Ctour *) contour)); }

static inline void reset_ctour()
{ _Ctour *object;
  Ctour  *ctour;
  pthread_mutex_lock(&Ctour_Mutex);
  while (Free_Ctour_List != NULL)
    { object = Free_Ctour_List;
      Free_Ctour_List = object->next;
      ctour = &(object->ctour);
      if (ctour->tour != NULL)
        free(ctour->tour);
      free(object);
    }
  pthread_mutex_unlock(&Ctour_Mutex);
}

void Reset_Contour()
{ reset_ctour(); }

int Contour_Usage()
{ return (Ctour_Inuse); }

void Contour_List(void (*handler)(Contour *))
{ _Ctour *a, *b;
  for (a = Use_Ctour_List; a != NULL; a = b)
    { b = a->next;
      handler((Contour *) &(a->ctour));
    }
}

static inline Ctour *read_ctour(FILE *input)
{ char name[7];
  Ctour *obj;
  Ctour read;
  fread(name,7,1,input);
  if (strncmp(name,"Contour",7) != 0)
    return (NULL);
  obj = new_ctour(0,"Read_Contour");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Ctour),1,input) == 0) goto error;
  obj->tour = read.tour;
  if (ctour_tsize(obj) != 0)
    { if (allocate_ctour_tour(obj,ctour_tsize(obj),"Read_Contour")) goto error;
      if (fread(obj->tour,(size_t) ctour_tsize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_ctour(obj);
  return (NULL);
}

Contour *Read_Contour(FILE *input)
{ return ((Contour *) read_ctour(input)); }

static inline void write_ctour(Ctour *ctour, FILE *output)
{ fwrite("Contour",7,1,output);
  fwrite(ctour,sizeof(Ctour),1,output);
  if (ctour_tsize(ctour) != 0)
    fwrite(ctour->tour,(size_t) ctour_tsize(ctour),1,output);
}

void Write_Contour(Contour *contour, FILE *output)
{ write_ctour(((Ctour *) contour),output); }

#define LEGAL_MOVE(good,p,d)		\
{ switch (d)				\
    { case 0:				\
        good = (p < top);		\
        break;				\
      case 1:				\
        good = ((p % width) < rgt);	\
        break;				\
      case 2:				\
        good = (p >= bot);		\
        break;				\
      case 3:				\
        good = ((p % width) > lft);	\
        break;				\
    }					\
}

Contour *Trace_Contour(APart *image, boolean iscon4, Indx_Type seed,
                       void *arg, boolean test(Indx_Type p, void *arg))

{ static char     *direction[] = { "S", "E", "N", "W" };

  Ctour  *my_cont;
  Array  *array = AForm_Array(image);

  Indx_Type  width;
  Indx_Type  rgt, lft, bot, top;
  Indx_Type  offset[4];
  Indx_Type  p, q, r;
  int        d, e;
  boolean    bnd, good;
  int        len;
  uint8     *tour;

  if (array->ndims != 2)
    { fprintf(stderr,"Image is not a 2D array (Trace_Contour)\n");
      exit (1);
    }

  width = array->dims[0];
  if (Is_Slice(image))
    { Dimn_Type *bcrd = ADIMN(Slice_First(image));
      Dimn_Type *ecrd = ADIMN(Slice_Last(image));

      lft  = bcrd[0];
      rgt  = ecrd[0];
      bot  = width * (bcrd[1]+1);
      top  = width * ecrd[1];
    }
  else
    { lft  = 0;
      rgt  = width-1;
      bot  = width;
      top  = width * (array->dims[1]-1);
    }

  offset[0] =  width;
  offset[1] =  1;
  offset[2] = -width;
  offset[3] = -1;

  len  = 1;
  bnd  = 0;
  p    = seed;
  d    = 0;
  good = 0;
#ifdef DEBUG_CONTOUR
  printf("\nContour:\n  (%3lld,%3lld) -> %s\n",p%width,p/width,direction[d]);
#endif

  if (iscon4)   // 4-connected contour
    do
      { LEGAL_MOVE(good,p,d);
        q = p + offset[d];
        if (good && test(q,arg))
          { e = (d+3) % 4;
            LEGAL_MOVE(good,q,e);
            r = q + offset[e];
            if (good && test(r,arg))
              { p = r;
                d = e;
#ifdef DEBUG_CONTOUR
                printf("  (%3lld,%3lld) -> (%3lld,%3lld) %s\n",q%width,q/width,
                                                       r%width,r/width,direction[d]);
                fflush(stdout);
#endif
              }
            else
              { p = q;
#ifdef DEBUG_CONTOUR
                printf("  (%3lld,%3lld) %s\n",q%width,q/width,direction[d]);
                fflush(stdout);
#endif
              }
          }
        else
          { d = (d+1) % 4;
            bnd = 1;
#ifdef DEBUG_CONTOUR
            printf("  . %s\n",direction[d]);
            fflush(stdout);
#endif
          }
        len += 1;
      }
    while (p != seed || d != 0);

  else                // 8-connected contour
    do
      { LEGAL_MOVE(good,p,d);
        q = p + offset[d];
        if (good)
          { e = (d+3) % 4;
            LEGAL_MOVE(good,q,e)
            r = q + offset[e];
            if (good && test(r,arg))
              { p = r;
                d = e;
#ifdef DEBUG_CONTOUR
                printf("  (%3lld,%3lld) %s\n",r%width,r/width,direction[d]);
                fflush(stdout);
#endif
              }
            else if (test(q,arg))
              { p = q;
#ifdef DEBUG_CONTOUR
                printf("  (%3lld,%3lld) %s\n",q%width,q/width,direction[d]);
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
        len += 1;
      }
    while (p != seed || d != 0);

  my_cont = new_ctour(len,"Trace_Contour");

  my_cont->length   = len;
  my_cont->boundary = bnd;
  my_cont->dims[0]  = width;
  my_cont->dims[1]  = array->dims[1];
  my_cont->iscon4   = iscon4;
  my_cont->seed     = seed;
  my_cont->direct   = 0;

  tour = my_cont->tour;

  len  = 0;
  p = seed;
  d = 0;

  tour[len++] = d;
  if (iscon4)   // 4-connected contour
    do
      { LEGAL_MOVE(good,p,d);
        q = p + offset[d];
        if (good && test(q,arg))
          { e = (d+3) % 4;
            LEGAL_MOVE(good,q,e);
            r = q + offset[e];
            if (good && test(r,arg))
              { p = r;
                d = e;
              }
            else
              p = q;
          }
        else
          d = (d+1) % 4;
        tour[len++] = d;
      }
    while (p != seed || d != 0);

  else                // 8-connected contour
    do
      { LEGAL_MOVE(good,p,d);
        q = p + offset[d];
        if (good)
          { e = (d+3) % 4;
            LEGAL_MOVE(good,q,e);
            r = q + offset[e];
            if (good && test(r,arg))
              { p = r;
                d = e;
              }
            else if (test(q,arg))
              p = q;
            else
              d = (d+1) % 4;
          }
        else
          d = (d+1) % 4;
        tour[len++] = d;
      }
    while (p != seed || d != 0);

  return ((Contour *) my_cont);
}

void For_Contour(Contour *ctour, void *arg, void handler(Indx_Type p, Indx_Type q, void *arg))
{ Ctour  *my_cont = (Ctour *) ctour;

  Indx_Type  width;
  Indx_Type  rgt, lft, bot, top;
  Indx_Type  offset[4];
  Indx_Type  p, q;
  int        d, e, x;
  boolean    good;
  int        i, len;
  uint8     *tour;

  width = my_cont->dims[0];
  len   = my_cont->length - 1;
  tour  = my_cont->tour;

  lft  = 0;
  rgt  = width-1;
  bot  = width;
  top  = width * (my_cont->dims[1]-1);

  offset[0] =  width;
  offset[1] =  1;
  offset[2] = -width;
  offset[3] = -1;

  p = my_cont->seed;
  LEGAL_MOVE(good,p,3);
  if (good)
    q = p-1;
  else
    q = -1;
  d = 0;
  handler(p,q,arg);
  for (i = 1; i < len; i++)
    { e = tour[i];
      x = (4+(e-d)) % 4;
      if (x == 0)
        p += offset[e];
      else if (x == 3)
        p += offset[d] + offset[e];
      d = (e+3)%4;
      LEGAL_MOVE(good,p,d)
      if (good)
        q = p+offset[d];
      else
        q = -1;
      handler(p,q,arg);
      d = e;
    }
}
//  static char     *direction[] = { "S", "E", "N", "W" };

Dimn_Type *Get_Contour_Dimensions(Contour *tour)
{ Ctour *cont = (Ctour *) tour;
  return (cont->dims);
}

boolean Get_Contour_Connectivity(Contour *tour)
{ Ctour *cont = (Ctour *) tour;
  return (cont->iscon4);
}

boolean Boundary_Countour(Contour *tour)
{ Ctour *cont = (Ctour *) tour;
  return (cont->boundary);
}


/****************************************************************************************
 *                                                                                      *
 *  REGION ABSTRACTION: RECORD_REGION, SPACE MANAGEMENT                                 *
 *                                                                                      *
 ****************************************************************************************/

//  Awk-generated (manager.awk) Region space management

typedef struct
  { Size_Type  rastlen;      // Length of the raster pair vector (always even)
    Indx_Type *raster;       // [0..surflen-1] of all surface pixels, with the sublist
    Size_Type  surflen;      // Length of all surface pixels (surflen >= rastlen)
    boolean    iscon2n;      // Is this of a 2n- or (3^n-1)-connected region?
    int        ndims;        // Dimensionality
    Dimn_Type *dims;         // Dims
                             // [0..rastlen-1] of 0-dim extreme pixels in sorted order (paired)
    uint8     *ishole;       // Is the space between a pair part of a hole?
    Size_Type  area;         // Surface area of the object
  } RasterCon;

#ifdef DEBUG_REGION

static void Print_Region(RasterCon *region, FILE *output)
{ int i;

  fprintf(output,"\nRegion [0..%llu..%llu] is2n = %d ndims = %d area = %llu\n",
                 region->rastlen,region->surflen,region->iscon2n,region->ndims,region->area);
  for (i = 0; i < region->rastlen; i += 2)
    fprintf(output,"  r %llu %llu (%d)\n",region->raster[i],region->raster[i+1],
                                          region->ishole[i>>1]);
  for (i = region->rastlen; i < region->surflen; i += 1)
    fprintf(output,"  s %llu\n",region->raster[i]);
}

#endif

#define SIZEOF(x) ((int) sizeof(x))

static inline int64 rastercon_rsize(RasterCon *region)
{ return (SIZEOF(Indx_Type) * region->surflen); }

static inline int64 rastercon_hsize(RasterCon *region)
{ return (SIZEOF(uint8) * (region->rastlen >> 1)); }

static inline int rastercon_dsize(RasterCon *region)
{ return (SIZEOF(int) * region->ndims); }


typedef struct __RasterCon
  { struct __RasterCon *next;
    struct __RasterCon *prev;
    int                 refcnt;
    int64              rsize;
    int64              hsize;
    int                 dsize;
    RasterCon           rastercon;
  } _RasterCon;

static _RasterCon *Free_RasterCon_List = NULL;
static _RasterCon *Use_RasterCon_List  = NULL;

static pthread_mutex_t RasterCon_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int RasterCon_Offset = sizeof(_RasterCon)-sizeof(RasterCon);
static int RasterCon_Inuse  = 0;

int Region_Refcount(Region *region)
{ _RasterCon *object = (_RasterCon *) (((char *) region) - RasterCon_Offset);
  return (object->refcnt);
}

static inline int allocate_rastercon_raster(RasterCon *rastercon, int64 rsize, char *routine)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  if (object->rsize < rsize)
    { void *x = Guarded_Realloc(rastercon->raster,(size_t) rsize,routine);
      if (x == NULL) return (1);
      rastercon->raster = x;
      object->rsize = rsize;
    }
  return (0);
}

static inline int64 sizeof_rastercon_raster(RasterCon *rastercon)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  return (object->rsize);
}

static inline int allocate_rastercon_ishole(RasterCon *rastercon, int64 hsize, char *routine)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  if (object->hsize < hsize)
    { void *x = Guarded_Realloc(rastercon->ishole,(size_t) hsize,routine);
      if (x == NULL) return (1);
      rastercon->ishole = x;
      object->hsize = hsize;
    }
  return (0);
}

static inline int64 sizeof_rastercon_ishole(RasterCon *rastercon)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  return (object->hsize);
}

static inline int allocate_rastercon_dims(RasterCon *rastercon, int dsize, char *routine)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  if (object->dsize < dsize)
    { void *x = Guarded_Realloc(rastercon->dims,(size_t) dsize,routine);
      if (x == NULL) return (1);
      rastercon->dims = x;
      object->dsize = dsize;
    }
  return (0);
}

static inline int sizeof_rastercon_dims(RasterCon *rastercon)
{ _RasterCon *object = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  return (object->dsize);
}

static inline void kill_rastercon(RasterCon *rastercon);

static inline RasterCon *new_rastercon(int64 rsize, int64 hsize, int dsize, char *routine)
{ _RasterCon *object;
  RasterCon  *rastercon;

  pthread_mutex_lock(&RasterCon_Mutex);
  if (Free_RasterCon_List == NULL)
    { object = (_RasterCon *) Guarded_Realloc(NULL,sizeof(_RasterCon),routine);
      if (object == NULL) return (NULL);
      rastercon = &(object->rastercon);
      object->rsize = 0;
      rastercon->raster = NULL;
      object->hsize = 0;
      rastercon->ishole = NULL;
      object->dsize = 0;
      rastercon->dims = NULL;
    }
  else
    { object = Free_RasterCon_List;
      Free_RasterCon_List = object->next;
      rastercon = &(object->rastercon);
    }
  RasterCon_Inuse += 1;
  object->refcnt = 1;
  if (Use_RasterCon_List != NULL)
    Use_RasterCon_List->prev = object;
  object->next = Use_RasterCon_List;
  object->prev = NULL;
  Use_RasterCon_List = object;
  pthread_mutex_unlock(&RasterCon_Mutex);
  if (allocate_rastercon_raster(rastercon,rsize,routine))
    { kill_rastercon(rastercon);
      return (NULL);
    }
  if (allocate_rastercon_ishole(rastercon,hsize,routine))
    { kill_rastercon(rastercon);
      return (NULL);
    }
  if (allocate_rastercon_dims(rastercon,dsize,routine))
    { kill_rastercon(rastercon);
      return (NULL);
    }
  return (rastercon);
}

static inline RasterCon *copy_rastercon(RasterCon *rastercon)
{ RasterCon *copy = new_rastercon(rastercon_rsize(rastercon),rastercon_hsize(rastercon),rastercon_dsize(rastercon),"Copy_Region");
  void *_raster = copy->raster;
  void *_ishole = copy->ishole;
  void *_dims = copy->dims;
  *copy = *rastercon;
  copy->raster = _raster;
  if (rastercon->raster != NULL)
    memcpy(copy->raster,rastercon->raster,(size_t) rastercon_rsize(rastercon));
  copy->ishole = _ishole;
  if (rastercon->ishole != NULL)
    memcpy(copy->ishole,rastercon->ishole,(size_t) rastercon_hsize(rastercon));
  copy->dims = _dims;
  if (rastercon->dims != NULL)
    memcpy(copy->dims,rastercon->dims,(size_t) rastercon_dsize(rastercon));
  return (copy);
}

Region *Copy_Region(Region *region)
{ return ((Region *) copy_rastercon(((RasterCon *) region))); }

static inline int pack_rastercon(RasterCon *rastercon)
{ _RasterCon *object  = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  if (object->rsize > rastercon_rsize(rastercon))
    { int64 ns = rastercon_rsize(rastercon);
      if (ns != 0)
        { void *x = Guarded_Realloc(rastercon->raster,(size_t) ns,"Pack_RasterCon");
          if (x == NULL) return (1);
          rastercon->raster = x;
        }
      else
        { free(rastercon->raster);
          rastercon->raster = NULL;
        }
      object->rsize = ns;
    }
  if (object->hsize > rastercon_hsize(rastercon))
    { int64 ns = rastercon_hsize(rastercon);
      if (ns != 0)
        { void *x = Guarded_Realloc(rastercon->ishole,(size_t) ns,"Pack_RasterCon");
          if (x == NULL) return (1);
          rastercon->ishole = x;
        }
      else
        { free(rastercon->ishole);
          rastercon->ishole = NULL;
        }
      object->hsize = ns;
    }
  if (object->dsize > rastercon_dsize(rastercon))
    { int ns = rastercon_dsize(rastercon);
      if (ns != 0)
        { void *x = Guarded_Realloc(rastercon->dims,(size_t) ns,"Pack_RasterCon");
          if (x == NULL) return (1);
          rastercon->dims = x;
        }
      else
        { free(rastercon->dims);
          rastercon->dims = NULL;
        }
      object->dsize = ns;
    }
  return (0);
}

Region *Pack_Region(Region *region)
{ if (pack_rastercon(((RasterCon *) region))) return (NULL);
  return (region);
}

Region *Inc_Region(Region *region)
{ _RasterCon *object  = (_RasterCon *) (((char *) region) - RasterCon_Offset);
  pthread_mutex_lock(&RasterCon_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&RasterCon_Mutex);
  return (region);
}

static inline void free_rastercon(RasterCon *rastercon)
{ _RasterCon *object  = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  pthread_mutex_lock(&RasterCon_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&RasterCon_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Region\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_RasterCon_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_RasterCon_List;
  Free_RasterCon_List = object;
  RasterCon_Inuse -= 1;
  pthread_mutex_unlock(&RasterCon_Mutex);
}

void Free_Region(Region *region)
{ free_rastercon(((RasterCon *) region)); }

static inline void kill_rastercon(RasterCon *rastercon)
{ _RasterCon *object  = (_RasterCon *) (((char *) rastercon) - RasterCon_Offset);
  pthread_mutex_lock(&RasterCon_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&RasterCon_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Region\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_RasterCon_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  RasterCon_Inuse -= 1;
  pthread_mutex_unlock(&RasterCon_Mutex);
  if (rastercon->dims != NULL)
    free(rastercon->dims);
  if (rastercon->ishole != NULL)
    free(rastercon->ishole);
  if (rastercon->raster != NULL)
    free(rastercon->raster);
  free(((char *) rastercon) - RasterCon_Offset);
}

void Kill_Region(Region *region)
{ kill_rastercon(((RasterCon *) region)); }

static inline void reset_rastercon()
{ _RasterCon *object;
  RasterCon  *rastercon;
  pthread_mutex_lock(&RasterCon_Mutex);
  while (Free_RasterCon_List != NULL)
    { object = Free_RasterCon_List;
      Free_RasterCon_List = object->next;
      rastercon = &(object->rastercon);
      if (rastercon->dims != NULL)
        free(rastercon->dims);
      if (rastercon->ishole != NULL)
        free(rastercon->ishole);
      if (rastercon->raster != NULL)
        free(rastercon->raster);
      free(object);
    }
  pthread_mutex_unlock(&RasterCon_Mutex);
}

void Reset_Region()
{ reset_rastercon(); }

int Region_Usage()
{ return (RasterCon_Inuse); }

void Region_List(void (*handler)(Region *))
{ _RasterCon *a, *b;
  for (a = Use_RasterCon_List; a != NULL; a = b)
    { b = a->next;
      handler((Region *) &(a->rastercon));
    }
}

static inline RasterCon *read_rastercon(FILE *input)
{ char name[6];
  RasterCon *obj;
  RasterCon read;
  fread(name,6,1,input);
  if (strncmp(name,"Region",6) != 0)
    return (NULL);
  obj = new_rastercon(0,0,0,"Read_Region");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(RasterCon),1,input) == 0) goto error;
  obj->raster = read.raster;
  if (rastercon_rsize(obj) != 0)
    { if (allocate_rastercon_raster(obj,rastercon_rsize(obj),"Read_Region")) goto error;
      if (fread(obj->raster,(size_t) rastercon_rsize(obj),1,input) == 0) goto error;
    }
  obj->ishole = read.ishole;
  if (rastercon_hsize(obj) != 0)
    { if (allocate_rastercon_ishole(obj,rastercon_hsize(obj),"Read_Region")) goto error;
      if (fread(obj->ishole,(size_t) rastercon_hsize(obj),1,input) == 0) goto error;
    }
  obj->dims = read.dims;
  if (rastercon_dsize(obj) != 0)
    { if (allocate_rastercon_dims(obj,rastercon_dsize(obj),"Read_Region")) goto error;
      if (fread(obj->dims,(size_t) rastercon_dsize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_rastercon(obj);
  return (NULL);
}

Region *Read_Region(FILE *input)
{ return ((Region *) read_rastercon(input)); }

static inline void write_rastercon(RasterCon *rastercon, FILE *output)
{ fwrite("Region",6,1,output);
  fwrite(rastercon,sizeof(RasterCon),1,output);
  if (rastercon_rsize(rastercon) != 0)
    fwrite(rastercon->raster,(size_t) rastercon_rsize(rastercon),1,output);
  if (rastercon_hsize(rastercon) != 0)
    fwrite(rastercon->ishole,(size_t) rastercon_hsize(rastercon),1,output);
  if (rastercon_dsize(rastercon) != 0)
    fwrite(rastercon->dims,(size_t) rastercon_dsize(rastercon),1,output);
}

void Write_Region(Region *region, FILE *output)
{ write_rastercon(((RasterCon *) region),output); }

typedef struct
  { Size_Type   size;
    Size_Type   area;
    Size_Type   bot;
    Size_Type   top;
    RasterCon  *reg;
    Indx_Type  *vec;
  } RegArg;

#define RA(a) ((RegArg *) (a))

boolean allocate_surface(Size_Type size, Size_Type area, Size_Type dbls, void *a)
{ allocate_rastercon_raster(RA(a)->reg,SIZEOF(Indx_Type)*(size+dbls),"Record_Region");
  RA(a)->vec  = RA(a)->reg->raster;
  RA(a)->top  = RA(a)->size = size+dbls;
  RA(a)->bot  = 0;
  RA(a)->area = area;
  return (1);
}

void add_surface(Indx_Type p, int x, void *a)
{ if (x)
    { RA(a)->vec[RA(a)->bot++] = p;
      if (x > 1)
        RA(a)->vec[RA(a)->bot++] = p;
    }
  else
    RA(a)->vec[--RA(a)->top] = p;
}

static int PSORT(const void *x, const void *y)
{ Indx_Type l = *((Indx_Type *) x);
  Indx_Type r = *((Indx_Type *) y);
  if (l < r)
    return (-1);
  else if (l > r)
    return (1);
  else
    return (0);
}

Region *G(Record_Region)(APart *source, int share, boolean iscon2n, Indx_Type leftmost,
                         boolean with_holes, void *argt, boolean (*test)(Indx_Type p, void *argt))
{ Array     *array = AForm_Array(source);
  RasterCon *region;
  Size_Type  rastop, more;
  Indx_Type *raster;
  RegArg     arg, *argp = &arg;

  region  = new_rastercon(0,0,array->ndims*SIZEOF(int),"Record_Region");
  arg.reg = region;
  Flood_Surface(source,share,iscon2n,leftmost,
                argt,test,NULL,NULL,argp,allocate_surface,argp,add_surface);

  rastop = arg.bot;
  raster = arg.vec;

  qsort(raster,(size_t) rastop,sizeof(Indx_Type),PSORT);

#ifdef DEBUG_RECORD
  { Indx_Type i;

    fprintf(stdout,"\nRegion [0..%llu..%llu] is2n = %d ndims = %d area = %llu\n",
                   rastop,arg.size,iscon2n,array->ndims,arg.area);
    for (i = 0; i < rastop; i += 2)
      fprintf(stdout,"  r %llu %llu\n",raster[i],raster[i+1]);
    for (i = rastop; i < arg.size; i += 1)
      fprintf(stdout,"  s %llu\n",raster[i]);
  }
#endif

  more = 0;
  if (with_holes)
    { Indx_Type i, v, w, p;

      for (i = 0; i < rastop; i += 2)
        { v = raster[i];
          w = raster[i+1];
          for (p = v+1; p < w; p++)
            if ( ! test(p,argt))
              { more += 2; 
                while ( ! test(p,argt))
                  p += 1;
              }
        }
    }

  { Indx_Type  i;

    region->iscon2n = iscon2n;
    region->ndims   = array->ndims;
    region->area    = arg.area;
    for (i = 0; i < region->ndims; i++)
      region->dims[i] = array->dims[i];
    region->rastlen = rastop + more;
    region->surflen = arg.size + more;

    allocate_rastercon_ishole(region,((region->rastlen)>>1)*SIZEOF(uint8),"Record_Region");

    if (with_holes)
      { Indx_Type v, w, p, j;
        uint8    *ishole = region->ishole;

        allocate_rastercon_raster(region,region->surflen*SIZEOF(Indx_Type),"Record_Region");

        raster = region->raster;

        memmove(raster+region->rastlen,raster+rastop,sizeof(Indx_Type)*((size_t)(arg.size-rastop)));

        j = region->rastlen-1;
        for (i = rastop; i-- > 0; i -= 1)
          { w = raster[i];
            v = raster[i-1];
            raster[j--]  = w;
            for (p = w-1; p > v; p--)
              if ( ! test(p,argt))
                { ishole[j>>1] = 1;
                  raster[j--]  = p+1;
                  p -= 1;
                  while ( ! test(p,argt))
                    p -= 1;
                  raster[j--] = p;
                }
            ishole[j>>1] = 0;
            raster[j--] = v;
          }
      }

    else
      memset(region->ishole, 0, ((size_t) ((region->rastlen)>>1))*sizeof(uint8));
  }

#ifdef DEBUG_RECORD
  Print_Region(region,stdout);
#endif

  return ((Region *) region);
}

typedef struct
  { void       *value;
    uint64      level_uval;
    int64       level_ival;
    float64     level_fval;
  } TestArg;

#define TA(a) ((TestArg *) (a))

    static boolean is_le_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] <= TA(a)->level_uval); }
    static boolean is_lt_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] < TA(a)->level_uval); }
    static boolean is_eq_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] == TA(a)->level_uval); }
    static boolean is_ne_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] != TA(a)->level_uval); }
    static boolean is_gt_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] > TA(a)->level_uval); }
    static boolean is_ge_uint8(Indx_Type p, void *a)
    { return (((uint8 *) TA(a)->value)[p] >= TA(a)->level_uval); }
    static boolean is_le_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] <= TA(a)->level_uval); }
    static boolean is_lt_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] < TA(a)->level_uval); }
    static boolean is_eq_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] == TA(a)->level_uval); }
    static boolean is_ne_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] != TA(a)->level_uval); }
    static boolean is_gt_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] > TA(a)->level_uval); }
    static boolean is_ge_uint16(Indx_Type p, void *a)
    { return (((uint16 *) TA(a)->value)[p] >= TA(a)->level_uval); }
    static boolean is_le_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] <= TA(a)->level_uval); }
    static boolean is_lt_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] < TA(a)->level_uval); }
    static boolean is_eq_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] == TA(a)->level_uval); }
    static boolean is_ne_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] != TA(a)->level_uval); }
    static boolean is_gt_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] > TA(a)->level_uval); }
    static boolean is_ge_uint32(Indx_Type p, void *a)
    { return (((uint32 *) TA(a)->value)[p] >= TA(a)->level_uval); }
    static boolean is_le_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] <= TA(a)->level_uval); }
    static boolean is_lt_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] < TA(a)->level_uval); }
    static boolean is_eq_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] == TA(a)->level_uval); }
    static boolean is_ne_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] != TA(a)->level_uval); }
    static boolean is_gt_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] > TA(a)->level_uval); }
    static boolean is_ge_uint64(Indx_Type p, void *a)
    { return (((uint64 *) TA(a)->value)[p] >= TA(a)->level_uval); }
    static boolean is_le_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] <= TA(a)->level_ival); }
    static boolean is_lt_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] < TA(a)->level_ival); }
    static boolean is_eq_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] == TA(a)->level_ival); }
    static boolean is_ne_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] != TA(a)->level_ival); }
    static boolean is_gt_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] > TA(a)->level_ival); }
    static boolean is_ge_int8(Indx_Type p, void *a)
    { return (((int8 *) TA(a)->value)[p] >= TA(a)->level_ival); }
    static boolean is_le_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] <= TA(a)->level_ival); }
    static boolean is_lt_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] < TA(a)->level_ival); }
    static boolean is_eq_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] == TA(a)->level_ival); }
    static boolean is_ne_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] != TA(a)->level_ival); }
    static boolean is_gt_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] > TA(a)->level_ival); }
    static boolean is_ge_int16(Indx_Type p, void *a)
    { return (((int16 *) TA(a)->value)[p] >= TA(a)->level_ival); }
    static boolean is_le_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] <= TA(a)->level_ival); }
    static boolean is_lt_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] < TA(a)->level_ival); }
    static boolean is_eq_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] == TA(a)->level_ival); }
    static boolean is_ne_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] != TA(a)->level_ival); }
    static boolean is_gt_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] > TA(a)->level_ival); }
    static boolean is_ge_int32(Indx_Type p, void *a)
    { return (((int32 *) TA(a)->value)[p] >= TA(a)->level_ival); }
    static boolean is_le_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] <= TA(a)->level_ival); }
    static boolean is_lt_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] < TA(a)->level_ival); }
    static boolean is_eq_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] == TA(a)->level_ival); }
    static boolean is_ne_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] != TA(a)->level_ival); }
    static boolean is_gt_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] > TA(a)->level_ival); }
    static boolean is_ge_int64(Indx_Type p, void *a)
    { return (((int64 *) TA(a)->value)[p] >= TA(a)->level_ival); }
    static boolean is_le_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] <= TA(a)->level_fval); }
    static boolean is_lt_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] < TA(a)->level_fval); }
    static boolean is_eq_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] == TA(a)->level_fval); }
    static boolean is_ne_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] != TA(a)->level_fval); }
    static boolean is_gt_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] > TA(a)->level_fval); }
    static boolean is_ge_float32(Indx_Type p, void *a)
    { return (((float32 *) TA(a)->value)[p] >= TA(a)->level_fval); }
    static boolean is_le_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] <= TA(a)->level_fval); }
    static boolean is_lt_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] < TA(a)->level_fval); }
    static boolean is_eq_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] == TA(a)->level_fval); }
    static boolean is_ne_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] != TA(a)->level_fval); }
    static boolean is_gt_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] > TA(a)->level_fval); }
    static boolean is_ge_float64(Indx_Type p, void *a)
    { return (((float64 *) TA(a)->value)[p] >= TA(a)->level_fval); }

static boolean (*Comparator_Table[])(Indx_Type, void *) = {
    is_le_uint8,
    is_lt_uint8,
    is_eq_uint8,
    is_ne_uint8,
    is_gt_uint8,
    is_ge_uint8,
    is_le_uint16,
    is_lt_uint16,
    is_eq_uint16,
    is_ne_uint16,
    is_gt_uint16,
    is_ge_uint16,
    is_le_uint32,
    is_lt_uint32,
    is_eq_uint32,
    is_ne_uint32,
    is_gt_uint32,
    is_ge_uint32,
    is_le_uint64,
    is_lt_uint64,
    is_eq_uint64,
    is_ne_uint64,
    is_gt_uint64,
    is_ge_uint64,
    is_le_int8,
    is_lt_int8,
    is_eq_int8,
    is_ne_int8,
    is_gt_int8,
    is_ge_int8,
    is_le_int16,
    is_lt_int16,
    is_eq_int16,
    is_ne_int16,
    is_gt_int16,
    is_ge_int16,
    is_le_int32,
    is_lt_int32,
    is_eq_int32,
    is_ne_int32,
    is_gt_int32,
    is_ge_int32,
    is_le_int64,
    is_lt_int64,
    is_eq_int64,
    is_ne_int64,
    is_gt_int64,
    is_ge_int64,
    is_le_float32,
    is_lt_float32,
    is_eq_float32,
    is_ne_float32,
    is_gt_float32,
    is_ge_float32,
    is_le_float64,
    is_lt_float64,
    is_eq_float64,
    is_ne_float64,
    is_gt_float64,
    is_ge_float64,
  };

Region *G(Record_Basic)(APart *source, int share, boolean iscon2n, Indx_Type leftmost,
                              boolean with_holes, Comparator cmprsn, Value level)
{ Array     *array = AForm_Array(source);
  boolean  (*test)(Indx_Type, void *);
  TestArg    argt;

  switch (array->type) {
      case UINT8_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT16_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT32_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT64_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case INT8_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT16_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT32_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT64_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case FLOAT32_TYPE:
        argt.value = array->data;
        argt.level_fval = level.fval;
        break;
      case FLOAT64_TYPE:
        argt.value = array->data;
        argt.level_fval = level.fval;
        break;
  }

  test = Comparator_Table[6*array->type + cmprsn];

  return (Record_Region(source,share,iscon2n,leftmost,with_holes,&argt,test));
}

Contour *G(Basic_Contour)(APart *image, boolean iscon4, Indx_Type seed,
                          Comparator cmprsn, Value level)
{ Array     *array = AForm_Array(image);
  boolean  (*test)(Indx_Type, void *);
  TestArg    argt;

  switch (array->type) {
      case UINT8_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT16_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT32_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case UINT64_TYPE:
        argt.value = array->data;
        argt.level_uval = level.uval;
        break;
      case INT8_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT16_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT32_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case INT64_TYPE:
        argt.value = array->data;
        argt.level_ival = level.ival;
        break;
      case FLOAT32_TYPE:
        argt.value = array->data;
        argt.level_fval = level.fval;
        break;
      case FLOAT64_TYPE:
        argt.value = array->data;
        argt.level_fval = level.fval;
        break;
  }

  test = Comparator_Table[6*array->type + cmprsn];

  return (Trace_Contour(image,iscon4,seed,&argt,test));
}

int Get_Region_Dimensionality(Region *region)
{ RasterCon *cont = (RasterCon *) region;
  return (cont->ndims);
}

Dimn_Type *Get_Region_Dimensions(Region *region)
{ RasterCon *cont = (RasterCon *) region;
  return (cont->dims);
}

boolean Get_Region_Connectivity(Region *region)
{ RasterCon *cont = (RasterCon *) region;
  return (cont->iscon2n);
}

void For_Region(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg))
{ RasterCon *trace = (RasterCon *) reg; 
  Indx_Type  len;
  Indx_Type *raster;

  raster = trace->raster;
  len    = trace->rastlen;

  { Indx_Type i, v, w, p;

    for (i = 0; i < len; i += 2)
      { v = raster[i];
        w = raster[i+1];
        for (p = v; p <= w; p++)
          handler(p,arg);
      }
  }
}

void For_Region_Outline(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg))
{ RasterCon *trace = (RasterCon *) reg; 
  Indx_Type  i, len;
  Indx_Type *raster;
  uint8     *ishole;

  raster = trace->raster;
  ishole = trace->ishole;
  len    = trace->rastlen;

  handler(raster[0],arg);
  for (i = 2; i < len; i += 2)
    if (!ishole[i>>1])
      { handler(raster[i-1],arg);
        handler(raster[i],arg);
      }

  len = trace->surflen;
  for (i = trace->rastlen-1; i < len; i++)
    handler(raster[i],arg);
}

void For_Region_Holes(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg))
{ RasterCon *trace = (RasterCon *) reg; 
  Indx_Type  len;
  Indx_Type *raster;
  uint8     *ishole;

  raster = trace->raster;
  ishole = trace->ishole;
  len    = trace->rastlen;

  { Indx_Type i, v, w, p;

    for (i = 2; i < len; i += 2)
      if (ishole[i>>1])
        { v = raster[i-1];
          w = raster[i];
          for (p = v+1; p < w; p++)
            handler(p,arg);
        }
  }
}

void For_Region_Exterior(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg))
{ RasterCon *trace = (RasterCon *) reg; 
  Indx_Type  len;
  Indx_Type *raster;
  uint8     *ishole;
  Size_Type  size;

  raster = trace->raster;
  ishole = trace->ishole;
  len    = trace->rastlen;

  { Indx_Type i, v, w, p;

    size = 1;
    for (i = 0; i < trace->ndims; i++)
      size *= trace->dims[i];

    for (p = 0; p < raster[0]; p++)
      handler(p,arg);
    for (i = 2; i < len; i += 2)
      if (!ishole[i>>1])
        { v = raster[i-1];
          w = raster[i];
          for (p = v+1; p < w; p++)
            handler(p,arg);
        }
    for (p = raster[len]+1; p < size; p++)
      handler(p,arg);
  }
}

Region *G(Record_Level_Set)(Level_Tree *t, Level_Set *r, boolean with_holes, int share)
{ APart  *image   = Get_Level_Tree_APart(t);
  boolean iscon2n = Get_Level_Tree_Connectivity(t);

  return (Record_Basic(image,share,iscon2n,Level_Set_Leftmost(t,r),with_holes,
                       GE_COMP,VALU(Level_Set_Level(t,r))));
}

Contour *G(Level_Set_Contour)(Level_Tree *t, Level_Set *r)
{ APart  *image   = Get_Level_Tree_APart(t);
  boolean iscon2n = Get_Level_Tree_Connectivity(t);

  return (Basic_Contour(image,iscon2n,Level_Set_Leftmost(t,r),
                        GE_COMP,VALU(Level_Set_Level(t,r))));
}

Region *G(Record_P_Vertex)(Partition *w, int cb, boolean with_holes, int share)

{ Array    *image   = Get_Partition_Labels(w);
  boolean   iscon2n = Is_Partition_2n_Connected(w);
  Indx_Type pixel   = Get_Partition_Vertex(w,cb)->seed;
  Value     val;

  if (image == NULL)
    { fprintf(stderr,"Partition does not have a label array (Record_P_Vertex)\n");
      exit (1);
    }

  switch (image->type) {
      case UINT8_TYPE:
        val = VALU(AUINT8(image)[pixel]);
        break;
      case UINT16_TYPE:
        val = VALU(AUINT16(image)[pixel]);
        break;
      case UINT32_TYPE:
        val = VALU(AUINT32(image)[pixel]);
        break;
      case UINT64_TYPE:
        val = VALU(AUINT64(image)[pixel]);
        break;
      case INT8_TYPE:
        val = VALI(AINT8(image)[pixel]);
        break;
      case INT16_TYPE:
        val = VALI(AINT16(image)[pixel]);
        break;
      case INT32_TYPE:
        val = VALI(AINT32(image)[pixel]);
        break;
      case INT64_TYPE:
        val = VALI(AINT64(image)[pixel]);
        break;
      case FLOAT32_TYPE:
        val = VALF(AFLOAT32(image)[pixel]);
        break;
      case FLOAT64_TYPE:
        val = VALF(AFLOAT64(image)[pixel]);
        break;
  }

  return (Record_Basic(image,share,iscon2n,pixel,with_holes,EQ_COMP,val));
}

Contour *G(P_Vertex_Contour)(Partition *w, int cb)

{ Array    *image   = Get_Partition_Labels(w);
  boolean   iscon2n = Is_Partition_2n_Connected(w);
  Indx_Type pixel   = Get_Partition_Vertex(w,cb)->seed;
  Value     val;

  if (image == NULL)
    { fprintf(stderr,"Partition does not have a label array (Record_P_Vertex)\n");
      exit (1);
    }

  switch (image->type) {
      case UINT8_TYPE:
        val = VALU(AUINT8(image)[pixel]);
        break;
      case UINT16_TYPE:
        val = VALU(AUINT16(image)[pixel]);
        break;
      case UINT32_TYPE:
        val = VALU(AUINT32(image)[pixel]);
        break;
      case UINT64_TYPE:
        val = VALU(AUINT64(image)[pixel]);
        break;
      case INT8_TYPE:
        val = VALI(AINT8(image)[pixel]);
        break;
      case INT16_TYPE:
        val = VALI(AINT16(image)[pixel]);
        break;
      case INT32_TYPE:
        val = VALI(AINT32(image)[pixel]);
        break;
      case INT64_TYPE:
        val = VALI(AINT64(image)[pixel]);
        break;
      case FLOAT32_TYPE:
        val = VALF(AFLOAT32(image)[pixel]);
        break;
      case FLOAT64_TYPE:
        val = VALF(AFLOAT64(image)[pixel]);
        break;
  }

  return (Basic_Contour(image,iscon2n,pixel,EQ_COMP,val));
}

Region *Fill_Region_Holes(Region *R(M(cont)))
{ RasterCon *region = (RasterCon *) cont;

  Indx_Type *raster;
  Size_Type  rlen, slen;
  uint8     *ishole;

  rlen   = region->rastlen;
  slen   = region->surflen;
  raster = region->raster;
  ishole = region->ishole;

  { Indx_Type i, j;

    j = 1;
    for (i = 2; i < rlen; i += 2)
      if ( ! ishole[i>>1]) 
        { raster[j++] = raster[i-1];
          raster[j++] = raster[i];
        }
    raster[j++] = raster[i-1];

    region->rastlen = j;

    for (i = rlen; i < slen; i++)
      raster[j++] = raster[i];

    region->surflen = j;

    rlen = (region->rastlen >> 1);
    for (i = 0; i < rlen; i++)
      ishole[i] = 0;
  }

#ifdef DEBUG_REGION
  Print_Region(region,stdout);
#endif

  return (cont);
}

//  Compute min and max values in 'a' of type 'type' with 'length' elements

Range_Bundle *Region_Range(Range_Bundle *R(O(rng)), APart *o, Region *reg)
{ RasterCon *cont = (RasterCon *) reg;
  Array     *a    = AForm_Array(o);
  Indx_Type *raster = cont->raster;
  Size_Type  len    = cont->rastlen;
  Indx_Type  k, v, w, p;

  switch (a->type) {
      case UINT8_TYPE:
        { uint8 *d = AUINT8(a);
          uint8  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.uval = max;
          rng->minval.uval = min;
          break;
        }
      case UINT16_TYPE:
        { uint16 *d = AUINT16(a);
          uint16  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.uval = max;
          rng->minval.uval = min;
          break;
        }
      case UINT32_TYPE:
        { uint32 *d = AUINT32(a);
          uint32  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.uval = max;
          rng->minval.uval = min;
          break;
        }
      case UINT64_TYPE:
        { uint64 *d = AUINT64(a);
          uint64  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.uval = max;
          rng->minval.uval = min;
          break;
        }
      case INT8_TYPE:
        { int8 *d = AINT8(a);
          int8  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.ival = max;
          rng->minval.ival = min;
          break;
        }
      case INT16_TYPE:
        { int16 *d = AINT16(a);
          int16  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.ival = max;
          rng->minval.ival = min;
          break;
        }
      case INT32_TYPE:
        { int32 *d = AINT32(a);
          int32  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.ival = max;
          rng->minval.ival = min;
          break;
        }
      case INT64_TYPE:
        { int64 *d = AINT64(a);
          int64  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.ival = max;
          rng->minval.ival = min;
          break;
        }
      case FLOAT32_TYPE:
        { float32 *d = AFLOAT32(a);
          float32  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.fval = max;
          rng->minval.fval = min;
          break;
        }
      case FLOAT64_TYPE:
        { float64 *d = AFLOAT64(a);
          float64  x, min, max;

          min = max = d[raster[0]];
          for (k = 0; k < len; k += 2)
            { v = raster[k];
              w = raster[k+1];
              for (p = v; p <= w; p++)
                { x = d[p];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          rng->maxval.fval = max;
          rng->minval.fval = min;
          break;
        }
  }

  return (rng);
}


/****************************************************************************************
 *                                                                                      *
 *  CONTOUR ANALYSIS: EXTENT, CENTER OF MASS                                            *
 *                                                                                      *
 ****************************************************************************************/

Extent_Bundle *Region_Extent(Extent_Bundle *R(O(bundle)), Region *region)
{ RasterCon *cont = (RasterCon *) region;

  int        ndims;
  Dimn_Type *dims;
  Size_Type  rlen;
  Indx_Type *raster;

  ndims = cont->ndims;
  dims  = cont->dims;

  raster = cont->raster;
  rlen   = cont->rastlen;

  { Array *e;

    e = bundle->min;
    if (e == NULL)
      bundle->min = Make_Array_With_Shape(PLAIN_KIND,DIMN_TYPE,Coord1(ndims));
    else if (e->size*type_size[e->type] < ndims*type_size[DIMN_TYPE])
      { Free_Array(e);
        bundle->min = Make_Array_With_Shape(PLAIN_KIND,DIMN_TYPE,Coord1(ndims));
      }
    else
      { e->kind  = PLAIN_KIND;
        e->type  = DIMN_TYPE;
        e->scale = DIMN_SCALE;
        e->ndims = 1;
        e->size  = ndims;
        e->dims[0] = ndims;
      }

    e = bundle->max;
    if (e == NULL)
      bundle->max = Make_Array_With_Shape(PLAIN_KIND,DIMN_TYPE,Coord1(ndims));
    else if (e->size*type_size[e->type] < ndims*type_size[DIMN_TYPE])
      { Free_Array(e);
        bundle->max = Make_Array_With_Shape(PLAIN_KIND,DIMN_TYPE,Coord1(ndims));
      }
    else
      { e->kind  = PLAIN_KIND;
        e->type  = DIMN_TYPE;
        e->scale = DIMN_SCALE;
        e->ndims = 1;
        e->size  = ndims;
        e->dims[0] = ndims;
      }
  }

  { Indx_Type  p, i;
    Dimn_Type  o, x;
    int        d;
    Dimn_Type *min, *max;
    Dimn_Type  min0, max0, dim0;

    min = ADIMN(bundle->min);
    max = ADIMN(bundle->max);

    for (d = 0; d < ndims; d++)
      { max[d] = 0;
        min[d] = dims[d];
      }

    min0 = min[0];
    max0 = max[0];
    dim0 = dims[0];
    for (i = 0; i < rlen; i += 2)
      { o = dim0;
        p = raster[i+1];
        x = (Dimn_Type) (p%o);
        if (x > max0)
          max0 = x;
        p = raster[i];
        x = (Dimn_Type) (p%o);
        if (x < min0)
          min0 = x;
        for (d = 1; d < ndims; d++)
          { p /= o;
            o  = dims[d];
            x  = (Dimn_Type) (p%o);
            if (x > max[d])
              max[d] = x;
            if (x < min[d])
              min[d] = x;
          }
      }
    min[0] = min0;
    max[0] = max0;
  }

  return (bundle);
}

boolean Touches_Boundary(Extent_Bundle *bundle, APart *part)
{ Array     *array = AForm_Array(part);
  int        i, ndims;
  Dimn_Type *low, *hgh;
  Dimn_Type *min, *max;

  min     = ADIMN(bundle->min);
  max     = ADIMN(bundle->max);
  ndims   = array->ndims;

  if (Is_Slice(part))
    { low = ADIMN(Slice_First(part));
      hgh = ADIMN(Slice_Last(part));
      for (i = 0; i < ndims; i++)
        if (min[i] <= low[i] || max[i] >= hgh[i])
          return (1);
    }
  else
    { hgh = array->dims;
      for (i = 0; i < ndims; i++)
        if (min[i] <= 0 || max[i] >= hgh[i]-1)
          return (1);
    }

  return (0);
}

/* Return the area covered by the outer region */

Size_Type Region_Volume(Region *reg)
{ Indx_Type *raster, i;
  Size_Type  rlen, area;

  RasterCon *cont = (RasterCon *) reg;

  raster = cont->raster;
  rlen   = cont->rastlen;

  area = (rlen >> 1);
  for (i = 0; i < rlen; i += 2)
    area += raster[i+1] - raster[i];
  return (area);
}

Size_Type Region_Area(Region *reg)
{ RasterCon *cont = (RasterCon *) reg;
  return (cont->area);
}

/* Assuming all pixels have equal weight, return the sub-pixel coordinate
   that is at the center of mass of the region defined by "region" */

Double_Vector *G(Region_COM)(Double_Vector *R(O(com)), Region *reg)
{ Indx_Type     *raster;
  Size_Type      rlen, cnt;
  int            ndims;
  Dimn_Type     *dims;
  int64          Sum[10], *sum;

  RasterCon *cont = (RasterCon *) reg;

  raster = cont->raster;
  rlen   = cont->rastlen;

  ndims = cont->ndims;
  dims  = cont->dims;

  if (com == NULL)
    com = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(ndims));
  else if (com->size < ndims)
    { Free_Array(com);
      com = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(ndims));
    }
  else
    { com->kind  = PLAIN_KIND;
      com->type  = FLOAT64_TYPE;
      com->scale = 64;
      com->ndims = 1;
      com->size  = ndims;
      com->dims[0] = ndims;
    }

  if (ndims > 10)
    sum = (int64 *) Guarded_Malloc(sizeof(uint64)*((size_t) ndims),"Region_COM");
  else
    sum = Sum;

  { Indx_Type  v, w, c, i;
    Dimn_Type  o;
    int        d;
    double    *x;

    x = AFLOAT64(com);

    cnt = 0;
    for (d = 0; d < ndims; d++)
      sum[d] = 0;

    for (i = 0; i < rlen; i += 2)
      { c = 1;
        o = dims[0];

        v = raster[i];
        w = (raster[i+1] - v) + 1;

        cnt    += w;
        sum[0] += (((((v%o) << 1) + (w-1)) * w) >> 1);
        for (d = 1; d < ndims; d++)
          { c *= o;
            o  = dims[d];
            sum[d] += ((v/c)%o) * w;
          }
      }

    if (cnt == 0)
      return (0);

    for (d = 0; d < ndims; d++)
      x[d] = (1.*sum[d]) / cnt;

    if (ndims > 10)
      free(sum);

    return (com);
  }
}

/* Assuming all pixels have equal weight, return the sub-pixel coordinate
   that is at the center of mass of the pixels in "region" that are level
   or brigher.                                                     */

Double_Vector *G(Region_Select_COM)(Double_Vector *R(O(com)), Region *reg,
                                    APart *source, Comparator cmprsn, Value level)
{ Array     *array = AForm_Array(source);
  boolean  (*test)(Indx_Type, void *);
  Indx_Type *raster;
  Size_Type  rlen, cnt;
  int        ndims;
  Dimn_Type *dims;
  int64      Sum[10], *sum;
  TestArg    arg, *argp = &arg;

  RasterCon *cont = (RasterCon *) reg;

  raster = cont->raster;
  rlen   = cont->rastlen;

  ndims = cont->ndims;
  dims  = cont->dims;

  if (com == NULL)
    com = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(ndims));
  else if (com->size < ndims)
    { Free_Array(com);
      com = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(ndims));
    }
  else
    { com->kind  = PLAIN_KIND;
      com->type  = FLOAT64_TYPE;
      com->scale = 64;
      com->ndims = 1;
      com->size  = ndims;
      com->dims[0] = ndims;
    }

  if (ndims > 10)
    sum = (int64 *) Guarded_Malloc(sizeof(uint64)*((size_t) ndims),"Region_Select_COM");
  else
    sum = Sum;

  switch (array->type) {
      case UINT8_TYPE:
        arg.value = array->data;
        arg.level_uval = level.uval;
        break;
      case UINT16_TYPE:
        arg.value = array->data;
        arg.level_uval = level.uval;
        break;
      case UINT32_TYPE:
        arg.value = array->data;
        arg.level_uval = level.uval;
        break;
      case UINT64_TYPE:
        arg.value = array->data;
        arg.level_uval = level.uval;
        break;
      case INT8_TYPE:
        arg.value = array->data;
        arg.level_ival = level.ival;
        break;
      case INT16_TYPE:
        arg.value = array->data;
        arg.level_ival = level.ival;
        break;
      case INT32_TYPE:
        arg.value = array->data;
        arg.level_ival = level.ival;
        break;
      case INT64_TYPE:
        arg.value = array->data;
        arg.level_ival = level.ival;
        break;
      case FLOAT32_TYPE:
        arg.value = array->data;
        arg.level_fval = level.fval;
        break;
      case FLOAT64_TYPE:
        arg.value = array->data;
        arg.level_fval = level.fval;
        break;
  }

  test = Comparator_Table[6*array->type + cmprsn];

  { Indx_Type    v, w, c, i;
    Dimn_Type    n, o, wgt;
    int          d;
    double      *x;
    int64        sumx;

    x = AFLOAT64(com);

    cnt = 0;
    for (d = 0; d < ndims; d++)
      sum[d] = 0;

    for (i = 0; i < rlen; i += 2)
      { c = 1;
        o = dims[0];

        v = raster[i];
        w = raster[i+1];
        n = (Dimn_Type) (v % o);
        w = w + n;

        wgt  = 0;
        sumx = 0;
        for (; v <= w; v++, n++)
          if (test(v,argp))
            { sumx += n;
              wgt  += 1;
            }

        cnt    += wgt;
        sum[0] += sumx;

        for (d = 1; d < ndims; d++)
          { c *= o;
            o  = dims[d];
            sum[d] += wgt * (w/c)%o;
          }
      }

    if (cnt == 0)
      return (0);

    for (d = 0; d < ndims; d++)
      x[d] = (1.*sum[d]) / cnt;

    if (ndims > 10)
      free(sum);

    return (com);
  }
}
