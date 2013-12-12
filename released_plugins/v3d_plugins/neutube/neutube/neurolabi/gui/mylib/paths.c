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
*  SWC Tree Data Abstraction                                                              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  February 2010                                                                 *
*                                                                                         *
*  (c) February 10, '10, Dr. Gene Myers and Howard Hughes Medical Institute               *
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
#include "connectivity.h"
#include "paths.h"

#undef  DEBUG_HEAP
#undef  DEBUG

/****************************************************************************************
 *                                                                                      *
 *  HEAP                                                                                *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Indx_Type *vertex;   //  vertex for this heap element
    Indx_Type *heapos;   //  heapos[p] gives the heap index of heap element containing p
    Size_Type  hmax;     //  size of heap
    float     *weight;   //  weight of heap element
  } Heap;

static void reheap(Indx_Type s, Heap *h)    // Tree above s is a heap,
{ Indx_Type  c, l, r;                       // propogate s downwards until a heap
  double     v, vl, vr;
  Indx_Type  p, ql, qr;
  Indx_Type *vertex, *heapos;
  float32   *weight;

  vertex = h->vertex;
  weight = h->weight;
  heapos = h->heapos;

  c = s;
  p = vertex[s];
  v = weight[p];
  while ((l = (c << 1)) <= h->hmax)
    { r  = l+1;
      ql = vertex[l];
      qr = vertex[r];
      vl = weight[ql];
      vr = weight[qr];
      if (r > h->hmax || vl <= vr)
        { if (v > vl)
            { vertex[c]  = ql;
              heapos[ql] = c;
              c = l;
            }
          else
            break;
        }
      else
        { if (v > vr)
            { vertex[c]  = qr;
              heapos[qr] = c;
              c = r;
            }
          else
            break;
        }
    }
  if (c != s)
    { vertex[c] = p;
      heapos[p] = c;
    }
}

static void upheap(Indx_Type s, Heap *h)   // Tree below s is a heap,
{ Indx_Type  c, f;                         // propagate s upwards until a heap
  double     v;
  Indx_Type  p, q;
  Indx_Type *vertex, *heapos;
  float32   *weight;

  vertex = h->vertex;
  weight = h->weight;
  heapos = h->heapos;

  c = s;
  p = vertex[s];
  v = weight[p];
  while (c > 1)
    { f = (c >> 1);
      q = vertex[f];
      if (weight[q] <= v) break;
      vertex[c] = q;
      heapos[q] = c;
      c = f;
    }
  if (c != s)
    { vertex[c] = p;
      heapos[p] = c;
    }
}

static Indx_Type pop_heap(Heap *h)
{ Indx_Type p, q;

  p = h->vertex[1];
  if (--h->hmax >= 1)
    { h->vertex[1] = q = h->vertex[h->hmax+1];
      h->heapos[q] = 1;
      reheap(1,h);
    }
  return (p);
}

static void heapify(Heap *h)     // Move elements so as to order inititlaly
{ Indx_Type i;                   //   unordered heap in O(n) time
  for (i = h->hmax/2; i >= 1; i--)
    reheap(i,h);
}

#ifdef DEBUG

static void print_heap(Heap *h)
{ Indx_Type i;
  printf("\nHeap state:\n");
  for (i = 1; i <= h->hmax; i++)
    printf("   %4lld:  %6lld %5g\n",i,h->vertex[i],h->weight[h->vertex[i]]);
}

#endif


/****************************************************************************************
 *                                                                                      *
 *  ZONES                                                                               *
 *                                                                                      *
 ****************************************************************************************/

#define SEED_OBJ  0
#define SLCE_OBJ  1
#define REGN_OBJ  2
#define ARRY_OBJ  3
#define VALS_OBJ  4
#define VALR_OBJ  5
#define VALA_OBJ  6

#define VALU_OBJ  7
#define COND_OBJ  8
#define FUNC_OBJ  9

typedef struct
  { int       refcnt;
    int       kind;
    Indx_Type seed;
    void     *object;
    double    val;
    boolean  (*cond)(Indx_Type,double);
    double   (*func)(float *,Indx_Type,Indx_Type,double);
  } SP_Arg;

SP_Item *SP_Value(double val)
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"SP_Value");
  arg->refcnt = 1;
  arg->kind   = VALU_OBJ;
  arg->val    = val;
  return ((SP_Item *) arg);
}

SP_Item *SP_Test(boolean (*cond)(Indx_Type p, double v))
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"SP_Test");
  arg->refcnt = 1;
  arg->kind   = COND_OBJ;
  arg->cond   = cond;
  return ((SP_Item *) arg);
}

SP_Item *SP_Edge(double (*func)(float *w, Indx_Type p, Indx_Type q, double d))
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"SP_Edge");
  arg->refcnt = 1;
  arg->kind   = FUNC_OBJ;
  arg->func   = func;
  return ((SP_Item *) arg);
}

SP_Item *Seed_Zone(Indx_Type seed)
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"Seed_Zone");
  arg->refcnt = 1;
  arg->kind   = SEED_OBJ;
  arg->seed   = seed;
  return ((SP_Item *) arg);
}

SP_Item *Slice_Zone(Slice *C(slice))
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"Slice_Zone");
  arg->refcnt = 1;
  arg->kind   = SLCE_OBJ;
  arg->object = (void *) slice;
  return ((SP_Item *) arg);
}

SP_Item *Region_Zone(Region *C(reg))
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"Region_Zone");
  arg->refcnt = 1;
  arg->kind   = REGN_OBJ;
  arg->object = (void *) Inc_Region(reg);
  return ((SP_Item *) arg);
}

SP_Item *Array_Zone()
{ SP_Arg *arg;

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"Array_Zone");
  arg->refcnt = 1;
  arg->kind   = ARRY_OBJ;
  arg->seed   = 0;
  return ((SP_Item *) arg);
}

SP_Item *Image_Zone(SP_Item *K(which), SP_Item *K(zone))
{ SP_Arg *arg;
  SP_Arg *z = (SP_Arg *) zone;
  SP_Arg *w = (SP_Arg *) which;

  if ( ! (SLCE_OBJ <= z->kind && z->kind <= ARRY_OBJ))
    { fprintf(stderr,"Image zone must be an array, slice, or region (Image_Zone)\n");
      exit (1);
    }
  if (w->kind != VALU_OBJ && w->kind != COND_OBJ)
    { fprintf(stderr,"Image zone selector must be a value or a condition (Image_Zone)\n");
      exit (1);
    }

  arg = (SP_Arg *) Guarded_Malloc(sizeof(SP_Arg),"Image_Zone");
  arg->kind   = ARRY_OBJ + z->kind;
  arg->object = z->object;
  if (w->kind == VALU_OBJ)
    { arg->val  = w->val;
      arg->cond = NULL;
    }
  else
    { arg->val  = 0.;
      arg->cond = w->cond;
    }
  free(z);
  free(w);
  return ((SP_Item *) arg);
}

void Kill_SP(SP_Item *p)
{ SP_Arg *arg = (SP_Arg *) p;

  if (--arg->refcnt > 0)
    return;
  if (arg->kind == VALS_OBJ || arg->kind == SLCE_OBJ)
    Kill_Slice((Slice *) (arg->object));
  else if (arg->kind == VALR_OBJ || arg->kind == REGN_OBJ)
    Kill_Region((Region *) (arg->object));
  free(arg);
}

SP_Item *Inc_SP(SP_Item *p)
{ ((SP_Arg *) p)->refcnt += 1;
  return (p);
}

static void check_arg(SP_Arg *arg, Float_Array *image, char *routine)
{ int        i;
  Dimn_Type *rdims;

  switch (arg->kind)
  { case SEED_OBJ:
      if (arg->seed < 0 || arg->seed >= image->size)
        { fprintf(stderr,"Seed is not within image array (%s)\n",routine);
          exit (1);
        }
      break;
    case SLCE_OBJ:
    case VALS_OBJ:
      if (AForm_Array(arg->object) != image)
        { fprintf(stderr,"Slice is not of the image array (%s)\n",routine);
          exit (1);
        }
      break;
    case REGN_OBJ:
    case VALR_OBJ:
      if (Get_Region_Dimensionality((Region *) (arg->object)) != image->ndims)
        { fprintf(stderr,"Region shape isn't the same as image array (%s)\n",routine);
          exit (1);
        }
      rdims = Get_Region_Dimensions((Region *) (arg->object));
      for (i = 0; i < image->ndims; i++)
        if (rdims[i] != image->dims[i])
          { fprintf(stderr,"Region shape isn't the same as image array (%s)\n",routine);
            exit (1);
          }
      break;
    case ARRY_OBJ:
    case VALA_OBJ:
      break;
    default:
      fprintf(stderr,"Item should be a zone (%s)\n",routine);
      exit (1);
      break;
  }
}


/****************************************************************************************
 *                                                                                      *
 *  SHORTEST PATH WITHIN A ZONE                                                         *
 *                                                                                      *
 ****************************************************************************************/

Float_Array *G(Shortest_Paths)(Float_Array *image, int iscon2n, SP_Item *K(cost),
                               SP_Item *K(source), SP_Item *K(zone))
{ Array     *pathwgt;
  SP_Arg    *spath, *zpath, *carg;
  Slice     *zslice, *sslice;
  Region    *zreg, *sreg;
  double     zval,  sval;
  boolean   (*zcond)(Indx_Type, double);
  boolean   (*scond)(Indx_Type, double);
  double    (*cfunc)(float *,Indx_Type,Indx_Type,double);

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  double    *distance;

  Heap       h, *hp = &h;
  Size_Type  hupper;

  Indx_Type *vertex;
  Indx_Type *heapos;
  float32   *weight;
  float32   *pixel;

  if (image->kind != PLAIN_KIND || image->type != FLOAT32_TYPE)
    { fprintf(stderr,"Subject array must be a PLAIN, FLOAT32 array (Shortest_Paths)\n");
      exit (1);
    }

  spath  = (SP_Arg *) source;
  zpath  = (SP_Arg *) zone;
  zslice = (Slice *) (zpath->object);
  sslice = (Slice *) (spath->object);
  zreg   = (Region *) (zpath->object);
  sreg   = (Region *) (spath->object);
  zval   = zpath->val;
  sval   = spath->val;
  zcond  = zpath->cond;
  scond  = spath->cond;

  check_arg(zpath,image,"Shortest_Paths");
  check_arg(spath,image,"Shortest_Paths");

  carg = (SP_Arg *) cost;
  if (carg->kind != VALU_OBJ && carg->kind != FUNC_OBJ)
    { fprintf(stderr,"Cost parameter must be a value or a function (Shortest_Paths)\n");
      exit (1);
    }

  { Indx_Type i;

    grid     = Setup_Grid(image,"Shortest_Paths");
    n_nbrs   = Grid_Size(grid,iscon2n);
    neighbor = Grid_Neighbors(grid,iscon2n);
    distance = Grid_Distances(grid,iscon2n);

    cfunc = NULL;
    if (carg->kind == VALU_OBJ)
      { double alpha = carg->val;
        for (i = 0; i < n_nbrs; i++)
          distance[i] *= alpha;
      }
    else
      cfunc = carg->func;
  }

  pathwgt = Make_Array(PLAIN_KIND,FLOAT32_TYPE,image->ndims,image->dims);

  pixel  = AFLOAT32(image);
  weight = AFLOAT32(pathwgt);

  hupper = image->size;
  switch (zpath->kind)
  { case ARRY_OBJ:
      Array_Op_Scalar(pathwgt,SET_OP,FLOAT32_TYPE,VALF(FLT_MAX));
    case VALA_OBJ:
      hupper = image->size;
      break;
    case SLCE_OBJ:
    case VALS_OBJ:
      { Coordinate *first, *last;
        Slice      *slice;
        Indx_Type   p, i;

        hupper = AForm_Size(zslice);

        first = Copy_Array(Slice_First(zslice));
        last  = Copy_Array(Slice_Last(zslice));
        for (i = 0; i < image->ndims; i++)
          { if (AINT32(first)[i] >= 1)
              AINT32(first)[i] -= 1; 
            if (AINT32(last)[i]+1 < image->dims[i])
              AINT32(last)[i] += 1;
          } 
	slice = Make_Slice(pathwgt,first,last);
	Array_Op_Scalar(slice,SET_OP,FLOAT32_TYPE,VALF(-1.));
        Kill_Slice(slice);

        p = Set_Slice_To_First(zslice);
        if (zpath->kind == SLCE_OBJ)
          for (i = 0; i < hupper; i++)
            { weight[p] = FLT_MAX;
              p = Next_Slice_Index(zslice);
            }
        else if (zpath->cond == NULL)
          for (i = 0; i < hupper; i++)
            { if (pixel[p] != zval)
                weight[p] = FLT_MAX;
              p = Next_Slice_Index(zslice);
            }
        else
          for (i = 0; i < hupper; i++)
            { if ( ! zcond(p,pixel[p]))
                weight[p] = FLT_MAX;
              p = Next_Slice_Index(zslice);
            }
        break;
      }
    case REGN_OBJ:
    case VALR_OBJ:
      { Indx_Type p, k;
        int       j;
        boolean  *b;

        hupper = Region_Volume(zreg);

        for (k = 0; k < zreg->rastlen; k += 2)
          for (p = zreg->raster[k]; p <= zreg->raster[k+1]; p++)
            { b = Boundary_Pixels(grid,p,iscon2n);
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  weight[p + neighbor[j]] = -1.;
            }

        if (zpath->kind == REGN_OBJ)
          for (k = 0; k < zreg->rastlen; k += 2)
            for (p = zreg->raster[k]; p <= zreg->raster[k+1]; p++)
              weight[p] = FLT_MAX;
        else if (zpath->cond == NULL)
          for (k = 0; k < zreg->rastlen; k += 2)
            for (p = zreg->raster[k]; p <= zreg->raster[k+1]; p++)
              if (pixel[p] != zval)
                weight[p] = FLT_MAX;
        else
          for (k = 0; k < zreg->rastlen; k += 2)
            for (p = zreg->raster[k]; p <= zreg->raster[k+1]; p++)
              if ( ! zcond(p,pixel[p]))
                weight[p] = FLT_MAX;
        break;
      }
    }
 
  h.vertex = (Indx_Type *)
               Guarded_Malloc(sizeof(Indx_Type)*((size_t) (hupper+1+image->size)),"Shortest_Paths");
  h.heapos = h.vertex + (hupper+1);
  h.weight = weight;

  vertex = h.vertex;
  heapos = h.heapos;
  
  if (zpath->kind != VALA_OBJ && zpath->kind != ARRY_OBJ)
    switch (spath->kind)
    { case SEED_OBJ:
        if (weight[spath->seed] == FLT_MAX)
          weight[spath->seed] = 0.;
        break;
      case SLCE_OBJ:
        { Indx_Type p, i, n;
  
          n = AForm_Size(sslice);
          p = Set_Slice_To_First(sslice);
          for (i = 0; i < n; i++)
            { if (weight[p] == FLT_MAX)
                weight[p] = 0.;
              p = Next_Slice_Index(sslice);
            }
          break;
        }
      case VALS_OBJ:
        { Indx_Type p, i, n;
  
          n = AForm_Size(sslice);
          p = Set_Slice_To_First(sslice);
          if (scond == NULL)
            for (i = 0; i < n; i++)
              { if (weight[p] == FLT_MAX && pixel[p] == sval)
                  weight[p] = 0.;
                p = Next_Slice_Index(sslice);
              }
          else
            for (i = 0; i < n; i++)
              { if (weight[p] == FLT_MAX && scond(p,pixel[p]))
                  weight[p] = 0.;
                p = Next_Slice_Index(sslice);
              }
          break;
        }
      case REGN_OBJ:
        { Indx_Type p, k;
  
          for (k = 0; k < sreg->rastlen; k += 2)
            for (p = sreg->raster[k]; p <= sreg->raster[k+1]; p++)
              if (weight[p] == FLT_MAX)
                weight[p] = 0.;
          break;
        }
      case VALR_OBJ:
        { Indx_Type p, k;
  
          if (scond == NULL)
            for (k = 0; k < sreg->rastlen; k += 2)
              for (p = sreg->raster[k]; p <= sreg->raster[k+1]; p++)
                if (weight[p] == FLT_MAX && pixel[p] == sval)
                  weight[p] = 0.;
          else
            for (k = 0; k < sreg->rastlen; k += 2)
              for (p = sreg->raster[k]; p <= sreg->raster[k+1]; p++)
                if (weight[p] == FLT_MAX && scond(p,pixel[p]))
                  weight[p] = 0.;
          break;
        }
      default:
        break;
    }

#define ADD_TO_HEAP(p,m,h)	\
{ m += 1;			\
  h.vertex[m] = p;		\
  h.heapos[p] = m;		\
}

#define SLICE_ADD(slice,cond,h)				\
{ m = 0;						\
  p = Set_Slice_To_First(slice);			\
  for (i = 0; i < n; i++, p = Next_Slice_Index(slice))	\
    if (cond)						\
      ADD_TO_HEAP(p,m,h)				\
  h.hmax = m;						\
}

#define REGION_ADD(reg,cond,h)					\
{ m = 0;							\
  for (k = 0; k < reg->rastlen; k += 2)				\
    for (p = reg->raster[k]; p <= reg->raster[k+1]; p++)	\
      if (cond)							\
        ADD_TO_HEAP(p,m,h)					\
  h.hmax = m;							\
}

  h.hmax = 0;
  switch (zpath->kind)
  { case ARRY_OBJ:
    case VALA_OBJ:
      switch (spath->kind)
      { case SEED_OBJ:
          { Indx_Type p, m;

            p = spath->seed;
            m = 0;
            h.hmax = m;
            if (zpath->kind == VALA_OBJ)
              { if (zcond == NULL)
                  { if (pixel[p] == zval)
                      break;
                  }
                else
                  { if (zcond(p,pixel[p]))
                      break;
                  }
              }
            ADD_TO_HEAP(p,m,h)
            h.hmax = m;
            break;
          }
        case SLCE_OBJ:
          { Indx_Type p, i, m, n;

            n = AForm_Size(sslice);
            if (zpath->kind == VALA_OBJ)
              if (zcond == NULL)
                SLICE_ADD(sslice,pixel[p] != zval,h)
              else
                SLICE_ADD(sslice,!zcond(p,pixel[p]),h)
            else
              SLICE_ADD(sslice,1,h)
            break;
          }
        case VALS_OBJ:
          { Indx_Type p, i, m, n;

            n = AForm_Size(sslice);
            if (zpath->kind == VALA_OBJ)
              if (zcond == NULL)
                SLICE_ADD(sslice,pixel[p] == sval && pixel[p] != zval,h)
              else
                SLICE_ADD(sslice,pixel[p] == sval && ! zcond(p,pixel[p]),h)
            else
              SLICE_ADD(sslice,pixel[p] == sval,h)
            break;
          }
        case REGN_OBJ:
          { Indx_Type p, k, m;

            if (zpath->kind == VALA_OBJ)
              if (zcond == NULL)
                REGION_ADD(sreg,pixel[p] != zval,h)
              else
                REGION_ADD(sreg, ! zcond(p,pixel[p]),h)
            else
              REGION_ADD(sreg,1,h)
            break;
          }
        case VALR_OBJ:
          { Indx_Type p, k, m;

            if (zpath->kind == VALA_OBJ)
              if (zcond == NULL)
                REGION_ADD(sreg,pixel[p] == sval && pixel[p] != zval,h)
              else
                REGION_ADD(sreg,pixel[p] == sval && ! zcond(p,pixel[p]),h)
            else
              REGION_ADD(sreg,pixel[p] == sval,h)
            break;
          }
        case VALA_OBJ:
          { Indx_Type p, m;

            m = 0;
            if (zpath->kind == VALA_OBJ)
              { if (zcond == NULL)
                  if (scond == NULL)
                    for (p = 0; p < image->size; p++)
                      if (pixel[p] == zval)
                        weight[p] = -1.;
                      else if (pixel[p] == sval)
                        ADD_TO_HEAP(p,m,h)
                      else
                        weight[p] = FLT_MAX;
                  else
                    for (p = 0; p < image->size; p++)
                      if (pixel[p] == zval)
                        weight[p] = -1.;
                      else if (scond(p,pixel[p]))
                        ADD_TO_HEAP(p,m,h)
                      else
                        weight[p] = FLT_MAX;
                else
                  if (scond == NULL)
                    for (p = 0; p < image->size; p++)
                      if (zcond(p,pixel[p]))
                        weight[p] = -1.;
                      else if (pixel[p] == sval)
                        ADD_TO_HEAP(p,m,h)
                      else
                        weight[p] = FLT_MAX;
                  else
                    for (p = 0; p < image->size; p++)
                      if (zcond(p,pixel[p]))
                        weight[p] = -1.;
                      else if (scond(p,pixel[p]))
                        ADD_TO_HEAP(p,m,h)
                      else
                        weight[p] = FLT_MAX;
              }
            else if (scond == NULL)
              { for (p = 0; p < image->size; p++)
                  if (pixel[p] == sval)
                    ADD_TO_HEAP(p,m,h)
              }
            else
              { for (p = 0; p < image->size; p++)
                  if (scond(p,pixel[p]))
                    ADD_TO_HEAP(p,m,h)
              }
            h.hmax = m;
            break;
          }
      }
      break;
    case SLCE_OBJ:
    case VALS_OBJ:
      { Indx_Type p, i, m, n;

        n = hupper;
        if (spath->kind == VALA_OBJ)
          if (zpath->kind == VALS_OBJ)
            if (scond == NULL)
              SLICE_ADD(zslice,pixel[p] == sval && weight[p] == FLT_MAX,h)
            else
              SLICE_ADD(zslice,scond(p,pixel[p]) && weight[p] == FLT_MAX,h)
          else
            if (scond == NULL)
              SLICE_ADD(zslice,pixel[p] == sval,h)
            else
              SLICE_ADD(zslice,scond(p,pixel[p]),h)
        else
          SLICE_ADD(zslice,weight[p] == 0.,h)
        break;
      }
    case REGN_OBJ:
    case VALR_OBJ:
      { Indx_Type p, k, m;

        if (spath->kind == VALA_OBJ)
          if (zpath->kind == VALR_OBJ)
            if (scond == NULL)
              REGION_ADD(zreg,pixel[p] == sval && weight[p] == FLT_MAX,h)
            else
              REGION_ADD(zreg,scond(p,pixel[p]) && weight[p] == FLT_MAX,h)
          else
            if (scond == NULL)
              REGION_ADD(zreg,pixel[p] == sval,h)
            else
              REGION_ADD(zreg,scond(p,pixel[p]),h)
        else
          REGION_ADD(zreg,weight[p] == 0.,h)
        break;
      }
    }

  if (h.hmax == 0)
    { fprintf(stderr,"No start points are in the supplied zone (Shortest_Paths)\n");
      exit (1);
    }

  { Indx_Type i, p;

    if (carg->kind == VALU_OBJ)
      { for (i = 1; i <= hp->hmax; i++)
          { p = hp->vertex[i];
            hp->weight[p] = fabs(pixel[p]);
          }
        heapify(hp);
      }
    else
      for (i = 1; i <= hp->hmax; i++)
        { p = hp->vertex[i];
          hp->weight[p] = 0.;
        }
  }

#ifdef DEBUG
  printf("\nWeights:\n");
  Print_Array(pathwgt,stdout,4,"%.1f");

  print_heap(hp);
#endif

  { Indx_Type p, q, j, r;
    boolean  *b;
    double    w;
    Indx_Type vis = image->size;

    if (carg->kind == VALU_OBJ)
     if (zpath->kind != VALA_OBJ || spath->kind == VALA_OBJ)

      while (h.hmax != 0)
        { p = pop_heap(hp);
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0)
                  { w = weight[p] + distance[j] + fabs(pixel[q]);
                    r = heapos[q];
                    if (weight[q] == FLT_MAX)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
              }
        }
     else if (zcond == NULL)

      while (h.hmax != 0)
        { p = pop_heap(hp);
          heapos[p] = vis;
          vertex[vis] = p;
          vis -= 1;
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (pixel[q] != zval)
                  { w = weight[p] + distance[j] + fabs(pixel[q]);
                    r = heapos[q];
                    if (((r <= 0 || r > h.hmax) &&
                         (r <= vis || r > image->size)) || vertex[r] != q)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
                else
                  weight[q] = -1.;
              }
        }
     else

      while (h.hmax != 0)
        { p = pop_heap(hp);
          heapos[p] = vis;
          vertex[vis] = p;
          vis -= 1;
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if ( ! zcond(q,pixel[q]))
                  { w = weight[p] + distance[j] + fabs(pixel[q]);
                    r = heapos[q];
                    if (((r <= 0 || r > h.hmax) &&
                         (r <= vis || r > image->size)) || vertex[r] != q)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
                else
                  weight[q] = -1.;
              }
        }
    else
     if (zpath->kind != VALA_OBJ || spath->kind == VALA_OBJ)

      while (h.hmax != 0)
        { p = pop_heap(hp);
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0)
                  { w = weight[p] + cfunc(pixel,p,q,distance[j]);
                    r = heapos[q];
                    if (weight[q] == FLT_MAX)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
              }
        }
     else if (zcond == NULL)

      while (h.hmax != 0)
        { p = pop_heap(hp);
          heapos[p] = vis;
          vertex[vis] = p;
          vis -= 1;
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (pixel[q] != zval)
                  { w = weight[p] + cfunc(pixel,p,q,distance[j]);
                    r = heapos[q];
                    if (((r <= 0 || r > h.hmax) &&
                         (r <= vis || r > image->size)) || vertex[r] != q)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
                else
                  weight[q] = -1.;
              }
        }
     else

      while (h.hmax != 0)
        { p = pop_heap(hp);
          heapos[p] = vis;
          vertex[vis] = p;
          vis -= 1;
#ifdef DEBUG_HEAP
          printf("  Pop %lld(%g)\n",p,weight[p]);
#endif
          b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if ( ! zcond(q,pixel[q]))
                  { w = weight[p] + cfunc(pixel,p,q,distance[j]);
                    r = heapos[q];
                    if (((r <= 0 || r > h.hmax) &&
                         (r <= vis || r > image->size)) || vertex[r] != q)
                      { weight[q] = (float) w;
                        r = ++h.hmax;
                        vertex[r] = q;
                        heapos[q] = r;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                    else if (w < weight[q])
                      { weight[q] = (float) w;
                        upheap(r,hp);
#ifdef DEBUG_HEAP
                        printf("   -> %lld(%g)\n",q,w);
#endif
                      }
                  }
                else
                  weight[q] = -1.;
              }
        }
  }

  free(h.vertex);

  Release_Grid(grid);

  Kill_SP(cost);
  Kill_SP(source);
  Kill_SP(zone);

  return (pathwgt);
}

Vector *G(Find_Path)(Float_Array *w, boolean iscon2n, SP_Item *cost, Float_Array *d, Indx_Type end)
{ SP_Arg    *carg;
  double    (*cfunc)(float *,Indx_Type,Indx_Type,double);
  Vector    *path;
  float     *weight, *pixel;
  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  double    *distance;

  carg = (SP_Arg *) cost;
  if (carg->kind != VALU_OBJ && carg->kind != FUNC_OBJ)
    { fprintf(stderr,"Cost parameter must be a value or a function (Shortest_Paths)\n");
      exit (1);
    }

  { Indx_Type i;

    grid     = Setup_Grid(w,"Find_Path");
    n_nbrs   = Grid_Size(grid,iscon2n);
    neighbor = Grid_Neighbors(grid,iscon2n);
    distance = Grid_Distances(grid,iscon2n);

    cfunc = NULL;
    if (carg->kind == VALU_OBJ)
      { double alpha = carg->val;
        for (i = 0; i < n_nbrs; i++)
          distance[i] *= alpha;
      }
    else
      cfunc = carg->func;
  }

  weight = AFLOAT32(d);
  pixel  = AFLOAT32(w);

  { boolean   *b;
    int        j;
    Indx_Type  p, q;
    Size_Type  n;
    double     w;
    Indx_Type *vec;

    q = 0;
    n = 1;
    if (carg->kind == VALU_OBJ)
      for (p = end; weight[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0.)
                  { w = (float) (weight[q] + distance[j] + fabs(pixel[p]));
                    if (w == weight[p])
                      break;
                  }
              }
          n += 1;
        } 
    else
      for (p = end; weight[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0.)
                  { w = (float) (weight[q] + cfunc(pixel,q,p,distance[j]));
                    if (w == weight[p])
                      break;
                  }
              }
          n += 1;
        } 
  
    path = Make_Array_With_Shape(PLAIN_KIND,INDX_TYPE,Coord1(n));
    vec  = AINDX(path);

    vec[--n] = end;
    if (carg->kind == VALU_OBJ)
      for (p = end; weight[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0.)
                  { w = (float) (weight[q] + distance[j] + fabs(pixel[p]));
                    if (w == weight[p])
                      break;
                  }
              }
          vec[--n] = q;
        } 
    else
      for (p = end; weight[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (weight[q] >= 0.)
                  { w = (float) (weight[q] + cfunc(pixel,q,p,distance[j]));
                    if (w == weight[p])
                      break;
                  }
              }
          vec[--n] = q;
        } 
  }

  Release_Grid(grid);

  return (path);
}


/****************************************************************************************
 *                                                                                      *
 *  SHORTEST PATH BETWEEN TWO ZONES                                                     *
 *                                                                                      *
 ****************************************************************************************/

Vector *G(Shortest_Between)(Float_Array *image, int iscon2n, SP_Item *K(cost),
                            SP_Item *K(source), SP_Item *K(target), double *score)
{ Vector    *path;
  Array     *spathwgt, *tpathwgt;
  SP_Arg    *spath, *tpath, *carg;
  Slice     *tslice, *sslice;
  Region    *treg, *sreg;
  double     tval,  sval;
  boolean   (*tcond)(Indx_Type, double);
  boolean   (*scond)(Indx_Type, double);
  double    (*cfunc)(float *,Indx_Type,Indx_Type,double);

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  double    *distance;

  Heap       hs, *hps = &hs;
  Heap       ht, *hpt = &ht;

  float32   *pixel;

  if (image->kind != PLAIN_KIND || image->type != FLOAT32_TYPE)
    { fprintf(stderr,"Subject array must be a PLAIN, FLOAT32 array (Shortest_Between)\n");
      exit (1);
    }

  spath  = (SP_Arg *) source;
  tpath  = (SP_Arg *) target;
  tslice = (Slice *) (tpath->object);
  sslice = (Slice *) (spath->object);
  treg   = (Region *) (tpath->object);
  sreg   = (Region *) (spath->object);
  tval   = tpath->val;
  sval   = spath->val;
  tcond  = tpath->cond;
  scond  = spath->cond;

  check_arg(tpath,image,"Shortest_Between");
  check_arg(spath,image,"Shortest_Between");
  if (spath->kind == ARRY_OBJ)
    { fprintf(stderr,"Cannot use an 'All_Zone' for this routine (Shortest_Between)\n");
      exit (1);
    }
  if (tpath->kind == ARRY_OBJ)
    { fprintf(stderr,"Cannot use an 'All_Zone' for this routine (Shortest_Between)\n");
      exit (1);
    }

  carg = (SP_Arg *) cost;
  if (carg->kind != VALU_OBJ && carg->kind != FUNC_OBJ)
    { fprintf(stderr,"Cost parameter must be a value or a function (Shortest_Paths)\n");
      exit (1);
    }

  { Indx_Type i;

    grid     = Setup_Grid(image,"Shortest_Between");
    n_nbrs   = Grid_Size(grid,iscon2n);
    neighbor = Grid_Neighbors(grid,iscon2n);
    distance = Grid_Distances(grid,iscon2n);

    cfunc = NULL;
    if (carg->kind == VALU_OBJ)
      { double alpha = carg->val;
        for (i = 0; i < n_nbrs; i++)
          distance[i] *= alpha;
      }
    else
      cfunc = carg->func;
  }

  spathwgt = Make_Array(PLAIN_KIND,FLOAT32_TYPE,image->ndims,image->dims);
  tpathwgt = Make_Array(PLAIN_KIND,FLOAT32_TYPE,image->ndims,image->dims);

  pixel   = AFLOAT32(image);

  hs.vertex = (Indx_Type *)
               Guarded_Malloc(sizeof(Indx_Type)*((size_t) (2*image->size+1)),"Shortest_Between");
  hs.heapos = hs.vertex + (image->size+1);
  hs.weight = AFLOAT32(spathwgt);

  ht.vertex = (Indx_Type *)
               Guarded_Malloc(sizeof(Indx_Type)*((size_t) (2*image->size+1)),"Shortest_Between");
  ht.heapos = ht.vertex + (image->size+1);
  ht.weight = AFLOAT32(tpathwgt);

  hs.hmax = 0;
  switch (spath->kind)
  { case SEED_OBJ:
      { Indx_Type p, m;

        p = spath->seed;
        m = 0;
        ADD_TO_HEAP(p,m,hs)
        hs.hmax = m;
        break;
      }
    case SLCE_OBJ:
    case VALS_OBJ:
      { Indx_Type p, i, m, n;

        n = AForm_Size(sslice);
        if (spath->kind == SLCE_OBJ)
          SLICE_ADD(sslice,1,hs)
        else if (scond == NULL)
          SLICE_ADD(sslice,pixel[p] == sval,hs)
        else
          SLICE_ADD(sslice,scond(p,pixel[p]),hs)
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled source (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
    case REGN_OBJ:
    case VALR_OBJ:
      { Indx_Type p, k, m;

        if (spath->kind == REGN_OBJ)
          REGION_ADD(sreg,1,hs)
        else if (scond == NULL)
          REGION_ADD(sreg,pixel[p] == sval,hs)
        else
          REGION_ADD(sreg,scond(p,pixel[p]),hs)
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled source (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
    case VALA_OBJ:
      { Indx_Type p, m;

        m = 0;
        if (scond == NULL)
          for (p = 0; p < image->size; p++)
            if (pixel[p] == sval)
              ADD_TO_HEAP(p,m,hs)
        else
          for (p = 0; p < image->size; p++)
            if (scond(p,pixel[p]))
              ADD_TO_HEAP(p,m,hs)
        hs.hmax = m;
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled source (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
  }
  
  ht.hmax = 0;
  switch (tpath->kind)
  { case SEED_OBJ:
      { Indx_Type p, m;

        p = tpath->seed;
        m = 0;
        ADD_TO_HEAP(p,m,ht)
        ht.hmax = m;
        break;
      }
    case SLCE_OBJ:
    case VALS_OBJ:
      { Indx_Type p, i, m, n;

        n = AForm_Size(tslice);
        if (tpath->kind == SLCE_OBJ)
          SLICE_ADD(tslice,1,ht)
        else if (tcond == NULL)
          SLICE_ADD(tslice,pixel[p] == tval,ht)
        else
          SLICE_ADD(tslice,tcond(p,pixel[p]),ht)
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled target (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
    case REGN_OBJ:
    case VALR_OBJ:
      { Indx_Type p, k, m;

        if (tpath->kind == REGN_OBJ)
          REGION_ADD(treg,1,ht)
        else if (tcond == NULL)
          REGION_ADD(treg,pixel[p] == tval,ht)
        else
          REGION_ADD(treg,tcond(p,pixel[p]),ht)
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled target (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
    case VALA_OBJ:
      { Indx_Type p, m;

        m = 0;
        if (tcond == NULL)
          for (p = 0; p < image->size; p++)
            if (pixel[p] == tval)
              ADD_TO_HEAP(p,m,ht)
        else
          for (p = 0; p < image->size; p++)
            if (tcond(p,pixel[p]))
              ADD_TO_HEAP(p,m,ht)
        ht.hmax = m;
        if (m == 0)
          { fprintf(stderr,"No start points are in labeled target (Shortest_Between)\n");
            exit (1);
          }
        break;
      }
  }

  { Indx_Type i, p;

    if (carg->kind == VALU_OBJ)
      { for (i = 1; i <= hps->hmax; i++)
          { p = hps->vertex[i];
            hps->weight[p] = fabs(pixel[p]);
          }
        heapify(hps);
      }
    else
      for (i = 1; i <= hps->hmax; i++)
        { p = hps->vertex[i];
          hps->weight[p] = 0.;
        }

    for (i = 1; i <= hpt->hmax; i++)
      { p = hpt->vertex[i];
        hpt->weight[p] = 0.;
      }
  }

#ifdef DEBUG
  printf("\nWeights:\n");
  Print_Array(spathwgt,stdout,4,"%.1f");
  Print_Array(tpathwgt,stdout,4,"%.1f");

  print_heap(hps);
  print_heap(hpt);
#endif

  if (carg->kind == VALU_OBJ)
  { Indx_Type mid, svis, tvis;
    double    cutoff = *score/2.;

    svis = image->size;
    tvis = image->size;
    while (hps->hmax != 0 || hpt->hmax != 0)
      { Indx_Type p, q, j, r;
        boolean  *b;
        double    w;

        if (hpt->hmax == 0 || hps->weight[hps->vertex[1]] <= hpt->weight[hpt->vertex[1]])
          { p = pop_heap(hps);
            r = hpt->heapos[p];
            if (r > tvis && r <= image->size && hpt->vertex[r] == p)
              { mid = p;
                break;
              }
            if (cutoff < hps->weight[hps->vertex[1]])
              { path = NULL;
                goto nopath;
              }
            hps->heapos[p] = svis;
            hps->vertex[svis] = p;
            svis -= 1;
#ifdef DEBUG_HEAP
            printf("  Pop S %lld(%g)\n",p,hps->weight[p]);
#endif

            b = Boundary_Pixels(grid,p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j])
                { q = p + neighbor[j];
                  w = hps->weight[p] + distance[j] + fabs(pixel[q]);
                  r = hps->heapos[q];
                  if (((r <= 0 || r > hps->hmax) &&
                       (r <= svis || r > image->size)) || hps->vertex[r] != q)
                    { hps->weight[q] = (float) w;
                      r = ++hps->hmax;
                      hps->vertex[r] = q;
                      hps->heapos[q] = r;
                      upheap(r,hps);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                  else if (w < hps->weight[q])
                    { hps->weight[q] = (float) w;
                      upheap(r,hps);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                }
          }
        else
          { p = pop_heap(hpt);
            r = hps->heapos[p];
            if (r > svis && r <= image->size && hps->vertex[r] == p)
              { mid = p;
                break;
              }
            if (cutoff < hpt->weight[hpt->vertex[1]])
              { path = NULL;
                goto nopath;
              }
            hpt->heapos[p] = tvis;
            hpt->vertex[tvis] = p;
            tvis -= 1;
#ifdef DEBUG_HEAP
            printf("  Pop T %lld(%g)\n",p,hpt->weight[p]);
#endif

            b = Boundary_Pixels(grid,p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j])
                { q = p + neighbor[j];
                  w = hpt->weight[p] + distance[j] + fabs(pixel[p]);
                  r = hpt->heapos[q];
                  if (((r <= 0 || r > hpt->hmax) &&
                       (r <= tvis || r > image->size)) || hpt->vertex[r] != q)
                    { hpt->weight[q] = (float) w;
                      r = ++hpt->hmax;
                      hpt->vertex[r] = q;
                      hpt->heapos[q] = r;
                      upheap(r,hpt);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                  else if (w < hpt->weight[q])
                    { hpt->weight[q] = (float) w;
                      upheap(r,hpt);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                }
          }
      }

#ifdef DEBUG
    printf("\nWeights At End:\n");
    Print_Array(spathwgt,stdout,4,"%.1f");
    Print_Array(tpathwgt,stdout,4,"%.1f");
#endif

    if (hps->hmax == 0 && hpt->hmax == 0)
      { fprintf(stderr,"A path between the two regions does not exist! (Shortest_Between)\n");
        exit (1);
      }

    // A shortest path passes through mid

    { float     *wgt;
      Indx_Type *hpo;
      Indx_Type *vtx;
      Size_Type  vis;
      Indx_Type *vec;

      boolean   *b;
      Indx_Type  p, q, j, r;
      Size_Type  n, m;
      double     w;

      wgt = hpt->weight;
      hpo = hpt->heapos;
      vtx = hpt->vertex;
      vis = tvis;

      q = 0;
      n = 0;
      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + distance[j] + fabs(pixel[q]));
                    if (w == wgt[p])
                      break;
                  }
              }
          n += 1;
        } 

      wgt = hps->weight;
      hpo = hps->heapos;
      vtx = hps->vertex;
      vis = svis;

      q = 0;
      m = 1;
      for (p = mid; wgt[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + distance[j] + fabs(pixel[p]));
                    if (w == wgt[p])
                      break;
                  }
              }
          m += 1;
        } 

      path = Make_Array_With_Shape(PLAIN_KIND,INDX_TYPE,Coord1(m+n));
      vec  = AINDX(path);

      n = m;
      vec[--m] = mid;
      for (p = mid; wgt[p] != fabs(pixel[p]); p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + distance[j] + fabs(pixel[p]));
                    if (w == wgt[p])
                      break;
                  }
              }
          vec[--m] = q;
        } 

      wgt = hpt->weight;
      hpo = hpt->heapos;
      vtx = hpt->vertex;
      vis = tvis;

      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + distance[j] + fabs(pixel[q]));
                    if (w == wgt[p])
                      break;
                  }
              }
          vec[n++] = q;
        } 
    }

    *score = hps->weight[mid] + hpt->weight[mid];
  }
  else
  { Indx_Type mid, svis, tvis;
    double    cutoff = *score/2.;

    svis = image->size;
    tvis = image->size;
    while (hps->hmax != 0 || hpt->hmax != 0)
      { Indx_Type p, q, j, r;
        boolean  *b;
        double    w;

        if (hpt->hmax == 0 || hps->weight[hps->vertex[1]] <= hpt->weight[hpt->vertex[1]])
          { p = pop_heap(hps);
            r = hpt->heapos[p];
            if (r > tvis && r <= image->size && hpt->vertex[r] == p)
              { mid = p;
                break;
              }
            if (cutoff < hps->weight[hps->vertex[1]])
              { path = NULL;
                goto nopath;
              }
            hps->heapos[p] = svis;
            hps->vertex[svis] = p;
            svis -= 1;
#ifdef DEBUG_HEAP
            printf("  Pop S %lld(%g)\n",p,hps->weight[p]);
#endif

            b = Boundary_Pixels(grid,p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j])
                { q = p + neighbor[j];
                  w = hps->weight[p] + cfunc(pixel,p,q,distance[j]);
                  r = hps->heapos[q];
                  if (((r <= 0 || r > hps->hmax) &&
                       (r <= svis || r > image->size)) || hps->vertex[r] != q)
                    { hps->weight[q] = (float) w;
                      r = ++hps->hmax;
                      hps->vertex[r] = q;
                      hps->heapos[q] = r;
                      upheap(r,hps);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                  else if (w < hps->weight[q])
                    { hps->weight[q] = (float) w;
                      upheap(r,hps);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                }
          }
        else
          { p = pop_heap(hpt);
            r = hps->heapos[p];
            if (r > svis && r <= image->size && hps->vertex[r] == p)
              { mid = p;
                break;
              }
            if (cutoff < hpt->weight[hpt->vertex[1]])
              { path = NULL;
                goto nopath;
              }
            hpt->heapos[p] = tvis;
            hpt->vertex[tvis] = p;
            tvis -= 1;
#ifdef DEBUG_HEAP
            printf("  Pop T %lld(%g)\n",p,hpt->weight[p]);
#endif

            b = Boundary_Pixels(grid,p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j])
                { q = p + neighbor[j];
                  w = hpt->weight[p] + cfunc(pixel,q,p,distance[j]);
                  r = hpt->heapos[q];
                  if (((r <= 0 || r > hpt->hmax) &&
                       (r <= tvis || r > image->size)) || hpt->vertex[r] != q)
                    { hpt->weight[q] = (float) w;
                      r = ++hpt->hmax;
                      hpt->vertex[r] = q;
                      hpt->heapos[q] = r;
                      upheap(r,hpt);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                  else if (w < hpt->weight[q])
                    { hpt->weight[q] = (float) w;
                      upheap(r,hpt);
#ifdef DEBUG_HEAP
                      printf("   -> %lld(%g)\n",q,w);
#endif
                    }
                }
          }
      }

#ifdef DEBUG
    printf("\nWeights At End:\n");
    Print_Array(spathwgt,stdout,4,"%.1f");
    Print_Array(tpathwgt,stdout,4,"%.1f");
#endif

    if (hps->hmax == 0 && hpt->hmax == 0)
      { fprintf(stderr,"A path between the two regions does not exist! (Shortest_Between)\n");
        exit (1);
      }

    // A shortest path passes through mid

    { float     *wgt;
      Indx_Type *hpo;
      Indx_Type *vtx;
      Size_Type  vis;
      Indx_Type *vec;

      boolean   *b;
      Indx_Type  p, q, j, r;
      Size_Type  n, m;
      double     w;

      wgt = hpt->weight;
      hpo = hpt->heapos;
      vtx = hpt->vertex;
      vis = tvis;

      q = 0;
      n = 0;
      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + cfunc(pixel,p,q,distance[j]));
                    if (w == wgt[p])
                      break;
                  }
              }
          n += 1;
        } 

      wgt = hps->weight;
      hpo = hps->heapos;
      vtx = hps->vertex;
      vis = svis;

      q = 0;
      m = 1;
      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + cfunc(pixel,q,p,distance[j]));
                    if (w == wgt[p])
                      break;
                  }
              }
          m += 1;
        } 

      path = Make_Array_With_Shape(PLAIN_KIND,INDX_TYPE,Coord1(m+n));
      vec  = AINDX(path);

      n = m;
      vec[--m] = mid;
      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + cfunc(pixel,q,p,distance[j]));
                    if (w == wgt[p])
                      break;
                  }
              }
          vec[--m] = q;
        } 

      wgt = hpt->weight;
      hpo = hpt->heapos;
      vtx = hpt->vertex;
      vis = tvis;

      for (p = mid; wgt[p] != 0.; p = q)
        { b = Boundary_Pixels(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                r = hpo[q];
                if (r > vis && r <= image->size && vtx[r] == q)
                  { w = (float) (wgt[q] + cfunc(pixel,p,q,distance[j]));
                    if (w == wgt[p])
                      break;
                  }
              }
          vec[n++] = q;
        } 
    }

    *score = hps->weight[mid] + hpt->weight[mid];
  }

nopath:
  free(hs.vertex);
  free(ht.vertex);

  Free_Array(spathwgt);
  Free_Array(tpathwgt);

  Release_Grid(grid);

  Kill_SP(cost);
  Kill_SP(source);
  Kill_SP(target);

  return (path);
}
