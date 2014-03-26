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

/****************************************************************************************\
*                                                                                         *
*  SVG drawing package                                                                    *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2011                                                                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mylib.h"
#include "utilities.h"
#include "svg.h"

#undef  DEBUG

#define SIZEOF(x) ((int) sizeof(x))

#define MO(canvas)  ((MyCanvas *) (canvas))
#define MC(canvas)  ((MyContext *) (MO(canvas)->context))
#define MF(canvas)  (MO(canvas)->hfile)
#define GC(context) ((MyContext *) (context))

#define SVG_OFF  1000   //  Any number bigger than the max predefined color, currently yellow_green

static double Rad2Ang;  //  Radians to degrees (setup by Begin_SVG_Drawing)
static double TwoPI;    //  360 degrees in radians

static int PredefColors[][3] =
  { { 240, 248, 255 }, { 250, 235, 215 }, {   0, 255, 255 }, { 127, 255, 212 },
    { 240, 255, 255 }, { 245, 245, 220 }, { 255, 228, 196 }, {   0,   0,   0 },
    { 255, 235, 205 }, {   0,   0, 255 }, { 138,  43, 226 }, { 165,  42,  42 },
    { 222, 184, 135 }, {  95, 158, 160 }, { 127, 255,   0 }, { 210, 105,  30 },
    { 255, 127,  80 }, { 100, 149, 237 }, { 255, 248, 220 }, { 220,  20,  60 },
    {   0, 255, 255 }, {   0,   0, 139 }, {   0, 139, 139 }, { 184, 134,  11 },
    { 169, 169, 169 }, {   0, 100,   0 }, { 169, 169, 169 }, { 189, 183, 107 },
    { 139,   0, 139 }, {  85, 107,  47 }, { 255, 140,   0 }, { 153,  50, 204 },
    { 139,   0,   0 }, { 233, 150, 122 }, { 143, 188, 143 }, {  72,  61, 139 },
    {  47,  79,  79 }, {  47,  79,  79 }, {   0, 206, 209 }, { 148,   0, 211 },
    { 255,  20, 147 }, {   0, 191, 255 }, { 105, 105, 105 }, { 105, 105, 105 },
    {  30, 144, 255 }, { 178,  34,  34 }, { 255, 250, 240 }, {  34, 139,  34 },
    { 255,   0, 255 }, { 220, 220, 220 }, { 248, 248, 255 }, { 255, 215,   0 },
    { 218, 165,  32 }, { 128, 128, 128 }, { 128, 128, 128 }, {   0, 128,   0 },
    { 173, 255,  47 }, { 240, 255, 240 }, { 255, 105, 180 }, { 205,  92,  92 },
    {  75,   0, 130 }, { 255, 255, 240 }, { 240, 230, 140 }, { 230, 230, 250 },
    { 255, 240, 245 }, { 124, 252,   0 }, { 255, 250, 205 }, { 173, 216, 230 },
    { 240, 128, 128 }, { 224, 255, 255 }, { 250, 250, 210 }, { 211, 211, 211 },
    { 144, 238, 144 }, { 211, 211, 211 }, { 255, 182, 193 }, { 255, 160, 122 },
    {  32, 178, 170 }, { 135, 206, 250 }, { 119, 136, 153 }, { 119, 136, 153 },
    { 176, 196, 222 }, { 255, 255, 224 }, {   0, 255,   0 }, {  50, 205,  50 },
    { 250, 240, 230 }, { 255,   0, 255 }, { 128,   0,   0 }, { 102, 205, 170 },
    {   0,   0, 205 }, { 186,  85, 211 }, { 147, 112, 219 }, {  60, 179, 113 },
    { 123, 104, 238 }, {   0, 250, 154 }, {  72, 209, 204 }, { 199,  21, 133 },
    {  25,  25, 112 }, { 245, 255, 250 }, { 255, 228, 225 }, { 255, 228, 181 },
    { 255, 222, 173 }, {   0,   0, 128 }, { 253, 245, 230 }, { 128, 128,   0 },
    { 107, 142,  35 }, { 255, 165,   0 }, { 255,  69,   0 }, { 218, 112, 214 },
    { 238, 232, 170 }, { 152, 251, 152 }, { 175, 238, 238 }, { 219, 112, 147 },
    { 255, 239, 213 }, { 255, 218, 185 }, { 205, 133,  63 }, { 255, 192, 203 },
    { 221, 160, 221 }, { 176, 224, 230 }, { 128,   0, 128 }, { 255,   0,   0 },
    { 188, 143, 143 }, {  65, 105, 225 }, { 139,  69,  19 }, { 250, 128, 114 },
    { 244, 164,  96 }, {  46, 139,  87 }, { 255, 245, 238 }, { 160,  82,  45 },
    { 192, 192, 192 }, { 135, 206, 235 }, { 106,  90, 205 }, { 112, 128, 144 },
    { 112, 128, 144 }, { 255, 250, 250 }, {   0, 255, 127 }, {  70, 130, 180 },
    { 210, 180, 140 }, {   0, 128, 128 }, { 216, 191, 216 }, { 255,  99,  71 },
    {  64, 224, 208 }, { 238, 130, 238 }, { 245, 222, 179 }, { 255, 255, 255 },
    { 245, 245, 245 }, { 255, 255,   0 }, { 154, 205,  50 }
  };


/****************************************************************************************
 *                                                                                      *
 *   SVG_CANVAS DEFINITION & ROUTINES                                                   *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { FILE        *hfile;
    SVG_Context *context;
    SVG_Context *arrow_context;
    int          mskcnt;
  } MyCanvas;

SVG_Canvas *G(Begin_SVG_Drawing)(string file_name, SVG_Box *F(viewport), SVG_Frame *F(viewbox))
{ MyCanvas  *canvas;
  FILE      *hfile;
  char      *buffer;

  Rad2Ang = 180./acos(-1.);
  TwoPI   = 2*acos(-1.);

  buffer = (char *) Guarded_Malloc(strlen(file_name)+5,"Begin_SVG_Drawing"); 

  sprintf(buffer,"%s.svg",file_name);
  hfile = Guarded_Fopen(buffer,"w","Begin_SVG_Drawing"); 
  if (hfile == NULL)
    { fprintf(stderr,"Cannot open %s\n",buffer);
      free(buffer);
      return (NULL);
    }

  free(buffer);

  canvas = (MyCanvas *) Guarded_Malloc(SIZEOF(MyCanvas),"Begin_SVG_Drawing");
  canvas->hfile = hfile;
  canvas->context = Make_SVG_Context();
  canvas->arrow_context = NULL;
  canvas->mskcnt  = 0;

  fprintf(hfile,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  fprintf(hfile,"<!-- Created by mylib 1.0 -->\n");
  fprintf(hfile,"\n");
  fprintf(hfile,"<svg\n");
  fprintf(hfile,"   xmlns:mylib=\"http://www.mylib.org/mylib1.0\"\n");
  fprintf(hfile,"   xmlns=\"http://www.w3.org/2000/svg\"\n");
  fprintf(hfile,"   width=\"%gin\"\n",viewport->w);
  fprintf(hfile,"   height=\"%gin\"\n",viewport->h);
  fprintf(hfile,"   viewBox=\"%g %g %g %g\"\n",viewbox->corner.x,viewbox->corner.y,
                                               viewbox->window.w,viewbox->window.h);
  fprintf(hfile,"   preserveAspectRatio=\"none\"\n");
  fprintf(hfile,"   version=\"1.1\">\n");

  Free_SVG_Box(viewport);
  Free_SVG_Frame(viewbox);

  return ((SVG_Canvas *) canvas);
}

void Finish_SVG_Drawing(SVG_Canvas *canvas)
{ MyCanvas *c = (MyCanvas *) canvas;

  fprintf(c->hfile,"</svg>\n");
  fclose(c->hfile);
  if (c->arrow_context != NULL)
    Free_SVG_Context(c->arrow_context);
  Free_SVG_Context(c->context);
  free(c);
}


/****************************************************************************************
 *                                                                                      *
 *   SVG_CONTEXT DEFINITION & ROUTINES                                                  *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { SVG_Color pen_color;
    int       pen_rgb[3];
    double    pen_width;
    double    pen_opacity;

    int       dash_len;
    double    dash_offset;
    double   *dash_lens;

    SVG_Join  join_style;
    SVG_Cap   cap_style;
    double    miter_limit;
 
    SVG_Color fill_color;
    int       fill_rgb[3];
    double    fill_opacity;
 
    double     point_size;
    SVG_Font   font;
    char      *custom;
    SVG_Style  style;
    SVG_Align  alignment;

    SVG_Arrow  arrow_beg;
    boolean    arrow_bin;
    SVG_Arrow  arrow_end;
    boolean    arrow_ein;
    double     arrow_width;
    double     arrow_height;
  } MyContext;

static inline int mycontext_dsize(MyContext *ctx)
{ return (SIZEOF(double)*ctx->dash_len); }

static inline int mycontext_csize(MyContext *ctx)
{ if (ctx->custom == NULL)
    return (0);
  else
    return (strlen(ctx->custom)+1);
}


typedef struct __MyContext
  { struct __MyContext *next;
    struct __MyContext *prev;
    int                 refcnt;
    int                 dsize;
    int                 csize;
    MyContext           mycontext;
  } _MyContext;

static _MyContext *Free_MyContext_List = NULL;
static _MyContext *Use_MyContext_List  = NULL;

static pthread_mutex_t MyContext_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int MyContext_Offset = sizeof(_MyContext)-sizeof(MyContext);
static int MyContext_Inuse  = 0;

int SVG_Context_Refcount(SVG_Context *svg_context)
{ _MyContext *object = (_MyContext *) (((char *) svg_context) - MyContext_Offset);
  return (object->refcnt);
}

static inline int allocate_mycontext_dash_lens(MyContext *mycontext, int dsize, char *routine)
{ _MyContext *object = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  if (object->dsize < dsize)
    { void *x = Guarded_Realloc(mycontext->dash_lens,(size_t) dsize,routine);
      if (x == NULL) return (1);
      mycontext->dash_lens = x;
      object->dsize = dsize;
    }
  return (0);
}

static inline int sizeof_mycontext_dash_lens(MyContext *mycontext)
{ _MyContext *object = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  return (object->dsize);
}

static inline int allocate_mycontext_custom(MyContext *mycontext, int csize, char *routine)
{ _MyContext *object = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  if (object->csize < csize)
    { void *x = Guarded_Realloc(mycontext->custom,(size_t) csize,routine);
      if (x == NULL) return (1);
      mycontext->custom = x;
      object->csize = csize;
    }
  return (0);
}

static inline int sizeof_mycontext_custom(MyContext *mycontext)
{ _MyContext *object = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  return (object->csize);
}

static inline void kill_mycontext(MyContext *mycontext);

static inline MyContext *new_mycontext(int dsize, int csize, char *routine)
{ _MyContext *object;
  MyContext  *mycontext;

  pthread_mutex_lock(&MyContext_Mutex);
  if (Free_MyContext_List == NULL)
    { object = (_MyContext *) Guarded_Realloc(NULL,sizeof(_MyContext),routine);
      if (object == NULL) return (NULL);
      mycontext = &(object->mycontext);
      object->dsize = 0;
      mycontext->dash_lens = NULL;
      object->csize = 0;
      mycontext->custom = NULL;
    }
  else
    { object = Free_MyContext_List;
      Free_MyContext_List = object->next;
      mycontext = &(object->mycontext);
    }
  MyContext_Inuse += 1;
  object->refcnt = 1;
  if (Use_MyContext_List != NULL)
    Use_MyContext_List->prev = object;
  object->next = Use_MyContext_List;
  object->prev = NULL;
  Use_MyContext_List = object;
  pthread_mutex_unlock(&MyContext_Mutex);
  if (allocate_mycontext_dash_lens(mycontext,dsize,routine))
    { kill_mycontext(mycontext);
      return (NULL);
    }
  if (allocate_mycontext_custom(mycontext,csize,routine))
    { kill_mycontext(mycontext);
      return (NULL);
    }
  return (mycontext);
}

static inline MyContext *copy_mycontext(MyContext *mycontext)
{ MyContext *copy = new_mycontext(mycontext_dsize(mycontext),mycontext_csize(mycontext),"Copy_SVG_Context");
  void *_dash_lens = copy->dash_lens;
  void *_custom = copy->custom;
  *copy = *mycontext;
  copy->dash_lens = _dash_lens;
  if (mycontext->dash_lens != NULL)
    memcpy(copy->dash_lens,mycontext->dash_lens,(size_t) mycontext_dsize(mycontext));
  copy->custom = _custom;
  if (mycontext->custom != NULL)
    memcpy(copy->custom,mycontext->custom,(size_t) mycontext_csize(mycontext));
  return (copy);
}

SVG_Context *Copy_SVG_Context(SVG_Context *svg_context)
{ return ((SVG_Context *) copy_mycontext(((MyContext *) svg_context))); }

static inline int pack_mycontext(MyContext *mycontext)
{ _MyContext *object  = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  if (object->dsize > mycontext_dsize(mycontext))
    { int ns = mycontext_dsize(mycontext);
      if (ns != 0)
        { void *x = Guarded_Realloc(mycontext->dash_lens,(size_t) ns,"Pack_MyContext");
          if (x == NULL) return (1);
          mycontext->dash_lens = x;
        }
      else
        { free(mycontext->dash_lens);
          mycontext->dash_lens = NULL;
        }
      object->dsize = ns;
    }
  if (object->csize > mycontext_csize(mycontext))
    { int ns = mycontext_csize(mycontext);
      if (ns != 0)
        { void *x = Guarded_Realloc(mycontext->custom,(size_t) ns,"Pack_MyContext");
          if (x == NULL) return (1);
          mycontext->custom = x;
        }
      else
        { free(mycontext->custom);
          mycontext->custom = NULL;
        }
      object->csize = ns;
    }
  return (0);
}

SVG_Context *Pack_SVG_Context(SVG_Context *svg_context)
{ if (pack_mycontext(((MyContext *) svg_context))) return (NULL);
  return (svg_context);
}

SVG_Context *Inc_SVG_Context(SVG_Context *svg_context)
{ _MyContext *object  = (_MyContext *) (((char *) svg_context) - MyContext_Offset);
  pthread_mutex_lock(&MyContext_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&MyContext_Mutex);
  return (svg_context);
}

static inline void free_mycontext(MyContext *mycontext)
{ _MyContext *object  = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  pthread_mutex_lock(&MyContext_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&MyContext_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SVG_Context\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_MyContext_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_MyContext_List;
  Free_MyContext_List = object;
  MyContext_Inuse -= 1;
  pthread_mutex_unlock(&MyContext_Mutex);
}

void Free_SVG_Context(SVG_Context *svg_context)
{ free_mycontext(((MyContext *) svg_context)); }

static inline void kill_mycontext(MyContext *mycontext)
{ _MyContext *object  = (_MyContext *) (((char *) mycontext) - MyContext_Offset);
  pthread_mutex_lock(&MyContext_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&MyContext_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SVG_Context\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_MyContext_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  MyContext_Inuse -= 1;
  pthread_mutex_unlock(&MyContext_Mutex);
  if (mycontext->custom != NULL)
    free(mycontext->custom);
  if (mycontext->dash_lens != NULL)
    free(mycontext->dash_lens);
  free(((char *) mycontext) - MyContext_Offset);
}

void Kill_SVG_Context(SVG_Context *svg_context)
{ kill_mycontext(((MyContext *) svg_context)); }

static inline void reset_mycontext()
{ _MyContext *object;
  MyContext  *mycontext;
  pthread_mutex_lock(&MyContext_Mutex);
  while (Free_MyContext_List != NULL)
    { object = Free_MyContext_List;
      Free_MyContext_List = object->next;
      mycontext = &(object->mycontext);
      if (mycontext->custom != NULL)
        free(mycontext->custom);
      if (mycontext->dash_lens != NULL)
        free(mycontext->dash_lens);
      free(object);
    }
  pthread_mutex_unlock(&MyContext_Mutex);
}

void Reset_SVG_Context()
{ reset_mycontext(); }

int SVG_Context_Usage()
{ return (MyContext_Inuse); }

void SVG_Context_List(void (*handler)(SVG_Context *))
{ _MyContext *a, *b;
  for (a = Use_MyContext_List; a != NULL; a = b)
    { b = a->next;
      handler((SVG_Context *) &(a->mycontext));
    }
}

SVG_Context *Set_To_Default_Context(SVG_Context *M(context))
{ MyContext *ctx = GC(context);

  ctx->pen_color   = SVG_black;
  ctx->pen_rgb[0]  = -1;
  ctx->pen_width   = 1.0;
  ctx->pen_opacity = 1.0;

  ctx->dash_len    = 0;
  ctx->join_style  = SVG_miter_join;
  ctx->cap_style   = SVG_butt_cap;
  ctx->miter_limit = 4.0;

  ctx->fill_color   = SVG_OFF;
  ctx->fill_rgb[0]  = -1;
  ctx->fill_opacity = 1.0;
 
  ctx->font       = SVG_Times;
  ctx->custom     = NULL;
  ctx->point_size = 12.;
  ctx->style      = SVG_plain;
  ctx->alignment  = SVG_left_align;

  ctx->arrow_beg     = SVG_no_arrow;
  ctx->arrow_bin     = 0;
  ctx->arrow_end     = SVG_no_arrow;
  ctx->arrow_ein     = 0;
  ctx->arrow_width   = 3.;
  ctx->arrow_height  = 3.;

  return ((SVG_Context *) ctx);
}

SVG_Context *Make_SVG_Context()
{ MyContext *ctx;

  ctx = new_mycontext(SIZEOF(double)*10,0,"Make_SVG_Context");

  Set_To_Default_Context((SVG_Context *) ctx);

  return ((SVG_Context *) ctx);
}

SVG_Context *Get_SVG_Context(SVG_Canvas *canvas)
{ return (MO(canvas)->context); }

void Set_SVG_Context(SVG_Canvas *canvas, SVG_Context *C(context))
{ MO(canvas)->context = context; }

void Set_Arrow_Context(SVG_Canvas *M(canvas), SVG_Context *C(context))
{ if (MO(canvas)->arrow_context != NULL)
    Free_SVG_Context(MO(canvas)->arrow_context);
  MO(canvas)->arrow_context = context;
}


/****************************************************************************************
 *                                                                                      *
 *   SVG_POINTS, BOXES, FRAMES, and PATHS                                               *
 *                                                                                      *
 ****************************************************************************************/


typedef struct __SVG_Point
  { struct __SVG_Point *next;
    struct __SVG_Point *prev;
    int                 refcnt;
    SVG_Point           svg_point;
  } _SVG_Point;

static _SVG_Point *Free_SVG_Point_List = NULL;
static _SVG_Point *Use_SVG_Point_List  = NULL;

static pthread_mutex_t SVG_Point_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int SVG_Point_Offset = sizeof(_SVG_Point)-sizeof(SVG_Point);
static int SVG_Point_Inuse  = 0;

int SVG_Point_Refcount(SVG_Point *svg_point)
{ _SVG_Point *object = (_SVG_Point *) (((char *) svg_point) - SVG_Point_Offset);
  return (object->refcnt);
}

static inline void kill_svg_point(SVG_Point *svg_point);

static inline SVG_Point *new_svg_point(char *routine)
{ _SVG_Point *object;
  SVG_Point  *svg_point;

  pthread_mutex_lock(&SVG_Point_Mutex);
  if (Free_SVG_Point_List == NULL)
    { object = (_SVG_Point *) Guarded_Realloc(NULL,sizeof(_SVG_Point),routine);
      if (object == NULL) return (NULL);
      svg_point = &(object->svg_point);
    }
  else
    { object = Free_SVG_Point_List;
      Free_SVG_Point_List = object->next;
      svg_point = &(object->svg_point);
    }
  SVG_Point_Inuse += 1;
  object->refcnt = 1;
  if (Use_SVG_Point_List != NULL)
    Use_SVG_Point_List->prev = object;
  object->next = Use_SVG_Point_List;
  object->prev = NULL;
  Use_SVG_Point_List = object;
  pthread_mutex_unlock(&SVG_Point_Mutex);
  return (svg_point);
}

static inline SVG_Point *copy_svg_point(SVG_Point *svg_point)
{ SVG_Point *copy = new_svg_point("Copy_SVG_Point");
  *copy = *svg_point;
  return (copy);
}

SVG_Point *Copy_SVG_Point(SVG_Point *svg_point)
{ return ((SVG_Point *) copy_svg_point(svg_point)); }

SVG_Point *Pack_SVG_Point(SVG_Point *svg_point)
{ return (svg_point); }

SVG_Point *Inc_SVG_Point(SVG_Point *svg_point)
{ _SVG_Point *object  = (_SVG_Point *) (((char *) svg_point) - SVG_Point_Offset);
  pthread_mutex_lock(&SVG_Point_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&SVG_Point_Mutex);
  return (svg_point);
}

static inline void free_svg_point(SVG_Point *svg_point)
{ _SVG_Point *object  = (_SVG_Point *) (((char *) svg_point) - SVG_Point_Offset);
  pthread_mutex_lock(&SVG_Point_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Point_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SVG_Point\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Point_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_SVG_Point_List;
  Free_SVG_Point_List = object;
  SVG_Point_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Point_Mutex);
}

void Free_SVG_Point(SVG_Point *svg_point)
{ free_svg_point(svg_point); }

static inline void kill_svg_point(SVG_Point *svg_point)
{ _SVG_Point *object  = (_SVG_Point *) (((char *) svg_point) - SVG_Point_Offset);
  pthread_mutex_lock(&SVG_Point_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Point_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SVG_Point\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Point_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  SVG_Point_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Point_Mutex);
  free(((char *) svg_point) - SVG_Point_Offset);
}

void Kill_SVG_Point(SVG_Point *svg_point)
{ kill_svg_point(svg_point); }

static inline void reset_svg_point()
{ _SVG_Point *object;
  pthread_mutex_lock(&SVG_Point_Mutex);
  while (Free_SVG_Point_List != NULL)
    { object = Free_SVG_Point_List;
      Free_SVG_Point_List = object->next;
      free(object);
    }
  pthread_mutex_unlock(&SVG_Point_Mutex);
}

void Reset_SVG_Point()
{ reset_svg_point(); }

int SVG_Point_Usage()
{ return (SVG_Point_Inuse); }

void SVG_Point_List(void (*handler)(SVG_Point *))
{ _SVG_Point *a, *b;
  for (a = Use_SVG_Point_List; a != NULL; a = b)
    { b = a->next;
      handler((SVG_Point *) &(a->svg_point));
    }
}

SVG_Point *G(Make_SVG_Point)(double x, double y)
{ SVG_Point *point = new_svg_point("Make_SVG_Point");
  point->x = x;
  point->y = y;
  return (point);
}


typedef struct __SVG_Box
  { struct __SVG_Box *next;
    struct __SVG_Box *prev;
    int               refcnt;
    SVG_Box           svg_box;
  } _SVG_Box;

static _SVG_Box *Free_SVG_Box_List = NULL;
static _SVG_Box *Use_SVG_Box_List  = NULL;

static pthread_mutex_t SVG_Box_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int SVG_Box_Offset = sizeof(_SVG_Box)-sizeof(SVG_Box);
static int SVG_Box_Inuse  = 0;

int SVG_Box_Refcount(SVG_Box *svg_box)
{ _SVG_Box *object = (_SVG_Box *) (((char *) svg_box) - SVG_Box_Offset);
  return (object->refcnt);
}

static inline void kill_svg_box(SVG_Box *svg_box);

static inline SVG_Box *new_svg_box(char *routine)
{ _SVG_Box *object;
  SVG_Box  *svg_box;

  pthread_mutex_lock(&SVG_Box_Mutex);
  if (Free_SVG_Box_List == NULL)
    { object = (_SVG_Box *) Guarded_Realloc(NULL,sizeof(_SVG_Box),routine);
      if (object == NULL) return (NULL);
      svg_box = &(object->svg_box);
    }
  else
    { object = Free_SVG_Box_List;
      Free_SVG_Box_List = object->next;
      svg_box = &(object->svg_box);
    }
  SVG_Box_Inuse += 1;
  object->refcnt = 1;
  if (Use_SVG_Box_List != NULL)
    Use_SVG_Box_List->prev = object;
  object->next = Use_SVG_Box_List;
  object->prev = NULL;
  Use_SVG_Box_List = object;
  pthread_mutex_unlock(&SVG_Box_Mutex);
  return (svg_box);
}

static inline SVG_Box *copy_svg_box(SVG_Box *svg_box)
{ SVG_Box *copy = new_svg_box("Copy_SVG_Box");
  *copy = *svg_box;
  return (copy);
}

SVG_Box *Copy_SVG_Box(SVG_Box *svg_box)
{ return ((SVG_Box *) copy_svg_box(svg_box)); }

SVG_Box *Pack_SVG_Box(SVG_Box *svg_box)
{ return (svg_box); }

SVG_Box *Inc_SVG_Box(SVG_Box *svg_box)
{ _SVG_Box *object  = (_SVG_Box *) (((char *) svg_box) - SVG_Box_Offset);
  pthread_mutex_lock(&SVG_Box_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&SVG_Box_Mutex);
  return (svg_box);
}

static inline void free_svg_box(SVG_Box *svg_box)
{ _SVG_Box *object  = (_SVG_Box *) (((char *) svg_box) - SVG_Box_Offset);
  pthread_mutex_lock(&SVG_Box_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Box_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SVG_Box\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Box_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_SVG_Box_List;
  Free_SVG_Box_List = object;
  SVG_Box_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Box_Mutex);
}

void Free_SVG_Box(SVG_Box *svg_box)
{ free_svg_box(svg_box); }

static inline void kill_svg_box(SVG_Box *svg_box)
{ _SVG_Box *object  = (_SVG_Box *) (((char *) svg_box) - SVG_Box_Offset);
  pthread_mutex_lock(&SVG_Box_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Box_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SVG_Box\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Box_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  SVG_Box_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Box_Mutex);
  free(((char *) svg_box) - SVG_Box_Offset);
}

void Kill_SVG_Box(SVG_Box *svg_box)
{ kill_svg_box(svg_box); }

static inline void reset_svg_box()
{ _SVG_Box *object;
  pthread_mutex_lock(&SVG_Box_Mutex);
  while (Free_SVG_Box_List != NULL)
    { object = Free_SVG_Box_List;
      Free_SVG_Box_List = object->next;
      free(object);
    }
  pthread_mutex_unlock(&SVG_Box_Mutex);
}

void Reset_SVG_Box()
{ reset_svg_box(); }

int SVG_Box_Usage()
{ return (SVG_Box_Inuse); }

void SVG_Box_List(void (*handler)(SVG_Box *))
{ _SVG_Box *a, *b;
  for (a = Use_SVG_Box_List; a != NULL; a = b)
    { b = a->next;
      handler((SVG_Box *) &(a->svg_box));
    }
}

SVG_Box *G(Make_SVG_Box)(double w, double h)
{ SVG_Box *box = new_svg_box("Make_SVG_Box");
  box->w = w;
  box->h = h;
  return (box);
}


typedef struct __SVG_Frame
  { struct __SVG_Frame *next;
    struct __SVG_Frame *prev;
    int                 refcnt;
    SVG_Frame           svg_frame;
  } _SVG_Frame;

static _SVG_Frame *Free_SVG_Frame_List = NULL;
static _SVG_Frame *Use_SVG_Frame_List  = NULL;

static pthread_mutex_t SVG_Frame_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int SVG_Frame_Offset = sizeof(_SVG_Frame)-sizeof(SVG_Frame);
static int SVG_Frame_Inuse  = 0;

int SVG_Frame_Refcount(SVG_Frame *svg_frame)
{ _SVG_Frame *object = (_SVG_Frame *) (((char *) svg_frame) - SVG_Frame_Offset);
  return (object->refcnt);
}

static inline void kill_svg_frame(SVG_Frame *svg_frame);

static inline SVG_Frame *new_svg_frame(char *routine)
{ _SVG_Frame *object;
  SVG_Frame  *svg_frame;

  pthread_mutex_lock(&SVG_Frame_Mutex);
  if (Free_SVG_Frame_List == NULL)
    { object = (_SVG_Frame *) Guarded_Realloc(NULL,sizeof(_SVG_Frame),routine);
      if (object == NULL) return (NULL);
      svg_frame = &(object->svg_frame);
    }
  else
    { object = Free_SVG_Frame_List;
      Free_SVG_Frame_List = object->next;
      svg_frame = &(object->svg_frame);
    }
  SVG_Frame_Inuse += 1;
  object->refcnt = 1;
  if (Use_SVG_Frame_List != NULL)
    Use_SVG_Frame_List->prev = object;
  object->next = Use_SVG_Frame_List;
  object->prev = NULL;
  Use_SVG_Frame_List = object;
  pthread_mutex_unlock(&SVG_Frame_Mutex);
  return (svg_frame);
}

static inline SVG_Frame *copy_svg_frame(SVG_Frame *svg_frame)
{ SVG_Frame *copy = new_svg_frame("Copy_SVG_Frame");
  *copy = *svg_frame;
  return (copy);
}

SVG_Frame *Copy_SVG_Frame(SVG_Frame *svg_frame)
{ return ((SVG_Frame *) copy_svg_frame(svg_frame)); }

SVG_Frame *Pack_SVG_Frame(SVG_Frame *svg_frame)
{ return (svg_frame); }

SVG_Frame *Inc_SVG_Frame(SVG_Frame *svg_frame)
{ _SVG_Frame *object  = (_SVG_Frame *) (((char *) svg_frame) - SVG_Frame_Offset);
  pthread_mutex_lock(&SVG_Frame_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&SVG_Frame_Mutex);
  return (svg_frame);
}

static inline void free_svg_frame(SVG_Frame *svg_frame)
{ _SVG_Frame *object  = (_SVG_Frame *) (((char *) svg_frame) - SVG_Frame_Offset);
  pthread_mutex_lock(&SVG_Frame_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Frame_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SVG_Frame\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Frame_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_SVG_Frame_List;
  Free_SVG_Frame_List = object;
  SVG_Frame_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Frame_Mutex);
}

void Free_SVG_Frame(SVG_Frame *svg_frame)
{ free_svg_frame(svg_frame); }

static inline void kill_svg_frame(SVG_Frame *svg_frame)
{ _SVG_Frame *object  = (_SVG_Frame *) (((char *) svg_frame) - SVG_Frame_Offset);
  pthread_mutex_lock(&SVG_Frame_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&SVG_Frame_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SVG_Frame\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_SVG_Frame_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  SVG_Frame_Inuse -= 1;
  pthread_mutex_unlock(&SVG_Frame_Mutex);
  free(((char *) svg_frame) - SVG_Frame_Offset);
}

void Kill_SVG_Frame(SVG_Frame *svg_frame)
{ kill_svg_frame(svg_frame); }

static inline void reset_svg_frame()
{ _SVG_Frame *object;
  pthread_mutex_lock(&SVG_Frame_Mutex);
  while (Free_SVG_Frame_List != NULL)
    { object = Free_SVG_Frame_List;
      Free_SVG_Frame_List = object->next;
      free(object);
    }
  pthread_mutex_unlock(&SVG_Frame_Mutex);
}

void Reset_SVG_Frame()
{ reset_svg_frame(); }

int SVG_Frame_Usage()
{ return (SVG_Frame_Inuse); }

void SVG_Frame_List(void (*handler)(SVG_Frame *))
{ _SVG_Frame *a, *b;
  for (a = Use_SVG_Frame_List; a != NULL; a = b)
    { b = a->next;
      handler((SVG_Frame *) &(a->svg_frame));
    }
}

SVG_Frame *G(Make_SVG_Frame)(SVG_Point *F(corner), SVG_Box *F(window))
{ SVG_Frame *frame = new_svg_frame("Make_SVG_Frame");
  frame->corner = *corner;
  frame->window = *window;
  Free_SVG_Point(corner);
  Free_SVG_Box(window);
  return (frame);
}

typedef struct
  { int         len;
    char       *kind;
    SVG_Point **pnts;
  } MyPath;

static inline int mypath_psize(MyPath *path)
{ return (SIZEOF(SVG_Point *)*path->len); }

static inline int mypath_ksize(MyPath *path)
{ return (SIZEOF(char)*path->len); }


typedef struct __MyPath
  { struct __MyPath *next;
    struct __MyPath *prev;
    int              refcnt;
    int              psize;
    int              ksize;
    MyPath           mypath;
  } _MyPath;

static _MyPath *Free_MyPath_List = NULL;
static _MyPath *Use_MyPath_List  = NULL;

static pthread_mutex_t MyPath_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int MyPath_Offset = sizeof(_MyPath)-sizeof(MyPath);
static int MyPath_Inuse  = 0;

int SVG_Path_Refcount(SVG_Path *svg_path)
{ _MyPath *object = (_MyPath *) (((char *) svg_path) - MyPath_Offset);
  return (object->refcnt);
}

static inline int allocate_mypath_pnts(MyPath *mypath, int psize, char *routine)
{ _MyPath *object = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  if (object->psize < psize)
    { void *x = Guarded_Realloc(mypath->pnts,(size_t) psize,routine);
      if (x == NULL) return (1);
      mypath->pnts = x;
      object->psize = psize;
    }
  return (0);
}

static inline int sizeof_mypath_pnts(MyPath *mypath)
{ _MyPath *object = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  return (object->psize);
}

static inline int allocate_mypath_kind(MyPath *mypath, int ksize, char *routine)
{ _MyPath *object = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  if (object->ksize < ksize)
    { void *x = Guarded_Realloc(mypath->kind,(size_t) ksize,routine);
      if (x == NULL) return (1);
      mypath->kind = x;
      object->ksize = ksize;
    }
  return (0);
}

static inline int sizeof_mypath_kind(MyPath *mypath)
{ _MyPath *object = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  return (object->ksize);
}

static inline void kill_mypath(MyPath *mypath);

static inline MyPath *new_mypath(int psize, int ksize, char *routine)
{ _MyPath *object;
  MyPath  *mypath;

  pthread_mutex_lock(&MyPath_Mutex);
  if (Free_MyPath_List == NULL)
    { object = (_MyPath *) Guarded_Realloc(NULL,sizeof(_MyPath),routine);
      if (object == NULL) return (NULL);
      mypath = &(object->mypath);
      object->psize = 0;
      mypath->pnts = NULL;
      object->ksize = 0;
      mypath->kind = NULL;
    }
  else
    { object = Free_MyPath_List;
      Free_MyPath_List = object->next;
      mypath = &(object->mypath);
    }
  MyPath_Inuse += 1;
  object->refcnt = 1;
  if (Use_MyPath_List != NULL)
    Use_MyPath_List->prev = object;
  object->next = Use_MyPath_List;
  object->prev = NULL;
  Use_MyPath_List = object;
  pthread_mutex_unlock(&MyPath_Mutex);
  if (allocate_mypath_pnts(mypath,psize,routine))
    { kill_mypath(mypath);
      return (NULL);
    }
  if (allocate_mypath_kind(mypath,ksize,routine))
    { kill_mypath(mypath);
      return (NULL);
    }
  return (mypath);
}

static inline MyPath *copy_mypath(MyPath *mypath)
{ MyPath *copy = new_mypath(mypath_psize(mypath),mypath_ksize(mypath),"Copy_SVG_Path");
  void *_pnts = copy->pnts;
  void *_kind = copy->kind;
  *copy = *mypath;
  copy->pnts = _pnts;
  if (mypath->pnts != NULL)
    memcpy(copy->pnts,mypath->pnts,(size_t) mypath_psize(mypath));
  copy->kind = _kind;
  if (mypath->kind != NULL)
    memcpy(copy->kind,mypath->kind,(size_t) mypath_ksize(mypath));
  return (copy);
}

SVG_Path *Copy_SVG_Path(SVG_Path *svg_path)
{ return ((SVG_Path *) copy_mypath(((MyPath *) svg_path))); }

static inline int pack_mypath(MyPath *mypath)
{ _MyPath *object  = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  if (object->psize > mypath_psize(mypath))
    { int ns = mypath_psize(mypath);
      if (ns != 0)
        { void *x = Guarded_Realloc(mypath->pnts,(size_t) ns,"Pack_MyPath");
          if (x == NULL) return (1);
          mypath->pnts = x;
        }
      else
        { free(mypath->pnts);
          mypath->pnts = NULL;
        }
      object->psize = ns;
    }
  if (object->ksize > mypath_ksize(mypath))
    { int ns = mypath_ksize(mypath);
      if (ns != 0)
        { void *x = Guarded_Realloc(mypath->kind,(size_t) ns,"Pack_MyPath");
          if (x == NULL) return (1);
          mypath->kind = x;
        }
      else
        { free(mypath->kind);
          mypath->kind = NULL;
        }
      object->ksize = ns;
    }
  return (0);
}

SVG_Path *Pack_SVG_Path(SVG_Path *svg_path)
{ if (pack_mypath(((MyPath *) svg_path))) return (NULL);
  return (svg_path);
}

SVG_Path *Inc_SVG_Path(SVG_Path *svg_path)
{ _MyPath *object  = (_MyPath *) (((char *) svg_path) - MyPath_Offset);
  pthread_mutex_lock(&MyPath_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&MyPath_Mutex);
  return (svg_path);
}

static inline void free_mypath(MyPath *mypath)
{ _MyPath *object  = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  pthread_mutex_lock(&MyPath_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&MyPath_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released SVG_Path\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_MyPath_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_MyPath_List;
  Free_MyPath_List = object;
  MyPath_Inuse -= 1;
  pthread_mutex_unlock(&MyPath_Mutex);
}

static inline void kill_mypath(MyPath *mypath)
{ _MyPath *object  = (_MyPath *) (((char *) mypath) - MyPath_Offset);
  pthread_mutex_lock(&MyPath_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&MyPath_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released SVG_Path\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_MyPath_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  MyPath_Inuse -= 1;
  pthread_mutex_unlock(&MyPath_Mutex);
  if (mypath->kind != NULL)
    free(mypath->kind);
  if (mypath->pnts != NULL)
    free(mypath->pnts);
  free(((char *) mypath) - MyPath_Offset);
}

static inline void reset_mypath()
{ _MyPath *object;
  MyPath  *mypath;
  pthread_mutex_lock(&MyPath_Mutex);
  while (Free_MyPath_List != NULL)
    { object = Free_MyPath_List;
      Free_MyPath_List = object->next;
      mypath = &(object->mypath);
      if (mypath->kind != NULL)
        free(mypath->kind);
      if (mypath->pnts != NULL)
        free(mypath->pnts);
      free(object);
    }
  pthread_mutex_unlock(&MyPath_Mutex);
}

void Reset_SVG_Path()
{ reset_mypath(); }

int SVG_Path_Usage()
{ return (MyPath_Inuse); }

void SVG_Path_List(void (*handler)(SVG_Path *))
{ _MyPath *a, *b;
  for (a = Use_MyPath_List; a != NULL; a = b)
    { b = a->next;
      handler((SVG_Path *) &(a->mypath));
    }
}

void Free_SVG_Path(SVG_Path *path)
{ MyPath *mypath = (MyPath *) path;
  int     i;

  for (i = 0; i < mypath->len; i++)
    if (mypath->pnts[i] != NULL)
      Free_SVG_Point(mypath->pnts[i]);
  free_mypath(mypath);
}

void Kill_SVG_Path(SVG_Path *path)
{ MyPath *mypath = (MyPath *) path;
  int     i;

  for (i = 0; i < mypath->len; i++)
    if (mypath->pnts[i] != NULL)
      Kill_SVG_Point(mypath->pnts[i]);
  kill_mypath(mypath);
}

SVG_Path *G(Make_SVG_Path)(int n)
{ MyPath *mypath;
  if (n < 10) n = 10;
  mypath = new_mypath(n*SIZEOF(SVG_Point *),n*SIZEOF(char),"Make_SVG_Path");
  mypath->len = 0;
  return ((SVG_Path *) mypath);
}

SVG_Path *Append_Move(SVG_Path *R(M(path)), SVG_Point *C(point))
{ MyPath *mypath = (MyPath *) path;

  if (mypath->len > 0 && mypath->kind[mypath->len-1] == 'M')
    { mypath->len -= 1;
      Free_SVG_Point(mypath->pnts[mypath->len]);
    }
  if (sizeof_mypath_pnts(mypath) <= mypath_psize(mypath))
    { int n = 1.1*mypath->len + 10;
      allocate_mypath_pnts(mypath,n*SIZEOF(SVG_Point *),"Append_Line");
      allocate_mypath_kind(mypath,n*SIZEOF(char),"Append_Line");
    }
  mypath->kind[mypath->len] = 'M';
  mypath->pnts[mypath->len] = point;
  mypath->len += 1;
  return (path);
}

SVG_Path *Append_Line(SVG_Path *R(M(path)), SVG_Point *C(point))
{ MyPath *mypath = (MyPath *) path;

  if (mypath->len == 0)
    { fprintf(stderr,"Cannot start a path with a line to (Append_Line)\n");
      exit (1);
    }
  else if (mypath->kind[mypath->len-1] == 'Z')
    { fprintf(stderr,"A close must be followed by a move (Append_Line)\n");
      exit (1);
    }
  if (sizeof_mypath_pnts(mypath) <= mypath_psize(mypath))
    { int n = 1.1*mypath->len + 10;
      allocate_mypath_pnts(mypath,n*SIZEOF(SVG_Point *),"Append_Line");
      allocate_mypath_kind(mypath,n*SIZEOF(char),"Append_Line");
    }
  mypath->kind[mypath->len] = 'L';
  mypath->pnts[mypath->len] = point;
  mypath->len += 1;
  return (path);
}

SVG_Path *Append_Bezier(SVG_Path *R(M(path)),
                        SVG_Point *C(in), SVG_Point *C(out), SVG_Point *C(pnt))
{ MyPath *mypath = (MyPath *) path;

  if (mypath->len == 0)
    { fprintf(stderr,"Cannot start a path with a bezier segment (Append_Bezier)\n");
      exit (1);
    }
  else if (mypath->kind[mypath->len-1] == 'Z')
    { fprintf(stderr,"A close must be followed by a move (Append_Bezier)\n");
      exit (1);
    }
  if (sizeof_mypath_pnts(mypath) <= mypath_psize(mypath) + 2*SIZEOF(SVG_Point *))
    { int n = 1.1*mypath->len + 10;
      allocate_mypath_pnts(mypath,n*SIZEOF(SVG_Point *),"Append_Bezier");
      allocate_mypath_kind(mypath,n*SIZEOF(char),"Append_Bezier");
    }
  mypath->kind[mypath->len] = 'B';
  mypath->pnts[mypath->len] = in;
  mypath->pnts[mypath->len+1] = out;
  mypath->kind[mypath->len+2] = 'B';
  mypath->pnts[mypath->len+2] = pnt;
  mypath->len += 3;
  return (path);
}

SVG_Path *Append_Arc(SVG_Path *R(M(path)), SVG_Point *C(rad), double xrot, boolean large_arc,
                                           boolean sweep, SVG_Point *C(pnt))
{ MyPath *mypath = (MyPath *) path;

  if (mypath->len == 0)
    { fprintf(stderr,"Cannot start a path with an arc (Append_Arc)\n");
      exit (1);
    }
  else if (mypath->kind[mypath->len-1] == 'Z')
    { fprintf(stderr,"A close must be followed by a move (Append_Arc)\n");
      exit (1);
    }
  if (rad->x == 0. || rad->y == 0.)
    { Append_Line(path,pnt);
      Free_SVG_Point(rad);
      return (path);
    }
  if (sizeof_mypath_pnts(mypath) <= mypath_psize(mypath) + 2*SIZEOF(SVG_Point *))
    { int n = 1.1*mypath->len + 10;
      allocate_mypath_pnts(mypath,n*SIZEOF(SVG_Point *),"Append_Arc");
      allocate_mypath_kind(mypath,n*SIZEOF(char),"Append_Arc");
    }
  if (rad->x < 0.)
    rad->x = -rad->x;
  if (rad->y < 0.)
    rad->y = -rad->y;
  mypath->kind[mypath->len] = 'A';
  mypath->pnts[mypath->len] = rad;
  mypath->pnts[mypath->len+1] = Make_SVG_Point(xrot,2*(large_arc != 0) + (sweep != 0));
  mypath->kind[mypath->len+2] = 'A';
  mypath->pnts[mypath->len+2] = pnt;
  mypath->len += 3;
  return (path);
}

SVG_Path *Append_Close(SVG_Path *R(M(path)))
{ MyPath *mypath = (MyPath *) path;

  if (mypath->len == 0)
    { fprintf(stderr,"Cannot start a path with a close (Append_Close)\n");
      exit (1);
    }
  else if (mypath->kind[mypath->len-1] == 'Z')
    { fprintf(stderr,"A close cannot be preceded by another close (Append_Close)\n");
      exit (1);
    }
  else if (mypath->kind[mypath->len-1] == 'M')
    { fprintf(stderr,"A close cannot be preceded by a move (Append_Close)\n");
      exit (1);
    }
  if (sizeof_mypath_pnts(mypath) <= mypath_psize(mypath))
    { int n = 1.1*mypath->len + 10;
      allocate_mypath_pnts(mypath,n*SIZEOF(SVG_Point *),"Append_SVG_Path");
      allocate_mypath_kind(mypath,n*SIZEOF(char),"Append_SVG_Path");
    }
  mypath->kind[mypath->len] = 'Z';
  mypath->pnts[mypath->len] = NULL;
  mypath->len += 1;
  return (path);
}


/****************************************************************************************
 *                                                                                      *
 *   GRAPHICS CONTEXT SETTING                                                           *
 *                                                                                      *
 ****************************************************************************************/

void SVG_Stroke_Off(SVG_Context *context)
{ GC(context)->pen_color  = SVG_OFF;
  GC(context)->pen_rgb[0] = -1;
}

void SVG_Stroke_Color(SVG_Context *context, SVG_Color color)
{ GC(context)->pen_color  = color;
  GC(context)->pen_rgb[0] = -1;
}

void SVG_Stroke_RGB(SVG_Context *context, int red, int green, int blue)
{ GC(context)->pen_color  = SVG_black;
  GC(context)->pen_rgb[0] = red;
  GC(context)->pen_rgb[1] = green;
  GC(context)->pen_rgb[2] = blue;
}

void SVG_Stroke_Width(SVG_Context *context, double thick)
{ GC(context)->pen_width = thick; }

void SVG_Stroke_Opacity(SVG_Context *context, double pcnt)
{ GC(context)->pen_opacity = pcnt; }

void SVG_Join_Style(SVG_Context *context, SVG_Join join_style)
{ GC(context)->join_style = join_style; }

void SVG_Cap_Style(SVG_Context *context, SVG_Cap cap_style)
{ GC(context)->cap_style = cap_style; }

void SVG_Miter_Limit(SVG_Context *context, double limit)
{ GC(context)->miter_limit = limit; }

void SVG_Dash_Spec(SVG_Context *context, double offset, int n, double *dashlens)
{ MyContext *c = GC(context);

  allocate_mycontext_dash_lens(c,SIZEOF(double)*n,"SVG_Dash_Spec");
  c->dash_len = n;
  c->dash_offset = offset;
  memcpy(c->dash_lens,dashlens,SIZEOF(double)*n);
}

void SVG_Fill_Off(SVG_Context *context)
{ GC(context)->fill_color  = SVG_OFF;
  GC(context)->fill_rgb[0] = -1;
}

void SVG_Fill_Color(SVG_Context *context, SVG_Color color)
{ GC(context)->fill_color = color;
  GC(context)->fill_rgb[0] = -1;
}

void SVG_Fill_RGB(SVG_Context *context, int red, int green, int blue)
{ GC(context)->fill_color  = SVG_black;
  GC(context)->fill_rgb[0] = red;
  GC(context)->fill_rgb[1] = green;
  GC(context)->fill_rgb[2] = blue;
}

void SVG_Fill_Opacity(SVG_Context *context, double pcnt)
{ GC(context)->fill_opacity = pcnt; }

void SVG_Arrow_Mode(SVG_Context *context, boolean bin, SVG_Arrow beg, boolean ein, SVG_Arrow end)
{ GC(context)->arrow_beg = beg;
  GC(context)->arrow_end = end;
  GC(context)->arrow_ein = ein;
  GC(context)->arrow_bin = bin;
}

void SVG_Arrow_Size(SVG_Context *context, double width, double height)
{ if (width <= 0. || height <= 0.)
    { fprintf(stderr,"Scale factor is <= 0. (SVG_Arrow_Size)\n");
      exit (1);
    }
  GC(context)->arrow_width  = width;
  GC(context)->arrow_height = height;
}

void SVG_Point_Size(SVG_Context *context, double size)
{ GC(context)->point_size = size; }

void SVG_Text_Font(SVG_Context *context, SVG_Font font, SVG_Style style)
{ GC(context)->font  = font;
  GC(context)->style = style;
}

void SVG_Custom_Font(SVG_Context *context, string css_font, SVG_Style style)
{ allocate_mycontext_custom(GC(context),strlen(css_font)+1,"SVG_Custom_Font");
  strcpy(GC(context)->custom,css_font); 
  GC(context)->font = SVG_Verdana+1;
  GC(context)->style = style;
}

void SVG_Text_Alignment(SVG_Context *context, SVG_Align align)
{ GC(context)->alignment = align; }


/****************************************************************************************
 *                                                                                      *
 *   CORE DRAWING ROUTINES                                                              *
 *                                                                                      *
 ****************************************************************************************/

static void emit_attributes(FILE *hfile, MyContext *ctx, boolean open, int mask)
{ int   i;
  int  *rgb;

  if (ctx->pen_color == SVG_OFF)
    fprintf(hfile," stroke=\"none\"");
  else
    { if (ctx->pen_rgb[0] >= 0)
        rgb = ctx->pen_rgb;
      else
        rgb = PredefColors[ctx->pen_color];
      fprintf(hfile," stroke=\"#%02x%02x%02x\"",rgb[0],rgb[1],rgb[2]);

      if (ctx->pen_opacity != 1.0)
        fprintf(hfile," stroke-opacity=\"%g\"",ctx->pen_opacity);

      fprintf(hfile," stroke-width=\"%g\"",ctx->pen_width);

      if (ctx->dash_len > 0)
        { fprintf(hfile," stroke-dasharray=\"%g",ctx->dash_lens[0]);
          for (i = 1; i < ctx->dash_len; i++)
            fprintf(hfile," %g",ctx->dash_lens[i]);
          fprintf(hfile,"\" stroke-dashoffset=\"%g\"",ctx->dash_offset);
        }

      if (ctx->join_style == SVG_round_join)
        fprintf(hfile," stroke-linejoin=\"round\"");
      else if (ctx->join_style == SVG_bevel_join)
        fprintf(hfile," stroke-linejoin=\"bevel\"");
      else // ctx->join_style == SVG_miter_join
        { if (ctx->miter_limit != 4.0)
            fprintf(hfile," stroke-miterlimit=\"%g\"",ctx->miter_limit);
        }

      if (ctx->cap_style == SVG_round_cap)
        fprintf(hfile," stroke-linecap=\"round\"");
      else if (ctx->cap_style == SVG_square_cap)
        fprintf(hfile," stroke-linecap=\"square\"");
    }

  if (ctx->fill_color == SVG_OFF || open)
    fprintf(hfile, " fill=\"none\"");
  else
    { if (ctx->fill_rgb[0] >= 0)
        rgb = ctx->fill_rgb;
      else
        rgb = PredefColors[ctx->fill_color];
      fprintf(hfile," fill=\"#%02x%02x%02x\"",rgb[0],rgb[1],rgb[2]);

      if (ctx->fill_opacity != 1.0)
        fprintf(hfile," fill-opacity=\"%g\"",ctx->fill_opacity);
    }

  if (mask > 0)
    fprintf(hfile," mask=\"url(#Mask%d)\"",mask);
}

static boolean need_extent(SVG_Canvas *canvas)
{ MyContext *ctx = MC(canvas);
  return ( ctx->pen_color != SVG_OFF &&
           ctx->fill_color != SVG_OFF && ctx->pen_opacity < ctx->fill_opacity);
}

static void draw_closed_object(SVG_Canvas *canvas, SVG_Frame *extent,
                               void *args,
                               void (*draw)(SVG_Canvas *canvas, int mask, void *args))
{ FILE      *hfile = MF(canvas);
  MyContext *ctx = MC(canvas);

  if (ctx->pen_color != SVG_OFF && ctx->fill_color != SVG_OFF)
    { if (ctx->pen_opacity < ctx->fill_opacity)
        { SVG_Color pc = ctx->pen_color;
          SVG_Color fc = ctx->fill_color;
          int       p0 = ctx->pen_rgb[0];
          int       f0 = ctx->fill_rgb[0];
          double    po = ctx->pen_opacity;
          double    fo = ctx->fill_opacity;
          int     mask = ++MO(canvas)->mskcnt;

          fprintf(hfile,"<g opacity=\"%g\">\n",ctx->fill_opacity);
      
          fprintf(hfile,"<mask id=\"Mask%d\" maskUnits=\"userSpaceOnUse\"",mask);
          fprintf(hfile," x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\">\n",
                        extent->corner.x, extent->corner.y,extent->window.w, extent->window.h);
          fprintf(hfile,"<g>\n");
          fprintf(hfile,"<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\"\n",
                        extent->corner.x, extent->corner.y,extent->window.w, extent->window.h);
          fprintf(hfile,"  stroke=\"none\" fill=\"#ffffff\" />\n");

          ctx->pen_opacity = 1.;
          ctx->fill_opacity = 1.;

          ctx->pen_rgb[0] = -1;
          ctx->pen_color = SVG_black;
          ctx->fill_rgb[0] = -1;
          ctx->fill_color = SVG_OFF;
            draw(canvas,-1,args);

          fprintf(hfile,"</g>\n</mask>\n");

          ctx->pen_rgb[0] = p0;
          ctx->pen_color = pc;
          if (fo <= 0.)
            fprintf(hfile,"<g opacity=\"0.\">\n");
          else
            fprintf(hfile,"<g opacity=\"%g\">\n",po/fo);
          draw(canvas,0,args);
          fprintf(hfile,"</g>\n");

          ctx->fill_rgb[0] = f0;
          ctx->fill_color = fc;
          ctx->pen_rgb[0] = -1;
          ctx->pen_color = SVG_OFF;
          ctx->fill_opacity = 1.;
            draw(canvas,mask,args);

          ctx->pen_color = pc;
          ctx->pen_rgb[0] = p0;

          ctx->pen_opacity = po;
          ctx->fill_opacity = fo;

          fprintf(hfile,"</g>\n");

          if (extent != NULL)
            Free_SVG_Frame(extent);
          return;
        }
      else if (ctx->pen_opacity >= ctx->fill_opacity && ctx->pen_opacity < 1.)
        { SVG_Color pc = ctx->pen_color;
          SVG_Color fc = ctx->fill_color;
          int       p0 = ctx->pen_rgb[0];
          int       f0 = ctx->fill_rgb[0];
          double    po = ctx->pen_opacity;
          double    fo = ctx->fill_opacity;

          fprintf(hfile,"<g opacity=\"%g\">\n",ctx->pen_opacity);

          ctx->pen_rgb[0] = -1;
          ctx->pen_color = SVG_OFF;
          if (ctx->pen_opacity <= 0.)
            ctx->fill_opacity = 0.;
          else
            ctx->fill_opacity = ctx->fill_opacity/ctx->pen_opacity;
            draw(canvas,0,args);
          ctx->fill_opacity = fo;
          ctx->pen_color = pc;
          ctx->pen_rgb[0] = p0;

          ctx->fill_rgb[0] = -1;
          ctx->fill_color = SVG_OFF;
          ctx->pen_opacity = 1.;
            draw(canvas,0,args);
          ctx->pen_opacity = po;
          ctx->fill_color = fc;
          ctx->fill_rgb[0] = f0;

          fprintf(hfile,"</g>\n");

          if (extent != NULL)
            Free_SVG_Frame(extent);
          return;
        }
    }

  if (extent != NULL)
    Free_SVG_Frame(extent);
  draw(canvas,0,args);
}


/****************************************************************************************
 *                                                                                      *
 *   CLOSED PATH DRAWER                                                                 *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { MyPath   *path;
    int       beg;
    int       end;
  } Path_Args;

static double arc_angle(SVG_Point *, SVG_Point *, double, boolean, boolean,
                        SVG_Point *, double, SVG_Frame *);

SVG_Frame *path_extent(Path_Args *args, MyContext *ctx)
{ MyPath    *pth      = args->path;
  int        m        = args->beg;
  int        n        = args->end;
  SVG_Frame *extent;
  SVG_Point *p;
  double     pad;
  int        i;

#define UPDATE_EXTENT(p)		\
{ if ((p)->x < extent->corner.x)	\
    extent->corner.x = (p)->x;		\
  else if ((p)->x > extent->window.w)	\
    extent->window.w = (p)->x;		\
  if ((p)->y < extent->corner.y)	\
    extent->corner.y = (p)->y;		\
  else if ((p)->y > extent->window.h)	\
    extent->window.h = (p)->y;		\
}

  p = pth->pnts[m];

  extent = Make_SVG_Frame(Copy_SVG_Point(p),Make_SVG_Box(p->x,p->y));

  for (i = m+1; i <= n; i++)
    if (pth->kind[i] == 'L')
      { p = pth->pnts[i];
        UPDATE_EXTENT(p);
      }
    else if (pth->kind[i] == 'B')
      { p = pth->pnts[i];
        UPDATE_EXTENT(p);
        p = pth->pnts[i+1];
        UPDATE_EXTENT(p);
        p = pth->pnts[i+2];
        UPDATE_EXTENT(p);
        i += 2;
      }
    else if (pth->kind[i] == 'A')
      { int flags = pth->pnts[i+1]->y;
        arc_angle(pth->pnts[i-1],pth->pnts[i],pth->pnts[i+1]->x,
                  (flags >= 2),(flags % 2),pth->pnts[i+2],0.,extent);
        i += 2;
      }

  if (ctx->cap_style == SVG_square_cap)
    pad = sqrt(.5)*ctx->pen_width;
  else
    pad = .5*ctx->pen_width;
  if (ctx->join_style == SVG_miter_join && ctx->miter_limit > pad)
    pad = ctx->miter_limit;

  extent->corner.x -= pad;
  extent->corner.y -= pad;
  extent->window.w -= (extent->corner.x - pad);
  extent->window.h -= (extent->corner.y - pad);

  return (extent);
}

void draw_path(SVG_Canvas *canvas, int mask, void *args)
{ FILE      *hfile    = MF(canvas);
  MyPath    *pth      = ((Path_Args *) args)->path;
  int        m        = ((Path_Args *) args)->beg;
  int        n        = ((Path_Args *) args)->end;
  boolean    open;

  MyContext *ctx = MC(canvas);
  int        i;

  open = 1;
  fprintf(hfile,"<path d=\"");
  for (i = m; i <= n; i++)
    if (pth->kind[i] == 'M')
      fprintf(hfile," M %g %g",pth->pnts[i]->x,pth->pnts[i]->y);
    else if (pth->kind[i] == 'L')
      fprintf(hfile," L %g %g",pth->pnts[i]->x,pth->pnts[i]->y);
    else if (pth->kind[i] == 'B')
      { if (pth->pnts[i] == NULL)
          fprintf(hfile," S");
        else
          fprintf(hfile," C %g %g",pth->pnts[i]->x,pth->pnts[i]->y);
        fprintf(hfile," %g %g",pth->pnts[i+1]->x,pth->pnts[i+1]->y);
        fprintf(hfile," %g %g",pth->pnts[i+2]->x,pth->pnts[i+2]->y);
        i += 2;
      }
    else if (pth->kind[i] == 'A')
      { int flags = pth->pnts[i+1]->y;
        fprintf(hfile," A %g %g",pth->pnts[i]->x,pth->pnts[i]->y);
        fprintf(hfile," %g %d %d",pth->pnts[i+1]->x,(flags >= 2),(flags%2));
        fprintf(hfile," %g %g",pth->pnts[i+2]->x,pth->pnts[i+2]->y);
        i += 2;
      }
    else //  pth->kind[i] == 'Z'
      { fprintf(hfile," Z");
        open = 0;
      }
  fprintf(hfile,"\"\n");

  emit_attributes(hfile,ctx,open,mask);

  fprintf(hfile," />\n");
}


/****************************************************************************************
 *                                                                                      *
 *   ARROWHEAD ANGLE CALCULATIONS                                                       *
 *                                                                                      *
 ****************************************************************************************/

static double line_angle(SVG_Point *p0, SVG_Point *p1)
{ double dx = p0->x - p1->x;
  double dy = p0->y - p1->y;
  double angle = atan(dy/dx) * Rad2Ang;
  if (dx < 0.)
    angle += 180.;
  return (angle);
}

static double bez_val(double t, double *c)
{ return (((c[3]*t + c[2])*t + c[1])*t + c[0]); }

static double bez_dist(double t, double *cx, double *cy)
{ double x = ((cx[3]*t + cx[2])*t + cx[1])*t;
  double y = ((cy[3]*t + cy[2])*t + cy[1])*t;
  return (x*x + y*y);
}

static double bezier_angle(SVG_Point *p0, SVG_Point *p1, SVG_Point *p2, SVG_Point *p3, double tail)
{ double     cx[4], cy[4];
  double     r, s, t;
  double     dx, dy, angle;

  cx[0] = p0->x;
  cx[1] = 3.*(p1->x - p0->x);
  cx[2] = 3.*(p0->x + p2->x - 2.*p1->x);
  cx[3] = (p3->x - p0->x) + 3.*(p1->x - p2->x);

  cy[0] = p0->y;
  cy[1] = 3.*(p1->y - p0->y);
  cy[2] = 3.*(p0->y + p2->y - 2.*p1->y);
  cy[3] = (p3->y - p0->y) + 3.*(p1->y - p2->y);

#ifdef DEBUG
  for (t = 0.; t <= 1.; t += .05)
    printf(" %g: (%g,%g) %g\n",t,bez_val(t,cx),bez_val(t,cy),bez_dist(t,cx,cy));
  printf(" 1.: (%g,%g) %g\n",bez_val(1.,cx),bez_val(1.,cy),bez_dist(1.,cx,cy));
#endif

  for (t = .05; t < 1.; t += .05)
    if (bez_dist(t,cx,cy) >= tail)
      break;
  if (t >= 1.)
    { dx = p1->x - p0->x;
      dy = p1->y - p0->y;
    }
  else
    { s = t-.05;
#ifdef DEBUG
      printf("  [%g,%g]\n",s,t);
#endif
      while (t-s > .0001)
        { r = (s+t)/2.;
          if (bez_dist(r,cx,cy) >= tail)
            t = r;
          else
            s = r;
#ifdef DEBUG
          printf("  [%g,%g]\n",s,t);
#endif
        }
      dx = p0->x - bez_val(t,cx);
      dy = p0->y - bez_val(t,cy);
#ifdef DEBUG
      printf("  t = %g  dx = %g  dy = %g\n",t,dx,dy);
#endif
    }

  angle = atan(dy/dx) * Rad2Ang;
  if (dx < 0.)
    angle += 180.;
  return (angle);
}

static double arc_val(double t, double *arc)
{ return (arc[0]*cos(t) + arc[1]*sin(t) + arc[2]); }

static double arc_dist(double t, double *arcx, double *arcy, SVG_Point *p)
{ double x = arc_val(t,arcx) - p->x;
  double y = arc_val(t,arcy) - p->y;
  return (x*x + y*y);
}

static double arc_angle(SVG_Point *p0, SVG_Point *rad, double orient, boolean large, boolean sweep,
                        SVG_Point *p1, double tail, SVG_Frame *extent)
{ double     sin0, cos0, theta;
  double     xm, ym, xp, yp;
  double     rx, ry, rx2, ry2, sc;
  double     bx, by, cx, cy;
  double     vx, vy, nv;
  double     t1, t2;
  double     arcx[3], arcy[3];
  double     r, s, t;
  double     dx, dy, angle;
  int        k;

  theta = orient / Rad2Ang;
  sin0  = sin(theta);
  cos0  = cos(theta);

  xm = .5*(p0->x - p1->x);
  ym = .5*(p0->y - p1->y);

  xp = cos0*xm + sin0*ym;
  yp = cos0*ym - sin0*xm;

  rx  = rad->x;
  ry  = rad->y;
  rx2 = rx*rx;
  ry2 = ry*ry;

  sc = xp*xp/rx2 + yp*yp/ry2;
  if (sc > 1.)
    { sc  = sqrt(sc); 
      rx  = sc*rx;
      ry  = sc*ry;
      rx2 = rx*rx;
      ry2 = ry*ry;
    }

  sc = rx2*yp*yp + ry2*xp*xp;
  sc = (rx2*ry2 - sc) / sc;
  if (sc <= 0.)
    sc = 0.;
  else
    sc = sqrt(sc);
  if (large == sweep)
    sc = -sc;
  bx = (sc*rx*yp)/ry;
  by = -(sc*ry*xp)/rx;

  cx = cos0*bx - sin0*by + .5*(p0->x + p1->x);
  cy = cos0*by + sin0*bx + .5*(p0->y + p1->y);

  vx = (xp-bx)/rx;
  vy = (yp-by)/ry;
  nv = sqrt(vx*vx + vy*vy);
  t1 = acos(vx/nv);
  if (vy < 0)
    t1 = -t1;

  vx = -(xp+bx)/rx;
  vy = -(yp+by)/ry;
  nv = sqrt(vx*vx + vy*vy);
  t2 = acos(vx/nv);
  if (vy < 0)
    t2 = -t2;

  t2 -= t1;
  if (t2 < 0.)
    { if (sweep)
        t2 += TwoPI;
    }
  else
    { if ( ! sweep)
        t2 -= TwoPI;
    }

  arcx[0] = rx*cos0;
  arcx[1] = -ry*sin0;
  arcx[2] = cx;

  arcy[0] = rx*sin0;
  arcy[1] = ry*cos0;
  arcy[2] = cy;

#ifdef DEBUG
  printf("  center = (%g,%g)  angle = %g , %g\n",cx,cy,t1*Rad2Ang,t2*Rad2Ang);
  printf("  At t1 = (%g,%g),  At t1+t2 = (%g,%g)\n",
         arc_val(t1,arcx),arc_val(t1,arcy),
         arc_val(t1+t2,arcx),arc_val(t1+t2,arcy));
  for (t = 0; t < 360; t += 5)
    printf("   %3g: (%g,%g)\n",t,arc_val(t/Rad2Ang,arcx),arc_val(t/Rad2Ang,arcy));
#endif

  if (extent == NULL)
    { for (k = 0, t = t1; k <= 36; k++, t += t2/72.)
        if (arc_dist(t,arcx,arcy,p0) >= tail)
          break;
      if (k > 36)
        { dx = p1->x - p0->x;
          dy = p1->y - p0->y;
        }
      else
        { s = t - t2/72.;
#ifdef DEBUG
          printf("  [%g,%g]\n",s*Rad2Ang,t*Rad2Ang);
#endif
          while (fabs(t-s) > .0001)
            { r = (s+t)/2.;
              if (arc_dist(r,arcx,arcy,p0) >= tail)
                t = r;
              else
                s = r;
#ifdef DEBUG
              printf("  [%g,%g]\n",s*Rad2Ang,t*Rad2Ang);
#endif
            }
          dx = p0->x - arc_val(t,arcx);
          dy = p0->y - arc_val(t,arcy);
#ifdef DEBUG
          printf("  t = %g  dx = %g  dy = %g  (%g,%g) -> (%g,%g) %g\n",
                  t,dx,dy,p0->x,p0->y,arc_val(t,arcx),arc_val(t,arcy),
                  arc_dist(t,arcx,arcy,p0));
#endif
        }
      angle = atan(dy/dx) * Rad2Ang;
      if (dx < 0.)
        angle += 180.;
    }
  else
    { double x, y, t;

      t = atan(arcx[1]/arcx[0]);
      x = arc_val(t,arcx);
      t = arc_val(t+TwoPI/2.,arcx);
      if (t < x)
        { if (t < extent->corner.x) extent->corner.x = t;
          if (x > extent->window.w) extent->window.w = x;
        }
      else
        { if (x < extent->corner.x) extent->corner.x = x;
          if (t > extent->window.w) extent->window.w = t;
        }
      t = atan(arcy[1]/arcy[0]);
      y = arc_val(t,arcy);
      t = arc_val(t+TwoPI/2.,arcy);
      if (t < y)
        { if (t < extent->corner.y) extent->corner.y = t;
          if (y > extent->window.h) extent->window.h = y;
        }
      else
        { if (y < extent->corner.y) extent->corner.y = y;
          if (t > extent->window.h) extent->window.h = t;
        }
      angle = 0.;
    }

  return (angle);
}


/****************************************************************************************
 *                                                                                      *
 *   ARROWHEAD DRAWING ROUTINES                                                         *
 *                                                                                      *
 ****************************************************************************************/

static SVG_Point APP[6] =
  { { -1., .5 }, {  0., 0. }, {-1., -.5 }, { -1., 0. }, { -.75, 0. }, { -1.5, 0. } };

static SVG_Point *APS[19] =
  { APP, APP+1, APP+2, APP+3,
    APP, APP+1, APP+2, APP+3, APP,
    APP, APP+1, APP+2, APP+4, APP,
    APP, APP+1, APP+2, APP+5, APP
  };

static char APK[9] =
  { 'M', 'L', 'L', 'M',
    'M', 'L', 'L', 'L', 'Z',
  };

static MyPath UnitArrows[5] =
  { { 0, APK  , APS    },
    { 4, APK  , APS    },
    { 5, APK+4, APS+ 4 },
    { 5, APK+4, APS+ 9 },
    { 5, APK+4, APS+14 },
  };

static SVG_Point SPP[4];

static SVG_Point *SPS[5] = { SPP, SPP+1, SPP+2, SPP+3, SPP+4 };

static MyPath ScaleArrows[5] =
  { { 0, APK  , SPS  },
    { 4, APK  , SPS  },
    { 5, APK+4, SPS  },
    { 5, APK+4, SPS  },
    { 5, APK+4, SPS  },
  };

static SVG_Point MPP[4] =
  { {  0., .5 }, { 0., -.5 }, { -1.5, -.5}, {-1.5, .5} };

static SVG_Point *MPS[34] =
  { APP, MPP  , MPP+1, APP+2, APP+1, NULL,
    APP, MPP  , MPP+1, APP+2, APP+3, NULL,
    APP, MPP  , MPP+1, APP+2, APP+4, NULL,
    APP, MPP  , MPP+1, APP+2, APP+5, NULL,
    APP, APP+1, APP+2, NULL,
    APP, APP+1, APP+2, MPP+2, MPP+3, NULL
  };

static char MPK[10] =
  { 'M', 'L', 'L', 'L', 'L', 'Z',
    'M', 'L', 'L', 'Z'
  };

static MyPath MaskArrowsOut[5] =
  { { 0, NULL, NULL   },
    { 6, MPK , MPS    },
    { 6, MPK , MPS+ 6 },
    { 6, MPK , MPS+12 },
    { 6, MPK , MPS+18 }
  };

static MyPath MaskArrowsIn[5] =
  { { 0, NULL , NULL   },
    { 4, MPK+6, MPS+24 },
    { 4, MPK+6, MPS+24 },
    { 4, MPK+6, MPS+24 },
    { 6, MPK  , MPS+28 },
  };

static void draw_arrow_mask(SVG_Canvas *canvas, SVG_Arrow akind, boolean ain)
{ MyPath    *pth;
  FILE      *hfile = MF(canvas);
  MyContext *ctx = MC(canvas);
  MyContext *atx;
  int        i;
  double     sx = ctx->arrow_width;
  double     sy = ctx->arrow_height;
  double     lx;

  if (ain)
    pth = MaskArrowsIn + akind; 
  else
    pth = MaskArrowsOut + akind; 

  fprintf(hfile,"<path d=\"");
  for (i = 0; i < pth->len; i++)
    if (pth->kind[i] == 'M')
      fprintf(hfile," M %g %g",sx*pth->pnts[i]->x,sy*pth->pnts[i]->y);
    else if (pth->kind[i] == 'L')
      fprintf(hfile," L %g %g",sx*pth->pnts[i]->x,sy*pth->pnts[i]->y);
    else //  pth->kind[i] == 'Z'
      fprintf(hfile," Z");
  fprintf(hfile,"\"\n");

  fprintf(hfile," stroke=\"none\" fill=\"#000000\" />\n");

  pth = UnitArrows + akind; 

  fprintf(hfile,"<path d=\"");
  for (i = 0; i < pth->len; i++)
    if (pth->kind[i] == 'M')
      fprintf(hfile," M %g %g",sx*pth->pnts[i]->x,sy*pth->pnts[i]->y);
    else if (pth->kind[i] == 'L')
      fprintf(hfile," L %g %g",sx*pth->pnts[i]->x,sy*pth->pnts[i]->y);
    else //  pth->kind[i] == 'Z'
      fprintf(hfile," Z");
  fprintf(hfile,"\"\n");

  if (MO(canvas)->arrow_context != NULL)
    atx = MO(canvas)->arrow_context;
  else
    atx = ctx;

  fprintf(hfile," stroke=\"#000000\" stroke-width=\"%g\"",atx->pen_width);

  if (atx->join_style == SVG_round_join)
    fprintf(hfile," stroke-linejoin=\"round\"");
  else if (atx->join_style == SVG_bevel_join)
    fprintf(hfile," stroke-linejoin=\"bevel\"");
  else // atx->join_style == SVG_miter_join
    { if (atx->miter_limit != 4.0)
        fprintf(hfile," stroke-miterlimit=\"%g\"",atx->miter_limit);
    }

  fprintf(hfile," fill=\"none\" />\n");

  if (ain)
    if (akind == SVG_flint_arrow)
      lx = -1.5;
    else
      lx = -1.0;
  else
    lx = 0.;
  fprintf(hfile,"<path d=\"M %g %g L %g %g\"\n",lx*sx,.5*sy,lx*sx,-.5*sy);
  fprintf(hfile," stroke=\"#000000\" stroke-width=\"%g\" fill=\"none\" />\n",ctx->pen_width);
}

#define DRAW_ARROW(atype,point,angle)						\
{ SVG_Context *ptx;								\
  MyPath      *arrow, *urrow;							\
  int          i;								\
										\
  fprintf(hfile,"<g transform=\"translate(%g %g) rotate(%g)\">\n",		\
                (point)->x,(point)->y,(angle));					\
										\
  ptx = MC(canvas);								\
  MO(canvas)->context = atx;							\
										\
  arrow = ScaleArrows + (atype);					        \
  urrow = UnitArrows + (atype);						        \
										\
  for (i = 0; i < arrow->len; i++)						\
    { arrow->pnts[i]->x = sx * urrow->pnts[i]->x;				\
      arrow->pnts[i]->y = sy * urrow->pnts[i]->y;				\
    }										\
										\
  parg.path = arrow;								\
  parg.beg  = 0;								\
  parg.end  = parg.path->len - 1;						\
  if ((atype) == SVG_line_arrow)						\
    draw_path(canvas,0,(void *) &parg);						\
  else										\
    draw_closed_object(canvas,path_extent(&parg,atx),(void *) &parg,draw_path);	\
										\
  MO(canvas)->context = ptx;							\
										\
  fprintf(hfile,"</g>\n");							\
}

static void draw_arrow_path(SVG_Canvas *canvas, MyPath *pth, int m, int n,
                            double tailb, double taile)
{ FILE      *hfile = MF(canvas);
  MyContext *ctx   = MC(canvas);
  MyContext *atx;

  double     bangle, eangle;
  SVG_Point  pbeg, pend;
  Path_Args  parg;

  double     sx = ctx->arrow_width;
  double     sy = ctx->arrow_height;

  if (MO(canvas)->arrow_context != NULL)
    atx = MO(canvas)->arrow_context;
  else
    atx = ctx;

  bangle = 0.;
  if (ctx->arrow_beg != SVG_no_arrow)
    { pbeg = *(pth->pnts[m]);
      if (pth->kind[m+1] == 'L')
        bangle = line_angle(&pbeg,pth->pnts[m+1]);
      else if (pth->kind[m+1] == 'B')
        bangle = bezier_angle(&pbeg,pth->pnts[m+1],pth->pnts[m+2],pth->pnts[m+3],tailb);
      else // pth->kind[m+1] == 'A'
        { int flags = pth->pnts[m+2]->y;
          bangle = arc_angle(&pbeg,pth->pnts[m+1],pth->pnts[m+2]->x,
                             (flags >= 2),(flags % 2),pth->pnts[m+3],tailb,NULL);
        }

      if (ctx->arrow_bin)
        { pbeg.x -= sqrt(tailb) * cos(bangle/Rad2Ang);
          pbeg.y -= sqrt(tailb) * sin(bangle/Rad2Ang);
          bangle  += 180.;
          if (bangle > 360.)
            bangle -= 360.;
        }

      DRAW_ARROW(ctx->arrow_beg,&pbeg,bangle);
    }

  eangle = 0.;
  if (ctx->arrow_end != SVG_no_arrow)
    { pend = *(pth->pnts[n]);
      if (pth->kind[n] == 'L')
        eangle = line_angle(&pend,pth->pnts[n-1]);
      else if (pth->kind[n] == 'B')
        eangle = bezier_angle(&pend,pth->pnts[n-1],pth->pnts[n-2],pth->pnts[n-3],taile);
      else // pth->kind[n] == 'A'
        { int flags = pth->pnts[n-1]->y;
          eangle = arc_angle(&pend,pth->pnts[n-2],pth->pnts[n-1]->x,
                             (flags >= 2), ! (flags % 2),pth->pnts[n-3],taile,NULL);
        }

      if (ctx->arrow_ein)
        { pend.x -= sqrt(taile) * cos(eangle/Rad2Ang);
          pend.y -= sqrt(taile) * sin(eangle/Rad2Ang);
          eangle  += 180.;
          if (eangle > 360.)
            eangle -= 360.;
        }

      DRAW_ARROW(ctx->arrow_end,&pend,eangle);
    }

  { int        maskid = ++MO(canvas)->mskcnt;
    SVG_Frame *mbox;

    parg.path = pth;
    parg.beg  = m;
    parg.end  = n;

    mbox = path_extent(&parg,ctx);

    fprintf(hfile,"  <mask id=\"Mask%d\" maskUnits=\"userSpaceOnUse\"",maskid);
    fprintf(hfile," x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\">\n",
                  mbox->corner.x,mbox->corner.y,mbox->window.w,mbox->window.h);
    fprintf(hfile,"    <g>\n");
    fprintf(hfile,"    <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\"",
                  mbox->corner.x,mbox->corner.y,mbox->window.w,mbox->window.h);
    fprintf(hfile," stroke=\"none\" fill=\"#ffffff\" fill-opacity=\"1.\" />\n");      

    Free_SVG_Frame(mbox);

    if (ctx->arrow_beg != SVG_no_arrow)
      { fprintf(hfile,"    <g transform=\"translate(%g %g) rotate(%g)\">\n",pbeg.x,pbeg.y,bangle);
        draw_arrow_mask(canvas,ctx->arrow_beg,ctx->arrow_bin);
 // (SVG_Path *) (MaskArrows + ctx->arrow_beg)); 
        fprintf(hfile,"    </g>\n");
      }

    if (ctx->arrow_end != SVG_no_arrow)
      { fprintf(hfile,"    <g transform=\"translate(%g %g) rotate(%g)\">\n",pend.x,pend.y,eangle);
        draw_arrow_mask(canvas,ctx->arrow_end,ctx->arrow_ein);
 // (SVG_Path *) (MaskArrows + ctx->arrow_end)); 
        fprintf(hfile,"    </g>\n");
      }

    fprintf(hfile,"    </g>\n");
    fprintf(hfile,"  </mask>\n");

    draw_path(canvas,maskid,(void *) &parg);
  }
}


/****************************************************************************************
 *                                                                                      *
 *   PATH DRAWING ROUTINE: TOP-LEVEL                                                    *
 *                                                                                      *
 ****************************************************************************************/

void Draw_SVG_Path(SVG_Canvas *canvas, SVG_Path *path)
{ MyPath    *pth   = (MyPath *) path;
  MyContext *ctx = MC(canvas);
  boolean    noa;
  double     tailb, taile;
  int        i, m;
  Path_Args  parg;

  if (ctx->arrow_beg == SVG_no_arrow && ctx->arrow_end == SVG_no_arrow)
    { noa = 1;
      tailb = taile = 0.;
    }
  else
    { SVG_Path  *barrow = (SVG_Path *) (UnitArrows + ctx->arrow_beg); 
      SVG_Path  *earrow = (SVG_Path *) (UnitArrows + ctx->arrow_end); 
      MyContext *atx;

      noa = 0;

      if (MO(canvas)->arrow_context != NULL)
        atx = MO(canvas)->arrow_context;
      else
        atx = ctx;

      tailb = ctx->arrow_width * ((MyPath *) barrow)->pnts[3]->x;
      taile = ctx->arrow_width * ((MyPath *) earrow)->pnts[3]->x;

      if (atx->pen_color != SVG_OFF)
        { if (ctx->arrow_beg == SVG_flat_arrow)
            tailb -= .5*atx->pen_width;
          else if (ctx->arrow_beg != SVG_line_arrow)
            { double ratio = (2.*tailb) / (3.*ctx->arrow_height);
              double addon = .5 * atx->pen_width * sqrt(1. - ratio);
              if (ctx->arrow_beg == SVG_flint_arrow)
                { if (atx->join_style == SVG_round_join)
                    addon = .5*atx->pen_width;
                  else if (atx->join_style == SVG_bevel_join || addon > atx->miter_limit)
                    addon /= (1. - ratio);
                }
              tailb -= addon;
            }

          if (ctx->arrow_end == SVG_flat_arrow)
            tailb -= .5*atx->pen_width;
          else if (ctx->arrow_end != SVG_line_arrow)
            { double ratio = (2.*taile) / (3.*ctx->arrow_height);
              double addon = .5 * atx->pen_width * sqrt(1. - ratio);
              if (ctx->arrow_end == SVG_flint_arrow)
                { if (atx->join_style == SVG_round_join)
                    addon = .5*atx->pen_width;
                  else if (atx->join_style == SVG_bevel_join || addon > atx->miter_limit)
                    addon /= (1. - ratio);
                }
              taile -= addon;
            }
        }

      tailb = tailb*tailb;
      taile = taile*taile;
    }

  parg.path = pth;
  m = -1;
  for (i = 0; i < pth->len; i++)
    if (pth->kind[i] == 'M')
      { if (m != -1)
          { if (noa)
              { parg.beg = m;
                parg.end = i-1;
                draw_closed_object(canvas,NULL,(void *) &parg,draw_path);
              }
            else
              draw_arrow_path(canvas,pth,m,i-1,tailb,taile);
          }
        m = i;
      }
    else if (pth->kind[i] == 'Z')
      { parg.beg = m;
        parg.end = i;
        if (need_extent(canvas))
          draw_closed_object(canvas,path_extent(&parg,ctx),(void *) &parg,draw_path);
        else
          draw_closed_object(canvas,NULL,(void *) &parg,draw_path);
        m = -1;
      }
  if (m != -1 && m < i-1)
    { if (noa)
        { parg.beg = m;
          parg.end = i-1;
          draw_closed_object(canvas,NULL,(void *) &parg,draw_path);
        }
      else
        draw_arrow_path(canvas,pth,m,i-1,tailb,taile);
    }
}


/****************************************************************************************
 *                                                                                      *
 *   DRAW SIMPLE CLOSED OBJECT: RECTANGLES, CIRCLES, & ELLIPSES                         *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { SVG_Frame *frame;
    double     round;
  } Rect_Args;

void draw_rectangle(SVG_Canvas *canvas, int mask, void *args)
{ FILE      *hfile    = MF(canvas);
  SVG_Frame *rect     = ((Rect_Args *) args)->frame;
  double    roundness = ((Rect_Args *) args)->round;

  fprintf(hfile,"<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\"",
                rect->corner.x,rect->corner.y,rect->window.w,rect->window.h);
  if (roundness > 0.)
    fprintf(hfile," rx=\"%g\" ry=\"%g\"",roundness,roundness);
  fprintf(hfile,"\n");

  emit_attributes(hfile,MC(canvas),0,mask);

  fprintf(hfile," />\n");
}

void SVG_Rectangle(SVG_Canvas *canvas, SVG_Frame *rect, double roundness)
{ Rect_Args rarg;

  rarg.frame = rect;
  rarg.round = roundness;

  if (need_extent(canvas))
    { SVG_Frame *extent = Copy_SVG_Frame(rect);
      double     pw     = .5*MC(canvas)->pen_width;
      extent->corner.x -= pw;
      extent->corner.y -= pw;
      extent->window.w += 2*pw;
      extent->window.h += 2*pw;
      draw_closed_object(canvas,extent,(void *) &rarg,draw_rectangle);
    }
  else
    draw_closed_object(canvas,NULL,(void *) &rarg,draw_rectangle);

  Free_SVG_Frame(rect);
}

typedef struct
  { SVG_Point *center;
    SVG_Point *radii;
    double     angle;
  } Ellip_Args;

void draw_ellipse(SVG_Canvas *canvas, int mask, void *args)
{ FILE      *hfile    = MF(canvas);
  SVG_Point *center   = ((Ellip_Args *) args)->center;
  SVG_Point *radii    = ((Ellip_Args *) args)->radii;
  double     angle    = ((Ellip_Args *) args)->angle;

  if (angle != 0. && mask >= 0)
    fprintf(hfile,"<g transform=\"rotate(%g %g %g)\">\n",angle,center->x,center->y);

  fprintf(hfile,"<ellipse cx=\"%g\" cy=\"%g\" rx=\"%g\" ry=\"%g\"",
                center->x,center->y,radii->x,radii->y);

  emit_attributes(hfile,MC(canvas),0,mask);

  fprintf(hfile," />\n");

  if (angle != 0. && mask >= 0)
    fprintf(hfile,"</g>\n");
}

void SVG_Ellipse(SVG_Canvas *canvas, SVG_Point *center, SVG_Point *radii, double angle)
{ Ellip_Args earg;

  earg.center = center;
  earg.radii  = radii;
  earg.angle  = angle;

  if (need_extent(canvas))
    { double     xrad;
      double     yrad;
      SVG_Point *corner;
      SVG_Box   *dims;
      SVG_Frame *extent;

      xrad = radii->x + .5*MC(canvas)->pen_width;
      yrad = radii->y + .5*MC(canvas)->pen_width;

      corner = Make_SVG_Point(center->x-xrad,center->y-yrad);
      dims   = Make_SVG_Box(2*xrad,2*yrad);
      extent = Make_SVG_Frame(corner,dims);
      draw_closed_object(canvas,extent,(void *) &earg,draw_ellipse);
    }
  else
    draw_closed_object(canvas,NULL,(void *) &earg,draw_ellipse);

  Free_SVG_Point(center);
  Free_SVG_Point(radii);
}

void draw_circle(SVG_Canvas *canvas, int mask, void *args)
{ FILE      *hfile    = MF(canvas);
  SVG_Point *center   = ((Ellip_Args *) args)->center;
  double     radius   = ((Ellip_Args *) args)->angle;

  fprintf(hfile,"<circle cx=\"%g\" cy=\"%g\" r=\"%g\"",center->x,center->y,radius);

  emit_attributes(hfile,MC(canvas),0,mask);

  fprintf(hfile," />\n");
}

void SVG_Circle(SVG_Canvas *canvas, SVG_Point *center, double radius)
{ Ellip_Args earg;

  earg.center = center;
  earg.angle  = radius;

  if (need_extent(canvas))
    { double     maxr;
      SVG_Point *corner;
      SVG_Box   *dims;
      SVG_Frame *extent;

      maxr = radius;
      maxr += .5*MC(canvas)->pen_width;

      corner = Make_SVG_Point(center->x-maxr,center->y-maxr);
      dims   = Make_SVG_Box(2*maxr,2*maxr);
      extent = Make_SVG_Frame(corner,dims);
      draw_closed_object(canvas,extent,(void *) &earg,draw_circle);
    }
  else
    draw_closed_object(canvas,NULL,(void *) &earg,draw_circle);

  Free_SVG_Point(center);
}


/****************************************************************************************
 *                                                                                      *
 *   DRAW TEXT                                                                          *
 *                                                                                      *
 ****************************************************************************************/

static char *Font_Families[] =
  { "serif",
    "sans-serif",
    "monospace",
    "Arial, Helvetica, sans-serif",
    "Arial Black, Gadget, sans-serif",
    "Book Antiqua, Palatino, serif",
    "Charcoal, sans-serif",
    "Courier, monospace",
    "Gadget, sans-serif",
    "Geneva, sans-serif",
    "Georgia, serif",
    "Helvetica, sans-serif",
    "Impact, Charcoal, sans-serif",
    "Lucida Console, Monaco, monospace",
    "Lucida Grande, sans-serif",
    "Monaco, monospace",
    "Palatino, serif",
    "Tahoma, Geneva, sans-serif",
    "Times New Roman, Times, serif",
    "Verdana, Geneva, sans-serif",
    NULL
  };

typedef struct
  { SVG_Point *anchor;
    string     text;
    double     angle;
  } Text_Args;

void draw_text(SVG_Canvas *canvas, int mask, void *args)
{ FILE      *hfile    = MF(canvas);
  MyContext *ctx      = MC(canvas);
  SVG_Point *anchor   = ((Text_Args *) args)->anchor;
  string     text     = ((Text_Args *) args)->text;
  double     angle    = ((Text_Args *) args)->angle;

  if (angle != 0. && mask >= 0)
    fprintf(hfile,"<g transform=\"rotate(%g %g %g)\">\n",angle,anchor->x,anchor->y);

  if (ctx->font > SVG_Verdana)
    fprintf(hfile,"<text x=\"%g\" y=\"%g\" font-family=\"%s\" font-size=\"%g\"",
                  anchor->x,anchor->y,ctx->custom,ctx->point_size);
  else
    fprintf(hfile,"<text x=\"%g\" y=\"%g\" font-family=\"%s\" font-size=\"%g\"",
                  anchor->x,anchor->y,Font_Families[ctx->font],ctx->point_size);

  if (ctx->style != SVG_plain)
    { if (ctx->style == SVG_italic || ctx->style == SVG_bold_italic)
        fprintf(hfile," font-style=\"italic\"");
      if (ctx->style == SVG_bold || ctx->style == SVG_bold_italic)
        fprintf(hfile," font-weight=\"bold\"");
    }

  if (ctx->alignment != SVG_left_align)
    { if (ctx->alignment == SVG_center_align)
        fprintf(hfile," text-anchor=\"middle\"");
      else // ctx->alignment == SVG_right_align
        fprintf(hfile," text-anchor=\"end\"");
    }

  emit_attributes(hfile,ctx,0,mask);

  fprintf(hfile,">\n");

  fprintf(hfile,"    %s\n",text);

  fprintf(hfile,"</text>\n");

  if (angle != 0. && mask >= 0)
    fprintf(hfile,"</g>\n");
}

void SVG_Text(SVG_Canvas *canvas, SVG_Point *anchor, double angle, string text)
{ Text_Args targ;

  targ.anchor = anchor;
  targ.text   = text;
  targ.angle  = angle;

  if (need_extent(canvas))
    { double     pt  = MC(canvas)->point_size;
      int        len = strlen(text);
      double     pw  = .5*MC(canvas)->pen_width;
      SVG_Point *corner;
      SVG_Box   *dims;
      SVG_Frame *extent;

      corner = Make_SVG_Point(anchor->x-(pt+pw),anchor->y-(pt+pw));
      if (MC(canvas)->alignment == SVG_center_align)
        corner->x -= .5*pt*len;
      else if (MC(canvas)->alignment == SVG_right_align)
        corner->x -= pt*len;
      dims   = Make_SVG_Box(pt*(len+2) + 2*pw,2*(pt+pw));
      extent = Make_SVG_Frame(corner,dims);
      draw_closed_object(canvas,extent,(void *) &targ,draw_text);
    }
  else
    draw_closed_object(canvas,NULL,(void *) &targ,draw_text);

  Free_SVG_Point(anchor);
}
