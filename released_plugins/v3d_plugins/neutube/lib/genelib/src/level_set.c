/*****************************************************************************************\
*                                                                                         *
*  Component Tree Abstractions                                                            *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007 (from August 2006 version)                                          *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "utilities.h"
#include "image_lib.h"
#include "level_set.h"

#undef  DEBUG_REGIONS

/****************************************************************************************
 *                                                                                      *
 *  UTILITY ROUTINES                                                                    *
 *                                                                                      *
 ****************************************************************************************/

static void error(char *msg, char *arg)
{ fprintf(stderr,"Gene's Level Sets: ");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

/****************************************************************************************
 *                                                                                      *
 *  LEVEL SET BRANCHING DATA STRUCTURES & ROUTINES                                      *
 *                                                                                      *
 ****************************************************************************************/

/* Union-find father linked tree of pixels */

typedef struct
  { int   father;  /* index of father in current set                          */
    int   set;     /* if a root (father = -1) then regtree index (+/- coding) */
  } pixel;

/* Binary tree of region fusion events */

typedef struct
  { int    left;   /* region is union of left and right regions:             */
    int    right;  /*   if > 0 then index of subtree, else - of single pixel */
    int    size;   /* # of pixels in region subtree                          */
    int    start;  /* Leftmost pixel in subtree (for finding outer contour)  */
    uint16 level;  /* Pixel value at which the two regions were joined       */
    uint16 peak;   /* Max pixel value in subtree rooted at this node         */
  } regtree;

typedef struct
  { Image   *image_ref;
    Stack   *stack_ref;
    regtree *array;
    int      iscon4;
  } Comtree;


/****************************************************************************************
 *                                                                                      *
 *  CURRENT COMPONENT TREE GLOBALS AND SET-ROUTINE                                      *
 *                                                                                      *
 ****************************************************************************************/


/* Current Component Tree globals */

static regtree *regtrees;  /* Binary tree of region tree islands */
static uint8   *value8;    /* Pixel values of image          */
static uint16  *value16;   /* Pixel values of image          */
static int      cwidth;    /* Width of current image         */
static int      cheight;   /* Height of current image        */
static int      cdepth;    /* Depth of current stack         */
static int      carea;     /* Area of current image          */
static int      cvolume;   /* Area of current image          */
static Comtree *ctree = NULL;

void Set_Current_Component_Tree(Component_Tree *atree)
{ Comtree *t = (Comtree *) atree;

  ctree = (Comtree*)atree;
  if (atree != NULL) {
    if (t->image_ref != NULL)
      { if (t->image_ref->kind == GREY16)
          value16 = (uint16 *) (t->image_ref->array);
        else
          value8  = t->image_ref->array;
        cwidth   = t->image_ref->width;
        carea    = cwidth * cheight;
        regtrees = t->array - 1;
      }
    else
      { if (t->stack_ref->kind == GREY16)
          value16 = (uint16 *) (t->stack_ref->array);
        else
          value8  = t->stack_ref->array;
        cwidth   = t->stack_ref->width;
        cheight  = t->stack_ref->height;
        carea    = cwidth * cheight;
        cvolume  = cwidth * cheight * cdepth;
        regtrees = t->array - 1;
      }
  }    
}

Component_Tree *Get_Current_Component_Tree()
{ return (ctree); }

Image *Get_Component_Tree_Image(Component_Tree *atree)
{ Comtree *t = (Comtree *) atree;
  return (t->image_ref);
}

Stack *Get_Component_Tree_Stack(Component_Tree *atree)
{ Comtree *t = (Comtree *) atree;
  return (t->stack_ref);
}

int Get_Component_Tree_Connectivity(Component_Tree *atree)
{ Comtree *t = (Comtree *) atree;
  return (t->iscon4);
}

/****************************************************************************************
 *                                                                                      *
 *  TREE TRAVERSAL ROUTINES                                                             *
 *                                                                                      *
 ****************************************************************************************/

static inline int size(int cont)
{ if (cont > 0)
    return (regtrees[cont].size);
  else
    return (1);
}

static inline int level(int cont)
{ if (cont > 0)
    return (regtrees[cont].level);
  else if (value8 != NULL)
    return (value8[-cont]);
  else
    return (value16[-cont]);
}

static inline int peak(int cont)
{ if (cont > 0)
    return (regtrees[cont].peak);
  else if (value8 != NULL)
    return (value8[-cont]);
  else
    return (value16[-cont]);
}

static inline int start(int cont)
{ if (cont > 0)
    return (regtrees[cont].start);
  else
    return (-cont);
}

inline Level_Set *Level_Set_Child(Level_Set *r)
{ regtree *p;
  int      x;

  x = ((regtree *) r)->right;
  if (x <= 0)
    return (NULL);
  p = regtrees + x;
  if (p->right <= 0)
    { if (value8 != NULL)
        { if (value8[-p->right] == p->level)
            return (NULL);
        }
      else
        { if (value16[-p->right] == p->level)
            return (NULL);
        }
    }
  return ((Level_Set *) p);
}

inline Level_Set *Level_Set_Sibling(Level_Set *r)
{ regtree *p;
  int      x;

  x = ((regtree *) r)->left;
  if (x <= 0)
    return (NULL);
  p = regtrees + x;
  if (p->right <= 0)
    { if (value8 != NULL)
        { if (value8[-p->right] == p->level)
            return (NULL);
        }
      else
        { if (value16[-p->right] == p->level)
            return (NULL);
        }
    }
  return ((Level_Set *) p);
}

inline int Level_Set_Size(Level_Set *r)
{ return (size(((regtree *) r)->right)); }

inline int Level_Set_Level(Level_Set *r)
{ return (level(((regtree *) r)->right)); }

inline int Level_Set_Peak(Level_Set *r)
{ return (peak(((regtree *) r)->right)); }

inline int Level_Set_Leftmost(Level_Set *r)
{ return (start(((regtree *) r)->right)); }

inline int Level_Set_Background(Level_Set *r)
{ return (((regtree *) r)->level); }

inline int Level_Set_Id(Level_Set *r)
{ return (((regtree *) r) - regtrees); }

inline Level_Set *Level_Set_Root()
{ return ((Level_Set *) (regtrees + carea)); }

  /* List every pixel in the subtree rooted at p */

void list_level_set(int p, void (*handler)(int))
{ if (p <= 0)
    handler(-p);
  else
    { while (p > 0)
        { list_level_set(regtrees[p].right,handler);
          p = regtrees[p].left;
        }
      list_level_set(p,handler);
    }
}

void List_Level_Set(Level_Set *r, void (*handler)(int))
{ list_level_set(((regtree *) r)->right,handler); }


/****************************************************************************************
 *                                                                                      *
 *  COMPONENT TREE SPACE MANAGEMENT ROUTINES                                            *
 *                                                                                      *
 ****************************************************************************************/

//  Workspace routines for building a component tree

static int *get_chord(int area, char *routine)
{ static int  Max_Area = 0;
  static int *Array = NULL;

  if (area < 0)
    { free(Array);
      Max_Area = 0;
      Array    = NULL;
    }
  else if (area > Max_Area)
    { Max_Area = area;
      Array    = (int *) Guarded_Realloc(Array,sizeof(int)*Max_Area,routine);
    }
  return (Array);
}

static pixel *get_pixels(int area, char *routine)
{ static int    Max_Area = 0;
  static pixel *Array = NULL;

  if (area < 0)
    { free(Array);
      Max_Area = 0;
      Array    = NULL;
    }
  else if (area > Max_Area)
    { Max_Area = area;
      Array    = (pixel *) Guarded_Realloc(Array,sizeof(pixel)*Max_Area,routine);
    }
  return (Array);
}

//  Awk-generated (manager.awk) Component_Tree space management

static inline int comtree_asize(Comtree *tree)
{ if (tree->image_ref != NULL)
    return (tree->image_ref->width * tree->image_ref->height * sizeof(regtree));
  else
    return (tree->stack_ref->width * tree->stack_ref->height *
            tree->stack_ref->depth * sizeof(regtree));
}


typedef struct __Comtree
  { struct __Comtree *next;
    size_t               asize;
    Comtree           comtree;
  } _Comtree;

static _Comtree *Free_Comtree_List = NULL;
static size_t    Comtree_Offset, Comtree_Inuse;

static inline void allocate_comtree_array(Comtree *comtree, size_t asize, char *routine)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  if (object->asize < asize)
    { if (object->asize == 0)
        object->comtree.array = NULL;
#ifdef _MSC_VER
      object->comtree.array  = (decltype(object->comtree.array))Guarded_Realloc(object->comtree.array,asize,routine);
#else
      object->comtree.array  = Guarded_Realloc(object->comtree.array,asize,routine);
#endif
      object->asize = asize;
    }
}

static inline Comtree *new_comtree(size_t asize, char *routine)
{ _Comtree *object;

  if (Free_Comtree_List == NULL)
    { object = (_Comtree *) Guarded_Malloc(sizeof(_Comtree),routine);
      Comtree_Offset = ((char *) &(object->comtree)) - ((char *) object);
      object->asize = 0;
    }
  else
    { object = Free_Comtree_List;
      Free_Comtree_List = object->next;
    }
  Comtree_Inuse += 1;
  allocate_comtree_array(&(object->comtree),asize,routine);
  return (&(object->comtree));
}

static inline Comtree *copy_comtree(Comtree *comtree)
{ Comtree *copy = new_comtree(comtree_asize(comtree),"Copy_Component_Tree");
  Comtree  temp = *copy;
  *copy = *comtree;
  copy->array = temp.array;
  if (comtree_asize(comtree) != 0)
    memcpy(copy->array,comtree->array,comtree_asize(comtree));
  return (copy);
}

Component_Tree *Copy_Component_Tree(Component_Tree *component_tree)
{ return ((Component_Tree *) copy_comtree((Comtree *) component_tree)); }

static inline void pack_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  if (object->asize > comtree_asize(comtree))
    { object->asize = comtree_asize(comtree);
      if (object->asize != 0)
#ifdef _MSC_VER
        object->comtree.array = (decltype(object->comtree.array))Guarded_Realloc(object->comtree.array,
#else
        object->comtree.array = Guarded_Realloc(object->comtree.array,
#endif
                                                object->asize,"Pack_Comtree");
      else
        { free(object->comtree.array);
          object->asize = 0;
        }
    }
}

void Pack_Component_Tree(Component_Tree *component_tree)
{ pack_comtree(((Comtree *) component_tree)); }

static inline void free_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  object->next = Free_Comtree_List;
  Free_Comtree_List = object;
  Comtree_Inuse -= 1;
}

void Free_Component_Tree(Component_Tree *component_tree)
{ free_comtree(((Comtree *) component_tree)); }

static inline void kill_comtree(Comtree *comtree)
{ _Comtree *object  = (_Comtree *) (((char *) comtree) - Comtree_Offset);
  if (object->asize != 0)
    free(comtree->array);
  free(((char *) comtree) - Comtree_Offset);
  Comtree_Inuse -= 1;
}

void Kill_Component_Tree(Component_Tree *component_tree)
{ kill_comtree(((Comtree *) component_tree)); }

static inline void reset_comtree()
{ _Comtree *object;
  while (Free_Comtree_List != NULL)
    { object = Free_Comtree_List;
      Free_Comtree_List = object->next;
      kill_comtree(&(object->comtree));
      Comtree_Inuse += 1;
    }
}

int Component_Tree_Usage()
{ return (Comtree_Inuse); }

void Reset_Component_Tree()
{ reset_comtree();
  get_chord(-1,NULL);
  get_pixels(-1,NULL);
}


/****************************************************************************************
 *                                                                                      *
 *  BUILD A COMPONENT TREE                                                              *
 *                                                                                      *
 ****************************************************************************************/

static pixel *pixels;         /* Union-find of image pixels  */

/* Find root of pixel x in union/find tree and compress path */

static int find(int x)
{ int y, z;

  y = x;
  while ((z = pixels[y].father) >= 0)
    y = z;
  z = pixels[x].father;
  while (z >= 0)
    { pixels[x].father = y;
      x = z;
      z = pixels[x].father;
    }
  return (y);
}

/* Union the regions containing pixels p and q *if they are in different
   regions, noting it happened at level c using "node" to model the new
   region.  Return node+1 if the union took place, and node otherwise.     */

static int fuse(int node, int p, int q, int c)
{ int x, y;

  x = find(p);
  y = find(q);
  if (x != y)
    { int      s,  t;
      int      m,  n;
      int      u,  v;
      regtree *b;

      s = pixels[x].set;
      t = pixels[y].set;
      m = size(s);
      n = size(t);

      b = regtrees + node;
      b->left  = s;
      b->right = t;
      b->size  = m + n;
      b->level = c;

      u = peak(s);
      v = peak(t);
      if (u < v)
        b->peak = v;
      else
        b->peak = u;

      u = start(s);
      v = start(t);
      if (u%cwidth < v%cwidth)
        b->start = u;
      else
        b->start = v;

      if (m < n)
        { pixels[x].father = y;
          pixels[y].set    = node;
        }
      else
        { pixels[y].father = x;
          pixels[x].set    = node;
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

static int flatten_tree(int p)
{ int final, cache;

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
    regtrees[final].level = level;
    regtrees[final].peak  = peak;
    regtrees[final].start = start;
  }

  { int p, q;        //  Flatten descendants and cap the chain (with cache)

    for (p = final; 1; p = q)
      { regtrees[p].right = flatten_tree(regtrees[p].right);
        if ((q = regtrees[p].left) <= 0)
          { regtrees[p].left = flatten_tree(cache);
            break;
          }
      }
  }

  { int c, f;      //  Permute the descendants so that pixels at this level are at the end
    int p, r;

    f = final;
    c = regtrees[final].level;
    if (value8 != NULL)
      { for (p = final; 1; p = r)
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
      }
    else
      { for (p = final; 1; p = r)
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
      }
  }

  return (final);
}

static int chk_width;
static int chk_height;
static int chk_depth;
static int chk_iscon4;

static inline int *boundary_pixels_2d(int p)
{ static int bound[8];
  int x, xn, xp;
  int y, yn, yp;

  y = p/cwidth;
  x = p%cwidth;

  yn = (y > 0);
  xn = (x > 0);
  xp = (x < chk_width);
  yp = (y < chk_height);
  if (chk_iscon4)
    { bound[0] = yn;
      bound[1] = xn;
      bound[2] = xp;
      bound[3] = yp;
    }
  else
    { bound[0] = yn && xn;
      bound[1] = yn;
      bound[2] = yn && xp;
      bound[3] = xn;
      bound[4] = xp;
      bound[5] = yp && xn;
      bound[6] = yp;
      bound[7] = yp && xp;
    }

  return (bound);
}

static inline int *boundary_pixels_3d(int p)
{ static int bound[26];
  int x, xn, xp;
  int y, yn, yp;
  int z, zn, zp;

  z = p/carea;
  y = p%carea;
  x = y%cwidth;
  y = y/cwidth;

  yn = (y > 0);
  xn = (x > 0);
  zn = (z > 0);
  xp = (x < chk_width);
  yp = (y < chk_height);
  zp = (z < chk_depth);
  if (chk_iscon4)
    { bound[0] = zn;
      bound[1] = yn;
      bound[2] = xn;
      bound[3] = xp;
      bound[4] = yp;
      bound[5] = zp;
    }
  else
    { int t;

      bound[ 1] = t = zn && yn;
      bound[ 0] = t && xn;
      bound[ 2] = t && xp;
      bound[ 3] = zn && xn;
      bound[ 4] = zn;
      bound[ 5] = zn && xp;
      bound[ 7] = t = zn && yp;
      bound[ 6] = t && xn;
      bound[ 8] = t && xp;

      bound[ 9] = yn && xn;
      bound[10] = yn;
      bound[11] = yn && xp;
      bound[12] = xn;
      bound[13] = xp;
      bound[14] = yp && xn;
      bound[15] = yp;
      bound[16] = yp && xp;

      bound[18] = t = zp && yn;
      bound[17] = t && xn;
      bound[19] = t && xp;
      bound[20] = zp && xn;
      bound[21] = zp;
      bound[22] = zp && xp;
      bound[24] = t = zp && yp;
      bound[23] = t && xn;
      bound[25] = t && xp;
    }

  return (bound);
}

Component_Tree *Build_2D_Component_Tree(Image *frame, int iscon4)
{ int index[0x10001];
  int maxval;

  int neighbor[8];
  int n_nbrs;

  int *chord;

  if (frame->kind == COLOR)
    { fprintf(stderr,"Build_2D_Component_Tree: Cannot apply to COLOR images\n");
      exit (1);
    }
  if (frame->kind == FLOAT32)
    { fprintf(stderr,"Build_2D_Component_Tree: Cannot apply to FLOAT32 images\n");
      exit (1);
    }

  cwidth   = frame->width;
  cheight  = frame->height;
  carea    = cwidth*cheight;
  ctree    = new_comtree(carea*sizeof(regtree),"Build_Component_Tree");
  ctree->image_ref = frame;
  ctree->stack_ref = NULL;
  ctree->iscon4    = iscon4;
  regtrees = ctree->array - 1;

  if (iscon4)
    { neighbor[0] = -cwidth;
      neighbor[1] = -1;
      neighbor[2] =  1;
      neighbor[3] =  cwidth;
      n_nbrs = 4;
    }
  else
    { int x, y;

      n_nbrs = 0;
      for (y = -cwidth; y <= cwidth; y += cwidth)
        for (x = -1; x <= 1; x += 1)
          if (! (x==0 && y == 0))
            neighbor[n_nbrs++] = y+x;
    }

  chk_width  = cwidth-1;
  chk_height = cheight-1;
  chk_iscon4 = iscon4;

  chord  = get_chord(carea,"Build_Component_Tree");
  pixels = get_pixels(carea,"Build_Component_Tree");

  if (frame->kind == GREY16)
    { maxval  = 0x10000;
      value8  = NULL;
      value16 = (uint16 *) (frame->array);
    }
  else
    { maxval  = 0x100;
      value8  = frame->array;
      value16 = NULL;
    }

  // Establish lists of pixels with each value

  { int p, v;

    for (v = 0; v <= maxval; v++)
      index[v] = -1;

    if (value8 != NULL)
      for (p = 0; p < carea; p++)
        { v        = value8[p];
          chord[p] = index[v];
          index[v] = p;
        }
    else  // value16 != NULL
      for (p = 0; p < carea; p++)
        { v        = value16[p];
          chord[p] = index[v];
          index[v] = p;
        }
  }

  { int    p;
    pixel *s;

    for (p = 0; p < carea; p++)
      { s = pixels + p;
        s->father = -1;
        s->set    = -p;
      }
  }

  { int c, p;
    int w, h;
    int node;

    w = cwidth-1;
    h = cheight-1;

    node = 1;
    if (value8 != NULL)
      { for (c = maxval-1; c >= 0; c--)
          { for (p = index[c]; p >= 0; p = chord[p])
              { int *b, j, q;

                b = boundary_pixels_2d(p);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { q = p + neighbor[j];
                      if (value8[q] >= c)
                        node = fuse(node,p,q,c);
                    }
              }
          }
      }
    else
      { for (c = maxval-1; c >= 0; c--)
          { for (p = index[c]; p >= 0; p = chord[p])
              { int *b, j, q;

                b = boundary_pixels_2d(p);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { q = p + neighbor[j];
                      if (value16[q] >= c)
                        node = fuse(node,p,q,c);
                    }
              }
          }
      }
  }

  regtrees[carea].right = flatten_tree(carea-1);
  regtrees[carea].left  = 0;
  regtrees[carea].level = -1;

#ifdef DEBUG_REGIONS
  printf("\nDecomposition of plane:\n");
  Print_Component_Tree((Level_Set *) (regtrees+carea),0,stdout);
  fflush(stdout);
#endif

  return ((Component_Tree *) ctree);
}

Component_Tree *Build_3D_Component_Tree(Stack *frame, int iscon6)
{ static int index[0x10001];

  int neighbor[26];
  int n_nbrs;

  int *chord;
  int  maxval;

  if (frame->kind == COLOR)
    { fprintf(stderr,"Build_3D_Component_Tree: Cannot apply to COLOR images\n");
      exit (1);
    }
  if (frame->kind == FLOAT32)
    { fprintf(stderr,"Build_3D_Component_Tree: Cannot apply to FLOAT32 images\n");
      exit (1);
    }

  cwidth   = frame->width;
  cheight  = frame->height;
  cdepth   = frame->depth;
  carea    = cwidth*cheight;
  cvolume  = cwidth*cheight*cdepth;
  ctree    = new_comtree(cvolume*sizeof(regtree),"Build_Component_Tree");
  ctree->stack_ref = frame;
  ctree->image_ref = NULL;
  ctree->iscon4    = iscon6;
  regtrees = ctree->array - 1;

  if (iscon6)
    { neighbor[0] = -carea;
      neighbor[1] = -cwidth;
      neighbor[2] = -1;
      neighbor[3] =  1;
      neighbor[4] =  cwidth;
      neighbor[5] =  carea;
      n_nbrs = 6;
    }
  else
    { int x, y, z;

      n_nbrs = 0;
      for (z = -carea; z <= carea; z += carea)
        for (y = -cwidth; y <= cwidth; y += cwidth)
          for (x = -1; x <= 1; x += 1)
            if (! (x==0 && y == 0 && z == 0))
              neighbor[n_nbrs++] = z+y+x;
    }

  chk_width  = cwidth-1;
  chk_height = cheight-1;
  chk_iscon4 = iscon6;

  chord  = get_chord(cvolume,"Build_Component_Tree");
  pixels = get_pixels(cvolume,"Build_Component_Tree");

  if (frame->kind == GREY16)
    { maxval  = 0x10000;
      value8  = NULL;
      value16 = (uint16 *) (frame->array);
    }
  else
    { maxval  = 0x100;
      value8  = frame->array;
      value16 = NULL;
    }

  // Establish lists of pixels with each value

  { int p, v;

    for (v = 0; v <= maxval; v++)
      index[v] = -1;

    if (value8 != NULL)
      for (p = 0; p < carea; p++)
        { v        = value8[p];
          chord[p] = index[v];
          index[v] = p;
        }
    else  // value16 != NULL
      for (p = 0; p < carea; p++)
        { v        = value16[p];
          chord[p] = index[v];
          index[v] = p;
        }
  }

  { int    p;
    pixel *s;

    for (p = 0; p < cvolume; p++)
      { s = pixels + p;
        s->father = -1;
        s->set    = -p;
      }
  }

  { int c, p;
    int w, h, d;
    int node;

    w = cwidth-1;
    h = cheight-1;
    d = cdepth-1;

    node = 1;
    if (value8 != NULL)
      { for (c = maxval-1; c >= 0; c--)
          { for (p = index[c]; p >= 0; p = chord[p])
              { int *b, j, q;

                b = boundary_pixels_3d(p);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { q = p + neighbor[j];
                      if (value8[q] >= c)
                        node = fuse(node,p,q,c);
                    }
              }
          }
      }
    else
      { for (c = maxval-1; c >= 0; c--)
          { for (p = index[c]; p >= 0; p = chord[p])
              { int *b, j, q;

                b = boundary_pixels_3d(p);
                for (j = 0; j < n_nbrs; j++)
                  if (b[j])
                    { q = p + neighbor[j];
                      if (value16[q] >= c)
                        node = fuse(node,p,q,c);
                    }
              }
          }
      }
  }

  regtrees[cvolume].right = flatten_tree(cvolume-1);
  regtrees[cvolume].left  = 0;
  regtrees[cvolume].level = -1;

#ifdef DEBUG_REGIONS
  printf("\nDecomposition of plane:\n");
  Print_Component_Tree((Level_Set *) (regtrees+cvolume),0,stdout);
  fflush(stdout);
#endif

  return ((Component_Tree *) ctree);
}


/****************************************************************************************
 *                                                                                      *
 *  PRINT A COMPONENT TREE                                                              *
 *                                                                                      *
 ****************************************************************************************/

/* Print the level set subtree rooted at r, indented "indent" spaces to file "file" */

static void _print_regions(Level_Set *r, int indent, FILE *file)
{ fprintf(file,"%3d:%*s  Element %d (c=%d, s=%d) [%d,%d]\n",
               indent,indent,"",Level_Set_Id(r),
               Level_Set_Level(r),Level_Set_Size(r),Level_Set_Background(r),Level_Set_Peak(r));
  fflush(file);
  for (r = Level_Set_Child(r); r != NULL; r = Level_Set_Sibling(r))
    _print_regions(r,indent+1,file);
}

void Print_Component_Tree(Component_Tree *tree, int indent, FILE *file)
{ Component_Tree *otree;

  otree = Get_Current_Component_Tree();
  Set_Current_Component_Tree(tree);
  _print_regions(Level_Set_Root(),indent,file);
  Set_Current_Component_Tree(otree);
}
