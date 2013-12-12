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
#include <ctype.h>

#include "utilities.h"
#include "array.h"
#include "swc.h"

#define SWC_LINE_MAX      1000     //  Maximum # of characters in a line of an SWC file
#define SWC_COMMENT_CHAR  '#'      //  Symbol that starts comments in an SWC file

static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };


/****************************************************************************************
 *                                                                                      *
 *  SWC TREE SPACE MANAGEMENT ROUTINES AND PRIMARY GENERATOR                            *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { SWC_Kind kind;         //  1st 5 fields are the same as an SWC_Node, the part of a node
    float    x;            //    a user sees
    float    y;
    float    z;
    float    r;
    int32    father;       //  offset (in SWC's) of father (or 0 if none)
    int32    children;     //  offset (in SWC's) of children (or 0 if none)
    int32    sibling;      //  offset (in SWC's) of sibling (or 0 if none)
  } SWC;                   //  children pts at a singly-linked 'sibling' list of the children
                           //    of this node
typedef struct
  { int    nverts;       //  number of vertices in tree
    int32  root;         //  root of tree (0 for all trees produced by this module)
    SWC   *nodes;        //  memory block containing the SWC's of the tree
  } Stree;   

#define SIZEOF(x) ((int) sizeof(x))

static inline int stree_nsize(Stree *t)
{ return (SIZEOF(SWC)*t->nverts); }


typedef struct __Stree
  { struct __Stree *next;
    struct __Stree *prev;
    int             refcnt;
    int             nsize;
    Stree           stree;
  } _Stree;

static _Stree *Free_Stree_List = NULL;
static _Stree *Use_Stree_List  = NULL;

static pthread_mutex_t Stree_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Stree_Offset = sizeof(_Stree)-sizeof(Stree);
static int Stree_Inuse  = 0;

int SWC_Tree_Refcount(SWC_Tree *swc_tree)
{ _Stree *object = (_Stree *) (((char *) swc_tree) - Stree_Offset);
  return (object->refcnt);
}

static inline int allocate_stree_nodes(Stree *stree, int nsize, char *routine)
{ _Stree *object = (_Stree *) (((char *) stree) - Stree_Offset);
  if (object->nsize < nsize)
    { void *x = Guarded_Realloc(stree->nodes,(size_t) nsize,routine);
      if (x == NULL) return (1);
      stree->nodes = x;
      object->nsize = nsize;
    }
  return (0);
}

static inline int sizeof_stree_nodes(Stree *stree)
{ _Stree *object = (_Stree *) (((char *) stree) - Stree_Offset);
  return (object->nsize);
}

static inline void kill_stree(Stree *stree);

static inline Stree *new_stree(int nsize, char *routine)
{ _Stree *object;
  Stree  *stree;

  pthread_mutex_lock(&Stree_Mutex);
  if (Free_Stree_List == NULL)
    { object = (_Stree *) Guarded_Realloc(NULL,sizeof(_Stree),routine);
      if (object == NULL) return (NULL);
      stree = &(object->stree);
      object->nsize = 0;
      stree->nodes = NULL;
    }
  else
    { object = Free_Stree_List;
      Free_Stree_List = object->next;
      stree = &(object->stree);
    }
  Stree_Inuse += 1;
  object->refcnt = 1;
  if (Use_Stree_List != NULL)
    Use_Stree_List->prev = object;
  object->next = Use_Stree_List;
  object->prev = NULL;
  Use_Stree_List = object;
  pthread_mutex_unlock(&Stree_Mutex);
  if (allocate_stree_nodes(stree,nsize,routine))
    { kill_stree(stree);
      return (NULL);
    }
  return (stree);
}

static inline Stree *copy_stree(Stree *stree)
{ Stree *copy = new_stree(stree_nsize(stree),"Copy_SWC_Tree");
  void *_nodes = copy->nodes;
  *copy = *stree;
  copy->nodes = _nodes;
  if (stree->nodes != NULL)
    memcpy(copy->nodes,stree->nodes,(size_t) stree_nsize(stree));
  return (copy);
}

SWC_Tree *Copy_SWC_Tree(SWC_Tree *swc_tree)
{ return ((SWC_Tree *) copy_stree(((Stree *) swc_tree))); }

static inline int pack_stree(Stree *stree)
{ _Stree *object  = (_Stree *) (((char *) stree) - Stree_Offset);
  if (object->nsize > stree_nsize(stree))
    { int ns = stree_nsize(stree);
      if (ns != 0)
        { void *x = Guarded_Realloc(stree->nodes,(size_t) ns,"Pack_Stree");
          if (x == NULL) return (1);
          stree->nodes = x;
        }
      else
        { free(stree->nodes);
          stree->nodes = NULL;
        }
      object->nsize = ns;
    }
  return (0);
}

SWC_Tree *Pack_SWC_Tree(SWC_Tree *swc_tree)
{ if (pack_stree(((Stree *) swc_tree))) return (NULL);
  return (swc_tree);
}

SWC_Tree *Inc_SWC_Tree(SWC_Tree *swc_tree)
{ _Stree *object  = (_Stree *) (((char *) swc_tree) - Stree_Offset);
  pthread_mutex_lock(&Stree_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Stree_Mutex);
  return (swc_tree);
}

static inline void free_stree(Stree *stree)
{ _Stree *object  = (_Stree *) (((char *) stree) - Stree_Offset);
  pthread_mutex_lock(&Stree_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Stree_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SWC_Tree\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Stree_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Stree_List;
  Free_Stree_List = object;
  Stree_Inuse -= 1;
  pthread_mutex_unlock(&Stree_Mutex);
}

void Free_SWC_Tree(SWC_Tree *swc_tree)
{ free_stree(((Stree *) swc_tree)); }

static inline void kill_stree(Stree *stree)
{ _Stree *object  = (_Stree *) (((char *) stree) - Stree_Offset);
  pthread_mutex_lock(&Stree_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Stree_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SWC_Tree\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Stree_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Stree_Inuse -= 1;
  pthread_mutex_unlock(&Stree_Mutex);
  if (stree->nodes != NULL)
    free(stree->nodes);
  free(((char *) stree) - Stree_Offset);
}

void Kill_SWC_Tree(SWC_Tree *swc_tree)
{ kill_stree(((Stree *) swc_tree)); }

static inline void reset_stree()
{ _Stree *object;
  Stree  *stree;
  pthread_mutex_lock(&Stree_Mutex);
  while (Free_Stree_List != NULL)
    { object = Free_Stree_List;
      Free_Stree_List = object->next;
      stree = &(object->stree);
      if (stree->nodes != NULL)
        free(stree->nodes);
      free(object);
    }
  pthread_mutex_unlock(&Stree_Mutex);
}

void Reset_SWC_Tree()
{ reset_stree(); }

int SWC_Tree_Usage()
{ return (Stree_Inuse); }

void SWC_Tree_List(void (*handler)(SWC_Tree *))
{ _Stree *a, *b;
  for (a = Use_Stree_List; a != NULL; a = b)
    { b = a->next;
      handler((SWC_Tree *) &(a->stree));
    }
}

static inline Stree *read_stree(FILE *input)
{ char name[8];
  Stree *obj;
  Stree read;
  fread(name,8,1,input);
  if (strncmp(name,"SWC_Tree",8) != 0)
    return (NULL);
  obj = new_stree(0,"Read_SWC_Tree");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Stree),1,input) == 0) goto error;
  obj->nodes = read.nodes;
  if (stree_nsize(obj) != 0)
    { if (allocate_stree_nodes(obj,stree_nsize(obj),"Read_SWC_Tree")) goto error;
      if (fread(obj->nodes,(size_t) stree_nsize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_stree(obj);
  return (NULL);
}

SWC_Tree *Read_SWC_Tree(FILE *input)
{ return ((SWC_Tree *) read_stree(input)); }

static inline void write_stree(Stree *stree, FILE *output)
{ fwrite("SWC_Tree",8,1,output);
  fwrite(stree,sizeof(Stree),1,output);
  if (stree_nsize(stree) != 0)
    fwrite(stree->nodes,(size_t) stree_nsize(stree),1,output);
}

void Write_SWC_Tree(SWC_Tree *swc_tree, FILE *output)
{ write_stree(((Stree *) swc_tree),output); }


/****************************************************************************************
 *                                                                                      *
 *  SWC TREE AND NODE ACCESS ROUTINES                                                   *
 *                                                                                      *
 ****************************************************************************************/

int Get_SWC_Size(SWC_Tree *t)
{ return (((Stree *) t)->nverts); }

SWC_Node *Get_SWC_Node(SWC_Tree *t, int i)
{ Stree *ti = (Stree *) t;
  if (i < 0 || i >= ti->nverts)
    { fprintf(stderr,"Get_SWC_Node: index not in range\n");
      exit (1);
    }
  return ((SWC_Node *) (ti->nodes+i));
}

SWC_Node *Get_SWC_Root(SWC_Tree *t)
{ Stree *ti = (Stree *) t;
  int    o  = ti->root;
  if (ti->root < 0)
    return (NULL);
  else
    return ((SWC_Node *) (ti->nodes + o));
}

SWC_Node *Get_SWC_Father(SWC_Node *n)
{ SWC   *ni = (SWC *) n;
  int32   o = ni->father;
  if (o == 0)
    return (NULL);
  else
    return ((SWC_Node *) (ni + o));
}

SWC_Node *Get_SWC_Child(SWC_Node *n)
{ SWC   *ni = (SWC *) n;
  int32   o = ni->children;
  if (o == 0)
    return (NULL);
  else
    return ((SWC_Node *) (ni + o));
}

SWC_Node *Get_SWC_Sibling(SWC_Node *n)
{ SWC   *ni = (SWC *) n;
  int32   o = ni->sibling;
  if (o == 0)
    return (NULL);
  else
    return ((SWC_Node *) (ni + o));
}

int Get_SWC_Outdegree(SWC_Node *n)
{ SWC   *ni = (SWC *) n;
  int32   o = ni->children;
  int   cnt = 0;
  while (o != 0)
    { cnt += 1;
      ni  += o;
      o   = ni->sibling;
    }
  return (cnt);
}


/****************************************************************************************
 *                                                                                      *
 *  SWC TREE IO ROUTINES                                                                *
 *                                                                                      *
 ****************************************************************************************/

static int size_of_tree(SWC *v)     //  Compute the size (in nodes) of the subtree rooted at v
{ int s, c;
 
  s = 1;
  c = v->children;
  while (c != 0)
    { v += c;
      s += size_of_tree(v);
      c = v->sibling;
    }
  return (s);
}

static int copy_tree(SWC *v, SWC *t)   //  Make a copy of the subtree rooted at v in preorder
{ int s, d, c;                         //    starting at the location t.
 
  *t = *v;

  s = 1;
  c = v->children;
  if (c != 0)
    { t->children = 1;
      d = 1;
      while (c != 0)
        { v += c;
          t += d;
          d  = copy_tree(v,t);
          t->father  = -s;
          t->sibling = d;
          s += d;
          c  = v->sibling;
        }
      t->sibling = 0;
    }
  else
    t->children = 0;
  return (s);
}

SWC_Bundle *Read_SWC_File(SWC_Bundle *R(G(forest)), string file_name)
{ char  Buffer[SWC_LINE_MAX+3];
  int   BufLen  = SWC_LINE_MAX+3;
  char *BufTest = Buffer + (SWC_LINE_MAX+1);

  FILE  *input;
  int    nverts, ntrees, lcount;
  Stree *tree;

  input = fopen(file_name,"r");
  if (input == NULL)
    { fprintf(stderr,"Cannot open %s for reading (Read_SWC_File)\n",file_name);
      exit (1);
    }

  //  Make a pass through the file count the number of roots and vertices
  //    in the file.  Check for errors in the input.

  *BufTest = '\0';
  nverts   = 0;
  ntrees   = 0;
  lcount   = 0;
  while (fgets(Buffer,BufLen,input) != NULL)
    { char    *s;
      int      label, father;
      int      kind;
      double   x, y, z, r;

      if (*BufTest != '\0')
        { fprintf(stderr,"File line longer than %d chars (Read_SWC_File)\n",SWC_LINE_MAX);
          exit (1);
        }
      lcount += 1;

      s = Buffer;
      while (isspace(*s))
        s += 1;
      if (*s == SWC_COMMENT_CHAR || *s == '\0') continue;

      if (sscanf(Buffer," %d %d %lg %lg %lg %lg %d ",&label,&kind,&x,&y,&z,&r,&father) != 7)
        { fprintf(stderr,"Line %d of file %s is not a valid SWC line (Read_SWC_File)\n",
                          lcount,file_name);
          exit (1);
        }

      nverts += 1;
      if (father < 0)
        ntrees += 1;
    }

  //  Make a temporary super tree that will contain all the vertices (of all trees in the file)
  //    and rewind the file in preparation for the next pass

  tree = new_stree(SIZEOF(SWC)*nverts,"Read_SWC_File");
  tree->nverts = nverts;

  rewind(input);

  //  On this pass read the data into the SWC's of tree.  No error checking is required.
  //    A vertex in the file with label "lab" gets put at the first unused position after
  //    lab%nverts, i.e. effectively a clumsy hash (the works brilliantly in the commmon
  //    case where labels are consecutive from 1 upwards).  Note that the labels are not
  //    part of an SWC tree internally and that we use a temporary array "labels" to map
  //    the labels to indices.

  { SWC *nodes = tree->nodes;
    int *labels;

    // Set up the labels map

    { int k;

      labels = (int *) Guarded_Malloc(sizeof(int)*((size_t) nverts),"Read_SWC_File");
      for (k = 0; k < nverts; k++)
        labels[k] = -1;
    }

    // Make the read pass (initializing children and sibling to -1)

    lcount = 0;
    while (fgets(Buffer,BufLen,input) != NULL)
      { char  *s;
        int    id, lab, kind;
        SWC   *n;

        lcount += 1;

        s = Buffer;
        while (isspace(*s))
          s += 1;
        if (*s == SWC_COMMENT_CHAR || *s == '\0') continue;
  
        sscanf(Buffer," %d",&lab);
        id = lab % nverts;
        while (labels[id] >= 0)
          { if (labels[id] == lab)
              { fprintf(stderr,"Label %d not unique, line %d (Read_SWC_File)\n",lab,lcount);
                exit (1);
              }
            id += 1;
            if (id >= nverts)
              id = 0;
          }
        labels[id] = lab;

        n = nodes+id;
        sscanf(Buffer," %d %d %g %g %g %g %d ",
               &lab,&kind,&(n->x),&(n->y),&(n->z),&(n->r),&(n->father));
        n->kind     = (SWC_Kind) kind;
        n->children = -1;
        n->sibling  = -1;
      }

    // Map father labels to indices

    { int k, id, lab;

      for (k = 0; k < nverts; k++)
        { lab = nodes[k].father;
          if (lab < 0)
            id = -1;
          else
            { id = lab % nverts;
              while (labels[id] != lab)
                { id += 1;
                  if (id >= nverts)
                    id = 0;
                }
            }
          nodes[k].father = id;
        }
    }

    free(labels);   //  The map is no longer required now that father links are mapped
  }

  //  Use the father links to establish the children/sibling lists in reverse preorder
  //  Then in a second sweep reverse these lists so they are in preorder!

  { SWC *nodes = tree->nodes;
    int k, f, p, s;
 
    for (k = 0; k < nverts; k++)
      { f = nodes[k].father;
        if (f >= 0)
          { nodes[k].sibling  = nodes[f].children;
            nodes[f].children = k;
          }
      }

    for (k = 0; k < nverts; k++)
      { f = nodes[k].children;
        if (f >= 0)
          { p = -1;
            while (f >= 0)
              { s = nodes[f].sibling;
                nodes[f].sibling = p;
                p = f;
                f = s;
              }
            nodes[k].children = p;
          }
      }
  }

  //  Up to now indices have been in absolute terms, make them all relative to the vertex
  //    containing the index.

  { SWC *nodes = tree->nodes;
    int k, h;
 
    for (k = 0; k < nverts; k++)
      { h = nodes[k].father;
        if (h < 0)
          nodes[k].father = 0;
        else
          nodes[k].father = h-k;
	h = nodes[k].children;
        if (h < 0)
          nodes[k].children = 0;
        else
	  nodes[k].children = h-k;
	h = nodes[k].sibling;
        if (h < 0)
          nodes[k].sibling = 0;
        else
	  nodes[k].sibling = h-k;
      }
  }

  //  Prepare the bundle for the trees to be extracted from trees

  { int i;
    if (forest->num_trees >= ntrees)
      { for (i = ntrees; i < forest->num_trees; i++)
          Kill_SWC_Tree(forest->trees[i]);
        for (i = ntrees-1; i >= 0; i--)
          Free_SWC_Tree(forest->trees[i]);
      }
    else
      { for (i = forest->num_trees-1; i >= 0; i--)
          Free_SWC_Tree(forest->trees[i]);
        forest->trees = (SWC_Tree **)
                     Guarded_Malloc(sizeof(SWC_Tree *)*((size_t) ntrees),"Read_SWC_File");
        if (forest->trees == NULL)
          { fprintf(stderr,"Out of memory (Read_SWC_File)\n");
            exit (1);
          }
      }
    forest->num_trees = ntrees;
  }

  //  Extract each tree from trees, putting it in its own SWC_Tree structure, then kill tree
      
  { SWC *nodes = tree->nodes;
    int  k;

    ntrees = 0;
    for (k = 0; k < nverts; k++)
      if (nodes[k].father == 0)
        { Stree *copy;
          int    size;

          size = size_of_tree(nodes+k);
          copy = new_stree(SIZEOF(SWC)*((int) (1.1*size+50)),"Read_SWC_File");

          copy->nverts = size;
          copy->root   = 0;
          copy_tree(nodes+k,copy->nodes);
          copy->nodes[0].father = 0;

          forest->trees[ntrees++] = (SWC_Tree *) copy;
        }

    Kill_SWC_Tree(tree);
  }

  return (forest);
}

  // Write subtree rooted a v in pre-order in SWC format, where label of v is id

static int write_swc_tree(FILE *output, SWC *v, int id)
{ int s, c;
 
  fprintf(output,"%d %d %g %g %g %g",id,v->kind,v->x,v->y,v->z,v->r);
  if (v->father == 0)
    fprintf(output," -1\n");
  else
    fprintf(output," %d\n",id + v->father);
  s = 1;
  c = v->children;
  while (c != 0)
    { v += c;
      s += write_swc_tree(output,v,id+s);
      c = v->sibling;
    }
  return (s);
}

boolean Write_SWC_File(string file_name, SWC_Bundle *forest)
{ int    k, s;
  FILE  *output;
  Stree *tree;

  output = fopen(file_name,"w");
  if (output == NULL)
    return (1);
  
  s = 1;
  for (k = 0; k < forest->num_trees; k++)    //  Write out a header followed by each tree
    { tree = (Stree *) forest->trees[k];
      fprintf(output,"\n# Tree %d (%d nodes):\n\n",k,tree->nverts);
      s += write_swc_tree(output,tree->nodes+tree->root,s);
    }
  return (0);
}

  // Print the non-branching branches of an SWC in a nested tree fasion where indentation
  //   is proportional to branch depth in tree.  Also draw connectors to help one see this
  //   connection structure.  The buffer Branches contains the connector symbols.


static void print_swc_tree(FILE *output, int indent, string format, int level, SWC *v, string bnch)
{ int c;

  while (1)
    { fprintf(output,"(");
      fprintf(output,format,v->x);
      fprintf(output,",");
      fprintf(output,format,v->y);
      fprintf(output,",");
      fprintf(output,format,v->z);
      fprintf(output,"):");
      fprintf(output,format,v->r);
      fprintf(output,"\n");
      c = v->children;
      if (c == 0)
        { fprintf(output,"%s\n",bnch);
          break;
        }
      v += c;
      if (v->sibling != 0)
        { fprintf(output,"%s|\n",bnch);
          do
            { fprintf(output,"%s+->",bnch);
              c = v->sibling;
              if (c != 0)
                strcat(bnch,"|  ");
              else
                strcat(bnch,"   ");
              print_swc_tree(output,indent,format,level+3,v,bnch);
              bnch[level] = '\0';
              v = v + c;
            }
          while (c != 0);
          break;
        }
      fprintf(output,"%s",bnch);
    }
}

void Print_SWC_Tree(SWC_Tree *tree, FILE *output, int indent, string format)
{ char Branches[1000];
  sprintf(Branches,"%*s",indent,"");
  fprintf(output,"%s",Branches);
  print_swc_tree(output,indent,format,indent,((Stree *) tree)->nodes,Branches);
}


/****************************************************************************************
 *                                                                                      *
 *  SWC TREE CONSTRUCTION ROUTINES                                                      *
 *                                                                                      *
 ****************************************************************************************/

SWC_Tree *Make_SWC_Tree(int size_estimate)
{ Stree *t;

  t = new_stree(SIZEOF(SWC)*size_estimate,"Make_SWC_Tree");
  t->nverts = 0;
  t->root   = 0;
  return ((SWC_Tree *) t);
}

SWC_Node *Add_Vertex_As_Root(SWC_Tree *tree,
                             SWC_Kind kind, double x, double y, double z, double r)
{ Stree *t = (Stree *) tree;
  SWC   *w;

  t->nverts = 1;
  t->root   = 0;

  w = t->nodes;
  w->kind     = kind;
  w->x        = (float) x;
  w->y        = (float) y;
  w->z        = (float) z;
  w->r        = (float) r;
  w->father   = 0;
  w->children = 0;
  w->sibling  = 0;

  return ((SWC_Node *) w);
}

SWC_Node *Add_Vertex_As_First_Child(SWC_Tree *tree, SWC_Node *node,
                                    SWC_Kind kind, double x, double y, double z, double r)
{ Stree *t = (Stree *) tree;
  SWC   *v = (SWC *) node;
  SWC   *w;
  int    n;

  n = t->nverts++;
  if (stree_nsize(t) > sizeof_stree_nodes(t))
    allocate_stree_nodes(t,SIZEOF(SWC)*((int) (1.1*n+50)),"Add_Vertex_As_First_Child");

  w = t->nodes + n;

  w->kind     = kind;
  w->x        = (float) x;
  w->y        = (float) y;
  w->z        = (float) z;
  w->r        = (float) r;
  w->father   = (int) (v-w);
  w->children = 0;
  if (v->children == 0)
    w->sibling = 0;
  else
    w->sibling = (int) ((v+v->children)-w);
  v->children = (int) (w-v);

  return ((SWC_Node *) w);
}

SWC_Node *Add_Vertex_As_Sibling(SWC_Tree *tree, SWC_Node *node,
                                    SWC_Kind kind, double x, double y, double z, double r)
{ Stree *t = (Stree *) tree;
  SWC   *v = (SWC *) node;
  SWC   *w;
  int    n;

  if (v->father == 0)
    { fprintf(stderr,"Trying to add vertex as sibling of root (Add_Vertex_As_Sibling)\n");
      exit (1);
    }

  n = t->nverts++;
  if (stree_nsize(t) > sizeof_stree_nodes(t))
    allocate_stree_nodes(t,SIZEOF(SWC)*((int) (1.1*n+50)),"Add_Vertex_As_Sibling");

  w = t->nodes + n;

  w->kind     = kind;
  w->x        = (float) x;
  w->y        = (float) y;
  w->z        = (float) z;
  w->r        = (float) r;
  w->father   = (int) ((v+v->father)-w);
  w->children = 0;
  if (v->sibling == 0)
    w->sibling = 0;
  else
    w->sibling = (int) ((v+v->sibling)-w);
  v->sibling = (int) (w-v);

  return ((SWC_Node *) w);
}

SWC_Node *Add_Vertex_As_Father(SWC_Tree *tree, SWC_Node *node,
                               SWC_Kind kind, double x, double y, double z, double r)
{ Stree *t = (Stree *) tree;
  SWC   *v = (SWC *) node;
  SWC   *w;
  int    n;

  n = t->nverts++;
  if (stree_nsize(t) > sizeof_stree_nodes(t))
    allocate_stree_nodes(t,SIZEOF(SWC)*((int) (1.1*n+50)),"Add_Vertex_As_Father");

  w = t->nodes + n;

  w->kind     = kind;
  w->x        = (float) x;
  w->y        = (float) y;
  w->z        = (float) z;
  w->r        = (float) r;

  if (v->father == 0)
    { v->father   = (int) (w-v);
      w->father   = 0;
      w->children = (int) (v-w);
      w->sibling  = 0;
    }
  else
    { SWC *f = v + v->father;

      w->father   = (int) (f-w);
      v->father   = (int) (w-v);
      w->children = (int) (v-w);
      w->sibling  = (int) ((v + v->sibling) - w);
      v->sibling  = 0;
      if (f + f->children == v)
        f->children = (int) (w-f);
      else
        { f += f->children;
          while (f + f->sibling != v)
            f += f->sibling;
          f->sibling = (int) (w-f);
        }
    }

  return ((SWC_Node *) w);
}

/****************************************************************************************
 *                                                                                      *
 *  SWC TREE GEOMETRY ROUTINES                                                          *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { double     bx, by, bz;     //  The region to clip to is [bx,ex] x [by,ey] x [bz,ez]
    double     ex, ey, ez;
    int        num;            //  next subtree to be processed
    int       *sizes;          //  sizes[i] = # of nodes in i'th subtree seen so far
    SWC_Tree **trees;          //  trees[i] = i'th subtree being built
  } Clip_Data;

  //  How many distinct subtrees of the sub-tree rooted at v intersect the bounded region
  //    (excluding the one containing v's father (if any))?

static int count_intersections(SWC *v, int inf, Clip_Data *box)
{ int inv, n, c;

  inv = (box->bx <= v->x && v->x <= box->ex &&
         box->by <= v->y && v->y <= box->ey &&
         box->bz <= v->z && v->z <= box->ez);

  n = (!inf && inv);
  for (c = v->children; c != 0; c = v->sibling)
    { v += c;
      n += count_intersections(v,inv,box);
    }
  return (n);
}

  //  Recursively traverse the source tree, but this time count the # of nodes in
  //    each clip sub-tree

static void intersection_sizes(SWC *v, int inf, int wch, Clip_Data *box)
{ int inv, c;

  inv = (box->bx <= v->x && v->x <= box->ex &&
         box->by <= v->y && v->y <= box->ey &&
         box->bz <= v->z && v->z <= box->ez);

  if (inf)
    box->sizes[wch] += 1; 
  else if (inv)
    { wch = box->num++;
      if (v->father == 0)
        box->sizes[wch] = 1;
      else
        box->sizes[wch] = 2;
    }
  
  for (c = v->children; c != 0; c = v->sibling)
    { v += c;
      intersection_sizes(v,inv,wch,box);
    }
}

  //  Find the intersection with the bounding box of an edge from in to out where in is inside
  //    and out is outside (the bounding box).

static int cut_edge(SWC *in, SWC *out, SWC_Node *cut, Clip_Data *box)
{ double     a, b;
  double     ix, iy, iz;
  double     ox, oy, oz;

  ix = in->x;
  iy = in->y;
  iz = in->z;

  ox = out->x;
  oy = out->y;
  oz = out->z;

  b = 1.;
  if (ox < box->bx  && ix >= box->bx)
    { a = (box->bx - ix) / (ox - ix);
      if (a < b)
        b = a;
    }
  if (ox > box->ex && ix <= box->ex)
    { a = (box->ex - ix) / (ox - ix);
      if (a < b)
        b = a;
    }
  if (oy < box->by  && iy >= box->by)
    { a = (box->by - iy) / (oy - iy);
      if (a < b)
        b = a;
    }
  if (oy > box->ey && iy <= box->ey)
    { a = (box->ey - iy) / (oy - iy);
      if (a < b)
        b = a;
    }
  if (oz < box->bz  && iz >= box->bz)
    { a = (box->bz - iz) / (oz - iz);
      if (a < b)
        b = a;
    }
  if (oz > box->ez && iz <= box->ez)
    { a = (box->ez - iz) / (oz - iz);
      if (a < b)
        b = a;
    }

  if (b <= 0.)
    return (0);

  cut->x = (float) (ix + b*(ox - ix));
  cut->y = (float) (iy + b*(oy - iy));
  cut->z = (float) (iz + b*(oz - iz));
  cut->r = (float) (in->r + b*(out->r - in->r));

  return (1);
}

  //  Recursively traverse the source tree, but in this final 3rd pass, build the subtree

static void intersection(SWC *v, int inf, SWC_Tree *wch, SWC_Node *f, Clip_Data *box)
{ int inv, c;

  inv = (box->bx <= v->x && v->x <= box->ex &&
         box->by <= v->y && v->y <= box->ey &&
         box->bz <= v->z && v->z <= box->ez);

  { SWC_Node cut;

    if (inf)
      { if (inv)
          f = Add_Vertex_As_First_Child(wch,f,v->kind,v->x,v->y,v->z,v->r);
        else
          { if (cut_edge(v+v->father,v,&cut,box))
              f = Add_Vertex_As_First_Child(wch,f,v->kind,cut.x,cut.y,cut.z,cut.r);
          }
      }
    else if (inv)
      { wch = box->trees[box->num++];
        if (v->father == 0)
          f = Add_Vertex_As_Root(wch,v->kind,v->x,v->y,v->z,v->r);
        else
          { if (cut_edge(v,v+v->father,&cut,box))
              { f = Add_Vertex_As_Root(wch,v->kind,cut.x,cut.y,cut.z,cut.r);
                f = Add_Vertex_As_First_Child(wch,f,v->kind,v->x,v->y,v->z,v->r);
              }
            else
              f = Add_Vertex_As_Root(wch,v->kind,v->x,v->y,v->z,v->r);
          }
      }
  }
  
  for (c = v->children; c != 0; c = v->sibling)
    { v += c;
      intersection(v,inv,wch,f,box);
    }
}

SWC_Bundle *Clip_SWC_Tree(SWC_Bundle *R(M(forest)),
                          SWC_Tree *tree, Double_Vector *beg, Double_Vector *end)
{ Stree    *t = (Stree *) tree;
  int       ntrees;
  SWC      *r;
  Clip_Data box;

  if (beg->ndims != 1 || beg->dims[0] != 3)
    { fprintf(stderr,"Begin coordinate of volume should be a 3D coordinate (Clip_SWC_Tree)\n");
      exit (1);
    }
  if (end->ndims != 1 || end->dims[0] != 3)
    { fprintf(stderr,"End coordinate of volume should be a 3D coordinate (Clip_SWC_Tree)\n");
      exit (1);
    }
 
  box.bx = AFLOAT64(beg)[0];
  box.ex = AFLOAT64(end)[0];
  box.by = AFLOAT64(beg)[1];
  box.ey = AFLOAT64(end)[1];
  box.bz = AFLOAT64(beg)[2];
  box.ez = AFLOAT64(end)[2];

  Free_Array(beg);
  Free_Array(end);

  //  Count # of sub-trees in result

  r = t->nodes + t->root;
  ntrees = count_intersections(r,0,&box);

  //  Prepare the bundle for the trees to be extracted from trees

  { int i;
    if (forest->num_trees >= ntrees)
      { for (i = ntrees; i < forest->num_trees; i++)
          Kill_SWC_Tree(forest->trees[i]);
        for (i = ntrees-1; i >= 0; i--)
          Free_SWC_Tree(forest->trees[i]);
      }
    else
      { for (i = forest->num_trees-1; i >= 0; i--)
          Free_SWC_Tree(forest->trees[i]);
        forest->trees = (SWC_Tree **)
                     Guarded_Malloc(sizeof(SWC_Tree *)*((size_t) ntrees),"Read_SWC_File");
        if (forest->trees == NULL)
          { fprintf(stderr,"Out of memory (Read_SWC_File)\n");
            exit (1);
          }
      }
    forest->num_trees = ntrees;
  }

  //  Count the number of nodes in each sub-tree that will result

  box.num   = 0;
  box.sizes = (int *) Guarded_Malloc(sizeof(int)*((size_t) ntrees),"Clip_SWC_Tree");
  intersection_sizes(r,0,0,&box);

  //  Allocate trees with enough room for nodes of each now that their sizes are known

  { int k;

    for (k = 0; k < ntrees; k++)
      forest->trees[k] = (SWC_Tree *) new_stree(SIZEOF(SWC)*box.sizes[k],"Clip_SWC_Tree");
  }

  free(box.sizes);   //  No longer need sizes

  //  Finally, build the subtree results

  box.num   = 0;
  box.trees = forest->trees;
  intersection(r,0,NULL,NULL,&box);

  { int  h, k;     //  Reverse sibling lists for all subtree;
    int  x;
    SWC *p, *v;
    SWC *nodes;
    int  nverts;
 
    for (h = 0; h < ntrees; h++)
      { nodes  = ((Stree *) forest->trees[h])->nodes;
        nverts = ((Stree *) forest->trees[h])->nverts;
        for (k = 0; k < nverts; k++)
          { p = nodes + k;
            x = p->children;
            if (x != 0)
              { v = p;
                p = v+x;
                while (x != 0)
                  { v = v + x;
                    x = v->sibling;
                    v->sibling = (int) (p - v);
                    p = v;
                  }
                nodes[k].children = (int) (p - (nodes+k));
              }
          }
      }
  }

  return (forest);
}

Extent_Bundle *SWC_Tree_Extent(Extent_Bundle *R(M(extent)), SWC_Tree *tree)
{ double *min, *max;
  float   minx, maxx;
  float   miny, maxy;
  float   minz, maxz;
  SWC    *v;
  int     i, n;
  Array  *e;

  n = ((Stree *) tree)->nverts;
  v = ((Stree *) tree)->nodes;

  minx = maxx = v->x;
  miny = maxy = v->y;
  minz = maxz = v->z;
  for (i = 0; i < n; i++)
    { if (v->x < minx)
        minx = v->x;
      else if (v->x > maxx)
        maxx = v->x;
      if (v->y < miny)
        miny = v->y;
      else if (v->y > maxy)
        maxy = v->y;
      if (v->z < minz)
        minz = v->z;
      else if (v->z > maxz)
        maxz = v->z;
      v += 1;
    }

  e = extent->min;
  if (e == NULL)
    extent->min = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));
  else if (e->size*type_size[e->type] < 3*type_size[DIMN_TYPE])
    { Free_Array(e);
      extent->min = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));
    }
  else
    { e->kind  = PLAIN_KIND;
      e->type  = FLOAT64_TYPE;
      e->scale = 64;
      e->ndims = 1;
      e->size  = 3;
      e->dims[0] = 3;
    }

  e = extent->max;
  if (e == NULL)
    extent->max = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));
  else if (e->size*type_size[e->type] < 3*type_size[DIMN_TYPE])
    { Free_Array(e);
      extent->max = Make_Array_With_Shape(PLAIN_KIND,FLOAT64_TYPE,Coord1(3));
    }
  else
    { e->kind  = PLAIN_KIND;
      e->type  = FLOAT64_TYPE;
      e->scale = 64;
      e->ndims = 1;
      e->size  = 3;
      e->dims[0] = 3;
    }

  min = AFLOAT64(extent->min);
  max = AFLOAT64(extent->max);

  min[0] = minx;
  max[0] = maxx;
  min[1] = miny;
  max[1] = maxy;
  min[2] = minz;
  max[2] = maxz;

  return (extent);
}

double SWC_Tree_Path_Length(SWC_Tree *tree)
{ SWC   *v, *f;
  double len;
  int    i, n;

  n = ((Stree *) tree)->nverts;
  v = ((Stree *) tree)->nodes;

  len = 0.;
  for (i = 0; i < n; i++)
    { if (v->father != 0)
        { double dx, dy, dz;

          f = v + v->father;
          dx = (v->x - f->x);
          dy = (v->y - f->y);
          dz = (v->z - f->z);
          len += sqrt(dx*dx + dy*dy + dz*dz);
        }
      v += 1;
    }

  return (len);
}
