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
*  Watershed Partitioning                                                                 *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*  Mod   :  Aug 2008 -- Added Waterhshed Graph and Tree concepts                          *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <stdint.h>
#include <float.h>

#ifdef __APPLE__

#include <malloc/malloc.h>

#else

#include <malloc.h>

#endif

#ifdef _MSC_VER

#define  malloc_size _msize

#elif defined __APPLE__
// Mac
// no action, Mac is the only platform with an actual malloc_size() method
#else
// Linux
#define  malloc_size malloc_usable_size
#endif

#include "mylib.h"
#include "utilities.h"
#include "connectivity.h"
#include "array.h"
#include "image.h"
#include "draw.h"
#include "histogram.h"
#include "water.shed.h"

#undef   DEBUG
#undef   SHOW_GRAPH
#undef   DRAW_WATERSHED
#undef   DEBUG_COLLAPSE


/**************************************************************************************\
*                                                                                      *
*  WATERSHED SPACE MANAGEMENT ROUTINES                                                 *
*                                                                                      *
\**************************************************************************************/

typedef struct
  { APart        *apart_ref; // Reference to the image component tree was made from
    boolean       iscon2n;   // (2n)-connected (vs. (3^n-1)-connected) regions?
    boolean       colored;   // The label field is a coloring
    int           nlabels;   // The number of distinct region labels (1..nlabels)
    Label_Array  *labels;    // A plain UINT8_, UINT16_, or UINT32_TYPE, PLAIN_KIND coloring array
    int           nbasins;   // The number of vertices in the partition
    P_Vertex     *verts;     // An array [0..nbasins-1] of region records
    P_Edge       *edges;     // An array [0..first[nbasins]>>1] of edge records
    int          *alist;     // The edges for CB v are edges[alist[j]],
    int          *first;     //   for all j in [first[v],first[v+1]]
  } Wshed;

#define SIZEOF(x) ((int) sizeof(x))

static inline int wshed_vsize(Wshed *part)
{ return (SIZEOF(P_Vertex)*(part->nbasins+1)); }

static inline int wshed_esize(Wshed *part)
{ return (SIZEOF(P_Edge)*((part->first[part->nbasins]) >> 1)); }

static inline int wshed_asize(Wshed *part)
{ return (SIZEOF(int)*(part->first[part->nbasins]+1)); }

static inline int wshed_fsize(Wshed *part)
{ return (SIZEOF(int)*(part->nbasins+1)); }


typedef struct __Wshed
  { struct __Wshed *next;
    struct __Wshed *prev;
    int             refcnt;
    int             vsize;
    int             esize;
    int             asize;
    int             fsize;
    Wshed           wshed;
  } _Wshed;

static _Wshed *Free_Wshed_List = NULL;
static _Wshed *Use_Wshed_List  = NULL;

static pthread_mutex_t Wshed_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Wshed_Offset = sizeof(_Wshed)-sizeof(Wshed);
static int Wshed_Inuse  = 0;

int Partition_Refcount(Partition *partition)
{ _Wshed *object = (_Wshed *) (((char *) partition) - Wshed_Offset);
  return (object->refcnt);
}

static inline int allocate_wshed_verts(Wshed *wshed, int vsize, char *routine)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  if (object->vsize < vsize)
    { void *x = Guarded_Realloc(wshed->verts,(size_t) vsize,routine);
      if (x == NULL) return (1);
      wshed->verts = x;
      object->vsize = vsize;
    }
  return (0);
}

static inline int sizeof_wshed_verts(Wshed *wshed)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  return (object->vsize);
}

static inline int allocate_wshed_edges(Wshed *wshed, int esize, char *routine)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  if (object->esize < esize)
    { void *x = Guarded_Realloc(wshed->edges,(size_t) esize,routine);
      if (x == NULL) return (1);
      wshed->edges = x;
      object->esize = esize;
    }
  return (0);
}

static inline int sizeof_wshed_edges(Wshed *wshed)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  return (object->esize);
}

static inline int allocate_wshed_alist(Wshed *wshed, int asize, char *routine)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  if (object->asize < asize)
    { void *x = Guarded_Realloc(wshed->alist,(size_t) asize,routine);
      if (x == NULL) return (1);
      wshed->alist = x;
      object->asize = asize;
    }
  return (0);
}

static inline int sizeof_wshed_alist(Wshed *wshed)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  return (object->asize);
}

static inline int allocate_wshed_first(Wshed *wshed, int fsize, char *routine)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  if (object->fsize < fsize)
    { void *x = Guarded_Realloc(wshed->first,(size_t) fsize,routine);
      if (x == NULL) return (1);
      wshed->first = x;
      object->fsize = fsize;
    }
  return (0);
}

static inline int sizeof_wshed_first(Wshed *wshed)
{ _Wshed *object = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  return (object->fsize);
}

static inline void kill_wshed(Wshed *wshed);

static inline Wshed *new_wshed(int vsize, int esize, int asize, int fsize, char *routine)
{ _Wshed *object;
  Wshed  *wshed;

  pthread_mutex_lock(&Wshed_Mutex);
  if (Free_Wshed_List == NULL)
    { object = (_Wshed *) Guarded_Realloc(NULL,sizeof(_Wshed),routine);
      if (object == NULL) return (NULL);
      wshed = &(object->wshed);
      object->vsize = 0;
      wshed->verts = NULL;
      object->esize = 0;
      wshed->edges = NULL;
      object->asize = 0;
      wshed->alist = NULL;
      object->fsize = 0;
      wshed->first = NULL;
    }
  else
    { object = Free_Wshed_List;
      Free_Wshed_List = object->next;
      wshed = &(object->wshed);
    }
  Wshed_Inuse += 1;
  object->refcnt = 1;
  if (Use_Wshed_List != NULL)
    Use_Wshed_List->prev = object;
  object->next = Use_Wshed_List;
  object->prev = NULL;
  Use_Wshed_List = object;
  pthread_mutex_unlock(&Wshed_Mutex);
  wshed->labels = NULL;
  wshed->apart_ref = NULL;
  if (allocate_wshed_verts(wshed,vsize,routine))
    { kill_wshed(wshed);
      return (NULL);
    }
  if (allocate_wshed_edges(wshed,esize,routine))
    { kill_wshed(wshed);
      return (NULL);
    }
  if (allocate_wshed_alist(wshed,asize,routine))
    { kill_wshed(wshed);
      return (NULL);
    }
  if (allocate_wshed_first(wshed,fsize,routine))
    { kill_wshed(wshed);
      return (NULL);
    }
  return (wshed);
}

static inline Wshed *copy_wshed(Wshed *wshed)
{ Wshed *copy = new_wshed(wshed_vsize(wshed),wshed_esize(wshed),wshed_asize(wshed),wshed_fsize(wshed),"Copy_Partition");
  void *_verts = copy->verts;
  void *_edges = copy->edges;
  void *_alist = copy->alist;
  void *_first = copy->first;
  *copy = *wshed;
  copy->verts = _verts;
  if (wshed->verts != NULL)
    memcpy(copy->verts,wshed->verts,(size_t) wshed_vsize(wshed));
  copy->edges = _edges;
  if (wshed->edges != NULL)
    memcpy(copy->edges,wshed->edges,(size_t) wshed_esize(wshed));
  copy->alist = _alist;
  if (wshed->alist != NULL)
    memcpy(copy->alist,wshed->alist,(size_t) wshed_asize(wshed));
  copy->first = _first;
  if (wshed->first != NULL)
    memcpy(copy->first,wshed->first,(size_t) wshed_fsize(wshed));
  if (wshed->labels != NULL)
    Inc_Array(wshed->labels);
  if (wshed->apart_ref != NULL)
    Inc_AForm(wshed->apart_ref);
  return (copy);
}

Partition *Copy_Partition(Partition *partition)
{ return ((Partition *) copy_wshed(((Wshed *) partition))); }

static inline int pack_wshed(Wshed *wshed)
{ _Wshed *object  = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  if (object->vsize > wshed_vsize(wshed))
    { int ns = wshed_vsize(wshed);
      if (ns != 0)
        { void *x = Guarded_Realloc(wshed->verts,(size_t) ns,"Pack_Wshed");
          if (x == NULL) return (1);
          wshed->verts = x;
        }
      else
        { free(wshed->verts);
          wshed->verts = NULL;
        }
      object->vsize = ns;
    }
  if (object->esize > wshed_esize(wshed))
    { int ns = wshed_esize(wshed);
      if (ns != 0)
        { void *x = Guarded_Realloc(wshed->edges,(size_t) ns,"Pack_Wshed");
          if (x == NULL) return (1);
          wshed->edges = x;
        }
      else
        { free(wshed->edges);
          wshed->edges = NULL;
        }
      object->esize = ns;
    }
  if (object->asize > wshed_asize(wshed))
    { int ns = wshed_asize(wshed);
      if (ns != 0)
        { void *x = Guarded_Realloc(wshed->alist,(size_t) ns,"Pack_Wshed");
          if (x == NULL) return (1);
          wshed->alist = x;
        }
      else
        { free(wshed->alist);
          wshed->alist = NULL;
        }
      object->asize = ns;
    }
  if (object->fsize > wshed_fsize(wshed))
    { int ns = wshed_fsize(wshed);
      if (ns != 0)
        { void *x = Guarded_Realloc(wshed->first,(size_t) ns,"Pack_Wshed");
          if (x == NULL) return (1);
          wshed->first = x;
        }
      else
        { free(wshed->first);
          wshed->first = NULL;
        }
      object->fsize = ns;
    }
  return (0);
}

Partition *Pack_Partition(Partition *partition)
{ if (pack_wshed(((Wshed *) partition))) return (NULL);
  return (partition);
}

Partition *Inc_Partition(Partition *partition)
{ _Wshed *object  = (_Wshed *) (((char *) partition) - Wshed_Offset);
  pthread_mutex_lock(&Wshed_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Wshed_Mutex);
  return (partition);
}

static inline void free_wshed(Wshed *wshed)
{ _Wshed *object  = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  pthread_mutex_lock(&Wshed_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Wshed_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Partition\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Wshed_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Wshed_List;
  Free_Wshed_List = object;
  Wshed_Inuse -= 1;
  pthread_mutex_unlock(&Wshed_Mutex);
  if (wshed->apart_ref != NULL)
    { Free_AForm(wshed->apart_ref);
      wshed->apart_ref = NULL;
    }
  if (wshed->labels != NULL)
    { Free_Array(wshed->labels);
      wshed->labels = NULL;
    }
}

void Free_Partition(Partition *partition)
{ free_wshed(((Wshed *) partition)); }

static inline void kill_wshed(Wshed *wshed)
{ _Wshed *object  = (_Wshed *) (((char *) wshed) - Wshed_Offset);
  pthread_mutex_lock(&Wshed_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Wshed_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Partition\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Wshed_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Wshed_Inuse -= 1;
  pthread_mutex_unlock(&Wshed_Mutex);
  if (wshed->apart_ref != NULL)
    Kill_AForm(wshed->apart_ref);
  if (wshed->labels != NULL)
    Kill_Array(wshed->labels);
  if (wshed->first != NULL)
    free(wshed->first);
  if (wshed->alist != NULL)
    free(wshed->alist);
  if (wshed->edges != NULL)
    free(wshed->edges);
  if (wshed->verts != NULL)
    free(wshed->verts);
  free(((char *) wshed) - Wshed_Offset);
}

void Kill_Partition(Partition *partition)
{ kill_wshed(((Wshed *) partition)); }

static inline void reset_wshed()
{ _Wshed *object;
  Wshed  *wshed;
  pthread_mutex_lock(&Wshed_Mutex);
  while (Free_Wshed_List != NULL)
    { object = Free_Wshed_List;
      Free_Wshed_List = object->next;
      wshed = &(object->wshed);
      if (wshed->first != NULL)
        free(wshed->first);
      if (wshed->alist != NULL)
        free(wshed->alist);
      if (wshed->edges != NULL)
        free(wshed->edges);
      if (wshed->verts != NULL)
        free(wshed->verts);
      free(object);
    }
  pthread_mutex_unlock(&Wshed_Mutex);
}

void Reset_Partition()
{ reset_wshed(); }

int Partition_Usage()
{ return (Wshed_Inuse); }

void Partition_List(void (*handler)(Partition *))
{ _Wshed *a, *b;
  for (a = Use_Wshed_List; a != NULL; a = b)
    { b = a->next;
      handler((Partition *) &(a->wshed));
    }
}

static inline Wshed *read_wshed(FILE *input)
{ char name[9];
  Wshed *obj;
  Wshed read;
  fread(name,9,1,input);
  if (strncmp(name,"Partition",9) != 0)
    return (NULL);
  obj = new_wshed(0,0,0,0,"Read_Partition");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Wshed),1,input) == 0) goto error;
  obj->verts = read.verts;
  if (wshed_vsize(obj) != 0)
    { if (allocate_wshed_verts(obj,wshed_vsize(obj),"Read_Partition")) goto error;
      if (fread(obj->verts,(size_t) wshed_vsize(obj),1,input) == 0) goto error;
    }
  obj->edges = read.edges;
  if (wshed_esize(obj) != 0)
    { if (allocate_wshed_edges(obj,wshed_esize(obj),"Read_Partition")) goto error;
      if (fread(obj->edges,(size_t) wshed_esize(obj),1,input) == 0) goto error;
    }
  obj->alist = read.alist;
  if (wshed_asize(obj) != 0)
    { if (allocate_wshed_alist(obj,wshed_asize(obj),"Read_Partition")) goto error;
      if (fread(obj->alist,(size_t) wshed_asize(obj),1,input) == 0) goto error;
    }
  obj->first = read.first;
  if (wshed_fsize(obj) != 0)
    { if (allocate_wshed_first(obj,wshed_fsize(obj),"Read_Partition")) goto error;
      if (fread(obj->first,(size_t) wshed_fsize(obj),1,input) == 0) goto error;
    }
  obj->labels = NULL;
  obj->apart_ref = NULL;
  return (obj);

error:
  kill_wshed(obj);
  return (NULL);
}

Partition *Read_Partition(FILE *input)
{ return ((Partition *) read_wshed(input)); }

static inline void write_wshed(Wshed *wshed, FILE *output)
{ fwrite("Partition",9,1,output);
  fwrite(wshed,sizeof(Wshed),1,output);
  if (wshed_vsize(wshed) != 0)
    fwrite(wshed->verts,(size_t) wshed_vsize(wshed),1,output);
  if (wshed_esize(wshed) != 0)
    fwrite(wshed->edges,(size_t) wshed_esize(wshed),1,output);
  if (wshed_asize(wshed) != 0)
    fwrite(wshed->alist,(size_t) wshed_asize(wshed),1,output);
  if (wshed_fsize(wshed) != 0)
    fwrite(wshed->first,(size_t) wshed_fsize(wshed),1,output);
}

void Write_Partition(Partition *partition, FILE *output)
{ write_wshed(((Wshed *) partition),output); }

/**************************************************************************************\
*                                                                                      *
*  ACCESS ROUTINES                                                                     *
*                                                                                      *
\**************************************************************************************/

#define WS(w) ((Wshed *) (w))

APart *Get_Partition_APart(Partition *w)
{ return (WS(w)->apart_ref); }

void  Set_Partition_APart(Partition *w, Pixel_APart *I(part))
{ if (WS(w)->apart_ref == part)
    return;
  if (WS(w)->apart_ref != NULL)
    Free_AForm(WS(w)->apart_ref);
  if (WS(w)->labels != NULL &&  ! Same_Shape(part,WS(w)->labels))
    { fprintf(stderr,"Array and label field do not have same shape (Set_Partition_APart)\n");
      exit (1);
    }
  WS(w)->apart_ref = Inc_AForm(part);
}

boolean Is_Partition_2n_Connected(Partition *w)
{ return (WS(w)->iscon2n); }

boolean Is_Partition_Colored(Partition *w)
{ return (WS(w)->colored); }

int Get_Partition_Vertex_Count(Partition *w)
{ return (WS(w)->nbasins); }

int Get_Partition_Edge_Count(Partition *w)
{ return (WS(w)->first[WS(w)->nbasins]/2); }

int Get_Partition_Color_Count(Partition *w)
{ return (WS(w)->nlabels); }

Label_Array *Get_Partition_Labels(Partition *w)
{ return (WS(w)->labels); }

void Set_Partition_Labels(Partition *w, Label_Array *C(labels))
{ if (WS(w)->labels == labels)
    return;
  if (WS(w)->labels != NULL)
    Free_Array(WS(w)->labels);
  if (labels == NULL)
    { WS(w)->labels = NULL;
      return;
    }
  if (WS(w)->apart_ref != NULL &&  ! Same_Shape(labels,WS(w)->apart_ref))
    { fprintf(stderr,"Array and label field do not have same shape (Set_Partition_Labels)\n");
      exit (1);
    }
  WS(w)->labels = labels;
}

P_Vertex *Get_Partition_Vertex(Partition *w, int c)
{ Wshed *v = WS(w);
  if (c < 0 || c >= v->nbasins)
    { fprintf(stderr,"Get_Partition_Vertex: index is not in range\n");
      exit (1);
    }
  return ((P_Vertex *) (v->verts + c));
}

P_Edge *Get_Partition_Edge(Partition *w, int d)
{ Wshed *v = WS(w);
  if (d < 0 || d >= (v->first[v->nbasins] >> 1))
    { fprintf(stderr,"Get_Partition_Edge: index is not in range\n");
      exit (1);
    }
  return ((P_Edge *) (v->edges + d));
}

int *Get_Partition_Neighbors(Partition *w, int c, int *O(nedges))
{ Wshed *v = WS(w);
  if (c < 0 || c >= v->nbasins)
    { fprintf(stderr,"Get_Partition_Neighbors: index is not in range\n");
      exit (1);
    }
  *nedges = v->first[c+1] - v->first[c];
  return ((int *) (v->alist + v->first[c]));
}


/**************************************************************************************\
*                                                                                      *
*  BUILD A WATERSHED DECOMPOSITION                                                     *
*                                                                                      *
*     1. LABEL ALL THE WATERSHEDS                                                      *
*                                                                                      *
\**************************************************************************************/

/* Allocator for workstorage for BFS traversals */

#define PUSH(p)										\
{ queue[qtop++] = (p);									\
  if (qtop >= qmax)									\
    qtop = 0;										\
  if (qtop == qbot)									\
    { int qhav = (qmax+1) / 2;								\
      int qnew = qmax + qhav;								\
      if (queue == Queue)								\
        { queue = (int *) Guarded_Malloc(sizeof(int)*((size_t) qnew),"Water_Shed");	\
          memcpy(queue,Queue,sizeof(int)*((size_t) qmax));				\
	}										\
      else										\
        queue = (int *) Guarded_Realloc(queue,sizeof(int)*((size_t) qnew),"Water_Shed");\
      if (qbot >= qhav)									\
        { qbot = qmax-qbot;								\
          qmax = qnew-qbot;								\
          memcpy(queue + qmax,queue + qtop, sizeof(int)*((size_t) qbot));		\
          qbot = qnew - qbot;								\
        }										\
      else										\
        { memcpy(queue + qmax, queue, sizeof(int)*((size_t) qbot));			\
          qtop = qmax + qbot;								\
        }										\
      qmax = qnew;									\
    }											\
}

#define POP(p)		\
{ p = queue[qbot++];	\
  if (qbot >= qmax)	\
    qbot = 0;		\
}

#define EPUSH(p)									\
{ estack[etop++] = (p);									\
  if (etop >= emax)									\
    { int ehav = (emax+1)/2;								\
      int enew = emax + ehav;								\
      if (estack == Estack)								\
        { estack = (int *) Guarded_Malloc(sizeof(int)*((size_t) enew),"Watershed");	\
          memcpy(estack,Estack,sizeof(int)*((size_t) emax));				\
        }										\
      else										\
        estack = (int *) Guarded_Realloc(estack,sizeof(int)*((size_t) enew),		\
                                         "Watershed");					\
      emax  = enew;									\
    }											\
}

#define EPOP(p)  { p = estack[--etop]; }


#define WSHED    0
#define ONQUEUE -1
#define MASK    -2
#define INIT    -3
#define LINK    -4

#define QLABELMAX 4096
#define ELABELMAX 4096

Label_Array *Label_Watershed(Pixel_APart *frame, Integer_Array *R(M(labels)),
                             int *O(nbasin), boolean iscon2n)
{ Array         *array = AForm_Array(frame);
  int           *out;
  int            size;
  int            maxval;

  int            map;
  Coordinate    *basis = NULL;
  Array_Bundle   sframe;

  int nbasins;           // Number of watersheds
  int bucket[0x10001];

  uint8  *value_uint8;
  uint16 *value_uint16;

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  boolean   *(*BOUNDS)(Grid_Id,Indx_Type,int);

  int *queue, qmax, Queue[QLABELMAX];
  int *estack, emax, Estack[ELABELMAX];

  if (array->type != UINT8_TYPE && array->type != UINT16_TYPE)
    { fprintf(stderr,"Image array is not a UINT8 or UINT16 arrays (Label_Watershed)\n");
      exit (1);
    }
  if (array->kind != PLAIN_KIND)
    { fprintf(stderr,"Image array is not a PLAIN array (Label_Watershed)\n");
      exit (1);
    }
  if (array->size > 0x7FFFFFFC)   // ..FC vs ..FF because of link trick (+/- encoding)
    { fprintf(stderr,"Image array is larger than 2G pixel (Label_Watershed)\n");
      exit (1);
    }
  if (labels->type != INT32_TYPE || labels->kind != PLAIN_KIND)
    { fprintf(stderr,"Label array is not an PLAIN INT32 array (Label_Watershed)\n");
      exit (1);
    }

  if (Same_Shape(array,labels))
    map = Is_Slice(frame);
  else
    { map = 2;

      basis = AForm_Shape(frame);
      sframe.dims  = ADIMN(basis);
      sframe.ndims = array->ndims;
      sframe.kind  = PLAIN_KIND;

      if ( ! Same_Shape(frame,labels))
        { fprintf(stderr,"Label array shape is not consistent with image array or slice");
          fprintf(stderr," (Label_Watershed)\n");
          exit (1);
        }
    }

  out      = AINT32(labels);
  grid     = Setup_Grid(frame,"Label_Watershed");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  qmax   = QLABELMAX;
  queue  = Queue;
  emax   = ELABELMAX;
  estack = Estack;

  if (array->type == UINT16_TYPE)
    maxval = 0x10000;
  else
    maxval = 0x100;
  value_uint16 = AUINT16(array);
  value_uint8  = AUINT8(array);

  nbasins = 0;
  switch (map) {
  case 2:
    { Indx_Type    curp;
      Grid_Id      gridi;
      Offs_Type   *neighbori;

      Dimn_Type   *offs = ADIMN(Slice_First(frame));
      Coordinate  *work = Copy_Array(basis);
      Dimn_Type   *tran = ADIMN(work);
      Dimn_Type   *adim = array->dims;
      Dimn_Type   *sdim = sframe.dims;
      int          dmax = array->ndims-1;

      gridi = Setup_Grid(&sframe,"Label_Watershed");
      neighbori = Grid_Neighbors(gridi,iscon2n);

      size  = (int) AForm_Size(frame);
      curp = Slice_Index(frame);

  { int qbot, qtop, p;
    int pi;						

    for (pi = 0; pi < size; pi++)
      { out[pi] = WSHED;
      }

    qtop = qbot = 0;
    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  PUSH(pi);				
                  out[pi] = INIT;
                  hgt    = value_uint8[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      int      qi, si;			
                      boolean *b;

                      POP(q);
                      POP(qi);				
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[qi],value_uint8[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            si = qi + (int) neighbori[j];	
                            v = value_uint8[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[si] == WSHED)
                              { PUSH(s);
                                PUSH(si);			
                                out[si] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      PUSH(pi);				
                      out[pi] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          int      qi, si;		
                          boolean *b;
      
                          POP(q);
                          POP(qi);				
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                si = qi + (int) neighbori[j];	
                                v = value_uint8[s];
                                if (v == hgt && out[si] == INIT)
                                  { PUSH(s);
                                    PUSH(si);			
                                    out[si] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[pi] = MASK;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  PUSH(pi);				
                  out[pi] = INIT;
                  hgt    = value_uint16[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      int      qi, si;			
                      boolean *b;

                      POP(q);
                      POP(qi);				
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[qi],value_uint16[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            si = qi + (int) neighbori[j];	
                            v = value_uint16[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[si] == WSHED)
                              { PUSH(s);
                                PUSH(si);			
                                out[si] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      PUSH(pi);				
                      out[pi] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          int      qi, si;		
                          boolean *b;
      
                          POP(q);
                          POP(qi);				
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                si = qi + (int) neighbori[j];	
                                v = value_uint16[s];
                                if (v == hgt && out[si] == INIT)
                                  { PUSH(s);
                                    PUSH(si);			
                                    out[si] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[pi] = MASK;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
          default:
            break;
      }
  }

  // Build pixel value list with only MASK pixels in each list, link to p is encoded as,
  //   LINK-p a negative number that does not conflict with any of the tag labels

  { int p, v;
    int pi;						

    for (v = 0; v <= maxval; v++)
      bucket[v] = INIT;

    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == MASK)
                { v         = value_uint8[p];
                  out[pi]  = LINK-bucket[v];
                  bucket[v] = pi;
                }
              p = (int) Next_Slice_Index(frame);	
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == MASK)
                { v         = value_uint16[p];
                  out[pi]  = LINK-bucket[v];
                  bucket[v] = pi;
                }
              p = (int) Next_Slice_Index(frame);	
            }
          break;
        default:
          break;
    }
  }

  { int p, v, t;
    int pi;						
    int qbot, qtop;
    int etop;

    qtop = qbot = 0;
    etop = 0;
    for (v = 0; v < maxval; v++)
      {
#ifdef DEBUG
        printf("Level %d\n",v);
#endif
        // Push all INIT v-pixels that are adjacent to a previously labeled pixel and
        //   label ONQUEUE, label all others MASK

        for (pi = bucket[v]; pi != INIT; pi = t)
          { t = LINK-out[pi];

            { Dimn_Type m;      //  Translate pi from slice coords to array coords, ugh!
              int       k;

              p = pi;
              for (k = 0; k < dmax; k++)
                { m = sdim[k];
                  tran[k] = (Dimn_Type) (p % m);
                  p /= m;
                }
              p += offs[dmax];
              for (k = dmax; k-- > 0; )
                p = adim[k]*p + (tran[k] + offs[k]);
            }

            PUSH(p);
            PUSH(pi);						
            out[pi] = MASK;
#ifdef DEBUG
            printf("  Labeling group\n");
#endif
            while (qtop != qbot)
              { boolean *b;
                int      j, starter;

                POP(p);
                POP(pi);					
                b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                starter = 0;
#ifdef DEBUG
                printf("    Group (");
                Print_Coord(stdout,Idx2CoordA(array,p));
                printf(") %d\n",out[pi]);
#endif

                switch (array->type) {
                    case UINT8_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;
                            int qi;				

                            q = p + (int) neighbor[j];
                            qi = pi + (int) neighbori[j];	
                            w = value_uint8[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[qi] == INIT)
                              { PUSH(q);
                                PUSH(qi);			
                                out[qi] = MASK;
                              }
                          }
                      break;
                    case UINT16_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;
                            int qi;				

                            q = p + (int) neighbor[j];
                            qi = pi + (int) neighbori[j];	
                            w = value_uint16[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[qi] == INIT)
                              { PUSH(q);
                                PUSH(qi);			
                                out[qi] = MASK;
                              }
                          }
                      break;
                    default:
                      break;
                  }

                if (starter)
                  EPUSH(pi);
              }

            while (etop > 0)
              { EPOP(p);
                PUSH(p);
                out[p] = ONQUEUE;
#ifdef DEBUG
                printf("    Starter (");
                Print_Coord(stdout,Idx2CoordA(&sframe,p));	
                printf(") %d\n",out[p]);
#endif
              }

            //  In BFS, reach all v-pixels (have MASK set) in order of distance, and label
            //    with catchment basin or WSHED

            while (qtop != qbot)
              { int      j, s;
                boolean *b;

                POP(p);
                b = BOUNDS(gridi,(Indx_Type) p,iscon2n);	

                s = ONQUEUE;
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { int q, o;

                      q = p + (int) neighbori[j];		
                      o = out[q];
                      if (o == MASK)
                        { out[q] = ONQUEUE;
                          PUSH(q);
                        }
                      else if (o > 0)
                        { if (s == ONQUEUE)
                            s = o;
                          else if (s != o)
                            s = WSHED;
                        }
                    }
                if (s == ONQUEUE)
                  s = WSHED;
                out[p] = s;
#ifdef DEBUG
                printf("    Proc (");
                Print_Coord(stdout,Idx2CoordA(&sframe,p));	
                printf(") %d\n",out[p]);
#endif
              }
          }
      }
  }

  { int p, qbot, qtop;
    int pi;							

    //  Almost all WSHED pixels are adjacent to a non-WSHED pixel and en-mass form one huge
    //    connected component.  So to keep the queue small, assign all of WSHED pixels that
    //    are one hop away from a non-WSHED pixel in a special sweep.

    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);			
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
                  int      qi;					
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        qi = pi + (int) neighbori[j];		
                        o = out[qi];
                        m = value_uint8[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[pi] = -label;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);			
          for (pi = 0; pi < size; pi++)
            { if (out[pi] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
                  int      qi;					
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        qi = pi + (int) neighbori[j];		
                        o = out[qi];
                        m = value_uint16[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[pi] = -label;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        default:
          break;
    }

    for (pi = 0; pi < size; pi++)
      { if (out[pi] < 0)
          out[pi] = -out[pi];
      }

    //  Find all remaining WSHED pixels in order of distance from a non-WSHED pixel.  Assign
    //    to the CB that has the minimal pixel adjacent to it.

    qtop = qbot = 0;
    p = (int) Set_Slice_To_First(frame);			
    for (pi = 0; pi < size; pi++)
      { if (out[pi] == WSHED)
          { boolean *b;
            int      j;

            b = BOUNDS(grid,(Indx_Type) p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j] && out[pi+(int) neighbori[j]] > 0)	
                { PUSH(p);
                  PUSH(pi);					
                  out[pi] = ONQUEUE;
                  break;
                }
          }
        p = (int) Next_Slice_Index(frame);			
      }

    switch (array->type) {
        case UINT8_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;
              int      qi;				

              POP(p);
              POP(pi);					
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    qi = pi + (int) neighbori[j];	
                    o = out[qi];
                    m = value_uint8[q];
                    if (o == WSHED)
                      { PUSH(q);
                        PUSH(qi);			
                        out[qi] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[pi] = ONQUEUE - label;
            }
          break;
        case UINT16_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;
              int      qi;				

              POP(p);
              POP(pi);					
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    qi = pi + (int) neighbori[j];	
                    o = out[qi];
                    m = value_uint16[q];
                    if (o == WSHED)
                      { PUSH(q);
                        PUSH(qi);			
                        out[qi] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[pi] = ONQUEUE - label;
            }
          break;
        default:
          break;
    }

    for (pi = 0; pi < size; pi++)
      { if (out[pi] < 0)
          out[pi] = ONQUEUE - out[pi];
      }
  }

      Free_Array(basis);
      Free_Array(work);
      Release_Grid(gridi);
      Set_Slice_To_Index(frame,curp);			
      break;
    }
  case 1:
    { Indx_Type curp;

      size = (int) AForm_Size(frame);
      curp = Slice_Index(frame);

  { int qbot, qtop, p;
    int pi;						

    p = (int) Set_Slice_To_First(frame);		
    for (pi = 0; pi < size; pi++)
      { out[p] = WSHED;
        p = (int) Next_Slice_Index(frame);		
      }

    qtop = qbot = 0;
    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[p] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  out[p] = INIT;
                  hgt    = value_uint8[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      boolean *b;

                      POP(q);
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[q],value_uint8[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            v = value_uint8[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[s] == WSHED)
                              { PUSH(s);
                                out[s] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      out[p] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          boolean *b;
      
                          POP(q);
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                v = value_uint8[s];
                                if (v == hgt && out[s] == INIT)
                                  { PUSH(s);
                                    out[s] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[p] = MASK;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[p] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  out[p] = INIT;
                  hgt    = value_uint16[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      boolean *b;

                      POP(q);
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[q],value_uint16[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            v = value_uint16[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[s] == WSHED)
                              { PUSH(s);
                                out[s] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      out[p] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          boolean *b;
      
                          POP(q);
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                v = value_uint16[s];
                                if (v == hgt && out[s] == INIT)
                                  { PUSH(s);
                                    out[s] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[p] = MASK;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
          default:
            break;
      }
  }

  // Build pixel value list with only MASK pixels in each list, link to p is encoded as,
  //   LINK-p a negative number that does not conflict with any of the tag labels

  { int p, v;
    int pi;						

    for (v = 0; v <= maxval; v++)
      bucket[v] = INIT;

    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[p] == MASK)
                { v         = value_uint8[p];
                  out[p]  = LINK-bucket[v];
                  bucket[v] = p;
                }
              p = (int) Next_Slice_Index(frame);	
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);		
          for (pi = 0; pi < size; pi++)
            { if (out[p] == MASK)
                { v         = value_uint16[p];
                  out[p]  = LINK-bucket[v];
                  bucket[v] = p;
                }
              p = (int) Next_Slice_Index(frame);	
            }
          break;
        default:
          break;
    }
  }

  { int p, v, t;
    int qbot, qtop;
    int etop;

    qtop = qbot = 0;
    etop = 0;
    for (v = 0; v < maxval; v++)
      {
#ifdef DEBUG
        printf("Level %d\n",v);
#endif
        // Push all INIT v-pixels that are adjacent to a previously labeled pixel and
        //   label ONQUEUE, label all others MASK

        for (p = bucket[v]; p != INIT; p = t)
          { t = LINK-out[p];


            PUSH(p);
            out[p] = MASK;
#ifdef DEBUG
            printf("  Labeling group\n");
#endif
            while (qtop != qbot)
              { boolean *b;
                int      j, starter;

                POP(p);
                b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                starter = 0;
#ifdef DEBUG
                printf("    Group (");
                Print_Coord(stdout,Idx2CoordA(array,p));
                printf(") %d\n",out[p]);
#endif

                switch (array->type) {
                    case UINT8_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;

                            q = p + (int) neighbor[j];
                            w = value_uint8[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[q] == INIT)
                              { PUSH(q);
                                out[q] = MASK;
                              }
                          }
                      break;
                    case UINT16_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;

                            q = p + (int) neighbor[j];
                            w = value_uint16[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[q] == INIT)
                              { PUSH(q);
                                out[q] = MASK;
                              }
                          }
                      break;
                    default:
                      break;
                  }

                if (starter)
                  EPUSH(p);
              }

            while (etop > 0)
              { EPOP(p);
                PUSH(p);
                out[p] = ONQUEUE;
#ifdef DEBUG
                printf("    Starter (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf(") %d\n",out[p]);
#endif
              }

            //  In BFS, reach all v-pixels (have MASK set) in order of distance, and label
            //    with catchment basin or WSHED

            while (qtop != qbot)
              { int      j, s;
                boolean *b;

                POP(p);
                b = BOUNDS(grid,(Indx_Type) p,iscon2n);		

                s = ONQUEUE;
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { int q, o;

                      q = p + (int) neighbor[j];		
                      o = out[q];
                      if (o == MASK)
                        { out[q] = ONQUEUE;
                          PUSH(q);
                        }
                      else if (o > 0)
                        { if (s == ONQUEUE)
                            s = o;
                          else if (s != o)
                            s = WSHED;
                        }
                    }
                if (s == ONQUEUE)
                  s = WSHED;
                out[p] = s;
#ifdef DEBUG
                printf("    Proc (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf(") %d\n",out[p]);
#endif
              }
          }
      }
  }

  { int p, qbot, qtop;
    int pi;							

    //  Almost all WSHED pixels are adjacent to a non-WSHED pixel and en-mass form one huge
    //    connected component.  So to keep the queue small, assign all of WSHED pixels that
    //    are one hop away from a non-WSHED pixel in a special sweep.

    switch (array->type) {
        case UINT8_TYPE:
          p = (int) Set_Slice_To_First(frame);			
          for (pi = 0; pi < size; pi++)
            { if (out[p] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        o = out[q];
                        m = value_uint8[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[p] = -label;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        case UINT16_TYPE:
          p = (int) Set_Slice_To_First(frame);			
          for (pi = 0; pi < size; pi++)
            { if (out[p] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        o = out[q];
                        m = value_uint16[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[p] = -label;
                }
              p = (int) Next_Slice_Index(frame);		
            }
          break;
        default:
          break;
    }

    p = (int) Set_Slice_To_First(frame);			
    for (pi = 0; pi < size; pi++)
      { if (out[p] < 0)
          out[p] = -out[p];
        p = (int) Next_Slice_Index(frame);			
      }

    //  Find all remaining WSHED pixels in order of distance from a non-WSHED pixel.  Assign
    //    to the CB that has the minimal pixel adjacent to it.

    qtop = qbot = 0;
    p = (int) Set_Slice_To_First(frame);			
    for (pi = 0; pi < size; pi++)
      { if (out[p] == WSHED)
          { boolean *b;
            int      j;

            b = BOUNDS(grid,(Indx_Type) p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j] && out[p+(int) neighbor[j]] > 0)   	
                { PUSH(p);
                  out[p] = ONQUEUE;
                  break;
                }
          }
        p = (int) Next_Slice_Index(frame);			
      }

    switch (array->type) {
        case UINT8_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;

              POP(p);
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    o = out[q];
                    m = value_uint8[q];
                    if (o == WSHED)
                      { PUSH(q);
                        out[q] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[p] = ONQUEUE - label;
            }
          break;
        case UINT16_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;

              POP(p);
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    o = out[q];
                    m = value_uint16[q];
                    if (o == WSHED)
                      { PUSH(q);
                        out[q] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[p] = ONQUEUE - label;
            }
          break;
        default:
          break;
    }

    p = (int) Set_Slice_To_First(frame);			
    for (pi = 0; pi < size; pi++)
      { if (out[p] < 0)
          out[p] = ONQUEUE - out[p];
        p = (int) Next_Slice_Index(frame);			
      }
  }

      Set_Slice_To_Index(frame,curp);			
      break;
    }
  case 0:
    { size = (int) array->size;

  { int qbot, qtop, p;

    for (p = 0; p < size; p++)
      { out[p] = WSHED;
      }

    qtop = qbot = 0;
    switch (array->type) {
        case UINT8_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  out[p] = INIT;
                  hgt    = value_uint8[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      boolean *b;

                      POP(q);
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[q],value_uint8[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            v = value_uint8[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[s] == WSHED)
                              { PUSH(s);
                                out[s] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      out[p] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          boolean *b;
      
                          POP(q);
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                v = value_uint8[s];
                                if (v == hgt && out[s] == INIT)
                                  { PUSH(s);
                                    out[s] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[p] = MASK;
                }
            }
          break;
        case UINT16_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == WSHED)
                { int hgt, minim;

                  //  Fill height pleateau of unset pixel v, marking all as INIT
                  //    and checking if a minimum

                  PUSH(p);
                  out[p] = INIT;
                  hgt    = value_uint16[p];
                  minim  = 1;

#ifdef DEBUG
                  printf("  Form height plateau\n");
#endif
                  while (qtop != qbot)
                    { int      q, s, j, v;
                      boolean *b;

                      POP(q);
#ifdef DEBUG
                      printf("    (");
                      Print_Coord(stdout,Idx2CoordA(array,q));
                      printf(") %d(%d)\n",out[q],value_uint16[q]);
#endif
                      b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { s = q + (int) neighbor[j];
                            v = value_uint16[s];
                            if (v < hgt)
                              minim = 0;
                            else if (v == hgt && out[s] == WSHED)
                              { PUSH(s);
                                out[s] = INIT;
                              }
                          }
                    }

                  // If a minimum, then fill again with new unique basin number starting at 1

#ifdef DEBUG
                  printf("    Minim = %d\n",minim);
#endif
                  if (minim)
                    { nbasins += 1;
                      PUSH(p);
                      out[p] = nbasins;
                      while (qtop != qbot)
                        { int      q, s, j, v;
                          boolean *b;
      
                          POP(q);
                          b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                          for (j = 0; j < n_nbrs; j++)
                            if (b[j])
                              { s = q + (int) neighbor[j];
                                v = value_uint16[s];
                                if (v == hgt && out[s] == INIT)
                                  { PUSH(s);
                                    out[s] = nbasins;
                                  }
                              }
                        }
                    }
                  else
                    out[p] = MASK;
                }
            }
          break;
          default:
            break;
      }
  }

  // Build pixel value list with only MASK pixels in each list, link to p is encoded as,
  //   LINK-p a negative number that does not conflict with any of the tag labels

  { int p, v;

    for (v = 0; v <= maxval; v++)
      bucket[v] = INIT;

    switch (array->type) {
        case UINT8_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == MASK)
                { v         = value_uint8[p];
                  out[p]  = LINK-bucket[v];
                  bucket[v] = p;
                }
            }
          break;
        case UINT16_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == MASK)
                { v         = value_uint16[p];
                  out[p]  = LINK-bucket[v];
                  bucket[v] = p;
                }
            }
          break;
        default:
          break;
    }
  }

  { int p, v, t;
    int qbot, qtop;
    int etop;

    qtop = qbot = 0;
    etop = 0;
    for (v = 0; v < maxval; v++)
      {
#ifdef DEBUG
        printf("Level %d\n",v);
#endif
        // Push all INIT v-pixels that are adjacent to a previously labeled pixel and
        //   label ONQUEUE, label all others MASK

        for (p = bucket[v]; p != INIT; p = t)
          { t = LINK-out[p];


            PUSH(p);
            out[p] = MASK;
#ifdef DEBUG
            printf("  Labeling group\n");
#endif
            while (qtop != qbot)
              { boolean *b;
                int      j, starter;

                POP(p);
                b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                starter = 0;
#ifdef DEBUG
                printf("    Group (");
                Print_Coord(stdout,Idx2CoordA(array,p));
                printf(") %d\n",out[p]);
#endif

                switch (array->type) {
                    case UINT8_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;

                            q = p + (int) neighbor[j];
                            w = value_uint8[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[q] == INIT)
                              { PUSH(q);
                                out[q] = MASK;
                              }
                          }
                      break;
                    case UINT16_TYPE:
                      for (j = 0; j < n_nbrs; j++)
                        if (b[j])
                          { int q, w;

                            q = p + (int) neighbor[j];
                            w = value_uint16[q];
                            if (w < v)
                              starter = 1;
                            else if (w == v && out[q] == INIT)
                              { PUSH(q);
                                out[q] = MASK;
                              }
                          }
                      break;
                    default:
                      break;
                  }

                if (starter)
                  EPUSH(p);
              }

            while (etop > 0)
              { EPOP(p);
                PUSH(p);
                out[p] = ONQUEUE;
#ifdef DEBUG
                printf("    Starter (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf(") %d\n",out[p]);
#endif
              }

            //  In BFS, reach all v-pixels (have MASK set) in order of distance, and label
            //    with catchment basin or WSHED

            while (qtop != qbot)
              { int      j, s;
                boolean *b;

                POP(p);
                b = BOUNDS(grid,(Indx_Type) p,iscon2n);		

                s = ONQUEUE;
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { int q, o;

                      q = p + (int) neighbor[j];		
                      o = out[q];
                      if (o == MASK)
                        { out[q] = ONQUEUE;
                          PUSH(q);
                        }
                      else if (o > 0)
                        { if (s == ONQUEUE)
                            s = o;
                          else if (s != o)
                            s = WSHED;
                        }
                    }
                if (s == ONQUEUE)
                  s = WSHED;
                out[p] = s;
#ifdef DEBUG
                printf("    Proc (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf(") %d\n",out[p]);
#endif
              }
          }
      }
  }

  { int p, qbot, qtop;

    //  Almost all WSHED pixels are adjacent to a non-WSHED pixel and en-mass form one huge
    //    connected component.  So to keep the queue small, assign all of WSHED pixels that
    //    are one hop away from a non-WSHED pixel in a special sweep.

    switch (array->type) {
        case UINT8_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        o = out[q];
                        m = value_uint8[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[p] = -label;
                }
            }
          break;
        case UINT16_TYPE:
          for (p = 0; p < size; p++)
            { if (out[p] == WSHED)
                { boolean *b;
                  int      j, q, o, m;
                  int      minim, label;
      
                  b = BOUNDS(grid,(Indx_Type) p,iscon2n);
                  minim = maxval;
                  label = 0;
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        o = out[q];
                        m = value_uint16[q];
                        if (o > 0 && m < minim)
                          { minim = m;
                            label = o;
                          }
                      }
                  if (minim < maxval)
                    out[p] = -label;
                }
            }
          break;
        default:
          break;
    }

    for (p = 0; p < size; p++)
      { if (out[p] < 0)
          out[p] = -out[p];
      }

    //  Find all remaining WSHED pixels in order of distance from a non-WSHED pixel.  Assign
    //    to the CB that has the minimal pixel adjacent to it.

    qtop = qbot = 0;
    for (p = 0; p < size; p++)
      { if (out[p] == WSHED)
          { boolean *b;
            int      j;

            b = BOUNDS(grid,(Indx_Type) p,iscon2n);
            for (j = 0; j < n_nbrs; j++)
              if (b[j] && out[p+(int) neighbor[j]] > 0)   	
                { PUSH(p);
                  out[p] = ONQUEUE;
                  break;
                }
          }
      }

    switch (array->type) {
        case UINT8_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;

              POP(p);
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    o = out[q];
                    m = value_uint8[q];
                    if (o == WSHED)
                      { PUSH(q);
                        out[q] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[p] = ONQUEUE - label;
            }
          break;
        case UINT16_TYPE:
          while (qtop != qbot)
            { boolean *b;
              int      j, q, o, m;
              int      minim, label;

              POP(p);
              b = BOUNDS(grid,(Indx_Type) p,iscon2n);
#ifdef DEBUG
              printf("  Wshed Pixel: (");
              Print_Coord(stdout,Idx2CoordA(array,p));
              printf(")\n");
#endif

              minim = maxval;
              label = 0;
              for (j = 0; j < n_nbrs; j++)
                if (b[j])
                  { q = p + (int) neighbor[j];
                    o = out[q];
                    m = value_uint16[q];
                    if (o == WSHED)
                      { PUSH(q);
                        out[q] = ONQUEUE;
                      }
                    else if (o != ONQUEUE)
                      { if (o < 0)
                          o = ONQUEUE - o;
                        if (m < minim)
                          { label = o;
                            minim = m;
                          }
                      }
                  }
              out[p] = ONQUEUE - label;
            }
          break;
        default:
          break;
    }

    for (p = 0; p < size; p++)
      { if (out[p] < 0)
          out[p] = ONQUEUE - out[p];
      }
  }

      break;
    }
  }

  if (emax > ELABELMAX)
    free(estack);
  if (qmax > QLABELMAX)
    free(queue);
  Release_Grid(grid);

  *nbasin = nbasins;
  labels->type = UINT32_TYPE;
  return (labels);
}


/**************************************************************************************\
*                                                                                      *
*  BUILD A WATERSHED DECOMPOSITION                                                     *
*                                                                                      *
*     2. BUILD A WATERSHED GRAPH FROM THE LABELS                                       *
*                                                                                      *
\**************************************************************************************/

#define EGRAPHMAX 512
#define QGRAPHMAX 512

Partition *Make_Partition(APart *I(frame), Label_Array *C(M(labels)), int nbasins,
                          boolean iscon2n, boolean color)
{ Array        *array = AForm_Array(frame);
  Wshed        *result;

  int           nlabels;   // The number of labels (1..nlabels)
  P_Vertex     *verts;     // A list of records, one for each catchment basin
  P_Edge       *edges;     // A list of edges, encoded as described in the .h file
  int          *first;     // Adjacency list indices
  int          *alist;     // Edge list indices

  int     map;
  int     maxval;
  int     size;

  uint8  *value_uint8;
  uint16 *value_uint16;
  int32  *value_uint32;
  int8   *value_int8;
  int16  *value_int16;
  int32  *value_int32;

  int8   *out_int8;
  int16  *out_int16;
  int32  *out_int32;

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  boolean   *(*BOUNDS)(Grid_Id,Indx_Type,int);

  int *queue, qmax, Queue[QGRAPHMAX];
  int *estack, emax, Estack[EGRAPHMAX];

  if (array->type > INT32_TYPE || array->type == UINT64_TYPE)
    { fprintf(stderr,"Image array should be a PLAIN, UINT or INT 8, 16, or 32-bit array");
      fprintf(stderr," (Make_Partition)\n");
      exit (1);
    }
  if (labels->type > UINT32_TYPE || labels->kind != PLAIN_KIND)
    { fprintf(stderr,"Label array should be a PLAIN, UINT8, 16, or 32 array (Make_Partition)\n");
      exit (1);
    }
  if (nbasins > 0x7FFF)
    { if (labels->type < UINT32_TYPE)
        Convert_Array_Inplace(labels,PLAIN_KIND,UINT32_TYPE,32,0);
    }
  else if (nbasins > 0x7F)
    { if (labels->type < UINT16_TYPE)
        Convert_Array_Inplace(labels,PLAIN_KIND,UINT16_TYPE,16,0);
    }

  if (Same_Shape(array,labels))
    map = Is_Slice(frame);
  else
    { if ( ! Same_Shape(frame,labels))
        { fprintf(stderr,"Label array shape is not consistent with image array or slice");
          fprintf(stderr," (Make_Partition)\n");
          exit (1);
        }
      map = 2;
    }

  //  Setup access variables

  grid     = Setup_Grid(frame,"Make_Partition");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  qmax   = QGRAPHMAX;
  queue  = Queue;
  emax   = EGRAPHMAX;
  estack = Estack;

  if (array->type == UINT16_TYPE)
    maxval = 0x10000;
  else
    maxval = 0x100;

  value_uint32 = AINT32(array);
  value_uint16 = AUINT16(array);
  value_uint8  = AUINT8(array);
  value_int32  = AINT32(array);
  value_int16  = AINT16(array);
  value_int8   = AINT8(array);

  out_int32    = AINT32(labels);
  out_int16    = AINT16(labels);
  out_int8     = AINT8(labels);

  //  Make the data structure now

  result = new_wshed(SIZEOF(P_Vertex)*(nbasins+1),0,0,SIZEOF(int)*(nbasins+1),"Make_Partition");
  result->apart_ref = Inc_AForm(frame);
  result->nbasins   = nbasins;
  result->iscon2n   = iscon2n;
  result->colored   = color;
  result->labels    = labels;

  switch (map) {
  case 2:
    { Indx_Type    curp;
      Grid_Id      gridi;
      Offs_Type   *neighbori;
      Coordinate  *basis;
      Array_Bundle sframe;

      basis = AForm_Shape(frame);
      sframe.dims  = ADIMN(basis);
      sframe.ndims = array->ndims;
      sframe.kind  = PLAIN_KIND;

      gridi = Setup_Grid(&sframe,"Make_Partition");
      neighbori = Grid_Neighbors(gridi,iscon2n);

      size = (int) AForm_Size(frame);
      curp = Slice_Index(frame);

  //  Greedily assign labels to catchment basins (CBs) so that no basin is adjacent to
  //    another with the same number.  Typically the number of labels required is very,
  //    very small, 6 or 7 in my experience.  At the end of this segment, out[p] is
  //    the negative of its label reassignment, verts[b].depth is the new label for CB b,
  //    and nlabels is the number of unique labels used.
  //  Also, determine the minimum pixel of each CB b in verts[b].seed and the number of
  //    CB's adjacent to this CB in first[b]

  { int   qbot, qtop;
    int   etop;
    int   olabel, count;
    int   adjacent[0x10000];
    int   p;

    verts   = result->verts;
    first   = result->first;
    nlabels = 0;

    for (p = 0; p < 0x10000; p++)
      adjacent[p] = 0;

    for (p = 0; p <= nbasins; p++)
      { first[p] = 0;
        verts[p].seed = -1;
      }

    qbot = qtop = 0;
    etop = 0;

    switch (labels->type) {
      case UINT8_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int8[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int8[p];
                out_int8[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(&sframe,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int8[q] = -olabel;
                    count += 1;

                    b = BOUNDS(gridi,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbori[j];		
                          o = out_int8[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int8[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int8[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      case UINT16_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int16[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int16[p];
                out_int16[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(&sframe,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int16[q] = -olabel;
                    count += 1;

                    b = BOUNDS(gridi,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbori[j];		
                          o = out_int16[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int16[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int16[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      case UINT32_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int32[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int32[p];
                out_int32[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(&sframe,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int32[q] = -olabel;
                    count += 1;

                    b = BOUNDS(gridi,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbori[j];		
                          o = out_int32[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int32[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(&sframe,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int32[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      default:
        break;
    }
  }

  //  first[p] contains the # of edges for CB p.  Make it be the index one beyond
  //    the last edge of the CB and now that you know how many edges there are, allocate

  { int p;

#ifdef DEBUG
    for (p = 1; p <= nbasins; p++)
      { printf("  Basin %d: %d %d %d\n",p,verts[p].depth,verts[p].seed,first[p]);
        fflush(stdout);
      }
#endif

    if (nlabels > 0x7FFF)
      { if (labels->type < UINT32_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT32_TYPE,32,0);
        out_int32 = AINT32(labels);
      }
    else if (nlabels > 0x7F)
      { if (labels->type < UINT16_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT16_TYPE,16,0);
        out_int16 = AINT16(labels);
      }

    for (p = 1; p <= nbasins; p++)
      first[p] += first[p-1];
    first[0] = first[nbasins];

    allocate_wshed_edges(result,SIZEOF(P_Edge)*(first[nbasins] >> 1),"Make_Partition");
    allocate_wshed_alist(result,SIZEOF(int)*(first[nbasins]+1),"Make_Partition");

    edges = result->edges;
    alist = result->alist;
  }

  //  Flood fill each CB again, this time inverting the sign of out back to a positive value.
  //    In this pass, the edges are filled in and barrier heights computed.

  { int   qbot, qtop;
    int   etop, ecur;
    int   olabel;
    int   p;
    int   pi;						

    qbot = qtop = 0;
    etop = ecur = 0;
    p = (int) Set_Slice_To_First(frame);		

    switch (labels->type) {
    case UINT8_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int8[pi] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              PUSH(pi);				
              olabel = -out_int8[pi];
              out_int8[pi] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int8[qi] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int8[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int8[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int8[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    case UINT16_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int16[pi] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              PUSH(pi);				
              olabel = -out_int16[pi];
              out_int16[pi] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int16[qi] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int16[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int16[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int16[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    case UINT32_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int32[pi] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              PUSH(pi);				
              olabel = -out_int32[pi];
              out_int32[pi] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;
                    int      qi, ri;			

                    POP(q);
                    POP(qi);				
                    out_int32[qi] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          ri = qi + (int) neighbori[j];	
                          o = out_int32[ri];
                          if (o == -olabel)
                            { PUSH(r);
                              PUSH(ri);			
                              out_int32[ri] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(ri);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int32[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    default:
      break;
    }
  }

  //  Make an image with the assigned labels, and make seeds contain the lexicographically
  //    smallest pixel in each region.  Also shift CB numbering from 1... to 0...
 
  { int    p, b, nedges;
    int    pi;						

    switch (labels->type) {
      case UINT8_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int8[pi];
            if (verts[b].seed < 0)
              { verts[b].seed = pi;
                if (color)
                  out_int8[pi] = -verts[b].depth;
                else
                  out_int8[pi] = -out_int8[pi];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[pi]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int8[pi] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int8[p] = -out_int8[p];
        break;
      case UINT16_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int16[pi];
            if (verts[b].seed < 0)
              { verts[b].seed = pi;
                if (color)
                  out_int16[pi] = -verts[b].depth;
                else
                  out_int16[pi] = -out_int16[pi];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[pi]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int16[pi] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int16[p] = -out_int16[p];
        break;
      case UINT32_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int32[pi];
            if (verts[b].seed < 0)
              { verts[b].seed = pi;
                if (color)
                  out_int32[pi] = -verts[b].depth;
                else
                  out_int32[pi] = -out_int32[pi];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[pi]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int32[pi] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int32[p] = -out_int32[p];
        break;
      default:
        break;
    }

    nedges = first[0];
    for (b = 1; b <= nbasins; b++)
      { verts[b-1].seed  = verts[b].seed;
        verts[b-1].depth = verts[b].depth;
        verts[b-1].size  = verts[b].size;
        first[b-1]       = first[b];
      }
    first[nbasins] = nedges;

    nedges >>= 1;
    for (p = 0; p < nedges; p++)
      { edges[p].region1 -= 1;
        edges[p].region2 -= 1;
      }

    result->nlabels = nlabels;
  }

      Free_Array(basis);
      Release_Grid(gridi);
      Set_Slice_To_Index(frame,curp);			
      break;
    }
  case 1:
    { Indx_Type curp;

      size = (int) array->size;
      curp = Slice_Index(frame);

  //  Greedily assign labels to catchment basins (CBs) so that no basin is adjacent to
  //    another with the same number.  Typically the number of labels required is very,
  //    very small, 6 or 7 in my experience.  At the end of this segment, out[p] is
  //    the negative of its label reassignment, verts[b].depth is the new label for CB b,
  //    and nlabels is the number of unique labels used.
  //  Also, determine the minimum pixel of each CB b in verts[b].seed and the number of
  //    CB's adjacent to this CB in first[b]

  { int   qbot, qtop;
    int   etop;
    int   olabel, count;
    int   adjacent[0x10000];
    int   p;
    int   pi;						

    verts   = result->verts;
    first   = result->first;
    nlabels = 0;

    for (p = 0; p < 0x10000; p++)
      adjacent[p] = 0;

    for (p = 0; p <= nbasins; p++)
      { first[p] = 0;
        verts[p].seed = -1;
      }

    qbot = qtop = 0;
    etop = 0;
    p = (int) Set_Slice_To_First(frame);		

    switch (labels->type) {
      case UINT8_TYPE:
        for (pi = 0; pi < size; pi++)			

          { if (out_int8[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int8[p];
                out_int8[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int8[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int8[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int8[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int8[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
            p = (int) Next_Slice_Index(frame);		
          }
        break;
      case UINT16_TYPE:
        for (pi = 0; pi < size; pi++)			

          { if (out_int16[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int16[p];
                out_int16[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int16[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int16[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int16[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int16[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
            p = (int) Next_Slice_Index(frame);		
          }
        break;
      case UINT32_TYPE:
        for (pi = 0; pi < size; pi++)			

          { if (out_int32[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int32[p];
                out_int32[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int32[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int32[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int32[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int32[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
            p = (int) Next_Slice_Index(frame);		
          }
        break;
      default:
        break;
    }
  }

  //  first[p] contains the # of edges for CB p.  Make it be the index one beyond
  //    the last edge of the CB and now that you know how many edges there are, allocate

  { int p;

#ifdef DEBUG
    for (p = 1; p <= nbasins; p++)
      { printf("  Basin %d: %d %d %d\n",p,verts[p].depth,verts[p].seed,first[p]);
        fflush(stdout);
      }
#endif

    if (nlabels > 0x7FFF)
      { if (labels->type < UINT32_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT32_TYPE,32,0);
        out_int32 = AINT32(labels);
      }
    else if (nlabels > 0x7F)
      { if (labels->type < UINT16_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT16_TYPE,16,0);
        out_int16 = AINT16(labels);
      }

    for (p = 1; p <= nbasins; p++)
      first[p] += first[p-1];
    first[0] = first[nbasins];

    allocate_wshed_edges(result,SIZEOF(P_Edge)*(first[nbasins] >> 1),"Make_Partition");
    allocate_wshed_alist(result,SIZEOF(int)*(first[nbasins]+1),"Make_Partition");

    edges = result->edges;
    alist = result->alist;
  }

  //  Flood fill each CB again, this time inverting the sign of out back to a positive value.
  //    In this pass, the edges are filled in and barrier heights computed.

  { int   qbot, qtop;
    int   etop, ecur;
    int   olabel;
    int   p;
    int   pi;						

    qbot = qtop = 0;
    etop = ecur = 0;
    p = (int) Set_Slice_To_First(frame);		

    switch (labels->type) {
    case UINT8_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int8[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int8[p];
              out_int8[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int8[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    case UINT16_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int16[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int16[p];
              out_int16[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int16[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    case UINT32_TYPE:
      for (pi = 0; pi < size; pi++)
        { if (out_int32[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int32[p];
              out_int32[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int32[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
          p = (int) Next_Slice_Index(frame);			
        }
      break;
    default:
      break;
    }
  }

  //  Make an image with the assigned labels, and make seeds contain the lexicographically
  //    smallest pixel in each region.  Also shift CB numbering from 1... to 0...
 
  { int    p, b, nedges;
    int    pi;						

    switch (labels->type) {
      case UINT8_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int8[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int8[p] = -verts[b].depth;
                else
                  out_int8[p] = -out_int8[p];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int8[p] = -out_int8[p];
        break;
      case UINT16_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int16[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int16[p] = -verts[b].depth;
                else
                  out_int16[p] = -out_int16[p];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int16[p] = -out_int16[p];
        break;
      case UINT32_TYPE:
        p = (int) Set_Slice_To_First(frame);		
        for (pi = 0; pi < size; pi++)
          { b = out_int32[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int32[p] = -verts[b].depth;
                else
                  out_int32[p] = -out_int32[p];
              }
            p = (int) Next_Slice_Index(frame);		
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            p = (int) Set_Slice_To_First(frame);	
            for (pi = 0; pi < size; pi++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
                p = (int) Next_Slice_Index(frame);	
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int32[p] = -out_int32[p];
        break;
      default:
        break;
    }

    nedges = first[0];
    for (b = 1; b <= nbasins; b++)
      { verts[b-1].seed  = verts[b].seed;
        verts[b-1].depth = verts[b].depth;
        verts[b-1].size  = verts[b].size;
        first[b-1]       = first[b];
      }
    first[nbasins] = nedges;

    nedges >>= 1;
    for (p = 0; p < nedges; p++)
      { edges[p].region1 -= 1;
        edges[p].region2 -= 1;
      }

    result->nlabels = nlabels;
  }

      Set_Slice_To_Index(frame,curp);			
      break;
    }
  case 0:
    { size = (int) array->size;

  //  Greedily assign labels to catchment basins (CBs) so that no basin is adjacent to
  //    another with the same number.  Typically the number of labels required is very,
  //    very small, 6 or 7 in my experience.  At the end of this segment, out[p] is
  //    the negative of its label reassignment, verts[b].depth is the new label for CB b,
  //    and nlabels is the number of unique labels used.
  //  Also, determine the minimum pixel of each CB b in verts[b].seed and the number of
  //    CB's adjacent to this CB in first[b]

  { int   qbot, qtop;
    int   etop;
    int   olabel, count;
    int   adjacent[0x10000];
    int   p;

    verts   = result->verts;
    first   = result->first;
    nlabels = 0;

    for (p = 0; p < 0x10000; p++)
      adjacent[p] = 0;

    for (p = 0; p <= nbasins; p++)
      { first[p] = 0;
        verts[p].seed = -1;
      }

    qbot = qtop = 0;
    etop = 0;

    switch (labels->type) {
      case UINT8_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int8[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int8[p];
                out_int8[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int8[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int8[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int8[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int8[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      case UINT16_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int16[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int16[p];
                out_int16[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int16[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int16[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int16[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int16[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      case UINT32_TYPE:
        for (p = 0; p < size; p++)			

          { if (out_int32[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
              { PUSH(p);
                olabel = out_int32[p];
                out_int32[p] = 0;
                count  = 0;
#ifdef DEBUG
                printf("Push (");
                Print_Coord(stdout,Idx2CoordA(array,p));	
                printf("): %d\n",olabel);
#endif

                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;

                    POP(q);
                    out_int32[q] = -olabel;
                    count += 1;

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);	
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];		
                          o = out_int32[r];
                          if (o == olabel)     
                            { PUSH(r);
                              out_int32[r] = 0;
#ifdef DEBUG
                              printf("  Add (");
                              Print_Coord(stdout,Idx2CoordA(array,r)); 	
                              printf(")\n");
#endif
                            }
                          else
                            { o = -o;
                              if (o > 0 && o != olabel)   //  Pixel in adjacent, already filled CB
                                { if (verts[o].seed < 0)  //    do so and push on A-stack
                                    { EPUSH(r);
                                      verts[o].seed = 1;
                                      adjacent[verts[o].depth] = 1;
                                    }
#ifdef DEBUG
                                  printf("  Border (");
                                  Print_Coord(stdout,Idx2CoordA(array,r)); 	
                                  printf(") %d\n",o);
#endif
                                }
                            }
                        }
                  }

                { int j, q;

                  for (j = 1; j < 0x10000; j++)    //  Determine the 1st available label for CB
                    if ( ! adjacent[j])            //    This is O(edges) so still linear
                      break;

                  if (j > nlabels)
                    { nlabels = j;
                      if (j > 0xFFFF)
                        { if (color)
                            { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                              fprintf(stderr,"to color partition, so will not do so.\n");
                              fflush(stderr);
                            }
                          color = 0;
                          nlabels = nbasins;
                          j = 0xFFFF;
                        }
                    }
#ifdef DEBUG
                  printf("Basin %d: %d\n",olabel,j);
#endif
                  verts[olabel].depth = j;
                  verts[olabel].size  = count;

                  while (etop > 0)          //  Count edges between this CB and the already-filled
                    { EPOP(q);              //    adjacent CBs
                      q = -out_int32[q];
                      first[q] += 1;
                      first[olabel] += 1;
                      verts[q].seed  = -1;
                      adjacent[verts[q].depth] = 0;
#ifdef DEBUG
                      printf("   Edge %d->%d (%d =? %d)\n",olabel,q,j,verts[q].depth);
#endif
                    }
                }
              }
          }
        break;
      default:
        break;
    }
  }

  //  first[p] contains the # of edges for CB p.  Make it be the index one beyond
  //    the last edge of the CB and now that you know how many edges there are, allocate

  { int p;

#ifdef DEBUG
    for (p = 1; p <= nbasins; p++)
      { printf("  Basin %d: %d %d %d\n",p,verts[p].depth,verts[p].seed,first[p]);
        fflush(stdout);
      }
#endif

    if (nlabels > 0x7FFF)
      { if (labels->type < UINT32_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT32_TYPE,32,0);
        out_int32 = AINT32(labels);
      }
    else if (nlabels > 0x7F)
      { if (labels->type < UINT16_TYPE)
          Convert_Array_Inplace(labels,PLAIN_KIND,UINT16_TYPE,16,0);
        out_int16 = AINT16(labels);
      }

    for (p = 1; p <= nbasins; p++)
      first[p] += first[p-1];
    first[0] = first[nbasins];

    allocate_wshed_edges(result,SIZEOF(P_Edge)*(first[nbasins] >> 1),"Make_Partition");
    allocate_wshed_alist(result,SIZEOF(int)*(first[nbasins]+1),"Make_Partition");

    edges = result->edges;
    alist = result->alist;
  }

  //  Flood fill each CB again, this time inverting the sign of out back to a positive value.
  //    In this pass, the edges are filled in and barrier heights computed.

  { int   qbot, qtop;
    int   etop, ecur;
    int   olabel;
    int   p;

    qbot = qtop = 0;
    etop = ecur = 0;

    switch (labels->type) {
    case UINT8_TYPE:
      for (p = 0; p < size; p++)
        { if (out_int8[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int8[p];
              out_int8[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int8[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int8[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int8[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int8[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
        }
      break;
    case UINT16_TYPE:
      for (p = 0; p < size; p++)
        { if (out_int16[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int16[p];
              out_int16[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int16[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int16[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int16[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int16[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
        }
      break;
    case UINT32_TYPE:
      for (p = 0; p < size; p++)
        { if (out_int32[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
            { PUSH(p);
              olabel = -out_int32[p];
              out_int32[p] = 0;

              switch (array->type) {
              case UINT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case UINT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_uint32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_uint32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT8_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int8[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int8[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT16_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int16[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int16[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              case INT32_TYPE:
                while (qtop != qbot)
                  { boolean *b;
                    int      j, q, r, o;
                    int      m1, m2;

                    POP(q);
                    out_int32[q] = olabel;
                    m1 = value_int32[q];

                    b = BOUNDS(grid,(Indx_Type) q,iscon2n);
                    for (j = 0; j < n_nbrs; j++)
                      if (b[j])
                        { r = q + (int) neighbor[j];
                          o = out_int32[r];
                          if (o == -olabel)
                            { PUSH(r);
                              out_int32[r] = 0;
                            }
                          else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                            { m2 = value_int32[r];
                              if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                                { EPUSH(r);
                                  verts[o].seed = 0x10000;
                                }
                              if (m2 < m1)                //  Determine implied barrier height and
                                m2 = m1;
                              if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                                verts[o].seed = m2;       //    of pixels between the CB's
                            }
                        }
                  }
                break;
              default:
                break;
              }

              while (etop > 0)   //  Add the new edges and barrier heights for edges
                { int q;         //    between the current CB and already-filled adjacent ones

                  EPOP(q);
                  q = out_int32[q];

                  alist[--first[olabel]] = ecur;
                  alist[--first[q]] = ecur;

                  edges[ecur].height  = verts[q].seed;
                  edges[ecur].region1 = q;
                  edges[ecur].region2 = olabel;

                  ecur += 1;

                  verts[q].seed = -1;
                }
            }
        }
      break;
    default:
      break;
    }
  }

  //  Make an image with the assigned labels, and make seeds contain the lexicographically
  //    smallest pixel in each region.  Also shift CB numbering from 1... to 0...
 
  { int    p, b, nedges;

    switch (labels->type) {
      case UINT8_TYPE:
        for (p = 0; p < size; p++)
          { b = out_int8[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int8[p] = -verts[b].depth;
                else
                  out_int8[p] = -out_int8[p];
              }
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int8[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int8[p] = out_int8[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int8[p] = -out_int8[p];
        break;
      case UINT16_TYPE:
        for (p = 0; p < size; p++)
          { b = out_int16[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int16[p] = -verts[b].depth;
                else
                  out_int16[p] = -out_int16[p];
              }
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int16[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int16[p] = out_int16[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int16[p] = -out_int16[p];
        break;
      case UINT32_TYPE:
        for (p = 0; p < size; p++)
          { b = out_int32[p];
            if (verts[b].seed < 0)
              { verts[b].seed = p;
                if (color)
                  out_int32[p] = -verts[b].depth;
                else
                  out_int32[p] = -out_int32[p];
              }
          }

        for (b = 1; b <= nbasins; b++)
          verts[b].depth = maxval;

        switch (array->type) {
          case UINT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint8[p] < verts[b].depth)
                      verts[b].depth = value_uint8[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint8[p] < verts[b].depth)
                  verts[b].depth = value_uint8[p];
              }
            break;
          case UINT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint16[p] < verts[b].depth)
                      verts[b].depth = value_uint16[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint16[p] < verts[b].depth)
                  verts[b].depth = value_uint16[p];
              }
            break;
          case UINT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_uint32[p] < verts[b].depth)
                      verts[b].depth = value_uint32[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_uint32[p] < verts[b].depth)
                  verts[b].depth = value_uint32[p];
              }
            break;
          case INT8_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int8[p] < verts[b].depth)
                      verts[b].depth = value_int8[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int8[p] < verts[b].depth)
                  verts[b].depth = value_int8[p];
              }
            break;
          case INT16_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int16[p] < verts[b].depth)
                      verts[b].depth = value_int16[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int16[p] < verts[b].depth)
                  verts[b].depth = value_int16[p];
              }
            break;
          case INT32_TYPE:
            for (p = 0; p < size; p++)
              { if ((b = out_int32[p]) > 0)
                  { if (value_int32[p] < verts[b].depth)
                      verts[b].depth = value_int32[p];
                    out_int32[p] = out_int32[verts[b].seed];
                  }
              }
            for (b = 1; b <= nbasins; b++)
              { p = verts[b].seed;
                if (value_int32[p] < verts[b].depth)
                  verts[b].depth = value_int32[p];
              }
            break;
          default:
            break;
        }

        for (p = 0; p < size; p++)
          out_int32[p] = -out_int32[p];
        break;
      default:
        break;
    }

    nedges = first[0];
    for (b = 1; b <= nbasins; b++)
      { verts[b-1].seed  = verts[b].seed;
        verts[b-1].depth = verts[b].depth;
        verts[b-1].size  = verts[b].size;
        first[b-1]       = first[b];
      }
    first[nbasins] = nedges;

    nedges >>= 1;
    for (p = 0; p < nedges; p++)
      { edges[p].region1 -= 1;
        edges[p].region2 -= 1;
      }

    result->nlabels = nlabels;
  }

      break;
    }
  }

  if (emax > EGRAPHMAX)
    free(estack);
  if (qmax > QGRAPHMAX)
    free(queue);
  Release_Grid(grid);

#ifdef SHOW_GRAPH

  { int i, j;

    for (i = 0; i < nbasins; i++)
      { j = verts[i].seed;
        printf("Basin %d(%d,%d,[",i,verts[i].depth,verts[i].size);
        Print_Coord(stdout,Idx2CoordA(array,j));
        printf("]) -> ");
        for (j = first[i]; j < first[i+1]; j++)
          if (edges[alist[j]].region1 == i)
            printf(" %d(%d)",edges[alist[j]].region2,edges[alist[j]].height);
          else
            printf(" %d(%d)",edges[alist[j]].region1,edges[alist[j]].height);
        printf("\n");
      }
  }

#endif

#ifdef DRAW_WATERSHED
  { Array *view;

    view = Copy_Array(array);
    Scale_Array_To_Range(view,VALU(0),VALU(255));
    Convert_Array_Inplace(view,RGB_KIND,UINT8_TYPE,8,0);
    Draw_Partition(view,result,.25);
    Write_Image("watersheda.tif",view,DONT_PRESS);

    Kill_Array(view);
  }
#endif

  return ((Partition *) result);
}

Partition *Color_Partition(Partition *M(part))
{ Wshed    *shed    = (Wshed *) part;
  int       nbasins = shed->nbasins;
  int      *first   = shed->first;
  int      *alist   = shed->alist;
  P_Edge   *edges   = shed->edges;
  Array    *labels  = shed->labels;

  if (labels == NULL || shed->colored)
    return (part);

  int *map = Guarded_Malloc(sizeof(int)*(nbasins+1),"Color_Partition");

  { int j, c, e, d, k;
    int colored[0x10000];
    int nlabels;

    for (j = 1; j < 0x10000; j++)
      colored[j] = 0;

    map += 1;

    nlabels = 0;
    for (c = 0; c < nbasins; c++)
      { for (e = first[c]; e < first[c+1]; e++)
          { k = alist[e];
            if (edges[k].region1 == c)
              d = edges[k].region2;
            else
              d = edges[k].region1;
            if (d < c)
              colored[map[d]] = 1;
          }

        for (j = 1; j < 0x10000; j++)
          if ( ! colored[j])
            break;

        if (j > nlabels)
          { nlabels = j;
            if (j > 0xFFFF)
              { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                fprintf(stderr,"to color partition, so will not do so.\n");
                fflush(stderr);
                free(map);
                return (NULL);
              }
          }

        map[c] = j;
        for (e = first[c]; e < first[c+1]; e++)
          { k = alist[e];
            if (edges[k].region1 == c)
              d = edges[k].region2;
            else
              d = edges[k].region1;
            if (d < c)
              colored[map[d]] = 0;
          }
      }

    map -= 1;
    map[0] = 0;

    shed->nlabels = nlabels;
  }

  { int32    *out = AINT32(labels);
    Indx_Type p;

    if (Same_Shape(AForm_Array(shed->apart_ref),labels) && Is_Slice(shed->apart_ref))
      { Slice    *frame = shed->apart_ref;
        Indx_Type q, r;

        r = Slice_Index(frame);
        p = Set_Slice_To_First(frame);
        for (q = 0; q < labels->size; q++)
          { out[p] = map[out[p]];
            p = Next_Slice_Index(frame);
          }
        Set_Slice_To_Index(frame,r);
      }
    else
      for (p = 0; p < labels->size; p++)
        out[p] = map[out[p]];
  }

  free(map);
  return (part);
}


/**************************************************************************************\
*                                                                                      *
*  BUILD A WATERSHED DECOMPOSITION (ROLLUP OF TWO PARTS ABOVE)                         *
*                                                                                      *
\**************************************************************************************/

Partition *G(Build_Watershed)(Pixel_APart *frame, boolean iscon2n, boolean color)
{ Array       *array = AForm_Array(frame);
  Label_Array *h;
  Partition   *s;
  int          nbasins;

  if (array->type != UINT8_TYPE && array->type != UINT16_TYPE)
    { fprintf(stderr,"Build_Watershed: Only applies to UINT8 and UINT16 arrays\n");
      exit (1);
    }
  if (array->kind != PLAIN_KIND)
    { fprintf(stderr,"Build_Watershed: Only applies to PLAIN arrays\n");
      exit (1);
    }
  if (array->size > 0x7FFFFFFF)
    { fprintf(stderr,"Build_Watershed: Array is larger than 2G pixel\n");
      exit (1);
    }

  h = Make_Array_With_Shape(PLAIN_KIND,INT32_TYPE,AForm_Shape(frame));
  Label_Watershed(frame,h,&nbasins,iscon2n);
  s = Make_Partition(frame,h,nbasins,iscon2n,color);
  if (Get_Partition_Color_Count(s) < 256)
    Convert_Array_Inplace(h,PLAIN_KIND,UINT8_TYPE,8,0);
  else if (Get_Partition_Color_Count(s) < 0x10000)
    Convert_Array_Inplace(h,PLAIN_KIND,UINT16_TYPE,16,0);
  Pack_Array(h);
  return (s);
}

typedef struct
  { uint8     *lab8;
    uint16    *lab16;
    uint32    *lab32;
    boolean    iscon2n;
    uint8     *src8;
    uint16    *src16;

    Grid_Id    grid;
    Size_Type  nbrs;
    Offs_Type *neighbor;
    boolean   *(*bounds)(Grid_Id,Indx_Type,int);

    uint32     color;
    int        target;
    int        null;
    int        num[256];
    int        den[256];
    int64      sqr[256];
  } AveArg;

#define AVEARG(a) ((AveArg *) (a))


  boolean ave_test8(Indx_Type p, void *ap)
  { return (AVEARG(ap)->lab8[p] == AVEARG(ap)->color); }

  void ave_color8(Indx_Type p, void *ap)
  { AVEARG(ap)->lab8[p] = AVEARG(ap)->target; }


    void ave_action_8_0_8(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab8[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab8[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_8_1_8(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab8[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab8[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_0_8(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab8[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab8[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_1_8(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab8[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab8[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


  boolean ave_test16(Indx_Type p, void *ap)
  { return (AVEARG(ap)->lab16[p] == AVEARG(ap)->color); }

  void ave_color16(Indx_Type p, void *ap)
  { AVEARG(ap)->lab16[p] = AVEARG(ap)->target; }


    void ave_action_8_0_16(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab16[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab16[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_8_1_16(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab16[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab16[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_0_16(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab16[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab16[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_1_16(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab16[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab16[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


  boolean ave_test32(Indx_Type p, void *ap)
  { return (AVEARG(ap)->lab32[p] == AVEARG(ap)->color); }

  void ave_color32(Indx_Type p, void *ap)
  { AVEARG(ap)->lab32[p] = AVEARG(ap)->target; }


    void ave_action_8_0_32(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab32[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab32[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_8_1_32(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src8[p];
      if (v == ave->null) return;
      u = ave->lab32[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab32[q];
            if (o != u)
              { w = ave->src8[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_0_32(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab32[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab32[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                      }
                    else
                      { ave->num[o] += w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


    void ave_action_16_1_32(Indx_Type p, void *ap)
    { AveArg *ave = (AveArg *) ap;

      boolean  *b;
      int       j;
      Indx_Type q;
      int       u, o;
      int       v, w;

      v = ave->src16[p];
      if (v == ave->null) return;
      u = ave->lab32[p];
      b = ave->bounds(ave->grid,p,ave->iscon2n);
      for (j = 0; j < ave->nbrs; j++)
        if (b[j])
          { q = (Indx_Type) ((Offs_Type) p + ave->neighbor[j]);
            o = ave->lab32[q];
            if (o != u)
              { w = ave->src16[q];
                if (w != ave->null)
                  { if (v > w)
                      { ave->num[o] += v;
                        ave->sqr[o] += v*v;
                      }
                    else
                      { ave->num[o] += w;
                        ave->sqr[o] += w*w;
                      }
                    ave->den[o] += 1;
                  }
              }
          }
    }


void Average_Watershed_Heights(Partition *shed, int *num, int *den, int64 *sqr, int null)
{ Wshed *w = (Wshed *) shed;

  int           nbasins = w->nbasins;
  int           nlabels = w->nlabels;
  Array        *labels  = w->labels;
  boolean       iscon2n = w->iscon2n;
  P_Vertex     *verts   = w->verts;
  P_Edge       *edges   = w->edges;
  int          *alist   = w->alist;
  int          *first   = w->first;
  Array        *basis   = AForm_Array(w->apart_ref);

  AveArg        arg, *ap = &arg;
  void          (*action)(Indx_Type, void *);
  uint8        *lab8;
  uint16       *lab16;
  uint32       *lab32;

  if (w->labels == NULL)
    { fprintf(stderr,"Partition has no label field (Average_Watershed_Heights)\n");
      exit (1);
    }

  arg.lab8    = lab8  = AUINT8(labels);
  arg.lab16   = lab16 = AUINT16(labels);
  arg.lab32   = lab32 = AUINT32(labels);
  arg.iscon2n = iscon2n;
  arg.src8    = AUINT8(basis);
  arg.src16   = AUINT16(basis);
  arg.null    = null;

  arg.grid      = Setup_Grid(w->apart_ref,"Average_Edges");
  arg.nbrs      = Grid_Size(arg.grid,iscon2n);
  arg.neighbor  = Grid_Neighbors(arg.grid,iscon2n);

  if (labels->ndims == 2)
    arg.bounds = Boundary_Pixels_2d;
  else if (labels->ndims == 3)
    arg.bounds = Boundary_Pixels_3d;
  else
    arg.bounds = Boundary_Pixels;

  { int i, j, k, c, w;
    Indx_Type s;
    int map[256];
    int assign, beg, end;

    for (c = 0; c < 256; c++)
      { arg.num[c] = arg.den[c] = map[c] = 0;
        arg.sqr[c] = 0;
      }

    switch (labels->type) {
      case UINT8_TYPE:
      case  INT8_TYPE:
        if (basis->type == UINT8_TYPE)
          if (sqr == NULL)
            action = ave_action_8_0_8;
          else
            action = ave_action_8_1_8;
        else
          if (sqr == NULL)
            action = ave_action_16_0_8;
          else
            action = ave_action_16_1_8;

        for (i = 0; i < nbasins; i++)
  
          { beg = first[i];
            end = first[i+1];
  
            s = (Indx_Type) verts[i].seed;
            c = lab8[s];
            map[c] = 1;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                map[lab8[verts[w].seed]] = 1;
              }
  
            assign = nlabels;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab8[s];
                if (map[c] == 1)
                  map[c] = 2;
                else
                  { map[++assign] = c;
                    arg.color  = c;
                    arg.target = assign;
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test8,NULL,NULL,
                                                    NULL,NULL,ap,ave_color8);
                  }
              }
  
            s = (Indx_Type) verts[i].seed;
            c = lab8[s];
            arg.color = c;
            Flood_Object(labels,0,iscon2n,s,ap,ave_test8,NULL,NULL,NULL,NULL,ap,action);
            map[c] = 0;
  
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab8[s];
                num[k] = arg.num[c];
                den[k] = arg.den[c];
                if (sqr != 0)
                  sqr[k] = arg.sqr[c];
  
                if (c > nlabels)
                  { arg.color  = c;
                    arg.target = map[c];
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test8,NULL,NULL,
                                                    NULL,NULL,ap,ave_color8);
                  }
                map[c] = 0;
                arg.num[c] = arg.den[c] = 0;
                arg.sqr[c] = 0;
              }
          }
        break;
      case UINT16_TYPE:
      case  INT16_TYPE:
        if (basis->type == UINT8_TYPE)
          if (sqr == NULL)
            action = ave_action_8_0_16;
          else
            action = ave_action_8_1_16;
        else
          if (sqr == NULL)
            action = ave_action_16_0_16;
          else
            action = ave_action_16_1_16;

        for (i = 0; i < nbasins; i++)
  
          { beg = first[i];
            end = first[i+1];
  
            s = (Indx_Type) verts[i].seed;
            c = lab16[s];
            map[c] = 1;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                map[lab16[verts[w].seed]] = 1;
              }
  
            assign = nlabels;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab16[s];
                if (map[c] == 1)
                  map[c] = 2;
                else
                  { map[++assign] = c;
                    arg.color  = c;
                    arg.target = assign;
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test16,NULL,NULL,
                                                    NULL,NULL,ap,ave_color16);
                  }
              }
  
            s = (Indx_Type) verts[i].seed;
            c = lab16[s];
            arg.color = c;
            Flood_Object(labels,0,iscon2n,s,ap,ave_test16,NULL,NULL,NULL,NULL,ap,action);
            map[c] = 0;
  
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab16[s];
                num[k] = arg.num[c];
                den[k] = arg.den[c];
                if (sqr != 0)
                  sqr[k] = arg.sqr[c];
  
                if (c > nlabels)
                  { arg.color  = c;
                    arg.target = map[c];
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test16,NULL,NULL,
                                                    NULL,NULL,ap,ave_color16);
                  }
                map[c] = 0;
                arg.num[c] = arg.den[c] = 0;
                arg.sqr[c] = 0;
              }
          }
        break;
      case UINT32_TYPE:
      case  INT32_TYPE:
        if (basis->type == UINT8_TYPE)
          if (sqr == NULL)
            action = ave_action_8_0_32;
          else
            action = ave_action_8_1_32;
        else
          if (sqr == NULL)
            action = ave_action_16_0_32;
          else
            action = ave_action_16_1_32;

        for (i = 0; i < nbasins; i++)
  
          { beg = first[i];
            end = first[i+1];
  
            s = (Indx_Type) verts[i].seed;
            c = lab32[s];
            map[c] = 1;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                map[lab32[verts[w].seed]] = 1;
              }
  
            assign = nlabels;
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab32[s];
                if (map[c] == 1)
                  map[c] = 2;
                else
                  { map[++assign] = c;
                    arg.color  = c;
                    arg.target = assign;
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test32,NULL,NULL,
                                                    NULL,NULL,ap,ave_color32);
                  }
              }
  
            s = (Indx_Type) verts[i].seed;
            c = lab32[s];
            arg.color = c;
            Flood_Object(labels,0,iscon2n,s,ap,ave_test32,NULL,NULL,NULL,NULL,ap,action);
            map[c] = 0;
  
            for (j = beg; j < end; j++)
              { k = alist[j];
                if (edges[k].region1 == i)
                  w = edges[k].region2;
                else
                  w = edges[k].region1;
                s = (Indx_Type) verts[w].seed;
                c = lab32[s];
                num[k] = arg.num[c];
                den[k] = arg.den[c];
                if (sqr != 0)
                  sqr[k] = arg.sqr[c];
  
                if (c > nlabels)
                  { arg.color  = c;
                    arg.target = map[c];
                    Flood_Object(labels,0,iscon2n,s,ap,ave_test32,NULL,NULL,
                                                    NULL,NULL,ap,ave_color32);
                  }
                map[c] = 0;
                arg.num[c] = arg.den[c] = 0;
                arg.sqr[c] = 0;
              }
          }
        break;
      default:
        fprintf(stderr,"Partition label field is not integer (Average_Watershed_Height)\n");
        break;
    }
  }
}

#ifdef UNDER_DEVELOPMENT

/**************************************************************************************\
*                                                                                      *
*  WATERGRAPH CONSTRUCTION IN PARTS FOR THREADED IMPLEMENTATION                        *
*                                                                                      *
\**************************************************************************************/

void watergraph_count(Pixel_APart *frame, int nbasins, boolean iscon2n, Label_Array *labels,
                      Wshed *result, int noffset)
{ Array        *array = AForm_Array(frame);

  P_Vertex     *verts;     // A list of records, one for each catchment basin
  int          *first;     // Adjacency list indices

  int  size;
  int *out;

  uint8  *value_uint8;
  uint16 *value_uint16;

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  boolean   *(*BOUNDS)(Grid_Id,Indx_Type,int);

  int *queue, qmax, Queue[QGRAPHMAX];
  int *estack, emax, Estack[EGRAPHMAX];

  //  Setup access variables

  out      = AINT32(labels);
  grid     = Setup_Grid(frame,"Make_Partition");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  qmax   = QGRAPHMAX;
  queue  = Queue;
  emax   = EGRAPHMAX;
  estack = Estack;

  if (array->type == UINT16_TYPE)
    value_uint16 = AUINT16(array);
  else
    value_uint8  = AUINT8(array);

  size = array->size;

  //  Greedily assign labels to catchment basins (CBs) so that no basin is adjacent to
  //    another with the same number.  Typically the number of labels required is very,
  //    very small, 6 or 7 in my experience.  At the end of this segment, out[p] is
  //    the negative of its label reassignment, verts[b].depth is the new label for CB b,
  //    and nlabels is the number of unique labels used.
  //  Also, determine the minimum pixel of each CB b in verts[b].seed and the number of
  //    CB's adjacent to this CB in first[b]

  { int   qbot, qtop;
    int   etop;
    int   olabel, nlabel, count;
    int   p, pi;

    verts   = result->verts;
    first   = result->first;

    for (p = noffset+nbasins; p > noffset; p--)
      { first[p] = 0;
        verts[p].seed = -1;
      }

    qbot = qtop = 0;
    etop = 0;
    p = Set_Slice_To_First(frame);
    for (pi = 0; pi < size; pi++)

      { if (out[p] > 0)         //  Flood fill each CB, marking it by flipping sign of out
          { PUSH(p);
            olabel = out[p];
            nlabel = olabel + noffset;
            out[p] = 0;
            count  = 0;
#ifdef DEBUG
            printf("Push (");
            Print_Coord(stdout,Idx2CoordA(array,p));
            printf("): %d\n",olabel);
#endif

            while (qtop != qbot)
              { boolean *b;
                int      j, q, r, o;

                POP(q);
                out[q] = -nlabel;
                count += 1;

                b = BOUNDS(grid,q,iscon2n);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { r = q + neighbor[j];
                      o = out[r];
                      if (o == olabel)     
                        { PUSH(r);
                          out[r] = 0;
#ifdef DEBUG
                          printf("  Add (");
                          Print_Coord(stdout,Idx2CoordA(array,r));
                          printf(")\n");
#endif
                        }
                      else
                        { o = -o;
                          if (o > 0 && o != nlabel)   //  Pixel in adjacent, already filled CB
                            { if (verts[o].seed < 0)  //    do so and push on A-stack
                                { EPUSH(r);
                                  verts[o].seed = 1;
                                }
#ifdef DEBUG
                              printf("  Border (");
                              Print_Coord(stdout,Idx2CoordA(array,r));
                              printf(") %d\n",o);
#endif
                            }
                        }
                    }
              }

            { int q;

              verts[nlabel].size  = count;

              while (etop > 0)              //  Count edges between this CB and the already-filled
                { EPOP(q);                  //    adjacent CBs
                  q = -out[q];
                  first[q] += 1;
                  first[nlabel] += 1;
                  verts[q].seed  = -1;
#ifdef DEBUG
                  printf("   Edge %d->%d\n",nlabel,q);
#endif
                }
            }
          }
        p = Next_Slice_Index(frame);
      }
  }

  if (emax > EGRAPHMAX)
    free(estack);
  if (qmax > QGRAPHMAX)
    free(queue);
  Release_Grid(grid);
}

void watergraph_edges(Pixel_APart *frame, int nbasins, boolean iscon2n, Label_Array *labels,
                      Wshed *result, int noffset)
{ Array        *array = AForm_Array(frame);

  P_Vertex     *verts;     // A list of records, one for each catchment basin
  P_Edge       *edges;     // A list of edges, encoded as described in the .h file
  int          *first;     // Adjacency list indices
  int          *alist;

  int  maxval;
  int  size;
  int *out;

  uint8  *value_uint8;
  uint16 *value_uint16;

  Grid_Id    grid;
  int        n_nbrs;
  Offs_Type *neighbor;
  boolean   *(*BOUNDS)(Grid_Id,Indx_Type,int);

  int *queue, qmax, Queue[QGRAPHMAX];
  int *estack, emax, Estack[EGRAPHMAX];

  //  Setup access variables

  out      = AINT32(labels);
  grid     = Setup_Grid(frame,"Make_Partition");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  if (array->ndims == 2)
    BOUNDS = Boundary_Pixels_2d;
  else if (array->ndims == 3)
    BOUNDS = Boundary_Pixels_3d;
  else
    BOUNDS = Boundary_Pixels;

  qmax   = QGRAPHMAX;
  queue  = Queue;
  emax   = EGRAPHMAX;
  estack = Estack;

  if (array->type == UINT16_TYPE)
    { maxval       = 0x10000;
      value_uint16 = AUINT16(array);
    }
  else
    { maxval       = 0x100;
      value_uint8  = AUINT8(array);
    }

  size = array->size;

  //  Flood fill each CB again, this time inverting the sign of out back to a positive value.
  //    In this pass, the edges are filled in and barrier heights computed.

  { int   qbot, qtop;
    int   etop, ecur;
    int   olabel;
    int   p, pi;

    qbot = qtop = 0;
    etop = ecur = 0;
    p = Set_Slice_To_First(frame);
    for (pi = 0; pi < size; pi++)
      { if (out[p] < 0)         //  Flood fill each CB, marking it by flipping sign of out
         
          { PUSH(p);
            olabel = -out[p];
            out[p] = 0;

            while (qtop != qbot)
              { boolean *b;
                int   j, q, r, o;
                int   m1, m2;

                POP(q);
                out[q] = olabel;
                if (value_uint8 == NULL)
                  m1 = value_uint16[q];
                else
                  m1 = value_uint8[q];

                b = BOUNDS(grid,q,iscon2n);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { r = q + neighbor[j];
                      o = out[r];
                      if (o == -olabel)
                        { PUSH(r);
                          out[r] = 0;
                        }
                      else if (o > 0 && o != olabel)  //  Pixel in adjacent, already filled CB
                        { if (value_uint8 == NULL)
                            m2 = value_uint16[r];
                          else
                            m2 = value_uint8[r];
                          if (verts[o].seed < 0)      //  Push on A-stack if first pixel in CB
                            { EPUSH(r);
                              verts[o].seed = 0x10000;
                            }
                          if (m2 < m1)                //  Determine implied barrier height and
                            m2 = m1;
                          if (m2 < verts[o].seed)     //    figure into minimum over all pairs
                            verts[o].seed = m2;       //    of pixels between the CB's
                        }
                    }
              }

            while (etop > 0)          //  Add the new edges and barrier heights for edges
              { int q;                //    between the current CB and already-filled adjacent ones

                EPOP(q);
                q = out[q];

                alist[--first[olabel]] = ecur;
                alist[--first[q]] = ecur;

                edges[ecur].height  = verts[q].seed;
                edges[ecur].region1 = q;
                edges[ecur].region2 = olabel;

                ecur += 1;

                verts[q].seed = -1;
              }
          }
        p = Next_Slice_Index(frame);
      }
  }

  //  Make an image with the assigned labels, and make seeds contain the lexicographically
  //    smallest pixel in each region.  Also shift CB numbering from 1... to 0...
 
  { int    p, pi, b, nedges;
    uint8 *lab = AUINT8(labels);

    switch (array->type) {
        case UINT8_TYPE:
          p = Set_Slice_To_First(frame);
          for (pi = 0; pi < size; pi++)
            { b = out[p];
              if (verts[b].seed < 0)
                { verts[b].seed = p;
                  out[p] = -verts[b].depth;
                }
              p = Next_Slice_Index(frame);
            }

          for (b = 1; b <= nbasins; b++)
            verts[b].depth = maxval;

          p = Set_Slice_To_First(frame);
          for (pi = 0; pi < size; pi++)
            { if ((b = out[p]) > 0)
                { if (value_uint8[p] < verts[b].depth)
                    verts[b].depth = value_uint8[p];
                  out[p] = out[verts[b].seed];
                }
              p = Next_Slice_Index(frame);
            }
          for (b = 1; b <= nbasins; b++)
            { p = verts[b].seed;
              if (value_uint8[p] < verts[b].depth)
                verts[b].depth = value_uint8[p];
            }
          break;
        case UINT16_TYPE:
          p = Set_Slice_To_First(frame);
          for (pi = 0; pi < size; pi++)
            { b = out[p];
              if (verts[b].seed < 0)
                { verts[b].seed = p;
                  out[p] = -verts[b].depth;
                }
              p = Next_Slice_Index(frame);
            }

          for (b = 1; b <= nbasins; b++)
            verts[b].depth = maxval;

          p = Set_Slice_To_First(frame);
          for (pi = 0; pi < size; pi++)
            { if ((b = out[p]) > 0)
                { if (value_uint16[p] < verts[b].depth)
                    verts[b].depth = value_uint16[p];
                  out[p] = out[verts[b].seed];
                }
              p = Next_Slice_Index(frame);
            }
          for (b = 1; b <= nbasins; b++)
            { p = verts[b].seed;
              if (value_uint16[p] < verts[b].depth)
                verts[b].depth = value_uint16[p];
            }
          break;
        default:
          break;
    }

    for (p = 0; p < size; p++)
      lab[p] = -out[p];

    nedges = first[0];
    for (b = 1; b <= nbasins; b++)
      { verts[b-1].seed  = verts[b].seed;
        verts[b-1].depth = verts[b].depth;
        verts[b-1].size  = verts[b].size;
        first[b-1]       = first[b];
      }
    first[nbasins] = nedges;

    nedges >>= 1;
    for (p = 0; p < nedges; p++)
      { edges[p].region1 -= 1;
        edges[p].region2 -= 1;
      }

    labels->type   = UINT8_TYPE;
    labels->scale  = 8;
    result->labels = Pack_Array(labels);
  }

  if (emax > EGRAPHMAX)
    free(estack);
  if (qmax > QGRAPHMAX)
    free(queue);
  Release_Grid(grid);
}

//  Threaded implementation of water shed !!

typedef struct
  { pthread_t thread;
    Slice    *slice;
    Array    *labels;
    boolean   iscon2n;
    int       nbasins;
    Wshed    *wshed;
    int       offset;
  } Water_Thread;

typedef struct
  { int v1, v2; } EdgePair;

void *do_label(void *arg)
{ Water_Thread *pack = (Water_Thread *) arg;
printf("S\n"); fflush(stdout);
  Label_Watershed(pack->slice,pack->labels,&(pack->nbasins),pack->iscon2n);
printf("F\n"); fflush(stdout);
  return (NULL);
}

void *do_count(void *arg)
{ Water_Thread *pack = (Water_Thread *) arg;
  watergraph_count(pack->slice,pack->nbasins,pack->iscon2n,pack->labels,
                   pack->wshed,pack->offset);
  return (NULL);
}

#define EPAIR(x)  ((EdgePair *) (x))

static int PSORT(const void *x, const void *y)
{ int r = EPAIR(x)->v1 - EPAIR(y)->v1;
  if (r != 0)
    return (r);
  else
    return (EPAIR(x)->v2 - EPAIR(y)->v2);
}

Partition *G(Build_Watershed_Threaded)(Pixel_Array *frame, boolean iscon2n, Coordinate *pieces)
{ int            npieces;
  Water_Thread  *pack;

  Label_Array *labels;
  int          nbasins;
  Wshed       *result;

  EdgePair      *elist;
  Size_Type      esize;

  int        ndims = frame->ndims;
  Dimn_Type *dims  = frame->dims;
  Dimn_Type *cuts  = ADIMN(pieces);

  { int j;

    npieces = 1;
    for (j = 0; j < ndims; j++)
      npieces *= cuts[j];
  }

  pack = (Water_Thread *)
             Guarded_Malloc(sizeof(Water_Thread)*((size_t) npieces),"Build_Watershed_Threaded");

  labels = Make_Array(PLAIN_KIND,INT32_TYPE,ndims,dims);

  //  Determine slices implied by cut counts & set up initial thread packets

  { Coordinate *begV, *endV, *cntV;
    Dimn_Type  *beg,  *end,  *cnt;
    int         i, j;

    begV = AForm_Shape(frame);
    endV = Copy_Array(begV);
    cntV = Copy_Array(pieces);

    beg  = ADIMN(begV);
    end  = ADIMN(endV);
    cnt  = ADIMN(cntV);

    for (i = 0; i < npieces; i++)
      { for (j = 0; j < ndims; j++)
          if (cnt[j]++ >= cuts[j])
            { cnt[j] = 1;
              beg[j] = 0;
              end[j] = dims[j]/cuts[j] - 1;
            }
          else
            { beg[j] = end[j]+1;
              end[j] = (dims[j]*cnt[j])/cuts[j] - 1;
              break;
            }
        pack[i].slice   = Make_Slice(frame,Copy_Array(begV),Copy_Array(endV));
        pack[i].labels  = labels;
        pack[i].iscon2n = iscon2n;
      }

for (i = 0; i < npieces; i++)
  { printf("Slice ");
    Print_Coord(stdout,Inc_Array(Slice_First(pack[i].slice)));
    printf(" - ");
    Print_Coord(stdout,Inc_Array(Slice_Last(pack[i].slice)));
    printf("\n");
    fflush(stdout);
  }

    Free_Array(begV);
    Free_Array(endV);
    Free_Array(cntV);
  }

  //  In parallel threads, label each slice

  { int i;

    for (i = 0; i < npieces; i++)
      pthread_create(&(pack[i].thread),NULL,do_label,pack+i);
 
    for (i = 0; i < npieces; i++)
      pthread_join(pack[i].thread,NULL);
  }

  //  Now know how many basins in each slice, allocate the watershed graph and its vertex indexed
  //    structures (but not its edges as we don't yet know these)

  { int i;

    nbasins = 0;
printf("\nOffsets:\n");
    for (i = 0; i < npieces; i++)
      { pack[i].offset = nbasins;
printf("   %1d: %3d\n",i,nbasins);
        nbasins += pack[i].nbasins;
      }
printf("nbasins = %d\n",nbasins);
fflush(stdout);

Print_Array(labels,stdout,4,"%3d");
fflush(stdout);

    result = new_wshed(SIZEOF(P_Vertex)*(nbasins+1), 0, 0
                       SIZEOF(int)*(nbasins+1), "Make_Partition");
    result->apart_ref = Inc_AForm(frame);
    result->nbasins   = nbasins;
    result->iscon2n   = iscon2n;
    result->labels    = labels;
  }

  //  Count the number of edges between bases within each slice in parallel.  Each label has
  //    had its sign inverted and been relabeled to be unique for the entire image.

  { int i;

    for (i = 0; i < npieces; i++)
      { pack[i].wshed = result;
        pthread_create(&(pack[i].thread),NULL,do_count,pack+i);
      }
            
    for (i = 0; i < npieces; i++)
      pthread_join(pack[i].thread,NULL);
  }

Print_Array(labels,stdout,4,"%3d");
fflush(stdout);

  //  Determine the edges between bases of different slices across each cutting planes

  { Grid_Id    grid;
    int        n_nbrs;
    Offs_Type *neighbor;
    int       *n_dir;
    boolean   *(*BOUNDS)(Grid_Id,Indx_Type,int);

    { int j;

      esize = 0;
      if (iscon2n)
        n_nbrs = 1;
      else
        { grid     = Setup_Grid(frame,"Build_Watershed_Threaded");
          n_nbrs   = Grid_Size(grid,iscon2n) / 3;
          neighbor = Grid_Neighbors(grid,iscon2n);
          n_dir    = Grid_Backtrack(grid,iscon2n);
          if (frame->ndims == 2)
            BOUNDS = Boundary_Pixels_2d;
          else if (frame->ndims == 3)
            BOUNDS = Boundary_Pixels_3d;
          else
            BOUNDS = Boundary_Pixels;
        }
      for (j = 0; j < ndims; j++)
        esize = (frame->size / dims[0]) * cuts[j] * n_nbrs;
    }

    elist = (EdgePair *)
                Guarded_Malloc(sizeof(EdgePair)*((size_t) esize),"Build_Watershed_Threaded");
    esize = 0;

    { Size_Type innr, outr, size;
      int32    *lab = AINT32(labels);
      int       j;

      size = frame->size;
      innr = 1;
      outr = dims[0];
      for (j = 0; j < ndims; j++)
        { if (iscon2n)
            { int32    *lab2 = lab+innr;
              Indx_Type i, b, h, n, p;

              for (i = 1; i < cuts[j]; i++)
                { b = ((dims[j]*i)/cuts[j] - 1) * innr;
                  printf("Cut %lld\n",b);
                  for (h = b; h < size; h = n)
                    { n = h+outr;
                      for (p = h; p < h+innr; p++)
                        { elist[esize].v1 = -lab[p];
                          elist[esize++].v2 = -lab2[p];
                          printf(" p = %lld (%d<->%d)\n",p,-lab[p],-lab2[p]);
                        }
                    }
                }
            }
          else
            { Indx_Type i, k, b, h, n, p;
              boolean  *bnd;

              h = 0;
              b = outr-1;
              for (i = 0; i < n_nbrs; i++)
                { n = neighbor[i];
                  if (n < 0)
                    { if ((-n) % outr == b)
                        n_dir[h++] = i;
                    }
                  else
                    { if (n % outr == 1)
                        n_dir[h++] = i;
                    }
                }
              for (i = 1; i < cuts[j]; i++)
                { b = ((dims[j]*i)/cuts[j] - 1) * innr;
                  printf("Cut %lld\n",b);
                  for (h = b; h < size; h = n)
                    { n = h+outr;
                      for (p = h; p < h+innr; p++)
                        { bnd = BOUNDS(grid,p,0);
                          for (k = 0; k < n_nbrs; k++)
                            if (bnd[n_dir[k]])
                              { elist[esize].v1 = -lab[p];
                                elist[esize++].v2 = -lab[p+neighbor[n_dir[k]]];
                                printf(" p = %lld (%d<->%d)\n",
                                       p,-lab[p],-lab[p+neighbor[n_dir[k]]]);
                              }
                        }
                    }
                }
            }
 
          innr  = outr;
          outr *= dims[j];
        } 
    }

    qsort(elist,esize,sizeof(EdgePair),PSORT);
  }

  //  first[p] contains the # of edges for CB p.  Make it be the index one beyond
  //    the last edge of the CB and now that you know how many edges there are, allocate

  { int p;

    for (p = 2; p <= nbasins; p++)
      first[p] += first[p-1];
    first[0] = first[nbasins];

    allocate_wshed_edges(result,SIZEOF(P_Edge)*(first[nbasins]>>1),"Build_Watershed");
    allocate_wshed_alist(result,SIZEOF(int)*(first[nbasins]+1),"Build_Watershed");

    edges = result->edges;
  }


  return (NULL);
}

#endif


/****************************************************************************************\
*                                                                                        *
*  MODEL PROGRESSIVE MERGES OF CB's IN ORDER OF BARRIER HEIGHT                           *
*                                                                                        *
*****************************************************************************************/

static int ESORT(const void *l, const void *r)
{ P_Edge *x = (P_Edge *) l;
  P_Edge *y = (P_Edge *) r;
  return (x->height - y->height);
}

static int FIND(int v, int *first)
{ int w, z;

  w = v;
  while (first[w] >= 0)
    w = first[w];
  while (first[v] >= 0)
    { z = first[v];
      first[v] = w;
      v = z;
    }
  return (w);
}

Map_Bundle *Static_Collapse(Map_Bundle *R(O(map)), Partition *eshed, void *info,
                            tristate (*handler)(int a, int h, int b, void *info) )
{ Wshed        *shed    = (Wshed *) eshed;
  int          *first   = shed->first;
  int          *alist   = shed->alist;
  P_Edge       *edges   = shed->edges;
  int           nbasins = shed->nbasins;
  int           nedges  = (first[nbasins] >> 1);

  int          *labels;

  int    v, w, s;
  int    e, f;

  if (nbasins <= 1)
    return (NULL);

  { Size_Type n = 2*SIZEOF(int)*(nbasins+1);

    labels = map->mapto;
    if (labels == NULL)
      labels = (int *) Guarded_Malloc((size_t) n,"Static_Collapse");
    else if (malloc_size(labels) < (size_t) n)
      labels = (int *) Guarded_Realloc(labels,(size_t) n,"Static_Collapse");
    map->mapto = labels;
  }

  qsort(edges,(size_t) nedges,sizeof(P_Edge),ESORT);

  // Do UNION/FIND merge to determine merges

  for (v = 0; v < nbasins; v++)
    first[v] = -1;

  f = nbasins-2;
  for (e = 0; e < nedges; e++)
    { v = FIND(edges[e].region1,first);
      w = FIND(edges[e].region2,first);
      if (v != w)
        { s = handler(v, edges[e].height, w, info);
          if (s > 0)
            first[w] = v;
          else if (s < 0)
            break;
        }
    }

  w = 1;
  for (v = 0; v < nbasins; v++)
    if (first[v] < 0)
      first[v] = -(w++);

  map->nrange = w-1;

  for (v = 0; v < nbasins; v++)
    labels[v] = -(first[FIND(v,first)]+1);

  // Restore the alist and first edge structure (but for the sorted list of edges)

  for (v = 0; v <= nbasins; v++)
    first[v] = 0;

  for (e = 0; e < nedges; e++)
    { first[edges[e].region1] += 1;
      first[edges[e].region2] += 1;
    }

  for (v = 1; v <= nbasins; v++)
    first[v] += first[v-1];

  for (e = 0; e < nedges; e++)
    { alist[--first[edges[e].region1]] = e;
      alist[--first[edges[e].region2]] = e;
    }

#ifdef SHOW_GRAPH

  { int       i, j, k;
    P_Vertex *verts = shed->verts;

    printf("\nRestored edges:\n");
    for (i = 0; i < nbasins; i++)
      { printf("Basin %d(%d) -> ",i,verts[i].depth);
        for (j = first[i]; j < first[i+1]; j++)
          { k = alist[j];
            if (edges[k].region1 == i)
              printf(" %d(%d)",edges[k].region2,edges[k].height);
            else
              printf(" %d(%d)",edges[k].region1,edges[k].height);
          }
        printf("\n");
      }
  }

#endif

  return (map);
}


/****************************************************************************************\
*                                                                                        *
*  MODEL PROGRESSIVE MERGES OF CB's IN ORDER SPECIFIED BY A HANDLER                      *
*                                                                                        *
*****************************************************************************************/

typedef struct
  { void       *info;
    int        *first;
    P_Edge     *edges;
    boolean   (*compare)(int,int,int,int,int,int,void*);
    int        *edgeid;
    int        *heapos;
    int         hmax;
  } Heap;

#ifdef DEBUG

static void check_heap(Heap *h)
{ int       c, l, r;                      // incorporate s to make subtree a heap
  int       p, ql, qr;
  int       b1, b2, l1, l2, r1, r2;
  int      *edgeid, *heapos, *first;
  P_Edge   *edges;
  void     *info;

  edgeid = h->edgeid;
  heapos = h->heapos;
  first  = h->first;
  edges  = h->edges;
  info   = h->info;

  for (c = 1; c <= h->hmax/2; c++)
    { l = 2*c;
      r = l+1;
      p  = edgeid[c];
      b1 = FIND(edges[p].region1,first);
      b2 = FIND(edges[p].region2,first);
      ql = edgeid[l];
      l1 = FIND(edges[ql].region1,first);
      l2 = FIND(edges[ql].region2,first);
      if (r > h->hmax)
        { if (h->compare(l1,ql,l2,b1,p,b2,info))
            { printf("Not a heap %d(%d:%d<->%d) to %d(%d:%d<->%d) (%d)\n",
                     l,ql,l1,l2,c,p,b1,b2,h->hmax);
              // h->compare(l1,ql,l2,b1,p,b2,(void *) 1);
            }
        }
      else
        { qr = edgeid[r];
          r1 = FIND(edges[qr].region1,first);
          r2 = FIND(edges[qr].region2,first);
          if (h->compare(l1,ql,l2,r1,qr,r2,info))
            { if (h->compare(l1,ql,l2,b1,p,b2,info))
                { printf("Not a heap %d(%d:%d<->%d) to %d(%d:%d<->%d) (%d)\n",
                         l,ql,l1,l2,c,p,b1,b2,h->hmax);
                  // h->compare(l1,ql,l2,b1,p,b2,(void *) 1);
                }
            }
          else
            { if (h->compare(r1,qr,r2,b1,p,b2,info))
                { printf("Not a heap %d(%d:%d<->%d) to %d(%d:%d<->%d) (%d)\n",
                         r,qr,r1,r2,c,p,b1,b2,h->hmax);
                  // h->compare(r1,qr,r2,b1,p,b2,(void *) 1);
                }
            }
        }
    }
}

#endif

static int reheap(int s, Heap *h)         // Tree below s is a heap,
{ int       c, l, r;                      // incorporate s to make subtree a heap
  int       p, ql, qr;
  int       b1, b2, l1, l2, r1, r2;
  int      *edgeid, *heapos, *first;
  P_Edge   *edges;
  void     *info;

  edgeid = h->edgeid;
  heapos = h->heapos;
  first  = h->first;
  edges  = h->edges;
  info   = h->info;

  c  = s;
  p  = edgeid[s];
  b1 = FIND(edges[p].region1,first);
  b2 = FIND(edges[p].region2,first);
  while ((l = (c << 1)) <= h->hmax)
    { r = l+1;
      ql = edgeid[l];
      l1 = FIND(edges[ql].region1,first);
      l2 = FIND(edges[ql].region2,first);
      if (r > h->hmax)
        { if (h->compare(l1,ql,l2,b1,p,b2,info))
            { edgeid[c]  = ql;
              heapos[ql] = c;
              c = l;
            }
          else
            break;
        }
      else
        { qr = edgeid[r];
          r1 = FIND(edges[qr].region1,first);
          r2 = FIND(edges[qr].region2,first);
          if (h->compare(l1,ql,l2,r1,qr,r2,info))
            { if (h->compare(l1,ql,l2,b1,p,b2,info))
                { edgeid[c]  = ql;
                  heapos[ql] = c;
                  c = l;
                }
              else
                break;
            }
          else
            { if (h->compare(r1,qr,r2,b1,p,b2,info))
                { edgeid[c]  = qr;
                  heapos[qr] = c;
                  c = r;
                }
              else
                break;
            }
        }
    }
  if (c != s)
    { edgeid[c] = p;
      heapos[p] = c;
      return (1);
    }
  else
    return (0);
}

static int upheap(int s, Heap *h)
{ int       c, f;
  int       p, q;
  int       b1, b2, f1, f2;
  int      *edgeid, *heapos, *first;
  P_Edge   *edges;
  void     *info;

  edgeid = h->edgeid;
  heapos = h->heapos;
  first  = h->first;
  edges  = h->edges;
  info   = h->info;

  c  = s;
  p  = edgeid[s];
  b1 = FIND(edges[p].region1,first);
  b2 = FIND(edges[p].region2,first);
  while (c > 1)
    { f = (c >> 1);
      q = edgeid[f];
      f1 = FIND(edges[q].region1,first);
      f2 = FIND(edges[q].region2,first);
      if (h->compare(f1,q,f2,b1,p,b2,info)) break;
      edgeid[c] = q;
      heapos[q] = c;
      c = f;
    }
  if (c != s)
    { edgeid[c] = p;
      heapos[p] = c;
      return (1);
    }
  else
    return (0);
}
 
static int pop_heap(Heap *h)
{ int p, q;

  p = h->edgeid[1];
  if (--h->hmax >= 1)
    { h->edgeid[1] = q = h->edgeid[h->hmax+1];
      h->heapos[q] = 1;
      reheap(1,h);
    }
  return (p);
}

static void heapify(Heap *h)     // Move elements so as to order inititlaly
{ int i;                         //   unordered heap in O(n) time
  for (i = h->hmax/2; i >= 1; i--)
    reheap(i,h);
}

static void adjust_heap(int e, Heap *h)
{ int p = h->heapos[e];
  reheap(p,h);
  int f = h->edgeid[p];
  if (upheap(p,h))
    { adjust_heap(h->edgeid[p],h);
      adjust_heap(f,h);
    }
}

static void remove_heap(int e, Heap *h)
{ int n = h->hmax;
  int p = h->heapos[e];
  int f = h->edgeid[n];
  h->edgeid[p] = f;
  h->heapos[f] = p;
  h->hmax = n-1;
  if (n >= 1)
    adjust_heap(f,h);
}

#define NOT_LINK(k)     ( (k) >= 0 )
#define GET_LINK(k)     ( - ((k)+2) )
#define LINK_END        ( -1 )

#define NOT_FLAGGED(k)  ( (k) < nedges )
#define DEL_FLAG(k)     ( (k) - nedges )
#define ADD_FLAG(k)     ( (k) + nedges )

static void clean(int v, int w, int *first, int *alist, P_Edge *edges, int nedges)
{ int p, j, s, t, k, h;

  p = -1;
  for (j = -(first[v]+2); j >= 0; j = GET_LINK(k))
    { s = t = j;
      for (k = DEL_FLAG(alist[j]); NOT_LINK(k) && NOT_FLAGGED(k); k = alist[++j])
        { int s1 = FIND(edges[k].region1,first);
          int s2 = FIND(edges[k].region2,first);
          if (edges[k].height >= 0)
            { if (s1 != w && s2 != w)
                alist[s++] = k;
              else
                edges[k].height = -(edges[k].height+1);
            }
        }
      if (NOT_LINK(k))
        k = LINK_END;
      if (s > t)
        { alist[s] = k; 
          h = alist[t];
          if (NOT_FLAGGED(h))
            alist[t] = ADD_FLAG(h);
          p = s;
        }
      else
        { if (p >= 0)
            alist[p] = k;
          else
            first[v] = k;
        }
    }
}

Map_Bundle *General_Collapse(Map_Bundle *R(O(map)), Partition *eshed, boolean dynamic, void *info,
                             tristate  (*decide)(int a, int ab, int b, void *info),
                             void      (*fuse)(int a, int ac, int c, int cb, int b, void *info),
                             boolean   (*compare)(int a, int ab, int b,
                                                  int c, int cd, int d, void *info) )
{ Wshed    *shed    = (Wshed *) eshed;
  int      *first   = shed->first;
  int      *alist   = shed->alist;
  P_Edge   *edges   = shed->edges;
  int       nbasins = shed->nbasins;
  int       nedges  = (first[nbasins] >> 1);

  int      *labels;
  int      *mark;
  Heap      hp, *h = &hp;

  int    v, w, s;
  int    e, f;

  if (nbasins == 1)
    return (NULL);

  { Size_Type n = 2*SIZEOF(int)*(nbasins+1);

    labels = map->mapto;
    if (labels == NULL)
      labels = (int *) Guarded_Malloc((size_t) n,"General_Collapse");
    else if (malloc_size(labels) < (size_t) n)
      labels = (int *) Guarded_Realloc(labels,(size_t) n,"General_Collapse");
    map->mapto = labels;
    memset(labels, 0, (size_t) n);
    mark = labels + (nbasins+1);
  }

  // Do UNION/FIND merge to determine sets

  for (v = 0; v < nbasins; v++)
    { int x = first[v];
      if (x < first[v+1])
        alist[x] = ADD_FLAG(alist[x]);
      first[v] = -(x+2);
      mark[v]  = -1;
    }
  alist[first[nbasins]] = nedges;

  { Size_Type n = 2*SIZEOF(int)*(nedges+1);

    hp.info    = info;
    hp.edges   = edges;
    hp.first   = first;
    hp.compare = compare;

    hp.edgeid  = (int *) Guarded_Malloc((size_t) n,"General_Collapse");
    hp.heapos  = hp.edgeid + (nedges+1);
    hp.hmax    = nedges;
    for (e = 1; e <= nedges; e++)
      { hp.edgeid[e] = e-1;
        hp.heapos[e-1] = e;
      }

    heapify(h);
  }

  while (h->hmax > 0)
    { f = pop_heap(h);

      v = FIND(edges[f].region1,first);
      w = FIND(edges[f].region2,first);

      if (v != w)
        { s = decide(v,f,w,info);
          if (s > 0)
            { int j, k, c, g;
              int wval;

              clean(v,w,first,alist,edges,nedges);
              clean(w,v,first,alist,edges,nedges);
              wval = first[w];
              first[w] = v;

              for (j = -(first[v]+2); j >= 0; j = GET_LINK(k))
                for (k = DEL_FLAG(alist[j]); NOT_LINK(k); k = alist[++j])
                  { c = FIND(edges[k].region1,first);
                    if (c == v)
                      c = FIND(edges[k].region2,first);
                    mark[c] = k;
                  }

              for (j = -(wval+2); j >= 0; j = GET_LINK(k))
                for (k = DEL_FLAG(alist[j]); NOT_LINK(k); k = alist[++j])
                  { c = FIND(edges[k].region1,first);
                    if (c == v)
                      c = FIND(edges[k].region2,first);
		    if ((g = mark[c]) >= 0)
                       { fuse(v,g,c,k,w,info);
                         remove_heap(k,h);
                         adjust_heap(g,h);
                         edges[k].height = -(edges[k].height+1);
                         mark[c] = -1;
                       }
                    else if (dynamic)
                      adjust_heap(k,h);
                  }

              g = -1;
              for (j = -(first[v]+2); j >= 0; j = GET_LINK(k))
                { for (k = DEL_FLAG(alist[j]); NOT_LINK(k); k = alist[++j])
                    { c = FIND(edges[k].region1,first);
                      if (c == v)
                        c = FIND(edges[k].region2,first);
                      if (mark[c] >= 0)
                        { if (dynamic)
                            adjust_heap(k,h);
                          mark[c] = -1;
                        }
                    }
                  g = j;
                }

              if (g < 0)
                first[v] = wval;
              else
                alist[g] = wval;
            }
          else if (s < 0)
            break;
        }
    }

  free(hp.edgeid);

  //  Given sets, establish map

  w = 1;
  for (v = 0; v < nbasins; v++)
    if (first[v] < 0)
      first[v] = -(w++);

  map->nrange = w-1;

  for (v = 0; v < nbasins; v++)
    labels[v] = -(first[FIND(v,first)]+1);

  // Restore the heights, alist, and first edge structure

  for (e = 0; e < nedges; e++)
    if (edges[e].height < 0)
      edges[e].height = -(edges[e].height+1);

  for (v = 0; v <= nbasins; v++)
    first[v] = 0;

  for (e = 0; e < nedges; e++)
    { first[edges[e].region1] += 1;
      first[edges[e].region2] += 1;
    }

  for (v = 1; v <= nbasins; v++)
    first[v] += first[v-1];

  for (e = 0; e < nedges; e++)
    { alist[--first[edges[e].region1]] = e;
      alist[--first[edges[e].region2]] = e;
    }
 
  return (map);
}


/**************************************************************************************\
*                                                                                      *
*  MERGE WATERSHEDS ALONG THE WATERSHED TREE TO PRODUCE A NEW WATERSHED OBJECT         *
*                                                                                      *
\**************************************************************************************/

Partition *G(Merge_Partition)(Partition *eshed, Map_Bundle *map,
                              Label_Array *C(label), boolean color)
{ Wshed    *shed    = (Wshed *) eshed;
  int      *labels  = map->mapto;
  int       cbasins = map->nrange;
  int       nbasins = shed->nbasins;
  P_Vertex *verts   = shed->verts;
  int      *first   = shed->first;
  int      *alist   = shed->alist;
  P_Edge   *edges   = shed->edges;

  Wshed        *result;
  int          *ilabels;
  P_Vertex     *cverts;
  int          *cfirst;
  int          *calist;
  P_Edge       *cedges;
  int           clabels;

  if (label != NULL)
    { if (shed->labels == NULL)
        { fprintf(stderr,"Partition does not have a label-field (Merge_Partition)\n");
          exit (1);
        }
      if ( ! Same_Shape(shed->labels,label))
        { fprintf(stderr,"Partition's label field and new label-field don't have the same shape");
          fprintf(stderr," (Merge_Partition)\n");
          exit (1);
        }
      if (label->type > UINT32_TYPE || label->kind != PLAIN_KIND)
        { fprintf(stderr,"Label array should be a PLAIN, UINT8-16-or-32 array (Merge_Partition)\n");
          exit (1);
        }
    }

  ilabels = labels + (nbasins+1);

  result = new_wshed(SIZEOF(P_Vertex)*(cbasins+1),0,0,SIZEOF(int)*(cbasins+1),"Merge_Partition");
  result->apart_ref = Inc_AForm(shed->apart_ref);
  result->nbasins   = cbasins;
  result->iscon2n   = shed->iscon2n;
  result->colored   = color && (label != NULL);
  result->labels    = label;

  cverts = result->verts;
  cfirst = result->first;

  { int c, b;   //  Establish inverse mapping to label

    for (c = 0; c < cbasins; c++)
      cverts[c].size = 0;

    for (b = 0; b < nbasins; b++)
      cverts[labels[b]].size += 1;

    for (c = 1; c < cbasins; c++)
      cverts[c].size += cverts[c-1].size;

    for (b = 0; b < nbasins; b++)
      ilabels[--cverts[labels[b]].size] = b;

    labels[nbasins]  = cbasins;
    ilabels[nbasins] = nbasins;
  }

#ifdef DEBUG_COLLAPSE
  { int b;

    printf("\nNew basins = %d\n",cbasins);
    for (b = 0; b < nbasins; b++)
      printf("  %3d -> %d (%d)\n",b,labels[b],ilabels[b]);
  }
#endif

  { int b, c, d, e, f, k;
    int p, q;
    int cnedge;

    //  Count the # of edges in the new graph (in cnedge).  Use cverts[?].seed as a mark bit

    cnedge = 0;
    for (c = 0; c < cbasins; c++)
      cverts[c].seed = -1;

    p = 0;
    for (c = 0; c < cbasins; c++)
      { cverts[c].seed = 1;
        for (q = p; labels[ilabels[q]] == c; q++) 
          { b = ilabels[q];
            for (e = first[b]; e < first[b+1]; e++)
              { k = alist[e];
                if (edges[k].region1 == b)
                  d = labels[edges[k].region2];
                else
                  d = labels[edges[k].region1];
                if (cverts[d].seed < 0)
                  { cverts[d].seed = 1;
                    cnedge += 1;
                  }
              }
          }
        for (q = p; labels[ilabels[q]] == c; q++) 
          { b = ilabels[q];
            for (e = first[b]; e < first[b+1]; e++)
              { k = alist[e];
                if (edges[k].region1 == b)
                  d = labels[edges[k].region2];
                else
                  d = labels[edges[k].region1];
                cverts[d].seed = -1;
              }
          }
        cverts[c].seed = -1;
        p = q;
      }

    cnedge >>= 1;

    //  Allocate the new edges;

    allocate_wshed_edges(result,SIZEOF(P_Edge)*cnedge,"Merge_Partition");
    allocate_wshed_alist(result,SIZEOF(int)*(2*cnedge+1),"Merge_Partition");
    cedges = result->edges;
    calist = result->alist;

    //  Same loop as above, but this time set the edge values

    p = f = 0;
    for (c = 0; c < cbasins; c++)
      { cverts[c].seed = 1;
        for (q = p; labels[ilabels[q]] == c; q++) 
          { b = ilabels[q];
            for (e = first[b]; e < first[b+1]; e++)
              { k = alist[e];
                if (edges[k].region1 == b)
                  d = labels[edges[k].region2];
                else
                  d = labels[edges[k].region1];
                if (d > c)
                  { if (cverts[d].seed < 0)
                      { cverts[d].seed = f;
                        cedges[f].region1 = c;
                        cedges[f].region2 = d;
                        cedges[f].height = edges[k].height;
                        f += 1;
                      }
                    else
                      { d = cverts[d].seed;
                        if (edges[k].height < cedges[d].height)
                          cedges[d].height = edges[k].height;
                      }
                  }
              }
          }

        for (q = p; labels[ilabels[q]] == c; q++) 
          { b = ilabels[q];
            for (e = first[b]; e < first[b+1]; e++)
              { k = alist[e];
                if (edges[k].region1 == b)
                  d = labels[edges[k].region2];
                else
                  d = labels[edges[k].region1];
                cverts[d].seed = -1;
              }
          }
        cverts[c].seed = -1;
        p = q;
      }

    // Set the alist and first edge structures

    for (c = 0; c <= cbasins; c++)
      cfirst[c] = 0;

    for (f = 0; f < cnedge; f++)
      { cfirst[cedges[f].region1] += 1;
        cfirst[cedges[f].region2] += 1;
      }

    for (c = 1; c <= cbasins; c++)
      cfirst[c] += cfirst[c-1];

    for (f = 0; f < cnedge; f++)
      { calist[--cfirst[cedges[f].region1]] = f;
        calist[--cfirst[cedges[f].region2]] = f;
      }
  }

  //  Color a new label field !  Use cverts[x].depth temporarily to encode label given to x

  if (label != NULL)
    { int j, c, e, d, k;
      int colored[0x10000];
      int sign;
      Value_Type   mint;
      Plain_Bundle _brush, *brush = &_brush;

      clabels = 0;
      if (color)
        { for (j = 1; j < 0x10000; j++)
            colored[j] = 0;

          for (c = 0; c < cbasins; c++)
            { for (e = cfirst[c]; e < cfirst[c+1]; e++)
                { k = calist[e];
                  if (cedges[k].region1 == c)
                    d = cedges[k].region2;
                  else
                    d = cedges[k].region1;
                  if (d < c)
                    colored[cverts[d].depth] = 1;
                }

              for (j = 1; j < 0x10000; j++)
                if ( ! colored[j])
                  break;
    
              if (j > clabels)
                { clabels = j;
                  if (j > 0xFFFF)
                    { fprintf(stderr,"Warning: More than %d colors needed ",0xFFFF);
                      fprintf(stderr,"to color partition, so will not do so.\n");
                      fflush(stderr);
                      color = 0;
                      break;
                    }
                }

              cverts[c].depth = j;
              for (e = cfirst[c]; e < cfirst[c+1]; e++)
                { k = calist[e];
                  if (cedges[k].region1 == c)
                    d = cedges[k].region2;
                  else
                    d = cedges[k].region1;
                  if (d < c)
                    colored[cverts[d].depth] = 0;
                }
            }
        }

      mint = UINT8_TYPE;
      if (label != shed->labels)
        { sign = 1;
          if (clabels > 0xFFFF)
            mint = UINT32_TYPE;
          else if (clabels > 0xFF)
            mint = UINT16_TYPE;
        }
      else
        { sign = -1;
          if (clabels > 0x7FFF)
            mint = UINT32_TYPE;
          else if (clabels > 0x7F)
            mint = UINT16_TYPE;
        }
      if (!color)
        mint = UINT32_TYPE;
          
      if (label->type < mint)
        { if (mint == UINT16_TYPE)
            Convert_Array_Inplace(label,PLAIN_KIND,mint,16,0);
          else
            Convert_Array_Inplace(label,PLAIN_KIND,mint,32,0);
        }
      
      if (color)
        { result->nlabels = clabels;
          brush->op       = SET_PIX;
          for (c = 0; c < nbasins; c++)
            { brush->val.ival = (int64) (sign*cverts[labels[c]].depth);
              Draw_P_Vertex(label,brush,shed,c,0);
            }
        }
      else
        { result->nlabels = cbasins;
          brush->op       = SET_PIX;
          for (c = 0; c < nbasins; c++)
            { brush->val.ival = (uint64) (sign*(labels[c]+1));
              Draw_P_Vertex(label,brush,shed,c,0);
            }
        }

      if (sign < 0)
        switch (label->type) {
          case UINT8_TYPE:
            { int8      *out = AINT8(label);
              Indx_Type p;

              if (Same_Shape(AForm_Array(shed->apart_ref),label) && Is_Slice(shed->apart_ref))
                { Slice    *frame = shed->apart_ref;
                  Indx_Type q, r;
  
                  r = Slice_Index(frame);
                  p = Set_Slice_To_First(frame);
                  for (q = 0; q < label->size; q++)
                    { out[p] = -out[p];
                      p = Next_Slice_Index(frame);
                    }
                  Set_Slice_To_Index(frame,r);
                }
              else
                for (p = 0; p < label->size; p++)
                  out[p] = -out[p];
              break;
            }
          case UINT16_TYPE:
            { int16      *out = AINT16(label);
              Indx_Type p;

              if (Same_Shape(AForm_Array(shed->apart_ref),label) && Is_Slice(shed->apart_ref))
                { Slice    *frame = shed->apart_ref;
                  Indx_Type q, r;
  
                  r = Slice_Index(frame);
                  p = Set_Slice_To_First(frame);
                  for (q = 0; q < label->size; q++)
                    { out[p] = -out[p];
                      p = Next_Slice_Index(frame);
                    }
                  Set_Slice_To_Index(frame,r);
                }
              else
                for (p = 0; p < label->size; p++)
                  out[p] = -out[p];
              break;
            }
          case UINT32_TYPE:
            { int32      *out = AINT32(label);
              Indx_Type p;

              if (Same_Shape(AForm_Array(shed->apart_ref),label) && Is_Slice(shed->apart_ref))
                { Slice    *frame = shed->apart_ref;
                  Indx_Type q, r;
  
                  r = Slice_Index(frame);
                  p = Set_Slice_To_First(frame);
                  for (q = 0; q < label->size; q++)
                    { out[p] = -out[p];
                      p = Next_Slice_Index(frame);
                    }
                  Set_Slice_To_Index(frame,r);
                }
              else
                for (p = 0; p < label->size; p++)
                  out[p] = -out[p];
              break;
            }
           default:
             break;
        }
    }

#ifdef DEBUG_COLLAPSE_PRINT
  { int i, j;

    for (j = 0; j < cheight; j++)
      { for (i = 0; i < cwidth; i++)
          printf(" %d",result->labels->array[j*cwidth+i]);
        printf("\n");
      }
  }
#endif

  //  Compute sizes, seeds, and depths of new CBs

  { int c, b;
    int area;

    area = 0x7FFFFFFF;

    for (c = 0; c < cbasins; c++)
      { cverts[c].seed  = area;
        cverts[c].size  = 0;
        cverts[c].depth = 0x10000;
      }

    for (b = 0; b < nbasins; b++)
      { c = labels[b];
        if (verts[b].seed < cverts[c].seed)
          cverts[c].seed = verts[b].seed;
        if (verts[b].depth < cverts[c].depth)
          cverts[c].depth = verts[b].depth;
        cverts[c].size += verts[b].size;
      }
  }

#ifdef DEBUG_COLLAPSE
  { int i, j, k;

    printf("\nCollapsed graph:\n");
    for (i = 0; i < cbasins; i++)
      { j = cverts[i].seed;
        printf("Basin %d[",i);
        Print_Coord(stdout,Idx2CoordA(shed->labels,j));
        printf("] -> ");
        for (j = cfirst[i]; j < cfirst[i+1]; j++)
          { k = calist[j];
            if (cedges[k].region1 == i)
              printf(" %d(%d)",cedges[k].region2,cedges[k].height);
            else
              printf(" %d(%d)",cedges[k].region1,cedges[k].height);
          }
        printf("\n");
      }
  }
#endif

  return ((Partition *) result);
}

/**************************************************************************************\
*                                                                                      *
*  PRODUCE A COLORED IMAGE OF A WATERSHED DECOMPOSITION                                *
*                                                                                      *
\**************************************************************************************/

static double red[3]      = {  1.,  0.,  0. };
static double green[3]    = {  0.,  1.,  0. };
static double blue[3]     = {  0.,  0.,  1. };
static double yellow[3]   = {  1.,  1.,  0. };
static double cyan[3]     = {  0.,  1.,  1. };
static double magenta[3]  = {  1.,  0.,  1. };
static double orange[3]   = {  1.,  .5,  0. };
static double brown[3]    = {  1., .25,  .5 };
static double *palette[8] = { magenta, red, green, yellow, cyan, orange, brown, blue };

Array *Draw_Partition(Array *R(M(canvas)), Partition *shed, double alpha)
{ APart       *part  = WS(shed)->apart_ref;
  Array       *array = AForm_Array(part);
  double       beta  = 1.-alpha;
  double       loblack[3], lopal[8][3], *color;

  int          map;
  Size_Type    size, span;
  Indx_Type    curp = 0;

  if (canvas->kind != RGB_KIND)
    { fprintf(stderr,"Canavs must be RGB (Draw_Partition)\n");
      exit (1);
    }
  if (WS(shed)->labels == NULL)
    { fprintf(stderr,"Partition does not have a label-field (Draw_Partition)\n");
      exit (1);
    }

  { Array_Bundle _cshape, *cshape = &_cshape;

    _cshape = *canvas;
    cshape  = Get_Array_Plane(cshape,0);

    if (Same_Shape(WS(shed)->labels,cshape))
      map = (Is_Slice(part) && ! Same_Shape(part,WS(shed)->labels));
    else if (Same_Shape(part,cshape))
      map = 2;
    else
      { if ( ! Same_Shape(AForm_Array(part),cshape))
          { fprintf(stderr,"Partition and canvas don't have compatible shapes");
            fprintf(stderr," (Draw_Partition)\n");
            exit (1);
          }
        map = 3;
      }
  }

  /* Cases:
       map   part   label   canvas
       0     array  array   array    p -> p  over p
       0     slice  slice   slice   pi -> pi over pi (use p)
       1     slice  array   array    p -> p  over pi
       2     slice  array   slice    p -> pi over pi
       3     slice  slice   array   pi -> p  over pi
  */

  size  = AForm_Size(part);
  if (map == 0 || map == 2)
    span = size;
  else
    span = array->size;
  if (map != 0)
    curp = Slice_Index(part);


  switch (WS(shed)->labels->type) {
    case UINT8_TYPE:
    case  INT8_TYPE:
      { uint8 *basin = AUINT8(WS(shed)->labels);
 
        switch (canvas->type) {
            case UINT8_TYPE:
              { uint8 *red   = AUINT8(canvas);
                uint8 *green = red + span;
                uint8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint8) (color[0] + red[pi] * beta);
                           green[pi] = (uint8) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT16_TYPE:
              { uint16 *red   = AUINT16(canvas);
                uint16 *green = red + span;
                uint16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint16) (color[0] + red[pi] * beta);
                           green[pi] = (uint16) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT32_TYPE:
              { uint32 *red   = AUINT32(canvas);
                uint32 *green = red + span;
                uint32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint32) (color[0] + red[pi] * beta);
                           green[pi] = (uint32) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT64_TYPE:
              { uint64 *red   = AUINT64(canvas);
                uint64 *green = red + span;
                uint64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint64) (color[0] + red[pi] * beta);
                           green[pi] = (uint64) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT8_TYPE:
              { int8 *red   = AINT8(canvas);
                int8 *green = red + span;
                int8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int8) (color[0] + red[pi] * beta);
                           green[pi] = (int8) (color[1] + green[pi] * beta);
                           blue[pi]  = (int8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT16_TYPE:
              { int16 *red   = AINT16(canvas);
                int16 *green = red + span;
                int16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int16) (color[0] + red[pi] * beta);
                           green[pi] = (int16) (color[1] + green[pi] * beta);
                           blue[pi]  = (int16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT32_TYPE:
              { int32 *red   = AINT32(canvas);
                int32 *green = red + span;
                int32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int32) (color[0] + red[pi] * beta);
                           green[pi] = (int32) (color[1] + green[pi] * beta);
                           blue[pi]  = (int32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT64_TYPE:
              { int64 *red   = AINT64(canvas);
                int64 *green = red + span;
                int64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int64) (color[0] + red[pi] * beta);
                           green[pi] = (int64) (color[1] + green[pi] * beta);
                           blue[pi]  = (int64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT32_TYPE:
              { float32 *red   = AFLOAT32(canvas);
                float32 *green = red + span;
                float32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = FLT_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float32) (color[0] + red[pi] * beta);
                           green[pi] = (float32) (color[1] + green[pi] * beta);
                           blue[pi]  = (float32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT64_TYPE:
              { float64 *red   = AFLOAT64(canvas);
                float64 *green = red + span;
                float64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = DBL_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float64) (color[0] + red[pi] * beta);
                           green[pi] = (float64) (color[1] + green[pi] * beta);
                           blue[pi]  = (float64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
        }
        break;
      }
    case UINT16_TYPE:
    case  INT16_TYPE:
      { uint16 *basin = AUINT16(WS(shed)->labels);
 
        switch (canvas->type) {
            case UINT8_TYPE:
              { uint8 *red   = AUINT8(canvas);
                uint8 *green = red + span;
                uint8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint8) (color[0] + red[pi] * beta);
                           green[pi] = (uint8) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT16_TYPE:
              { uint16 *red   = AUINT16(canvas);
                uint16 *green = red + span;
                uint16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint16) (color[0] + red[pi] * beta);
                           green[pi] = (uint16) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT32_TYPE:
              { uint32 *red   = AUINT32(canvas);
                uint32 *green = red + span;
                uint32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint32) (color[0] + red[pi] * beta);
                           green[pi] = (uint32) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT64_TYPE:
              { uint64 *red   = AUINT64(canvas);
                uint64 *green = red + span;
                uint64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint64) (color[0] + red[pi] * beta);
                           green[pi] = (uint64) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT8_TYPE:
              { int8 *red   = AINT8(canvas);
                int8 *green = red + span;
                int8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int8) (color[0] + red[pi] * beta);
                           green[pi] = (int8) (color[1] + green[pi] * beta);
                           blue[pi]  = (int8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT16_TYPE:
              { int16 *red   = AINT16(canvas);
                int16 *green = red + span;
                int16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int16) (color[0] + red[pi] * beta);
                           green[pi] = (int16) (color[1] + green[pi] * beta);
                           blue[pi]  = (int16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT32_TYPE:
              { int32 *red   = AINT32(canvas);
                int32 *green = red + span;
                int32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int32) (color[0] + red[pi] * beta);
                           green[pi] = (int32) (color[1] + green[pi] * beta);
                           blue[pi]  = (int32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT64_TYPE:
              { int64 *red   = AINT64(canvas);
                int64 *green = red + span;
                int64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int64) (color[0] + red[pi] * beta);
                           green[pi] = (int64) (color[1] + green[pi] * beta);
                           blue[pi]  = (int64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT32_TYPE:
              { float32 *red   = AFLOAT32(canvas);
                float32 *green = red + span;
                float32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = FLT_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float32) (color[0] + red[pi] * beta);
                           green[pi] = (float32) (color[1] + green[pi] * beta);
                           blue[pi]  = (float32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT64_TYPE:
              { float64 *red   = AFLOAT64(canvas);
                float64 *green = red + span;
                float64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = DBL_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float64) (color[0] + red[pi] * beta);
                           green[pi] = (float64) (color[1] + green[pi] * beta);
                           blue[pi]  = (float64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
        }
        break;
      }
    case UINT32_TYPE:
    case  INT32_TYPE:
      { uint32 *basin = AUINT32(WS(shed)->labels);
 
        switch (canvas->type) {
            case UINT8_TYPE:
              { uint8 *red   = AUINT8(canvas);
                uint8 *green = red + span;
                uint8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint8) (color[0] + red[pi] * beta);
                           green[pi] = (uint8) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint8) (color[0] + red[p] * beta);
                           green[p] = (uint8) (color[1] + green[p] * beta);
                           blue[p]  = (uint8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT16_TYPE:
              { uint16 *red   = AUINT16(canvas);
                uint16 *green = red + span;
                uint16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint16) (color[0] + red[pi] * beta);
                           green[pi] = (uint16) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint16) (color[0] + red[p] * beta);
                           green[p] = (uint16) (color[1] + green[p] * beta);
                           blue[p]  = (uint16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT32_TYPE:
              { uint32 *red   = AUINT32(canvas);
                uint32 *green = red + span;
                uint32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint32) (color[0] + red[pi] * beta);
                           green[pi] = (uint32) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint32) (color[0] + red[p] * beta);
                           green[p] = (uint32) (color[1] + green[p] * beta);
                           blue[p]  = (uint32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case UINT64_TYPE:
              { uint64 *red   = AUINT64(canvas);
                uint64 *green = red + span;
                uint64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = UINT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (uint64) (color[0] + red[pi] * beta);
                           green[pi] = (uint64) (color[1] + green[pi] * beta);
                           blue[pi]  = (uint64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (uint64) (color[0] + red[p] * beta);
                           green[p] = (uint64) (color[1] + green[p] * beta);
                           blue[p]  = (uint64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT8_TYPE:
              { int8 *red   = AINT8(canvas);
                int8 *green = red + span;
                int8 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT8_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int8) (color[0] + red[pi] * beta);
                           green[pi] = (int8) (color[1] + green[pi] * beta);
                           blue[pi]  = (int8) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int8) (color[0] + red[p] * beta);
                           green[p] = (int8) (color[1] + green[p] * beta);
                           blue[p]  = (int8) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT16_TYPE:
              { int16 *red   = AINT16(canvas);
                int16 *green = red + span;
                int16 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT16_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int16) (color[0] + red[pi] * beta);
                           green[pi] = (int16) (color[1] + green[pi] * beta);
                           blue[pi]  = (int16) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int16) (color[0] + red[p] * beta);
                           green[p] = (int16) (color[1] + green[p] * beta);
                           blue[p]  = (int16) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT32_TYPE:
              { int32 *red   = AINT32(canvas);
                int32 *green = red + span;
                int32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT32_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int32) (color[0] + red[pi] * beta);
                           green[pi] = (int32) (color[1] + green[pi] * beta);
                           blue[pi]  = (int32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int32) (color[0] + red[p] * beta);
                           green[p] = (int32) (color[1] + green[p] * beta);
                           blue[p]  = (int32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case INT64_TYPE:
              { int64 *red   = AINT64(canvas);
                int64 *green = red + span;
                int64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = INT64_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (int64) (color[0] + red[pi] * beta);
                           green[pi] = (int64) (color[1] + green[pi] * beta);
                           blue[pi]  = (int64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (int64) (color[0] + red[p] * beta);
                           green[p] = (int64) (color[1] + green[p] * beta);
                           blue[p]  = (int64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT32_TYPE:
              { float32 *red   = AFLOAT32(canvas);
                float32 *green = red + span;
                float32 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = FLT_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float32) (color[0] + red[pi] * beta);
                           green[pi] = (float32) (color[1] + green[pi] * beta);
                           blue[pi]  = (float32) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float32) (color[0] + red[p] * beta);
                           green[p] = (float32) (color[1] + green[p] * beta);
                           blue[p]  = (float32) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
            case FLOAT64_TYPE:
              { float64 *red   = AFLOAT64(canvas);
                float64 *green = red + span;
                float64 *blue  = green + span;
      
                { int i, j;
                  for (i = 0; i < 8; i++)
                   for (j = 0; j < 3; j++)
                    lopal[i][j] = DBL_MAX * palette[i][j] * alpha;
                  for (j = 0; j < 3; j++)
                    loblack[j] = 0;
                }
      
                switch (map) {
                   case 0:
                     { Indx_Type p;					
                       for (p = 0; p < size; p++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                         }
                       break;
                     }
                   case 1:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 2:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[p] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[p]%8];
                           red[pi]   = (float64) (color[0] + red[pi] * beta);
                           green[pi] = (float64) (color[1] + green[pi] * beta);
                           blue[pi]  = (float64) (color[2] + blue[pi] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                   case 3:
                     { Indx_Type p, pi;					
                       p = Set_Slice_To_First(part);			
                       for (pi = 0; pi < size; pi++)
                         { if (basin[pi] <= 0)
                             color = loblack;
                           else
                             color = lopal[basin[pi]%8];
                           red[p]   = (float64) (color[0] + red[p] * beta);
                           green[p] = (float64) (color[1] + green[p] * beta);
                           blue[p]  = (float64) (color[2] + blue[p] * beta);
                           p = Next_Slice_Index(part);		
                         }
                       break;
                     }
                }
                break;
              }
        }
        break;
      }
    default:
      fprintf(stderr,"Partition label field is not integer (Draw_Partition)\n");
      exit (1);
  }

  if (map != 0)
    Set_Slice_To_Index(part,curp);

  return (canvas);
}


/**************************************************************************************\
*                                                                                      *
*  SEEDED WATERSHED VARIATION                                                          *
*                                                                                      *
\**************************************************************************************/

boolean SEED_COLLAPSE(int lft, int hgt, int rgt, void *info)
{ int *in = (int *) info;
  (void) hgt;
  in[lft] = (in[lft] || in[rgt]);
  return ( ! (in[lft] && in[rgt]));
}

Partition *G(Build_Seeded_Watershed)(Pixel_APart *I(image), boolean iscon2n,
                                     boolean color, Vector *seeds)
{ int         *in;
  Label_Array *labels;
  Partition   *rshed;
  Partition   *fshed;
  Map_Bundle   map;
  int          nbasins;

  if (seeds->ndims != 1)
    { fprintf(stderr,"Seeds should be a vector (Build_Seeded_Watershed)\n");
      exit (1);
    }
  if (seeds->type != INDX_TYPE)
    { fprintf(stderr,"Seeds must be Indx_Type indices into image array (Build_Seeded_Watershed)\n");
      exit (1);
    }

  { Dimn_Type  i;
    Indx_Type *a = AINDX(seeds);

    if (Is_Slice(image))
      { Indx_Type  curp;
        curp = Slice_Index(image);
        for (i = 0; i < seeds->dims[0]; i++)
          if ( ! Set_Slice_To_Index(image,a[i]))
            { fprintf(stderr,"Seeds must be indices into image slice (Build_Seeded_Watershed)\n");
              exit (1);
            }
        Set_Slice_To_Index(image,curp);
      }
    else
      { Size_Type  size = AForm_Size(image);
        for (i = 0; i < seeds->dims[0]; i++)
          if (a[i] >= size)
            { fprintf(stderr,"Seeds must be indices into image array (Build_Seeded_Watershed)\n");
              exit (1);
            }
      }
  }

  labels = Make_Array_With_Shape(PLAIN_KIND,INT32_TYPE,AForm_Shape(image));

  Label_Watershed(image,labels,&nbasins,iscon2n);

  in = Guarded_Malloc(sizeof(int)*((size_t) nbasins),"Build_Seeded_Watershed");

  { int        i;
    Indx_Type *a = AINDX(seeds);
    int32     *b = AINT32(labels);

    for (i = 0; i < nbasins; i++)
      in[i] = 0;
    for (i = 0; i < seeds->dims[0]; i++)
      in[b[a[i]]] = 1;
  }

  rshed = Make_Partition(image,labels,nbasins,iscon2n,1);

  map.mapto = NULL;
  Static_Collapse(&map,rshed,(void *) in,SEED_COLLAPSE);

  fshed = Merge_Partition(rshed,&map,Inc_Array(labels),color);
  Set_Partition_Labels(rshed,NULL);

  if (Get_Partition_Color_Count(fshed) < 256)
    Convert_Array_Inplace(labels,PLAIN_KIND,UINT8_TYPE,8,0);
  else if (Get_Partition_Color_Count(fshed) < 0x10000)
    Convert_Array_Inplace(labels,PLAIN_KIND,UINT16_TYPE,16,0);
  Pack_Array(labels);

  free(map.mapto);
  Kill_Partition(rshed);
  free(in);

  return (fshed);
}
