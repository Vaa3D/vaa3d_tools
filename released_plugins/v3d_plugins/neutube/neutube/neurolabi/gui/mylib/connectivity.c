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
*  Connectivity and floodfill routines                                                    *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  Jan  2008                                                                     *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mylib.h"
#include "utilities.h"
#include "array.h"
#include "connectivity.h"

#undef  DEBUG_SURFACE

#define NUM_GRIDS  2*NUM_THREADS
#define NUM_FLOODS NUM_THREADS

#define SIZEOF(x) ((int) sizeof(x))

/****************************************************************************************
 *                                                                                      *
 *  ARRAY CONNECTIVITY ROUTINES: NEIGHBORS & BOUNDARIES                                 *
 *                                                                                      *
 ****************************************************************************************/

/* Subroutines to handle connectivity and boundary checks.  The 2D and 3D cases
     are customized to make them a little more efficient, in all other cases a
     general routines that work for any dimensionality can be invoked (including 1D!)
*/

typedef struct
  { int        ndims;     /* Array dimensions */
    Dimn_Type *dims;

    Size_Type  max;       /* General case globals */
    int        numbrs2;
    int        numbrs3;
    int        midway3;

    Offs_Type *neighbor2;
    Offs_Type *neighbor3;
    double    *distance2;
    double    *distance3;
    int       *bcktrack2;
    int       *bcktrack3;
    boolean   *bound2;
    boolean   *bound3;
    Dimn_Type *upper;
    Dimn_Type *lower;

    Dimn_Type upper0;      /* 2D & 3D case globals */
    Dimn_Type upper1;
    Dimn_Type upper2;
    Dimn_Type lower0;
    Dimn_Type lower1;
    Dimn_Type lower2;

    Dimn_Type width;
    Dimn_Type height;

  } Grid;

static pthread_mutex_t Grid_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  Grid_Cond  = PTHREAD_COND_INITIALIZER;

static int  Grid_NoInit = 1;
static int  Grid_Alloc[NUM_GRIDS];
static Grid Grids[NUM_GRIDS];

static Grid_Id get_grid()
{ Grid_Id id;

  pthread_mutex_lock(&Grid_Mutex);
  if (Grid_NoInit)
    { Grid_NoInit = 0;
      for (id = 0; id < NUM_GRIDS; id++)
        { Grid_Alloc[id]      = 0;
          Grids[id].max       = 0;
          Grids[id].neighbor2 = NULL;
        }
    }
  while (1)
    { for (id = 0; id < NUM_GRIDS; id++)
        if (Grid_Alloc[id] == 0)
          { Grid_Alloc[id] = 1;
            break;
          }
      if (id < NUM_GRIDS)
        break;
      pthread_cond_wait(&Grid_Cond,&Grid_Mutex);
    }
  pthread_mutex_unlock(&Grid_Mutex);

  return (id);
}

void Release_Grid(Grid_Id id)
{ pthread_mutex_lock(&Grid_Mutex);
  Grid_Alloc[id] = 0;
  pthread_cond_signal(&Grid_Cond);
  pthread_mutex_unlock(&Grid_Mutex);
}

void Reset_Grids()
{ Grid_Id id;
  pthread_mutex_lock(&Grid_Mutex);
  if (! Grid_NoInit)
    { for (id = 0; id < NUM_GRIDS; id++)
        { if (Grid_Alloc[id] != 0)
            pthread_cond_wait(&Grid_Cond,&Grid_Mutex);
          free(Grids[id].neighbor2);
        }
      Grid_NoInit = 1;
    }
  pthread_mutex_unlock(&Grid_Mutex);
}

  typedef struct
    { boolean   *b3;
      boolean   *b2;
      Size_Type  cnt;
      Size_Type  mid;
    } Bound_3N;

  static void check_3n_neighbors(Bound_3N *a, int d, boolean c)
  { if (d == 0)
      { if (a->cnt == a->mid)
          a->b3 -= 1;
        else
          a->b3[a->cnt] = c;
        a->cnt++;
      }
    else
      { d -= 2;
        check_3n_neighbors(a,d,c && a->b2[d]);
        check_3n_neighbors(a,d,c  );
        check_3n_neighbors(a,d,c && a->b2[d+1]);
      }
  }

boolean *Boundary_Pixels(Grid_Id id, Indx_Type p, boolean iscon2n)
{ Grid      *grid  = Grids + id;
  int       i, q;
  Dimn_Type x, c;

  boolean   *b2    = grid->bound2;
  Dimn_Type *up    = grid->upper;
  Dimn_Type *low   = grid->lower;
  int        ndims = grid->ndims;
  Dimn_Type *dims  = grid->dims;

  q = 0;
  for (i = 0; i < ndims; i++)
    { c = dims[i];
      x = (Dimn_Type) (p % c);
      b2[q++] = (x > low[i]);
      b2[q++] = (x < up[i]);
      p /= c;
    }
  if (iscon2n)
    return (b2);
  else
    { Bound_3N pack;

      pack.b3  = grid->bound3;
      pack.b2  = b2;
      pack.cnt = 0;
      pack.mid = grid->midway3;

      check_3n_neighbors(&pack,2*ndims,1);
      return (grid->bound3);
    }
}

boolean *Boundary_Pixels_2d(Grid_Id id, Indx_Type p, boolean iscon4)
{ Grid *grid   = Grids + id;
  Dimn_Type cw = grid->width;

  Dimn_Type x, y;
  int       xn, xp;
  int       yn, yp;
      
  y = (Dimn_Type) (p/cw);
  x = (Dimn_Type) (p%cw);

  xn = (x > grid->lower0);
  yn = (y > grid->lower1);
  xp = (x < grid->upper0);
  yp = (y < grid->upper1);
  if (iscon4)
    { boolean *b2 = grid->bound2;
      b2[0] = xn;
      b2[1] = xp;
      b2[2] = yn;
      b2[3] = yp;
      return (b2);
    }
  else
    { boolean *b3 = grid->bound3;
      b3[0] = yn && xn;
      b3[1] = yn;
      b3[2] = yn && xp;
      b3[3] = xn;
      b3[4] = xp;
      b3[5] = yp && xn;
      b3[6] = yp;
      b3[7] = yp && xp;
      return (b3);
    }
}

boolean *Boundary_Pixels_3d(Grid_Id id, Indx_Type p, boolean iscon6)
{ Grid *grid   = Grids + id;
  Dimn_Type cw = grid->width;
  Dimn_Type ch = grid->height;

  Dimn_Type x, y, z;
  int       xn, xp;
  int       yn, yp;
  int       zn, zp;

  x = (Dimn_Type) (p%cw);
  p = p/cw;
  y = (Dimn_Type) (p%ch);
  z = (Dimn_Type) (p/ch);

  xn = (x > grid->lower0);
  yn = (y > grid->lower1);
  zn = (z > grid->lower2);
  xp = (x < grid->upper0);
  yp = (y < grid->upper1);
  zp = (z < grid->upper2);
  if (iscon6)
    { boolean *b2 = grid->bound2;
      b2[0] = xn;
      b2[1] = xp;
      b2[2] = yn;
      b2[3] = yp;
      b2[4] = zn;
      b2[5] = zp;
      return (b2);
    }
  else
    { boolean *b3 = grid->bound3;
      boolean  t;

      b3[ 1] = t = zn && yn;
      b3[ 0] = t && xn;
      b3[ 2] = t && xp;
      b3[ 3] = zn && xn;
      b3[ 4] = zn;
      b3[ 5] = zn && xp;
      b3[ 7] = t = zn && yp;
      b3[ 6] = t && xn;
      b3[ 8] = t && xp;

      b3[ 9] = yn && xn;
      b3[10] = yn;
      b3[11] = yn && xp;
      b3[12] = xn;
      b3[13] = xp;
      b3[14] = yp && xn;
      b3[15] = yp;
      b3[16] = yp && xp;

      b3[18] = t = zp && yn;
      b3[17] = t && xn;
      b3[19] = t && xp;
      b3[20] = zp && xn;
      b3[21] = zp;
      b3[22] = zp && xp;
      b3[24] = t = zp && yp;
      b3[23] = t && xn;
      b3[25] = t && xp;
      return (b3);
    }
}

Offs_Type *Grid_Neighbors(Grid_Id id, boolean iscon2n)
{ if (iscon2n)
    return (Grids[id].neighbor2);
  else
    return (Grids[id].neighbor3);
}

int Grid_Size(Grid_Id id, boolean iscon2n)
{ if (iscon2n)
    return (Grids[id].numbrs2);
  else
    return (Grids[id].numbrs3);
}

double *Grid_Distances(Grid_Id id, boolean iscon2n)
{ if (iscon2n)
    return (Grids[id].distance2);
  else
    return (Grids[id].distance3);
}

int *Grid_Backtrack(Grid_Id id, boolean iscon2n)
{ if (iscon2n)
    return (Grids[id].bcktrack2);
  else
    return (Grids[id].bcktrack3);
}

  typedef struct
    { Size_Type  cnt;
      Size_Type  mid;
      Dimn_Type *dim;
      Offs_Type *ngh;
      double    *dst;
      double    *sqr;
    } Fill_3N;

  static void fill_3n_neighbors(Fill_3N *a, int d, Offs_Type o, int m)
  { if (d == 0)
      { if (a->cnt == a->mid)
          { a->ngh -= 1;
            a->dst -= 1;
          }
        else
          { a->ngh[a->cnt] = o;
            a->dst[a->cnt] = a->sqr[m];
          }
        a->cnt++;
      }
    else
      { d -= 1;
        o *= a->dim[d];
        fill_3n_neighbors(a,d,o-1,m+1);
        fill_3n_neighbors(a,d,o  ,m);
        fill_3n_neighbors(a,d,o+1,m+1);
      }
  }

Grid_Id Setup_Grid(APart *part, string routine)
{ Array     *array = AForm_Array(part);
  int        id    = get_grid();
  Dimn_Type *dims  = array->dims;
  int        ndims = array->ndims;
  Grid      *grid = Grids + id;

  int       n2, n3;
  Size_Type space;
  int       i;

  if (ndims > 19)
    { fprintf(stderr,"Grids support up to a maximum of 19 dimensional arrays (Setup_Grid)\n");
      exit (1);
    }

  grid->ndims = ndims;
  grid->dims  = dims;

  n2 = 2*ndims;
  grid->numbrs2 = n2;

  n3 = 1;
  for (i = 0; i < ndims; i++)
    n3 *= 3;
  grid->midway3 = (n3 >> 1);
  n3 -= 1;
  grid->numbrs3 = n3;

  space = (2*SIZEOF(int)+SIZEOF(Offs_Type)+SIZEOF(double))*(n2+n3) + SIZEOF(Dimn_Type)*2*ndims;
  if (space > grid->max)
    { grid->max       = space;
      grid->neighbor2 = (Offs_Type *) Guarded_Realloc(grid->neighbor2,(size_t) grid->max,routine);
      grid->neighbor3 = grid->neighbor2 + n2;
      grid->bound2    = (int *) (grid->neighbor3 + n3);
      grid->bound3    = grid->bound2 + n2;
      grid->distance2 = (double *) (grid->bound3 + n3);
      grid->distance3 = grid->distance2 + n2;
      grid->bcktrack2 = (int *) (grid->distance3 + n3);
      grid->bcktrack3 = grid->bcktrack2 + n2;
      grid->upper     = (Dimn_Type *) (grid->bcktrack3 + n3);
      grid->lower     = grid->upper + ndims;
    }

  if (Is_Slice(part))
    { Dimn_Type *bcrd = ADIMN(Slice_First(part));
      Dimn_Type *ecrd = ADIMN(Slice_Last(part));

      grid->upper0 = ecrd[0];
      grid->lower0 = bcrd[0];
      grid->width  = dims[0];
      if (ndims >= 2)
        { grid->upper1 = ecrd[1];
          grid->lower1 = bcrd[1];
          grid->height = dims[1];
          if (ndims >= 3)
            { grid->upper2 = ecrd[2];
              grid->lower2 = bcrd[2];
            }
        }

      for (i = 0; i < ndims; i++)
        { grid->upper[i] = ecrd[i];
          grid->lower[i] = bcrd[i];
        }
    }
  else
    { grid->upper0 = (grid->width = dims[0]) - 1;
      grid->lower0 = 0;
      if (ndims >= 2)
        { grid->upper1 = (grid->height = dims[1]) - 1;
          grid->lower1 = 0;
          if (ndims >= 3)
            { grid->upper2 = dims[2]-1;
              grid->lower2 = 0;
            }
        }

      for (i = 0; i < ndims; i++)
        { grid->upper[i] = dims[i]-1;
          grid->lower[i] = 0;
        }
    }

  { Offs_Type *cn2 = grid->neighbor2;
    double    *ds2 = grid->distance2;
    int       *bk2 = grid->bcktrack2;
    Offs_Type  p;

    p = 1;
    for (i = 0; i < ndims; i++)
      { cn2[2*i]   = -p;
        cn2[2*i+1] = p;
        p *= dims[i];

        ds2[2*i]   = 1.;
        ds2[2*i+1] = 1.;

        bk2[2*i]   = 2*i+1;
        bk2[2*i+1] = 2*i;
      }
  }

  { double    Squares[10];
    double   *square;
    Fill_3N   pack;
    int      *b3;
    Indx_Type p;

    if (ndims < 10)
      square = Squares;
    else
      square = (double *) Guarded_Malloc(sizeof(double)*((size_t) (ndims+1)),routine);
    for (i = 0; i <= ndims; i++)
      square[i] = sqrt(1.*i);

    pack.cnt  = 0;
    pack.mid  = grid->midway3;
    pack.dim  = dims;
    pack.ngh  = grid->neighbor3;
    pack.dst  = grid->distance3;
    pack.sqr  = square;
    fill_3n_neighbors(&pack,ndims,0,0);

    if (ndims >= 10)
      free(square);

    b3 = grid->bcktrack3;
    for (p = 0; p < n3; p++)
      b3[p] = (int) (n3-(p+1));
  }

  return (id);
}


/****************************************************************************************
 *                                                                                      *
 *  THREADED BFS-QUEUE AND MARK-VECTOR RESOURCES                                        *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Size_Type   max;
    Indx_Type  *queue;
  } Queue;

static pthread_mutex_t Queue_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  Queue_Cond  = PTHREAD_COND_INITIALIZER;

static int   Queue_NoInit = 1;
static int   Queue_Alloc[NUM_FLOODS];
static Queue Queues[NUM_FLOODS];

static Queue *get_queue()
{ int id;

  pthread_mutex_lock(&Queue_Mutex);
  if (Queue_NoInit)
    { Queue_NoInit = 0;
      for (id = 0; id < NUM_FLOODS; id++)
        { Queue_Alloc[id]  = 0;
          Queues[id].max   = 0;
          Queues[id].queue = NULL;
        }
    }
  while (1)
    { for (id = 0; id < NUM_FLOODS; id++)
        if (Queue_Alloc[id] == 0)
          { Queue_Alloc[id] = 1;
            break;
          }
      if (id < NUM_FLOODS)
        break;
      pthread_cond_wait(&Queue_Cond,&Queue_Mutex);
    }
  pthread_mutex_unlock(&Queue_Mutex);

  return (Queues+id);
}

static void release_queue(Queue *que)
{ pthread_mutex_lock(&Queue_Mutex);
  Queue_Alloc[que-Queues] = 0;
  pthread_cond_signal(&Queue_Cond);
  pthread_mutex_unlock(&Queue_Mutex);
}

static void reset_queues()
{ int id;
  pthread_mutex_lock(&Queue_Mutex);
  if (! Queue_NoInit)
    { for (id = 0; id < NUM_FLOODS; id++)
        { if (Queue_Alloc[id] != 0)
            pthread_cond_wait(&Queue_Cond,&Queue_Mutex);
          free(Queues[id].queue);
        }
      Queue_NoInit = 1;
    }
  pthread_mutex_unlock(&Queue_Mutex);
}

static pthread_mutex_t Mark_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  Mark_Cond  = PTHREAD_COND_INITIALIZER;

static boolean   Mark_NoInit = 1;
static boolean   Mark_Alloc[NUM_FLOODS];
static int       Mark_Share[NUM_FLOODS];
static Size_Type Mark_Size[NUM_FLOODS];
static uint8    *Marks[NUM_FLOODS];

uint8 *get_mark(Size_Type size, int share)
{ int       alloc, id;
  boolean   match;

  pthread_mutex_lock(&Mark_Mutex);
  if (Mark_NoInit)
    { Mark_NoInit = 0;
      for (id = 0; id < NUM_FLOODS; id++)
        { Mark_Alloc[id] = 0;
          Mark_Size[id]  = 0;
          Marks[id]      = NULL;
        }
    }
  while (1)
    { alloc = -1;
      match = 0;
      for (id = 0; id < NUM_FLOODS; id++)
        if (Mark_Alloc[id] == 0)
          { if (alloc >= 0)
              { if (Mark_Size[id] >= size)
                  { if (Mark_Size[alloc] < size || Mark_Size[id] < Mark_Size[alloc])
                      alloc = id;
                  }
                else
                  { if (Mark_Size[alloc] < Mark_Size[id])
                      alloc = id;
                  }
              }
            else
              alloc = id;
          }
        else if (share > 0 && Mark_Share[id] == share)
          { if (size != Mark_Size[id])
              { fprintf(stderr,"Array size different than other members");
                fprintf(stderr," of share group %d (Flood_<X>)\n",share);
                exit (1);
              }
            alloc = id;
            match = 1;
            break;
          }
      if (alloc >= 0)
        break;
      pthread_cond_wait(&Mark_Cond,&Mark_Mutex);
    }
  if (match)
    Mark_Alloc[alloc] += 1;
  else
    { Size_Type osize = Mark_Size[alloc];
      Mark_Alloc[alloc]  = 1;
      Mark_Share[alloc]  = share;
      if (osize < size)
        { Mark_Size[alloc] = size;
          Marks[alloc]     = (uint8 *) Guarded_Realloc(Marks[alloc],(size_t) size,"Flood_<X>");
          memset(Marks[alloc]+osize, 0, (size_t) (size-osize));
        }
    }
  pthread_mutex_unlock(&Mark_Mutex);
  
  return (Marks[alloc]);
}

void release_mark(uint8 *mark)
{ int id;
  pthread_mutex_lock(&Mark_Mutex);
  for (id = 0; id < NUM_FLOODS; id++)
    if (mark == Marks[id])
      break;
  Mark_Alloc[id] -= 1;
  if (Mark_Alloc[id] == 0)
    pthread_cond_signal(&Mark_Cond);
  pthread_mutex_unlock(&Mark_Mutex);
}

static void reset_mark()
{ int id;
  pthread_mutex_lock(&Mark_Mutex);
  if (! Mark_NoInit)
    { for (id = 0; id < NUM_FLOODS; id++)
        { while (Mark_Alloc[id] != 0)
            pthread_cond_wait(&Mark_Cond,&Mark_Mutex);
          free(Marks[id]);
        }
      Mark_NoInit = 1;
    }
  pthread_mutex_unlock(&Mark_Mutex);
}

static pthread_mutex_t QAndM_Mutex = PTHREAD_MUTEX_INITIALIZER;

static uint8 *get_queue_and_mark(Size_Type size, int share, Queue **queue)
{ uint8 *mark;

  pthread_mutex_lock(&QAndM_Mutex);
  *queue = get_queue();
  mark = get_mark(size,share);
  pthread_mutex_unlock(&QAndM_Mutex);
  return (mark);
}

void Reset_Flood()
{ reset_mark();
  reset_queues();
}


/****************************************************************************************
 *                                                                                      *
 *  BFS FLOOD_FILL ROUTINE TEMPLATES                                                    *
 *                                                                                      *
 ****************************************************************************************/

/* Allocator for workstorage for BFS traversals */

#define PUSH(p)                                                         	\
{ queue[qtop++] = (p);                                                  	\
  if (qtop >= qmax)                                                     	\
    qtop = 0;                                                           	\
  if (qtop == qbot)                                                     	\
    { Size_Type qhav = (qmax+1) / 2;                                    	\
      Size_Type qnew = qmax + qhav;                                     	\
      queue = (Indx_Type *) Guarded_Realloc(queue,				\
                                            sizeof(Indx_Type)*((size_t) qnew),	\
                                            "Flood_Fill");              	\
      if (qbot >= qhav)                                                 	\
        { qbot = qmax-qbot;                                             	\
          qmax = qnew-qbot;                                             	\
          memcpy(queue + qmax,queue + qtop, sizeof(Indx_Type)*((size_t) qbot));	\
          qbot = qnew - qbot;  	                                        	\
        }                                                               	\
      else                                                              	\
        { memcpy(queue + qmax, queue, sizeof(Indx_Type)*((size_t) qbot));	\
          qtop = qmax + qbot;                                           	\
        }                                                               	\
      qmax = qnew;                                                      	\
    }                                                                   	\
}

#define POP(p)          \
{ p = queue[qbot++];    \
  if (qbot >= qmax)     \
    qbot = 0;           \
}

static void NullEvaluate(Indx_Type p, void *arge)
{ (void) p;
  (void) arge;
}

static boolean NullAccept(Size_Type n, void *argo)
{ (void) n;
  (void) argo;
  return (1);
}

static void NullAction(Indx_Type p, void *arga)
{ (void) p;
  (void) arga;
}

/* Flood fill (using connectivity specified by iscon2n) from pixel seed all pixels p
     for which test(p) is non-zero.  In a first pass the total number of such pixels are
     counted and total is called with the count.  In a second pass action(p) is called on
     each pixel p.  Total may be NULL in which case it is not called.

   In order for the routine to be re-entrant, the caller must supply a uint8 vector whose
     length is equal to the # of elements in the underlying array of source, i.e.
     AForm_Size(source).  The array must further be zero'd.  The array is used by Flood_Object
     but is guaranteed to be zero'd upon return, so the caller need only zero it once.

   Moreover, any additional information from the caller to the routines test, total, and
     action can be passed through whatever structure the argument argp points to as this
     pointer is passed as the second argument to each of these routines.
*/ 


void Flood_Object(APart *source, int share, boolean iscon2n, Indx_Type seed,
                  void *argt, boolean (*test)(Indx_Type p, void *argpt),
                  void *arge, void    (*evaluate)(Indx_Type p, void *arge),
                  void *argo, boolean (*accept)(Size_Type n, void *argo),
                  void *arga, void    (*action)(Indx_Type p, void *arga))

{ Array     *array = AForm_Array(source);

  Queue     *qrec;
  Indx_Type *queue, qmax;
  uint8     *mark;

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  boolean   *(*BOUNDS)(Grid_Id,Indx_Type,boolean);

  Indx_Type  qtop, qbot;
  Indx_Type  cnt;
 
  grid     = Setup_Grid(source,"Flood_Object");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  if (share == 0)
    mark = get_queue_and_mark(AForm_Size(source),share,&qrec);
  else
    mark = get_queue_and_mark(array->size,share,&qrec);
  qmax  = qrec->max;
  queue = qrec->queue;
  if (qmax < 16384)
    { qmax  = 16384;
      queue = (Indx_Type *) Guarded_Realloc(queue,sizeof(Indx_Type)*((size_t) qmax),"Flood_Object");
    }

  if (evaluate == NULL)
    evaluate = NullEvaluate;
  if (accept == NULL)
    accept = NullAccept;
  if (action == NULL)
    action = NullAction;

  if (Is_Slice(source) && share == 0)
    { Offs_Type *neighbori;
      Indx_Type  seedi;
      Grid_Id    gridi;

      { Indx_Type    curp;
        Coordinate  *seedc, *basis;
        Array_Bundle sbdl;
 
        basis = AForm_Shape(source);
        sbdl.dims  = ADIMN(basis);
        sbdl.ndims = basis->dims[0];
        sbdl.kind  = PLAIN_KIND;

        gridi = Setup_Grid(&sbdl,"Flood_Object");
        neighbori = Grid_Neighbors(gridi,iscon2n);

        curp = Slice_Index(source);

        if ( ! Set_Slice_To_Index(source,seed))
          { fprintf(stderr,"Seed is not inside slice! (Flood_Object)\n");
            exit (1);
          }
        seedc = Copy_Array(Slice_Coordinate(source));
        Array_Op_Array(seedc,SUB_OP,Slice_First(source));
        seedi = Coord2IdxA(&sbdl,seedc); 
    
        Set_Slice_To_Index(source,curp);

        Free_Array(basis);
      }

      cnt  = 0;
      qtop = qbot = 0;

      //  BFS flood from seed, mark = 1 if in, 2 if not, just test & eval in this pass

      PUSH(seed);
      PUSH(seedi);				
      mark[seedi] = 1;
      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;
          Indx_Type pi, qi;			

          POP(p);
          POP(pi);				
          cnt += 1;
      
          evaluate(p,arge);

          b = BOUNDS(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                qi = pi + neighbori[j];		
                if (mark[qi] == 0)
                  { if (test(q,argt))
                      { PUSH(q);
                        PUSH(qi);		
                        mark[qi] = 1;
                      }
                    else
                      mark[qi] = 2;
                  }
              }
        }

      //  If not NULL call total with object size
  
      if ( ! accept(cnt,argo))
        action = NullAction;
  
      //  BFS flood from seed again to reset mark and call action on each pixel in object

      PUSH(seed);
      PUSH(seedi);				
      mark[seedi] = 0;
      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;
          Indx_Type pi, qi;			
  
          POP(p);
          POP(pi);				
          action(p,arga);
          b = BOUNDS(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                qi = pi + neighbori[j];		
                if (mark[qi] == 1)
                  { PUSH(q);
                    PUSH(qi);			
                  }
                mark[qi] = 0;
              }
        }

      Release_Grid(gridi);			
    }
  else
    { if (Is_Slice(source))
        { Indx_Type  curp;

          curp = Slice_Index(source);
          if ( ! Set_Slice_To_Index(source,seed))
            { fprintf(stderr,"Seed is not inside slice! (Flood_Object)\n");
              exit (1);
            }
          Set_Slice_To_Index(source,curp);
        }

      cnt  = 0;
      qtop = qbot = 0;

      //  BFS flood from seed, mark = 1 if in, 2 if not, just test & eval in this pass

      PUSH(seed);
      mark[seed] = 1;
      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;

          POP(p);
          cnt += 1;
      
          evaluate(p,arge);

          b = BOUNDS(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (mark[q] == 0)
                  { if (test(q,argt))
                      { PUSH(q);
                        mark[q] = 1;
                      }
                    else
                      mark[q] = 2;
                  }
              }
        }

      //  If not NULL call total with object size
  
      if ( ! accept(cnt,argo))
        action = NullAction;
  
      //  BFS flood from seed again to reset mark and call action on each pixel in object

      PUSH(seed);
      mark[seed] = 0;
      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;
  
          POP(p);
          action(p,arga);
          b = BOUNDS(grid,p,iscon2n);
          for (j = 0; j < n_nbrs; j++)
            if (b[j])
              { q = p + neighbor[j];
                if (mark[q] == 1)
                  { PUSH(q);
                  }
                mark[q] = 0;
              }
        }

    }

  qrec->max   = qmax;
  qrec->queue = queue;
  release_queue(qrec);
  release_mark(mark);
  Release_Grid(grid);
}


/* Scan source, executing a flood fill (connectivity specified by iscon2n) whenever
     a pixel is encountered for which test(p) is non-zero.  The flood fill reaches
     each pixel connected to this one for which test(p) is true and calls evaluate(p)
     on it.  Upon completion of each flood fill it calls accept(cnt) to see if it should
     call action on each pixel in this flood-fill region.  The value cnt passed to action
     is the number of pixels in connected object.  If accept returns a non-zero value then
     action is called on each pixel in the iscon2n-connected region.                        */

void Flood_All(APart *source, int share, boolean iscon2n,
               void *argt, boolean (*test)(Indx_Type p, void *argt),
               void *arge, void    (*evaluate)(Indx_Type p, void *arge),
               void *argo, boolean (*accept)(Size_Type n, void *argo),
               void *arga, void    (*action)(Indx_Type p, void *arga))

{ Array      *array  = AForm_Array(source);
  Size_Type   size   = AForm_Size(source);

  Queue      *qrec;
  Indx_Type  *queue, qmax;
  uint8      *mark;

  Grid_Id     grid;
  int         n_nbrs;
  Offs_Type  *neighbor;
  boolean    *(*BOUNDS)(Grid_Id,Indx_Type,int);

  Indx_Type   cnt;
  Indx_Type   seed, seedi;
  Indx_Type   qtop, qbot;
  Indx_Type   curp = 0;
  boolean     isslice;

  grid     = Setup_Grid(source,"Flood_All");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);
  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  if (share == 0)
    mark = get_queue_and_mark(size,share,&qrec);
  else
    mark = get_queue_and_mark(array->size,share,&qrec);
  qmax  = qrec->max;
  queue = qrec->queue;
  if (qmax < 16384)
    { qmax  = 16384;
      queue = (Indx_Type *) Guarded_Realloc(queue,sizeof(Indx_Type)*((size_t) qmax),"Flood_All");
    }

  isslice = Is_Slice(source);

  if (evaluate == NULL)
    evaluate = NullEvaluate;
  if (accept == NULL)
    accept = NullAccept;
  if (action == NULL)
    action = NullAction;

  if (isslice && share == 0)
    { Grid_Id    gridi;
      Offs_Type *neighbori;
 
      { Coordinate  *basis;
        Array_Bundle sbdl;

        basis = AForm_Shape(source);
        sbdl.dims  = ADIMN(basis);
        sbdl.ndims = basis->dims[0];
        sbdl.kind  = PLAIN_KIND;

        gridi = Setup_Grid(&sbdl,"Flood_All");
        neighbori = Grid_Neighbors(gridi,iscon2n);

        Free_Array(basis);
      }

      curp = Slice_Index(source);
      seed = Set_Slice_To_First(source);
      qtop = qbot = 0;
      for (seedi = 0; seedi < size; seedi++)
        { if (mark[seedi] == 0)
            { if (test(seed,argt))
                { PUSH(seed);
                  PUSH(seedi);					
                  mark[seedi] = 1;

                  //  BFS flood from seed, evaluate and mark = 1 if in, 2 if not, as you go

                  cnt = 0;
                  while (qtop != qbot)
                    { boolean  *b;
                      int       j;
                      Indx_Type p, q;
                      Indx_Type pi, qi;				

                      POP(p);
                      POP(pi);					
                      cnt += 1;
      
                      evaluate(p,arge);
      
                      b = BOUNDS(grid,p,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { q = p + neighbor[j];
                            qi = pi + neighbori[j];		
                            if (mark[qi] == 0)
                              { if (test(q,argt))
                                  { PUSH(q);
                                    PUSH(qi);			
                                    mark[qi] = 1;
                                  }
                                else
                                  mark[qi] = 2;
                              }
                          }
                    }

                  //  If accept, BFS flood from seed, action and mark = 3 as you go

                  if (accept(cnt,argo) && action != NullAction)
                    { PUSH(seed);
                      PUSH(seedi);				
                      mark[seedi] = 3;
                      while (qtop != qbot)
                        { boolean  *b;
                          int       j;
                          Indx_Type p, q;
                          Indx_Type pi, qi;			

                          POP(p);
                          POP(pi);				
		          action(p,arga);

                          b = BOUNDS(grid,p,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { q = p + neighbor[j];
                                qi = pi  + neighbor [j];	
                                if (mark[qi] == 1)
                                  { PUSH(q);
                                    PUSH(qi);			
                                  }
                                mark[qi] = 3;
                              }
                        }
                    }
                }
              else
                mark[seedi] = 3;
            }
          seed = Next_Slice_Index(source);			
        }

      Release_Grid(gridi);					

      memset(mark, 0, (size_t) size);
    }
  else if (isslice && share != 0)
    { curp = Slice_Index(source);
      seed = Set_Slice_To_First(source);
      qtop = qbot = 0;
      for (seedi = 0; seedi < size; seedi++)
        { if (mark[seed] == 0)
            { if (test(seed,argt))
                { PUSH(seed);
                  mark[seed] = 1;

                  //  BFS flood from seed, evaluate and mark = 1 if in, 2 if not, as you go

                  cnt = 0;
                  while (qtop != qbot)
                    { boolean  *b;
                      int       j;
                      Indx_Type p, q;

                      POP(p);
                      cnt += 1;
      
                      evaluate(p,arge);
      
                      b = BOUNDS(grid,p,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { q = p + neighbor[j];
                            if (mark[q] == 0)
                              { if (test(q,argt))
                                  { PUSH(q);
                                    mark[q] = 1;
                                  }
                                else
                                  mark[q] = 2;
                              }
                          }
                    }

                  //  If accept, BFS flood from seed, action and mark = 3 as you go

                  if (accept(cnt,argo) && action != NullAction)
                    { PUSH(seed);
                      mark[seed] = 3;
                      while (qtop != qbot)
                        { boolean  *b;
                          int       j;
                          Indx_Type p, q;

                          POP(p);
		          action(p,arga);

                          b = BOUNDS(grid,p,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { q = p + neighbor[j];
                                if (mark[q] == 1)
                                  { PUSH(q);
                                  }
                                mark[q] = 3;
                              }
                        }
                    }
                }
              else
                mark[seed] = 3;
            }
          seed = Next_Slice_Index(source);			
        }


      seed = Set_Slice_To_First(source);
      for (seedi = 0; seedi < size; seedi++)
        { mark[seed] = 0;
          seed = Next_Slice_Index(source);
        }
    }
  else
    { qtop = qbot = 0;
      for (seed = 0; seed < size; seed++)
        { if (mark[seed] == 0)
            { if (test(seed,argt))
                { PUSH(seed);
                  mark[seed] = 1;

                  //  BFS flood from seed, evaluate and mark = 1 if in, 2 if not, as you go

                  cnt = 0;
                  while (qtop != qbot)
                    { boolean  *b;
                      int       j;
                      Indx_Type p, q;

                      POP(p);
                      cnt += 1;
      
                      evaluate(p,arge);
      
                      b = BOUNDS(grid,p,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { q = p + neighbor[j];
                            if (mark[q] == 0)
                              { if (test(q,argt))
                                  { PUSH(q);
                                    mark[q] = 1;
                                  }
                                else
                                  mark[q] = 2;
                              }
                          }
                    }

                  //  If accept, BFS flood from seed, action and mark = 3 as you go

                  if (accept(cnt,argo) && action != NullAction)
                    { PUSH(seed);
                      mark[seed] = 3;
                      while (qtop != qbot)
                        { boolean  *b;
                          int       j;
                          Indx_Type p, q;

                          POP(p);
		          action(p,arga);

                          b = BOUNDS(grid,p,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { q = p + neighbor[j];
                                if (mark[q] == 1)
                                  { PUSH(q);
                                  }
                                mark[q] = 3;
                              }
                        }
                    }
                }
              else
                mark[seed] = 3;
            }
        }


      memset(mark, 0, (size_t) size);
    }

  if (isslice)
    Set_Slice_To_Index(source,curp);

  qrec->max   = qmax;
  qrec->queue = queue;
  release_queue(qrec);
  release_mark((uint8 *) mark);
  Release_Grid(grid);
}

/* Flood fill (using connectivity specified by iscon2n) the surface of the test-is-true object
    containing seed.  In a first pass the total number of such pixels are counted and total(c,s)
    is called with c = # of all pixels in an iscon2n-connected boundary of the object, and
    s (<= c) = # of all pixels with a face on the surface.  In a second pass action(p,x) is
    called on each pixel p on the surface of the object where x is the # of pixel faces in the
    x-direction not adjacent to a pixel in the object (0, 1, or 2).  Total may be NULL in which
    case it is not called.                                                                     */

#define O_PIXEL   1
#define OUTSIDE   2
#define SURFACE   3

#define l_PIXEL   4
#define INSIDE    5
#define SEARCHED  6
#define BRIDGE    7
#define BORDER    8

static boolean NullSAccept(Size_Type n, Size_Type a, Size_Type d, void *argo)
{ (void) n;
  (void) a;
  (void) d;
  (void) argo;
  return (1);
}

static void NullSAction(Indx_Type p, int x, void *arga)
{ (void) p;
  (void) x;
  (void) arga;
}

void Flood_Surface(APart *source, int share, boolean iscon2n, Indx_Type seed,
                   void *argt, boolean (*test)(Indx_Type p, void *argt),
                   void *arge, void    (*evaluate)(Indx_Type p, void *arge),
                   void *argo, boolean (*accept)(Size_Type n, Size_Type a, Size_Type d, void *argo),
                   void *arga, void    (*action)(Indx_Type p, int x, void *arga))

{ Array      *array = AForm_Array(source);

  Queue      *qrec;
  Indx_Type  *queue, qmax;
  uint8      *mark;

  Grid_Id     grid;
  boolean     conB, conS;
  int         n_nbrs2, n_nbrs3;
  Offs_Type  *neighB, *neighS;
  int         n_nbrsB, n_nbrsS;
  Offs_Type  *neigh2, *neigh3;
  boolean     *(*BOUNDS)(Grid_Id,Indx_Type,int);

  Indx_Type   qtop, qbot;
  Size_Type   ext, srf, dbl;

#ifdef DEBUG_SURFACE
  Array_Bundle debug;
#endif

  grid    = Setup_Grid(source,"Flood_Surface");
  conB    = iscon2n;
  conS    = 1-iscon2n;
  n_nbrsB = Grid_Size(grid,conB);
  n_nbrsS = Grid_Size(grid,conS);
  neighB  = Grid_Neighbors(grid,conB);
  neighS  = Grid_Neighbors(grid,conS);

  if (iscon2n)
    { neigh2  = neighB;
      n_nbrs2 = n_nbrsB;
      neigh3  = neighS;
      n_nbrs3 = n_nbrsS;
    }
  else
    { neigh2  = neighS;
      n_nbrs2 = n_nbrsS;
      neigh3  = neighB;
      n_nbrs3 = n_nbrsB;
    }

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;
  if (share == 0)
    mark = get_queue_and_mark(AForm_Size(source),share,&qrec);
  else
    mark = get_queue_and_mark(array->size,share,&qrec);
  qmax  = qrec->max;
  queue = qrec->queue;
  if (qmax < 16384)
    { qmax  = 16384;
      queue = (Indx_Type *)
                 Guarded_Realloc(queue,sizeof(Indx_Type)*((size_t) qmax),"Flood_Surface");
    }

  if (evaluate == NULL)
    evaluate = NullEvaluate;
  if (accept == NULL)
    accept = NullSAccept;
  if (action == NULL)
    action = NullSAction;

#ifdef DEBUG_SURFACE
  debug       = *array;
  debug.kind  = PLAIN_KIND;
  debug.type  = INT8;
  debug.scale = 8;
  debug.data  = mark;
#endif

  if (Is_Slice(source) && share == 0)
    { Grid_Id     gridi;
      Offs_Type  *neighBi, *neighSi;
      Offs_Type  *neigh2i, *neigh3i;
      Indx_Type   seedi;
      Coordinate *basis;

      { Indx_Type    curp;
        Coordinate  *seedc;
        Array_Bundle sbdl;

        basis      = AForm_Shape(source);
        sbdl.dims  = ADIMN(basis);
        sbdl.ndims = basis->dims[0];
        sbdl.kind  = PLAIN_KIND;
        sbdl.size  = AForm_Size(source);
#ifdef DEBUG_SURFACE
        debug.dims = sbdl.dims;
        debug.size = sbdl.size;
#endif

        gridi = Setup_Grid(&sbdl,"Flood_Object");
        neighBi = Grid_Neighbors(gridi,conB);
        neighSi = Grid_Neighbors(gridi,conS);
        if (iscon2n)
          { neigh2i = neighBi;
            neigh3i = neighSi;
          }
        else
          { neigh3i = neighBi;
            neigh2i = neighSi;
          }

        curp = Slice_Index(source);

        if ( ! Set_Slice_To_Index(source,seed))
          { fprintf(stderr,"Seed is not inside slice! (Flood_Surface)\n");
            exit (1);
          }
        seedc = Copy_Array(Slice_Coordinate(source));
        Array_Op_Array(seedc,SUB_OP,Slice_First(source));
        seedi = Coord2IdxA(&sbdl,seedc); 

        Set_Slice_To_Index(source,curp);

#ifndef DEBUG_SURFACE
        Free_Array(basis);
#endif
      }

      //  Starting with seed traverse the pixels of the object that are S-connected to a 0-pixel
      //    determining which may be surface and border pixels.

      qtop = qbot = 0;
      PUSH(seed);
      PUSH(seedi);					
      mark[seedi] = INSIDE;

      while (qtop != qbot)
        { boolean  *b;
          int       j, bnd, cnt;
          Indx_Type p, q;
          Indx_Type pi, qi;				

          POP(p);    //  p has state INSIDE
          POP(pi);					

          cnt = bnd = 0;

          b = BOUNDS(grid,p,conS);         //  Does p connect to a 0-pixel or boundary?
          for (j = 0; j < n_nbrsS; j++)    //    To save time calling test, mark
            if (b[j])                      //    each test with l_PIXEL or O_PIXEL states
              { q = p + neighS[j];
                qi = pi + neighSi[j];			
                if (mark[qi] == 0)
                  { if (test(q,argt))
                      mark[qi] = l_PIXEL;
                    else
                      { mark[qi] = O_PIXEL;
                        cnt += 1;
                      }
                  }
                else if (mark[qi] <= SURFACE)   
                  cnt += 1;
              }
            else
              { bnd += 1;
                cnt += 1;
              }

          if (cnt > 0)                //  Yes: All B-connected 1-pixels should be explored
            { if (bnd > 0)            //    and p is marked as having been SEARCHED or BRIDGE
                mark[pi] = BRIDGE;
              else
                mark[pi] = SEARCHED;

              b = BOUNDS(grid,p,conB);
              for (j = 0; j < n_nbrsB; j++)
                if (b[j])
                  { q = p + neighB[j];
                    qi = pi + neighBi[j];		
                    if (mark[qi] == l_PIXEL)
                      { PUSH(q)
                        PUSH(qi);			
                        mark[qi] = INSIDE;
                      }
                    else if (mark[qi] == O_PIXEL)
                      mark[qi] = OUTSIDE;
                    else if (mark[qi] == 0)
                      { if (test(q,argt))
                          { PUSH(q)
                            PUSH(qi);			
                            mark[qi] = INSIDE;
                          }
                        else
                          mark[qi] = OUTSIDE;
                      }
                  }
            }
        }

      //  BRIDGE:     1-pixel that touches the boundary of the array ==> is a border pixel
      //  SEARCHED:   1-pixel S-Connected to a 0-pixel => may be on the border of object
      //  l_PIXEL:    1-pixel S-Connected to an S+B+I pixel but definitely not on the border
      //  INSIDE:     1-pixel B-Connected to an S+B pixel but definitely not on the border
      //  OUTSIDE:    0-pixel B-Connected to an S+B pixel ==> may be on the surface
      //  O_PIXEL:    0-pixel S-Connected to an S+B pixel but definitely not on the surface
      //
      //  The S+B pixels form a single B-connected set and every other marked pixel is
      //    S-connected to this set.  The true border is a subset of S+B and the true
      //    surface is a subset of O.

#ifdef DEBUG_SURFACE
      printf("\nAfter inside traversal:\n");
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
#endif

      //  Next traverse the surface to determine it and the true border

      ext = srf = dbl = 0;

      { Indx_Type lx = (Indx_Type) (seed % array->dims[0]);
        qtop = qbot = 0;
        if (Is_Slice(source))
          if (lx > ADIMN(Slice_First(source))[0])
            { PUSH(seed-1);
              PUSH(seedi-1);				
              mark[seedi-1] = SURFACE;
            }
          else
            { PUSH(seed);
              ext += 1;
              PUSH(seedi);				
              mark[seedi] = BORDER;
              evaluate(seed,arge);
            }
        else
          if (lx > 0)
            { PUSH(seed-1);
              PUSH(seedi-1);				
              mark[seedi-1] = SURFACE;
            }
          else
            { PUSH(seed);
              ext += 1;
              PUSH(seedi);				
              mark[seedi] = BORDER;
              evaluate(seed,arge);
            }
      }

      while (qtop != qbot)     //  Traverse the surface pixels with an S-connected search
        { boolean  *b;
          int       j;
          Indx_Type p, q;
          Indx_Type pi, qi;				

          POP(p);
          POP(pi);					

          if (mark[pi] == SURFACE)
            { b = BOUNDS(grid,p,conS);
              for (j = 0; j < n_nbrsS; j++)
                if (b[j])
                  { q = p + neighS[j];
                    qi = pi + neighSi[j];		
                    if (mark[qi] == OUTSIDE)        //  S-connected to p ==> on the surface
                      { PUSH(q)
                        PUSH(qi);			
                        mark[qi] = SURFACE;
                      }
                    else if (mark[qi] == SEARCHED)  //  S-connected to p ==> on the border
                      { ext += 1;
                        mark[qi] = BORDER;
                        evaluate(q,arge);
                      }
                    else if (mark[qi] == BRIDGE)    //  Bridges have to be explored as they may
                      { PUSH(q);                     //    be the only connection to another surface
                        ext += 1;                    //    pixel.
                        PUSH(qi);			
                        mark[qi] = BORDER;
                        evaluate(q,arge);
                      }
                  }

              b = BOUNDS(grid,p,1);           //  Search 2n-neighborhood to count surface area and
              for (j = 0; j < n_nbrs2; j++)   //    determine extremal border pixels
                if (b[j])
                  { qi = pi + neigh2i[j];		
                    if (mark[qi] >= BORDER)
                      { srf += 1;
                        if (j < 2)
                          { if (mark[qi] > BORDER)
                              dbl += 1;
                            mark[qi] += 1;
                          }
                      }
                  }
            }

          else  //  Former BRIDGE pixel (now a BORDER)
            { int opp;

               b = BOUNDS(grid,p,1);

               opp = -1;
               for (j = 0; j < n_nbrs2; j++)
                 if (!b[j])
                   { if (opp >= 0)
                       { opp = -1;
                         break;
                       }
                     else
                       opp = j;
                   }
               if (opp >= 0)
                 { if (opp%2)
                     opp -= 1;
                   else
                     opp += 1;
                 }
  
              for (j = 0; j < n_nbrs2; j++)
                { if (j == opp) continue;
                  if (b[j])
                    { q = p + neigh2[j];
                      qi = pi + neigh2i[j];		
                      if (mark[qi] == OUTSIDE)     //  2n-connected to p ==> on the surface
                        { PUSH(q)
                          PUSH(qi);			
                          mark[qi] = SURFACE;
                        }
                      else if (mark[qi] == BRIDGE)
                        { PUSH(q);
                          ext += 1;
                          PUSH(qi);			
                          mark[qi] = BORDER;
                          evaluate(q,arge);
                        }
                    }
                  else
                    { srf += 1;
                      if (j < 2)
                        { if (mark[pi] > BORDER)
                            dbl += 1;
                          mark[pi] += 1;
                        }
                    }
                }
            }
        }

      // SURFACE: all surface pixels
      // BORDER + {0,1,2}:  all border pixels

#ifdef DEBUG_SURFACE
      printf("\nAfter outside traversal: c=%lld s=%lld\n",ext,srf);
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
#endif

      if ( ! accept(ext,srf,dbl,argo))
        action = NullSAction;

      // Traverse the S+B pixels to turn off all marks and call action on all the border pixels

      qtop = qbot = 0;
      PUSH(seed)
      PUSH(seedi)					
      action(seed,mark[seedi]-BORDER,arga);
      mark[seedi] = 0;

      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;
          Indx_Type pi, qi;				

          POP(p);
          POP(pi);					
          b = BOUNDS(grid,p,0);
          for (j = 0; j < n_nbrs3; j++)
            if (b[j])
              { q = p + neigh3[j];
                qi = pi + neigh3i[j];			
                if (mark[qi] >= INSIDE)
                  { PUSH(q);
                    PUSH(qi);				
                    if (mark[qi] >= BORDER)
                      action(q,mark[qi]-BORDER,arga);
                  }
                mark[qi] = 0;
              }
        }

#ifdef DEBUG_SURFACE
      printf("\nAfter final traversal:\n");
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
      Free_Array(basis);				
#endif

      Release_Grid(gridi);				
    }
  else
    { if (Is_Slice(source))
        { Indx_Type  curp;

          curp = Slice_Index(source);
          if ( ! Set_Slice_To_Index(source,seed))
            { fprintf(stderr,"Seed is not inside slice! (Flood_Surface)\n");
              exit (1);
            }
          Set_Slice_To_Index(source,curp);
        }

      //  Starting with seed traverse the pixels of the object that are S-connected to a 0-pixel
      //    determining which may be surface and border pixels.

      qtop = qbot = 0;
      PUSH(seed);
      mark[seed] = INSIDE;

      while (qtop != qbot)
        { boolean  *b;
          int       j, bnd, cnt;
          Indx_Type p, q;

          POP(p);    //  p has state INSIDE

          cnt = bnd = 0;

          b = BOUNDS(grid,p,conS);         //  Does p connect to a 0-pixel or boundary?
          for (j = 0; j < n_nbrsS; j++)    //    To save time calling test, mark
            if (b[j])                      //    each test with l_PIXEL or O_PIXEL states
              { q = p + neighS[j];
                if (mark[q] == 0)
                  { if (test(q,argt))
                      mark[q] = l_PIXEL;
                    else
                      { mark[q] = O_PIXEL;
                        cnt += 1;
                      }
                  }
                else if (mark[q] <= SURFACE)   
                  cnt += 1;
              }
            else
              { bnd += 1;
                cnt += 1;
              }

          if (cnt > 0)                //  Yes: All B-connected 1-pixels should be explored
            { if (bnd > 0)            //    and p is marked as having been SEARCHED or BRIDGE
                mark[p] = BRIDGE;
              else
                mark[p] = SEARCHED;

              b = BOUNDS(grid,p,conB);
              for (j = 0; j < n_nbrsB; j++)
                if (b[j])
                  { q = p + neighB[j];
                    if (mark[q] == l_PIXEL)
                      { PUSH(q)
                        mark[q] = INSIDE;
                      }
                    else if (mark[q] == O_PIXEL)
                      mark[q] = OUTSIDE;
                    else if (mark[q] == 0)
                      { if (test(q,argt))
                          { PUSH(q)
                            mark[q] = INSIDE;
                          }
                        else
                          mark[q] = OUTSIDE;
                      }
                  }
            }
        }

      //  BRIDGE:     1-pixel that touches the boundary of the array ==> is a border pixel
      //  SEARCHED:   1-pixel S-Connected to a 0-pixel => may be on the border of object
      //  l_PIXEL:    1-pixel S-Connected to an S+B+I pixel but definitely not on the border
      //  INSIDE:     1-pixel B-Connected to an S+B pixel but definitely not on the border
      //  OUTSIDE:    0-pixel B-Connected to an S+B pixel ==> may be on the surface
      //  O_PIXEL:    0-pixel S-Connected to an S+B pixel but definitely not on the surface
      //
      //  The S+B pixels form a single B-connected set and every other marked pixel is
      //    S-connected to this set.  The true border is a subset of S+B and the true
      //    surface is a subset of O.

#ifdef DEBUG_SURFACE
      printf("\nAfter inside traversal:\n");
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
#endif

      //  Next traverse the surface to determine it and the true border

      ext = srf = dbl = 0;

      { Indx_Type lx = (Indx_Type) (seed % array->dims[0]);
        qtop = qbot = 0;
        if (Is_Slice(source))
          if (lx > ADIMN(Slice_First(source))[0])
            { PUSH(seed-1);
              mark[seed-1] = SURFACE;
            }
          else
            { PUSH(seed);
              ext += 1;
              mark[seed] = BORDER;
              evaluate(seed,arge);
            }
        else
          if (lx > 0)
            { PUSH(seed-1);
              mark[seed-1] = SURFACE;
            }
          else
            { PUSH(seed);
              ext += 1;
              mark[seed] = BORDER;
              evaluate(seed,arge);
            }
      }

      while (qtop != qbot)     //  Traverse the surface pixels with an S-connected search
        { boolean  *b;
          int       j;
          Indx_Type p, q;

          POP(p);

          if (mark[p] == SURFACE)
            { b = BOUNDS(grid,p,conS);
              for (j = 0; j < n_nbrsS; j++)
                if (b[j])
                  { q = p + neighS[j];
                    if (mark[q] == OUTSIDE)        //  S-connected to p ==> on the surface
                      { PUSH(q)
                        mark[q] = SURFACE;
                      }
                    else if (mark[q] == SEARCHED)  //  S-connected to p ==> on the border
                      { ext += 1;
                        mark[q] = BORDER;
                        evaluate(q,arge);
                      }
                    else if (mark[q] == BRIDGE)    //  Bridges have to be explored as they may
                      { PUSH(q);                     //    be the only connection to another surface
                        ext += 1;                    //    pixel.
                        mark[q] = BORDER;
                        evaluate(q,arge);
                      }
                  }

              b = BOUNDS(grid,p,1);           //  Search 2n-neighborhood to count surface area and
              for (j = 0; j < n_nbrs2; j++)   //    determine extremal border pixels
                if (b[j])
                  { q = p + neigh2[j];			
                    if (mark[q] >= BORDER)
                      { srf += 1;
                        if (j < 2)
                          { if (mark[q] > BORDER)
                              dbl += 1;
                            mark[q] += 1;
                          }
                      }
                  }
            }

          else  //  Former BRIDGE pixel (now a BORDER)
            { int opp;

               b = BOUNDS(grid,p,1);

               opp = -1;
               for (j = 0; j < n_nbrs2; j++)
                 if (!b[j])
                   { if (opp >= 0)
                       { opp = -1;
                         break;
                       }
                     else
                       opp = j;
                   }
               if (opp >= 0)
                 { if (opp%2)
                     opp -= 1;
                   else
                     opp += 1;
                 }
  
              for (j = 0; j < n_nbrs2; j++)
                { if (j == opp) continue;
                  if (b[j])
                    { q = p + neigh2[j];
                      if (mark[q] == OUTSIDE)     //  2n-connected to p ==> on the surface
                        { PUSH(q)
                          mark[q] = SURFACE;
                        }
                      else if (mark[q] == BRIDGE)
                        { PUSH(q);
                          ext += 1;
                          mark[q] = BORDER;
                          evaluate(q,arge);
                        }
                    }
                  else
                    { srf += 1;
                      if (j < 2)
                        { if (mark[p] > BORDER)
                            dbl += 1;
                          mark[p] += 1;
                        }
                    }
                }
            }
        }

      // SURFACE: all surface pixels
      // BORDER + {0,1,2}:  all border pixels

#ifdef DEBUG_SURFACE
      printf("\nAfter outside traversal: c=%lld s=%lld\n",ext,srf);
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
#endif

      if ( ! accept(ext,srf,dbl,argo))
        action = NullSAction;

      // Traverse the S+B pixels to turn off all marks and call action on all the border pixels

      qtop = qbot = 0;
      PUSH(seed)
      action(seed,mark[seed]-BORDER,arga);
      mark[seed] = 0;

      while (qtop != qbot)
        { boolean  *b;
          int       j;
          Indx_Type p, q;

          POP(p);
          b = BOUNDS(grid,p,0);
          for (j = 0; j < n_nbrs3; j++)
            if (b[j])
              { q = p + neigh3[j];
                if (mark[q] >= INSIDE)
                  { PUSH(q);
                    if (mark[q] >= BORDER)
                      action(q,mark[q]-BORDER,arga);
                  }
                mark[q] = 0;
              }
        }

#ifdef DEBUG_SURFACE
      printf("\nAfter final traversal:\n");
      Print_Array(&debug,stdout,4,"%2hhd");
      fflush(stdout);
#endif

    }

  qrec->max   = qmax;
  qrec->queue = queue;
  release_queue(qrec);
  release_mark(mark);
  Release_Grid(grid);
}

/* Find a seed that is guaranteed to be at the surface (namely the smallest) */

#define Small(a) *((Indx_Type *) (a))

static void note_smallest(Indx_Type p, void *a)
{ if (p < Small(a))
    Small(a) = p;
}

Indx_Type Find_Leftmost_Seed(APart *source, int share, boolean iscon2n, Indx_Type seed,
                             void *arg, int (*test)(Indx_Type p, void *arg))
{ Indx_Type smallest = AForm_Array(source)->size;
  Flood_Object(source,share,iscon2n,seed,arg,test,NULL,NULL,NULL,NULL,&smallest,note_smallest);
  return (smallest);
}
