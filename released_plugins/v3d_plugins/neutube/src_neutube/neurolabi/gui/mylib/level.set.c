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
*  Level Set Tree Abstraction                                                             *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007 (from August 2006 version)                                          *
*  Mods  :  Dec  2008, generalized to arrays of any dimension                             *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "utilities.h"
#include "connectivity.h"
#include "image.h"
#include "level.set.h"

#undef DEBUG_REGIONS

/****************************************************************************************
 *                                                                                      *
 *  LEVEL SET BRANCHING DATA STRUCTURES AND OBJECT MANAGER                              *
 *                                                                                      *
 ****************************************************************************************/

/* Binary tree of region fusion events */

typedef struct
  { int    left;   /* region is union of left and right regions:             */
    int    right;  /*   if > 0 then index of subtree, else - of single pixel */
    int    size;   /* # of pixels in region subtree                          */
    int    start;  /* Leftmost pixel in subtree (for finding outer contour)  */
    uint16 level;  /* Pixel value at which the two regions were joined       */
    uint16 peak;   /* Max pixel value in subtree rooted at this node         */
  } vertex;

/* Internal representation of a component tree */

typedef struct
  { APart  *apart_ref;   //  Reference to the image or slice level tree was made from
    vertex *level_tree;  //  Memory block containing the vertices of the tree
    boolean iscon2n;     //  Connectivity used in building the component tree
    vertex *regtrees;    //  Binary tree of islands = level_tree-1;
    int     csize;       //  Size of array
    boolean is8;         //  Array is 8-bit
    uint8  *value8;      //  Pixel values of array (undefined if is8 == 0)
    uint16 *value16;     //  Pixel values of array (undefined if is8 != 0)
  } Comtree;

#define CT(a) ((Comtree *) (a))
#define VT(p) ((vertex  *) (p))

#define SIZEOF(x) ((int) sizeof(x))

static inline int comtree_lsize(Comtree *t)
{ return (t->csize * SIZEOF(vertex)); }


typedef struct __Comtree
  { struct __Comtree *next;
    struct __Comtree *prev;
    int               refcnt;
    int               lsize;
    Comtree           comtree;
  } _Comtree;

static _Comtree *Free_Comtree_List = NULL;
static _Comtree *Use_Comtree_List  = NULL;

static pthread_mutex_t Comtree_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Comtree_Offset = sizeof(_Comtree)-sizeof(Comtree);
static int Comtree_Inuse  = 0;

int Level_Tree_Refcount(Level_Tree *level_tree)
{ _Comtree *object = (_Comtree *) (((char *) level_tree) - Comtree_Offset);
  return (object->refcnt);
}

static inline int allocate_comtree_level_tree(Comtree *comtree, int lsize, char *routine)
{ _Comtree *object = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  if (object->lsize < lsize)
    { void *x = Guarded_Realloc(comtree->level_tree,(size_t) lsize,routine);
      if (x == NULL) return (1);
      comtree->level_tree = x;
      object->lsize = lsize;
    }
  return (0);
}

static inline int sizeof_comtree_level_tree(Comtree *comtree)
{ _Comtree *object = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  return (object->lsize);
}

static inline void kill_comtree(Comtree *comtree);

static inline Comtree *new_comtree(int lsize, char *routine)
{ _Comtree *object;
  Comtree  *comtree;

  pthread_mutex_lock(&Comtree_Mutex);
  if (Free_Comtree_List == NULL)
    { object = (_Comtree *) Guarded_Realloc(NULL,sizeof(_Comtree),routine);
      if (object == NULL) return (NULL);
      comtree = &(object->comtree);
      object->lsize = 0;
      comtree->level_tree = NULL;
    }
  else
    { object = Free_Comtree_List;
      Free_Comtree_List = object->next;
      comtree = &(object->comtree);
    }
  Comtree_Inuse += 1;
  object->refcnt = 1;
  if (Use_Comtree_List != NULL)
    Use_Comtree_List->prev = object;
  object->next = Use_Comtree_List;
  object->prev = NULL;
  Use_Comtree_List = object;
  pthread_mutex_unlock(&Comtree_Mutex);
  comtree->apart_ref = NULL;
  if (allocate_comtree_level_tree(comtree,lsize,routine))
    { kill_comtree(comtree);
      return (NULL);
    }
  return (comtree);
}

static inline Comtree *copy_comtree(Comtree *comtree)
{ Comtree *copy = new_comtree(comtree_lsize(comtree),"Copy_Level_Tree");
  void *_level_tree = copy->level_tree;
  *copy = *comtree;
  copy->level_tree = _level_tree;
  if (comtree->level_tree != NULL)
    memcpy(copy->level_tree,comtree->level_tree,(size_t) comtree_lsize(comtree));
  if (comtree->apart_ref != NULL)
    Inc_AForm(comtree->apart_ref);
  return (copy);
}

Level_Tree *Copy_Level_Tree(Level_Tree *level_tree)
{ return ((Level_Tree *) copy_comtree(((Comtree *) level_tree))); }

static inline int pack_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  if (object->lsize > comtree_lsize(comtree))
    { int ns = comtree_lsize(comtree);
      if (ns != 0)
        { void *x = Guarded_Realloc(comtree->level_tree,(size_t) ns,"Pack_Comtree");
          if (x == NULL) return (1);
          comtree->level_tree = x;
        }
      else
        { free(comtree->level_tree);
          comtree->level_tree = NULL;
        }
      object->lsize = ns;
    }
  return (0);
}

Level_Tree *Inc_Level_Tree(Level_Tree *level_tree)
{ _Comtree *object  = (_Comtree *) (((char *) level_tree) - Comtree_Offset);
  pthread_mutex_lock(&Comtree_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Comtree_Mutex);
  return (level_tree);
}

static inline void free_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  pthread_mutex_lock(&Comtree_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Comtree_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Level_Tree\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Comtree_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Comtree_List;
  Free_Comtree_List = object;
  Comtree_Inuse -= 1;
  pthread_mutex_unlock(&Comtree_Mutex);
  if (comtree->apart_ref != NULL)
    { Free_AForm(comtree->apart_ref);
      comtree->apart_ref = NULL;
    }
}

void Free_Level_Tree(Level_Tree *level_tree)
{ free_comtree(((Comtree *) level_tree)); }

static inline void kill_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  pthread_mutex_lock(&Comtree_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Comtree_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Level_Tree\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Comtree_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Comtree_Inuse -= 1;
  pthread_mutex_unlock(&Comtree_Mutex);
  if (comtree->apart_ref != NULL)
    Kill_AForm(comtree->apart_ref);
  if (comtree->level_tree != NULL)
    free(comtree->level_tree);
  free(((char *) comtree) - Comtree_Offset);
}

void Kill_Level_Tree(Level_Tree *level_tree)
{ kill_comtree(((Comtree *) level_tree)); }

static inline void reset_comtree()
{ _Comtree *object;
  Comtree  *comtree;
  pthread_mutex_lock(&Comtree_Mutex);
  while (Free_Comtree_List != NULL)
    { object = Free_Comtree_List;
      Free_Comtree_List = object->next;
      comtree = &(object->comtree);
      if (comtree->level_tree != NULL)
        free(comtree->level_tree);
      free(object);
    }
  pthread_mutex_unlock(&Comtree_Mutex);
}

void Reset_Level_Tree()
{ reset_comtree(); }

int Level_Tree_Usage()
{ return (Comtree_Inuse); }

void Level_Tree_List(void (*handler)(Level_Tree *))
{ _Comtree *a, *b;
  for (a = Use_Comtree_List; a != NULL; a = b)
    { b = a->next;
      handler((Level_Tree *) &(a->comtree));
    }
}

static inline Comtree *read_comtree(FILE *input)
{ char name[10];
  Comtree *obj;
  Comtree read;
  fread(name,10,1,input);
  if (strncmp(name,"Level_Tree",10) != 0)
    return (NULL);
  obj = new_comtree(0,"Read_Level_Tree");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Comtree),1,input) == 0) goto error;
  obj->level_tree = read.level_tree;
  if (comtree_lsize(obj) != 0)
    { if (allocate_comtree_level_tree(obj,comtree_lsize(obj),"Read_Level_Tree")) goto error;
      if (fread(obj->level_tree,(size_t) comtree_lsize(obj),1,input) == 0) goto error;
    }
  obj->apart_ref = NULL;
  return (obj);

error:
  kill_comtree(obj);
  return (NULL);
}

Level_Tree *Read_Level_Tree(FILE *input)
{ return ((Level_Tree *) read_comtree(input)); }

static inline void write_comtree(Comtree *comtree, FILE *output)
{ fwrite("Level_Tree",10,1,output);
  fwrite(comtree,sizeof(Comtree),1,output);
  if (comtree_lsize(comtree) != 0)
    fwrite(comtree->level_tree,(size_t) comtree_lsize(comtree),1,output);
}

void Write_Level_Tree(Level_Tree *level_tree, FILE *output)
{ write_comtree(((Comtree *) level_tree),output); }

Level_Tree *Pack_Level_Tree(Level_Tree *a)
{ boolean nok = pack_comtree(CT(a));
  CT(a)->regtrees = CT(a)->level_tree - 1;
  if (nok) return (NULL);
  return (a);
}


/****************************************************************************************
 *                                                                                      *
 *  LEVEL TREE GET AND SET-ROUTINES                                                     *
 *                                                                                      *
 ****************************************************************************************/


boolean Get_Level_Tree_Connectivity(Level_Tree *a)
{ return (CT(a)->iscon2n); }

APart *Get_Level_Tree_APart(Level_Tree *a)
{ return (CT(a)->apart_ref); }

void Set_Level_Tree_APart(Level_Tree *a, Pixel_APart *I(part))
{ if (CT(a)->apart_ref != part)
    { if (CT(a)->apart_ref != NULL)
        Free_AForm(CT(a)->apart_ref);
      if (AForm_Size(part) != CT(a)->csize)
        { fprintf(stderr,"Size of array or slice and level tree don't match");
          fprintf(stderr," (Set_Level_Tree_APart)\n");
          exit (1);
        }
      CT(a)->apart_ref = Inc_AForm(part);
    }
  if (CT(a)->is8)
    CT(a)->value8  = AUINT8 (AForm_Array(part));
  else
    CT(a)->value16 = AUINT16(AForm_Array(part));
}

/****************************************************************************************
 *                                                                                      *
 *  TREE TRAVERSAL ROUTINES                                                             *
 *                                                                                      *
 ****************************************************************************************/

static inline int size(Comtree *t, int cont)
{ if (cont > 0)
    return (t->regtrees[cont].size);
  else
    return (1);
}

static inline int level(Comtree *t, int cont)
{ if (cont > 0)
    return (t->regtrees[cont].level);
  else if (t->is8)
    return (t->value8[-cont]);
  else
    return (t->value16[-cont]);
}

static inline int peak(Comtree *t, int cont)
{ if (cont > 0)
    return (t->regtrees[cont].peak);
  else if (t->is8)
    return (t->value8[-cont]);
  else
    return (t->value16[-cont]);
}

static inline int start(Comtree *t, int cont)
{ if (cont > 0)
    return (t->regtrees[cont].start);
  else
    return (-cont);
}

Level_Set *Level_Set_Child(Level_Tree *a, Level_Set *r)
{ vertex *p;
  int     x;

  x = ((vertex *) r)->right;
  if (x <= 0)
    return (NULL);
  p = CT(a)->regtrees + x;
  if (p->right <= 0)
    { if (CT(a)->is8)
        { if (CT(a)->value8[-p->right] == p->level)
            return (NULL);
        }
      else
        { if (CT(a)->value16[-p->right] == p->level)
            return (NULL);
        }
    }
  return ((Level_Set *) p);
}

Level_Set *Level_Set_Sibling(Level_Tree *a, Level_Set *r)
{ vertex *p;
  int     x;

  x = ((vertex *) r)->left;
  if (x <= 0)
    return (NULL);
  p = CT(a)->regtrees + x;
  if (p->right <= 0)
    { if (CT(a)->is8)
        { if (CT(a)->value8[-p->right] == p->level)
            return (NULL);
        }
      else
        { if (CT(a)->value16[-p->right] == p->level)
            return (NULL);
        }
    }
  return ((Level_Set *) p);
}

int Level_Set_Size(Level_Tree *a, Level_Set *r)
{ return ( size( CT(a), VT(r)->right) ); }

int Level_Set_Level(Level_Tree *a, Level_Set *r)
{ return ( level( CT(a), VT(r)->right) ); }

int Level_Set_Peak(Level_Tree *a, Level_Set *r)
{ return ( peak( CT(a), VT(r)->right) ); }

int Level_Set_Leftmost(Level_Tree *a, Level_Set *r)
{ return ( start( CT(a), VT(r)->right) ); }

int Level_Set_Background(Level_Tree *a, Level_Set *r)
{ (void) a;
  return ( VT(r)->level );
}

int Level_Set_Id(Level_Tree *a, Level_Set *r)
{ return ( (int) (VT(r) - CT(a)->regtrees) ); }

Level_Set *Level_Set_Root(Level_Tree *a)
{ return ((Level_Set *) (CT(a)->regtrees + CT(a)->csize)); }

  /* List every pixel in the subtree rooted at p */

static void list_level_set(Comtree *t, int p, void *arg, void (*handler)(Indx_Type,void *))
{ if (p <= 0)
    handler((Indx_Type) (-p),arg);
  else
    { while (p > 0)
        { list_level_set(t,t->regtrees[p].right,arg,handler);
          p = t->regtrees[p].left;
        }
      list_level_set(t,p,arg,handler);
    }
}

void List_Level_Set(Level_Tree *a, Level_Set *r, void *arg, void (*handler)(Indx_Type p,void * arg))
{ list_level_set( CT(a), VT(r)->right, arg, handler); }


/****************************************************************************************
 *                                                                                      *
 *  COMPONENT TREE FUSION AND FLATTENING ROUTINES                                       *
 *                                                                                      *
 ****************************************************************************************/

/* Find root of pixel x in union/find tree and compress path */

static int find(int *father, int x)
{ int y, z;

  y = x;
  while ((z = father[y]) >= 0)
    y = z;
  z = father[x];
  while (z >= 0)
    { father[x] = y;
      x = z;
      z = father[x];
    }
  return (y);
}

/* Union the regions containing pixels p and q *if they are in different
   regions, noting it happened at level c using "node" to model the new
   region.  Return node+1 if the union took place, and node otherwise.     */

static int fuse(int node, int p, int q, int c, int *father, int *set, Comtree *tree, int cwidth)
{ int x, y;

  x = find(father,p);
  y = find(father,q);
  if (x != y)
    { int     s,  t;
      int     m,  n;
      int     u,  v;
      vertex *b;

      s = set[x];
      t = set[y];
      m = size(tree,s);
      n = size(tree,t);

      b = tree->regtrees + node;
      b->left  = s;
      b->right = t;
      b->size  = m + n;
      b->level = (uint16) c;

      u = peak(tree,s);
      v = peak(tree,t);
      if (u < v)
        b->peak = (uint16) v;
      else
        b->peak = (uint16) u;

      u = start(tree,s);
      v = start(tree,t);
      if (u%cwidth < v%cwidth)
        b->start = u;
      else
        b->start = v;

      if (m < n)
        { father[x] = y;
          set[y]    = node;
        }
      else
        { father[y] = x;
          set[x]    = node;
        }

      node += 1;
    }
  return (node);
}

/* Flatten the merge tree so that all nodes at a given level are organized
   in a linear chain with .left field ended by a leaf at the given level (there
   must always be at least one).  The .right fields points to the level below or
   to pixels at the given level.  These are organized so that all the pixels
   that are part of the level set are in the later half of the chain.  The reason
   that the chain cannot be NULL terminated is because there are n nodes to build
   a chain of n+1 elements (A full binary tree with n interior elements has n+1
   leaves).                                                                       */

static int flatten_tree(Comtree *tree, int p)
{ int     final, cache;
  vertex *regtrees = tree->regtrees;

  if (p <= 0) return (p);

  { int r, t, l;                    //  Flatten the current level
    int size, level, peak, start;
    int stack;

    size  = regtrees[p].size;
    level = regtrees[p].level;
    peak  = regtrees[p].peak;
    start = regtrees[p].start;
  
    final = 0;
    r     = p;
    stack = 0;
    while (r > 0 && regtrees[r].level == level)
      { t = regtrees[r].left;
        regtrees[r].left = stack;
        stack = r;
        r = t;
      }
    cache = r;

    while (stack > 0)
      { l = regtrees[stack].left;
        r = regtrees[stack].right;
        regtrees[stack].left  = final;
        regtrees[stack].right = cache;
        final = stack;
        stack = l;

        while (r > 0 && regtrees[r].level == level)
          { t = regtrees[r].left;
            regtrees[r].left = stack;
            stack = r;
            r = t;
          }
        cache = r;
      } 

    regtrees[final].size  = size;
    regtrees[final].level = (uint16) level;
    regtrees[final].peak  = (uint16) peak;
    regtrees[final].start = start;
  }

  { int p, q;        //  Flatten descendants and cap the chain (with cache)

    for (p = final; 1; p = q)
      { regtrees[p].right = flatten_tree(tree,regtrees[p].right);
        if ((q = regtrees[p].left) <= 0)
          { regtrees[p].left = flatten_tree(tree,cache);
            break;
          }
      }
  }

  { int c, f;      //  Permute the descendants so that pixels at this level are at the end
    int p, r;

    f = final;
    c = regtrees[final].level;
    switch (tree->is8) {
        case 1:
          { uint8 *value8 = tree->value8;
            for (p = final; 1; p = r)
              { r = regtrees[p].right;
                if (r > 0 || value8[-r] != c)
                  { int x = regtrees[f].right;
                    regtrees[f].right = r;
                    regtrees[p].right = x; 
                    f = regtrees[f].left;
                  }
                r = regtrees[p].left;
                if (r <= 0 || regtrees[r].level != c)
                  { if (r > 0 || value8[-r] != c)
                      { int x = regtrees[f].right;
                        regtrees[f].right = r;
                        regtrees[p].left  = x;
                      }
                    break;
                  }
              }
            break;
          }
        case 0:
          { uint16 *value16 = tree->value16;
            for (p = final; 1; p = r)
              { r = regtrees[p].right;
                if (r > 0 || value16[-r] != c)
                  { int x = regtrees[f].right;
                    regtrees[f].right = r;
                    regtrees[p].right = x; 
                    f = regtrees[f].left;
                  }
                r = regtrees[p].left;
                if (r <= 0 || regtrees[r].level != c)
                  { if (r > 0 || value16[-r] != c)
                      { int x = regtrees[f].right;
                        regtrees[f].right = r;
                        regtrees[p].left  = x;
                      }
                    break;
                  }
              }
            break;
          }
    }
  }

  return (final);
}


/****************************************************************************************
 *                                                                                      *
 *  BUILD COMPONENT_TREE, THE MAIN ROUTINE                                              *
 *                                                                                      *
 ****************************************************************************************/

Level_Tree *G(Build_Level_Tree)(Pixel_APart *source, boolean iscon2n)
{ Array     *array = AForm_Array(source);
  Indx_Type  curp = 0;
  boolean    islice;

  int        bucket[0x10001];
  int       *father;
  int       *set;

  Offs_Type *neighbor;
  Grid_Id    grid;
  int        n_nbrs;

  Comtree   *ctree;
  int        cdimN;
  int        cwidth;
  int        csize;
  int        maxval;
  uint8     *value8 = NULL;
  uint16    *value16 = NULL;
  vertex    *regtrees;

  if (array->type != UINT8_TYPE && array->type != UINT16_TYPE)
    { fprintf(stderr,"Build_Level_Tree: Applies only to UINT8 and UINT16 arrays\n");
      exit (1);
    }
  if (array->kind != PLAIN_KIND)
    { fprintf(stderr,"Build_Level_Tree: Applies only to PLAIN arrays\n");
      exit (1);
    }
  if (AForm_Size(source) > 0x7FFFFFFC)   //  Not ...FF in order to accommodate sneaky trick below
    { fprintf(stderr,"Build_Level_Tree: Array is larger than 1G pixel\n");
      exit (1);
    }

  cdimN    = array->ndims;
  cwidth   = array->dims[0];
  csize    = (int) AForm_Size(source);
  if (array->type == UINT8_TYPE)
    { maxval  = 0x100;
      value8  = AUINT8(array);
    }
  else
    { maxval  = 0x10000;
      value16 = AUINT16(array);
    }

  ctree = new_comtree(csize*SIZEOF(vertex),"Build_Level_Tree");
  ctree->apart_ref = Inc_AForm(source);
  ctree->iscon2n   = iscon2n;

  ctree->regtrees  = regtrees = ctree->level_tree - 1;
  ctree->csize     = csize;
  ctree->is8       = (array->type == UINT8_TYPE);
  ctree->value8    = value8;
  ctree->value16   = value16;
  
  father = (int *) Guarded_Malloc(sizeof(int)*2*((size_t) csize),"Build_Level_Tree");
  set    = father + csize;

  islice = Is_Slice(source);
  if (islice)
    { Coordinate  *basis;
      Array_Bundle sbdl;

      curp = Slice_Index(source);

      basis = AForm_Shape(source);
      sbdl.dims  = ADIMN(basis);
      sbdl.ndims = basis->dims[0];
      sbdl.kind  = PLAIN_KIND;

      grid = Setup_Grid(&sbdl,"Build_Level_Tree");

      Free_Array(basis);
    }
  else
    grid = Setup_Grid(source,"Build_Level_Tree");
  n_nbrs   = Grid_Size(grid,iscon2n);
  neighbor = Grid_Neighbors(grid,iscon2n);

  // Establish lists of pixels with each value
  // Very sneaky: Use father as a link field for buckets and also for the union-find.
  //   I get away with this because a pixel is not used in the union find until it is
  //   taken off the bucket list.  However, I need to know that a pixel has been removed
  //   from the bucket list (in order to know it is greater in value than the current
  //   pixel and hence should be fused with it).  Therefore, I must use distinct ranges
  //   to encode the links:
  //       union-find: father[x] in [-1,csize-1] and is the index of its father (-1 if none)
  //       bucket list: father[x] in [-2,-csize-2] and -3-father[x] is the index of the
  //                       next element in the list (-1 if none).

  { int p, q, v;

    for (v = 0; v <= maxval; v++)
      bucket[v] = -2;

    switch (array->type) {
        case UINT8_TYPE:
          if (islice)
            { q = (int) Set_Slice_To_Last(source);
              for (p = 0; p < csize; p++)
                { set[p]    = -q;
                  v         = value8[q];
                  father[p] = bucket[v];
                  bucket[v] = -3-p;
                  q = (int) Prev_Slice_Index(source);
                }
            }
          else
            { for (p = 0; p < csize; p++)
                { set[p]    = -p;
                  v         = value8[p];
                  father[p] = bucket[v];
                  bucket[v] = -3-p;
                }
            }
          break;
        case UINT16_TYPE:
          if (islice)
            { q = (int) Set_Slice_To_Last(source);
              for (p = 0; p < csize; p++)
                { set[p]    = -q;
                  v         = value16[q];
                  father[p] = bucket[v];
                  bucket[v] = -3-p;
                  q = (int) Prev_Slice_Index(source);
                }
            }
          else
            { for (p = 0; p < csize; p++)
                { set[p]    = -p;
                  v         = value16[p];
                  father[p] = bucket[v];
                  bucket[v] = -3-p;
                }
            }
          break;
        default:
          break;   //  Can't happen as checked against earlier
    }
  }

  { int p, c, node;

    if (cdimN != 2 && cdimN != 3)
      cdimN = 4;

    node = 1;
    switch (cdimN) {
        case 2:
          for (c = maxval-1; c >= 0; c--)
            { p = -3-bucket[c];
              while (p >= 0)
                { boolean *b;
                  int   j, q, r;

                  r = -3-father[p];
                  father[p] = -1;

                  b = Boundary_Pixels_2d(grid,(Indx_Type) p,iscon2n);
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        if (father[q] >= -1)
                          node = fuse(node,p,q,c,father,set,ctree,cwidth);
                      }
                  p = r;
                }
            }
          break;
        case 3:
          for (c = maxval-1; c >= 0; c--)
            { p = -3-bucket[c];
              while (p >= 0)
                { boolean *b;
                  int   j, q, r;

                  r = -3-father[p];
                  father[p] = -1;

                  b = Boundary_Pixels_3d(grid,(Indx_Type) p,iscon2n);
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        if (father[q] >= -1)
                          node = fuse(node,p,q,c,father,set,ctree,cwidth);
                      }
                  p = r;
                }
            }
          break;
        case 4:
          for (c = maxval-1; c >= 0; c--)
            { p = -3-bucket[c];
              while (p >= 0)
                { boolean *b;
                  int   j, q, r;

                  r = -3-father[p];
                  father[p] = -1;

                  b = Boundary_Pixels(grid,(Indx_Type) p,iscon2n);
                  for (j = 0; j < n_nbrs; j++)
                    if (b[j])
                      { q = p + (int) neighbor[j];
                        if (father[q] >= -1)
                          node = fuse(node,p,q,c,father,set,ctree,cwidth);
                      }
                  p = r;
                }
            }
          break;
    }
  }

  regtrees[csize].right = flatten_tree(ctree,csize-1);
  regtrees[csize].left  = 0;
  regtrees[csize].level = 0;

  free(father);

  Release_Grid(grid);

  if (islice)
    Set_Slice_To_Index(source,curp);

#ifdef DEBUG_REGIONS
  printf("\nDecomposition of plane:\n");
  Print_Level_Tree(ctree,0,stdout);
  fflush(stdout);
#endif

  return ((Level_Tree *) ctree);
}


/****************************************************************************************
 *                                                                                      *
 *  PRINT A COMPONENT TREE                                                              *
 *                                                                                      *
 ****************************************************************************************/

/* Print the level set subtree rooted at r, indented "indent" spaces to file "file" */

typedef struct
  { int         indent;
    FILE       *file;
    Level_Tree *tree;
    int         idxW;
    int         valW;
  } Print_Data;

static void _print_regions(Print_Data *data, Level_Set *r, int nest)
{ Level_Tree *a = data->tree;

  fprintf(data->file,"%*s%*d:%*s  Element %*d size=%*d) [%*d .. %*d .. %*d]\n",
                     data->indent,"",data->idxW,nest+1,nest,"",
                     data->idxW,Level_Set_Id(a,r),data->idxW,Level_Set_Size(a,r),
                     data->valW,Level_Set_Background(a,r),
                     data->valW,Level_Set_Level(a,r),
                     data->valW,Level_Set_Peak(a,r));
  fflush(data->file);
  for (r = Level_Set_Child(a,r); r != NULL; r = Level_Set_Sibling(a,r))
    _print_regions(data,r,nest+1);
}

void Print_Level_Tree(Level_Tree *tree, int indent, int idwidth, int valwidth, FILE *file)
{ Print_Data data;

  data.indent = indent;
  data.file   = file;
  data.tree   = tree;
  data.idxW   = idwidth;
  data.valW   = valwidth;
  _print_regions(&data,Level_Set_Root(tree),0);
}
