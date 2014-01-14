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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "utilities.h"
#include "array.h"
#include "linear.algebra.h"

typedef enum { FIXED_SNAKE, VARIABLE_SNAKE } Snake_Type;

typedef struct
  { Snake_Type    type;
    int           len;
    Float_Matrix *curve;
    Float_Matrix *params;
  } Snake;

double *get_dist_vector(int n, char *routine)
{ static double *Distance = NULL;
  static int     Dist_Max = 0;

  if (n > Dist_Max)
    { Dist_Max = (int) (1.2*n + 100);
      Distance = (double *) Guarded_Realloc(Distance,sizeof(double)*((size_t) Dist_Max),routine);
    }
  else if (n <= 0)
    { free(Distance);
      Distance = NULL;
      Dist_Max = 0;
    }
  return (Distance);
}

static int Vector_Top = 0;


typedef struct __Snake
  { struct __Snake *next;
    struct __Snake *prev;
    int             refcnt;
    Snake           snake;
  } _Snake;

static _Snake *Free_Snake_List = NULL;
static _Snake *Use_Snake_List  = NULL;

static pthread_mutex_t Snake_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Snake_Offset = sizeof(_Snake)-sizeof(Snake);
static int Snake_Inuse  = 0;

int Snake_Refcount(Snake *snake)
{ _Snake *object = (_Snake *) (((char *) snake) - Snake_Offset);
  return (object->refcnt);
}

static inline void kill_snake(Snake *snake);

static inline Snake *new_snake(char *routine)
{ _Snake *object;
  Snake  *snake;

  pthread_mutex_lock(&Snake_Mutex);
  if (Free_Snake_List == NULL)
    { object = (_Snake *) Guarded_Realloc(NULL,sizeof(_Snake),routine);
      if (object == NULL) return (NULL);
      snake = &(object->snake);
    }
  else
    { object = Free_Snake_List;
      Free_Snake_List = object->next;
      snake = &(object->snake);
    }
  Snake_Inuse += 1;
  object->refcnt = 1;
  if (Use_Snake_List != NULL)
    Use_Snake_List->prev = object;
  object->next = Use_Snake_List;
  object->prev = NULL;
  Use_Snake_List = object;
  pthread_mutex_unlock(&Snake_Mutex);
  snake->curve = NULL;
  snake->params = NULL;
  return (snake);
}

static inline Snake *copy_snake(Snake *snake)
{ Snake *copy = new_snake("Copy_Snake");
  *copy = *snake;
  if (snake->curve != NULL)
    copy->curve = Copy_Array(snake->curve);
  if (snake->params != NULL)
    copy->params = Copy_Array(snake->params);
  return (copy);
}

Snake *Copy_Snake(Snake *snake)
{ return ((Snake *) copy_snake(snake)); }

static inline int pack_snake(Snake *snake)
{
  if (snake->curve != NULL)
    if (Pack_Array(snake->curve) == NULL) return (1);
  if (snake->params != NULL)
    if (Pack_Array(snake->params) == NULL) return (1);
  return (0);
}

Snake *Pack_Snake(Snake *snake)
{ if (pack_snake(snake)) return (NULL);
  return (snake);
}

Snake *Inc_Snake(Snake *snake)
{ _Snake *object  = (_Snake *) (((char *) snake) - Snake_Offset);
  pthread_mutex_lock(&Snake_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Snake_Mutex);
  return (snake);
}

static inline void free_snake(Snake *snake)
{ _Snake *object  = (_Snake *) (((char *) snake) - Snake_Offset);
  pthread_mutex_lock(&Snake_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Snake_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Snake\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Snake_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Snake_List;
  Free_Snake_List = object;
  Snake_Inuse -= 1;
  pthread_mutex_unlock(&Snake_Mutex);
  if (snake->params != NULL)
    { Free_Array(snake->params);
      snake->params = NULL;
    }
  if (snake->curve != NULL)
    { Free_Array(snake->curve);
      snake->curve = NULL;
    }
}

void Free_Snake(Snake *snake)
{ free_snake(snake); }

static inline void kill_snake(Snake *snake)
{ _Snake *object  = (_Snake *) (((char *) snake) - Snake_Offset);
  pthread_mutex_lock(&Snake_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Snake_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Snake\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Snake_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Snake_Inuse -= 1;
  pthread_mutex_unlock(&Snake_Mutex);
  if (snake->params != NULL)
    Kill_Array(snake->params);
  if (snake->curve != NULL)
    Kill_Array(snake->curve);
  free(((char *) snake) - Snake_Offset);
}

void Kill_Snake(Snake *snake)
{ kill_snake(snake); }

static inline void reset_snake()
{ _Snake *object;
  pthread_mutex_lock(&Snake_Mutex);
  while (Free_Snake_List != NULL)
    { object = Free_Snake_List;
      Free_Snake_List = object->next;
      free(object);
    }
  pthread_mutex_unlock(&Snake_Mutex);
}

void Reset_Snake()
{ reset_snake(); }

int Snake_Usage()
{ return (Snake_Inuse); }

void Snake_List(void (*handler)(Snake *))
{ _Snake *a, *b;
  for (a = Use_Snake_List; a != NULL; a = b)
    { b = a->next;
      handler((Snake *) &(a->snake));
    }
}

static inline Snake *read_snake(FILE *input)
{ char name[5];
  Snake *obj;
  Snake read;
  fread(name,5,1,input);
  if (strncmp(name,"Snake",5) != 0)
    return (NULL);
  obj = new_snake("Read_Snake");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Snake),1,input) == 0) goto error;
  if (read.curve != NULL)
    { obj->curve = Read_Array(input);
      if (obj->curve == NULL) goto error;
    }
  else
    obj->curve = NULL;
  if (read.params != NULL)
    { obj->params = Read_Array(input);
      if (obj->params == NULL) goto error;
    }
  else
    obj->params = NULL;
  return (obj);

error:
  kill_snake(obj);
  return (NULL);
}

Snake *Read_Snake(FILE *input)
{ return ((Snake *) read_snake(input)); }

static inline void write_snake(Snake *snake, FILE *output)
{ fwrite("Snake",5,1,output);
  fwrite(snake,sizeof(Snake),1,output);
  if (snake->curve != NULL)
    Write_Array(snake->curve,output);
  if (snake->params != NULL)
    Write_Array(snake->params,output);
}

void Write_Snake(Snake *snake, FILE *output)
{ write_snake(snake,output); }

void resample(Double_Matrix *old, Double_Matrix *new, double *dist, double grid)
{ double *xn = AFLOAT64(new);
  double *yn = xn + new->dims[0];
  double *xo = AFLOAT64(old);
  double *yo = xo + old->dims[0];
  int     np = new->dims[0]-1;
  double  cx, cy;
  double  nx, ny;
  double  p, g;
  int     i, k;

  k = 0;
  xn[0] = xn[np] = cx = xo[-1];
  yn[0] = yn[np] = cy = yo[-1];
  nx = xo[0];
  ny = yo[0];
  for (i = 1, p = grid; i < np; i++, p += grid)
    { while (dist[k] < p)
        { p -= dist[k];
          k += 1;
          cx = nx;
          cy = ny;
          nx = xo[k];
          ny = yo[k];
        }
      g = p / dist[k];
      xn[i] = cx + g * (nx - cx);
      yn[i] = cy + g * (ny - cy);
    }
}

Snake *resample_snake(Snake *snake, double grid)
{ static Dimn_Type dims[2];

  int     n = snake->len;
  double *x = AFLOAT64(snake->curve);
  double *y = x + n;

  double *dist, len;
  int     np;
  Snake  *resamp;

  dist = get_dist_vector(n,"resample_snake");

  { double  lx, ly;
    double  cx, cy;
    double  dx, dy;
    double *dm;
    int     i;

    dm  = dist-1;
    lx  = x[0];
    ly  = y[0];
    len = 0.;
    for (i = 1; i <= n; i++)
      { cx = x[i];
        cy = y[i];
        dx = cx - lx;
        dy = cy - ly;
        lx = cx;
        ly = cy;
        len += dm[i] = sqrt(dx*dx + dy*dy);
      }
  }

  np   = (int) (len / grid);
  grid = len / np;

  if (np+1 > Vector_Top)
    Vector_Top = (int) (1.1*(np+1) + 100);

  resamp = new_snake("resample_snake");
  dims[0] = np+1;
  dims[1] = 2;
  resamp->curve = Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,dims);

  if (snake->type == VARIABLE_SNAKE)
    { resamp->params = Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,dims);
      resamp->type   = VARIABLE_SNAKE;
    }
  else
    { resamp->params = Inc_Array(snake->params);
      resamp->type   = FIXED_SNAKE;
    }

  resample(snake->curve,resamp->curve,dist,grid);
  if (snake->type == VARIABLE_SNAKE)
    resample(snake->params,resamp->params,dist,grid);

  return (resamp);
}

void deform_snake(Snake *snake, double gamma, double kappa, Double_Matrix *force)
{ static Double_Matrix *penta = NULL;
  static int            pmax = 0;
  static Dimn_Type      dims[2];

  int width = force->dims[0];
  int area  = force->dims[1] * width;

  double *fx = AFLOAT64(force);
  double *fy = fx + area;
  int      n = snake->len;

  double *a, *b, *c, *d, *e;

  if (n > pmax)
    { pmax = (int) (1.2*n + 300);
      if (penta != NULL)
        Free_Array(penta);
      dims[0] = pmax;
      dims[1] = 5;
      penta = Make_Array(PLAIN_KIND,FLOAT64_TYPE,2,dims);
    }

  penta->dims[0] = n;
  a = AFLOAT64(penta);
  b = a + n;
  c = b + n;
  d = c + n;
  e = d + n;

  if (snake->type == VARIABLE_SNAKE)
    { double *alpha = AFLOAT64(snake->params);
      double *beta  = alpha + snake->len;
      int     i;

      a[0] = alpha[n-1];
      b[0] = - (alpha[0] + 2*beta[n-1] + 2*beta[0]);
      c[0] = alpha[0] + alpha[1] + beta[n-1] + 4*beta[0] + beta[1];
      d[0] = - (alpha[1] + 2*beta[0] + 2*beta[1]);
      e[0] = beta[1];
      for (i = 1; i < n; i++)
        { a[i] = beta[i-1];
          b[i] = - (alpha[i] + 2*beta[i-1] + 2*beta[i]);
          c[i] = alpha[i] + alpha[i+1] + beta[i-1] + 4*beta[i] + beta[i+1];
          d[i] = - (alpha[i+1] + 2*beta[i] + 2*beta[i+1]);
          e[i] = beta[i+1];
        }
    }
  else
    { double alpha = AFLOAT64(snake->params)[0];
      double beta  = AFLOAT64(snake->params)[1];
      int   i;

      for (i = 0; i < n; i++)
        { a[i] = e[i] = beta;
          b[i] = d[i] = - (alpha + 4*beta);
          c[i] = 2*alpha + 6*beta;
        }
    }

  { Band_Factor  *lu = Pentaband_Decompose(penta);
    double       *x  = AFLOAT64(snake->curve);
    double       *y  = x + snake->len;
    Array_Bundle  abundle;
    int           i;

    for (i = 0; i < n; i++)
      { double u, v;
        double xi, yi;
        double a, b;
        int    r, s, p;

        r = (int) (xi = x[i]);
        s = (int) (yi = y[i]);

        p = s*width + r;

        a = (xi - r);
        b = (yi - s);

        u = (fx[p] * a + fx[p+1] * (1.-a)) * b
          + (fx[p+width] * a + fx[p+width+1] * (1.-a)) * (1.-b);
        v = (fy[p] * a + fy[p+1] * (1.-a)) * b
          + (fy[p+width] * a + fy[p+width+1] * (1.-a)) * (1.-b);

        x[i] = gamma * xi + kappa * u;
        y[i] = gamma * yi + kappa * v;
      }

    abundle = *(snake->curve);
    Pentaband_Solve(Get_Array_Plane(&abundle,0),lu);
    abundle = *(snake->curve);
    Pentaband_Solve(Get_Array_Plane(&abundle,1),lu);

    Free_Band_Factor(lu);
  }
}
