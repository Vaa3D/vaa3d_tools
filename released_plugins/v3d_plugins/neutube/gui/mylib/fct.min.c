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
*  Function minimization routines for 1 and multiple dimensions                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  May 2007                                                                      *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "utilities.h"
#include "fct.min.h"

#undef  SHOW_MINIMIZATION
#undef  SHOW_MULTI_MIN

#define ITMAX  100                 // Maximum # of iterations
#define CGOLD  0.381966011250105   // The Golden Ratio (to DBL_DIG = 15 digits)
#define TINY   1e-20               // Minimum fractional accuracy
#define RND    1.00000001          // Rounding error in a half precision add
#define TOL     .00000001          // For doubles, cannot generally hope for more
                                   //   than half-precision = 7.5 digits

/* ONE-DIMENSIONAL MINIMIZATION:

     Find_Min_Bracket:  find a bracket enclosing a minimum
       find_min_bracket:  find a bracket enclosing a minimum (takes argument bundle)
     Minimize_Function: general entry point for finding a minimum
       min_fct_nod:       find a minimum using parabolic acceleration of Golden sections
       min_fct_wd:        find a minimum using derivative driven bisection
*/

typedef struct
  { double (*f)(double);
    double (*df)(double);
  } Pack1;

static double f1(double x, void *pack)
{ return (((Pack1 *) pack)->f(x)); }

static double df1(double x, void *pack)
{ return (((Pack1 *) pack)->df(x)); }

static Minimum_Bundle *find_min_bracket(Minimum_Bundle *R(M(brack)), double p, double s,
                                        double (*f)(double,void *), void *pack)
{ double  a,  b,  c,  u;
  double fa, fb, fc, fu;
  double gstep, gratio;
  int    swap, iter;

  a  = p;
  b  = p + s;
  fa = (*f)(a,pack);         //  Swap a and b if necessary so that b is downhill of a
  fb = (*f)(b,pack);
  if (fb >= fa)
    { u = a;  fu = fa;
      a = b;  fa = fb;
      b = u;  fb = fu;
    }
  swap = (b < a);           //  Have to reverse a,b,c in final answer?

  gratio = 2.-CGOLD;
  gstep  = gratio * (b-a);  //  Always keep track of the last (and largest Golden ratio step)

  c  = b + gstep;           //  Take a first step
  fc = (*f)(c,pack);

#ifdef SHOW_MINIMIZATION
  printf("\n  0: % 17.14e   % 17.14e   % 17.14e\n",a,b,c);
  printf("     % 17.14e > % 17.14e > % 17.14e\n",fa,fb,fc);
#endif

  for (iter = 1; iter < 100; iter++)

    if (fc > fb)                    //  If f(a) > f(b) < f(c) then your done
      { if (swap)
          { brack->lft = c;
            brack->rgt = a;
          }
        else
          { brack->lft = a;
            brack->rgt = c;
          }
        brack->mid = b;
        return (brack);
      }

    else
      { double r, q, p;
        int    gold;

#ifdef SHOW_MINIMIZATION
        printf("\n%3d: % 17.14e   % 17.14e   % 17.14e\n",iter,a,b,c);
        printf("     % 17.14e > % 17.14e > % 17.14e\n",fa,fb,fc);
#endif
        gold = 0;
        r = (b-a)*(fb-fc);           // NB: r and q have the same sign
        q = (b-c)*(fb-fa);
#ifdef SHOW_MINIMIZATION
        printf("\n         q = %17.14e, r = %17.14e\n",q,r);
#endif
        if ((q-r)/q > TOL)           // => |q| > |r| (so parabolic step finds a minimum in correct
          { p = (b-a)*r - (b-c)*q;   //  direction) and cancelation in denominator is under control
            q = 2.0*(q-r);
            u  = b + p/q;            // u is the parabolic step for a,b,c
            fu = (*f)(u,pack);
#ifdef SHOW_MINIMIZATION
            printf("         p/q = %17.14e, u = %17.14e, fu = %17.14e\n",p/q,u,fu);
#endif
            if ((b < u) == (u < c))  // u is between b and c?

              { if (fu < fc)         // [b,u,c] is a bracket
                  { a = b; fa = fb;
                    b = u; fb = fu;
#ifdef SHOW_MINIMIZATION
                    printf("         f(b) > f(u) < f(c)\n");
#endif
                  }
                else if (fu > fb)    // [a,b,u] is a bracket
                  { c = u; fu = fc;
#ifdef SHOW_MINIMIZATION
                    printf("         f(a) > f(b) < f(u)\n");
#endif
                  }
                else                 //  No luck, take a Golden ratio step
                  gold = 1;
              }

            else if ((c < u) == (u < c + 100.*(c-b)))   // u is between c and jump limit?

              { if (fu >= fc)                // [b,c,u] is a bracket
                  { a = b;  fa = fb;
                    b = c;  fb = fc;
                    c = u;  fc = fu;
#ifdef SHOW_MINIMIZATION
                    printf("         f(b) > f(c) < f(u)\n");
#endif
                  }
                else                         // try [c,u,u+gstep] as the next bracket
                  { a = c;  fa = fc;         //   note carefully that we only project forward
                    b = u;  fb = fu;         //   gstep beyond the potential minimum u as we
                    c = u+gstep;             //   may otherwise overshoot by starting to use
                    fc = (*f)(c,pack);       //   too big a step size
#ifdef SHOW_MINIMIZATION
                    printf("         Jump to u + gstep(%17.14e)\n",gstep);
#endif
                  }
              }

            else           //  jump is too big or parabolic step not possible:
              gold = 1;    //    take a Golden ratio step
          }
        else
          gold = 1;

        if (gold)                        //  Actually take a Golden ratio step
          { gstep = gratio * (c-b);
            a  = b;  fa = fb;
            b  = c;  fb = fc;
            c  = b + gstep;
            fc = (*f)(c,pack);
#ifdef SHOW_MINIMIZATION
            printf("         Gold: c = %17.14e, fc = %17.14e\n",c,fc);
#endif
          }
      }

  fprintf(stderr,"Warning: Find_Min_Bracket did not find a bracket in %d iteratios\n",ITMAX);

  return (NULL);
}

Minimum_Bundle *Find_Min_Bracket(Minimum_Bundle *R(M(brack)),
                                 double p, double s, double (*f)(double))
{ Pack1 pack;

  pack.f = f;
  find_min_bracket(brack,p,s,f1,&pack);
  return (brack);
}

static double min_fct_nod(Minimum_Bundle *brack, double (*f)(double, void *),
                          double *xmin, void *pack)
{
  double  a, c;       // Current bracket [a,c]
  double  x,  y,  z;  // Lowest f(x) < f(y) < f(z) with x in [a,c]
  double fx, fy, fz;  // function values at x, y, and z
  double  d, e;       // step in iteration (d), and the one before (e)
  int     iter;

  a = brack->lft;
  c = brack->rgt;
  x  = y  = z  = brack->mid;     // 1st two iterations will be Golden ratio divisions, so y and z
  fx = fy = fz = (*f)(x,pack);   //   will not be used, nor will e or d, until they have been set

  d = e = 0.;
  for (iter = 1; iter <= ITMAX; iter++)
    { double rel, rle;
      double u, fu;
      int    gold;

#ifdef SHOW_MINIMIZATION
      printf("\n%3d: [ % 17.14e , % 17.14e ]\n",iter,a,c);
      printf("              % 17.14e   % 17.14e   % 17.14e\n",x,y,z);
      printf("              % 17.14e < % 17.14e < % 17.14e\n",fx,fy,fz);
#endif
      rel = TOL*fabs(x) + TINY;        //  Absolute error
      rle = RND*rel;

      if (x-a <= rle && c-x <= rle)    //  Termination: bracket within absolute error of x
        { *xmin = x;                   //    on both sides.
          return (fx);
        }

      gold = 1;
      if (iter > 2 && fabs(e) > rle)   //  Try parabolic fit only if displacement will be
        { double p, q, r;              //    larger than rel and initialization is over

          r = (x-y)*(fx-fz);
          q = (x-z)*(fx-fy);
          p = (x-z)*q - (x-y)*r;
          q = 2.0*(r-q);               //  Parabolic displacement is p/q if q is not 0.
#ifdef SHOW_MINIMIZATION
          printf("\n         p/q = %17.14e, |q/r| = %17.14e, |e| = %17.14e\n",
                 p/q,fabs(q/r),fabs(e));
#endif

          if (fabs(q/r) >= TOL)         //  Not nearly colinear, can try parabolic
            { r = d;
              d = p/q;
              u = x+d;
              if (fabs(d/e) < 0.5 && a < u && u < c)   // Point is inside bracket and
                { double drl = 2.0*rel;                //   step size is converging
#ifdef SHOW_MINIMIZATION
                  q = d;
#endif
                  gold = 0;            // If x and the bracket point in the direction of the
                  if (d < 0)           //    jump are less than rel apart then jump rel the
                    { p = x-a;         //    *other* way, if they are less than 2*rel apart then
                      if (p < rel)     //    jump half-way between, and otherwise make sure the
                        d = rel;       //    new point is at least rel from both x and the bracket.
                      else if (p < drl) 
                        d = -p/2.0;
                      else if (u < a+rel)
                        d = rel - p;
                      else if (d > -rel)
                        d = -rel;
                    }
                  else
                    { p = c-x;
                      if (p < rel)
                        d = -rel;
                      else if (p < drl)
                        d = p/2.0;
                      else if (u > c-rel)
                        d = p - rel;
                      else if (d < rel)
                        d = rel;
                    }
                  u = x+d;
                  e = r;
#ifdef SHOW_MINIMIZATION
                  if (q != d)
                    printf("         Adjusted d from %17.14e to %17.14e\n",q,d);
                  printf("         QUAD STEP: %17.14e\n",d);
#endif
                }
	    }
        }

      if (gold)            //  Golden ratio division
        { u = x-a;
          e = c-x;
          if (u >= e)
            e = -u;
          d = CGOLD*e;
          u = x+d;
#ifdef SHOW_MINIMIZATION
          printf("         GOLD STEP: %17.14e\n",d);
#endif
        }

      fu = (*f)(u,pack);
#ifdef SHOW_MINIMIZATION
      printf("         u = %17.14e, fu = %17.14e\n",u,fu);
#endif
      if (fu <= fx)          //  Update bracket [a,x,c], and 3 minimums, x,y,z, and their values
        { if (u >= x)        //    fx,fy,fz with respect to the new point u.
            a = x;
          else
            c = x;
          z = y;  fz = fy;
          y = x;  fy = fx;
          x = u;  fx = fu;
#ifdef SHOW_MINIMIZATION
          printf("         Shift z <- y <- x <- u\n");
#endif
        }
      else
        { if (u < x)
            a = u;
          else
            c = u;
          if (iter <= 1 || fu <= fy)
            { z = y;  fz = fy;
              y = u;  fy = fu;
#ifdef SHOW_MINIMIZATION
              printf("         Shift z <- y <- u\n");
#endif
            }
          else if (iter <= 2 || fu <= fz)
            { z = u;  fz = fu;
#ifdef SHOW_MINIMIZATION
              printf("         Shift z <- u\n");
#endif
            }
        }
    }

  fprintf(stderr,"Warning: Too many iterations (%d) in Minimize_Fct (no derivative)\n",ITMAX);

  *xmin = x;
  return (fx);
}

static double min_fct_wd(Minimum_Bundle *brack, double (*f)(double, void *),
                         double (*df)(double, void *), double *xmin, void *pack)
{
  double  a, c;       // Current bracket [a,c]
  double  x,  y,  z;  // Lowest f(x) < f(y) < f(z) with x in [a,c]
  double fx, fy, fz;  // function values at x, y, and z
  double dx, dy, dz;  // derivative values at fx, fy, and fz
  double  d, e;       // step in iteration (d), and the one before (e)
  double  fatstep;
  int     iter;

  a = brack->lft;
  c = brack->rgt;
  x  = y  = z  = brack->mid;       // 1st two iterations will be Golden ratio divisions, so y and z
  fx = fy = fz = (*f)(x,pack);     //   will not be used, nor will e or d, until they have been set
  dx = dy = dz = (*df)(x,pack);
  fatstep = 2.0*(c-a);

  d = e = 0.;
  for (iter = 1; iter <= ITMAX; iter++)
    { double rel, rle;
      double u, fu, du;
      int    bisect;

#ifdef SHOW_MINIMIZATION
      printf("\n%3d: [ % 17.14e , % 17.14e ]\n",iter,a,c);
      printf("              % 17.14e   % 17.14e   % 17.14e\n",x,y,z);
      printf("              % 17.14e < % 17.14e < % 17.14e\n",fx,fy,fz);
#endif
      rel = TOL*fabs(x) + TINY;        //  Absolute error
      rle = RND*rel;

      if (x-a <= rle && c-x <= rle)    //  Termination: bracket within absolute error of x
        { *xmin = x;                   //    on both sides.
          return (fx);
        }

      bisect = 1;
      if (iter > 2 && fabs(e) > rle)   //  Try secant method only if displacement will be
        { double  d1,  d2;             //    larger than rel and initialization is over
          int    ok1, ok2;

          d1 = d2 = fatstep;
          if (dy != dx)
            d1 = (y-x)*dx/(dx-dy);
          if (dz != dx)
            d2 = (z-x)*dx/(dx-dz);
          if (dx < 0)
            { ok1 = (d1 >= 0.0 && x + d1 <= c);
              ok2 = (d2 >= 0.0 && x + d2 <= c);
            }
          else
            { ok1 = (d1 <= 0.0 && x + d1 >= a);
              ok2 = (d2 <= 0.0 && x + d2 >= a);
            }
#ifdef SHOW_MINIMIZATION
          printf("\n         d1 = %17.14e(%d), d2 = %17.14e(%d)\n",d1,ok1,d2,ok2);
#endif
          if (ok1 || ok2)
            { double r, p;

              r = d;
              if (fabs(d1/d2) < 1.0)
                d = d1;
              else
                d = d2;
              if (fabs(d/e) <= 0.5)
                { double drl = 2.0*rel;
#ifdef SHOW_MINIMIZATION
                  double q = d;
#endif
                  u = x+d;
                  bisect = 0;          // If x and the bracket point in the direction of the
                  if (d < 0)           //    jump are less than rel apart then jump rel the
                    { p = x-a;         //    *other* way, if they are less than 2*rel apart then
                      if (p < rel)     //    jump half-way between, and otherwise make sure the
                        d = rel;       //    new point is at least rel from both x and the bracket.
                      else if (p < drl) 
                        d = -p/2.0;
                      else if (u < a+rel)
                        d = rel - p;
                      else if (d > -rel)
                        d = -rel;
                    }
                  else
                    { p = c-x;
                      if (p < rel)
                        d = -rel;
                      else if (p < drl)
                        d = p/2.0;
                      else if (u > c-rel)
                        d = p - rel;
                      else if (d < rel)
                        d = rel;
                    }
                  u = x+d;
                  e = r;
#ifdef SHOW_MINIMIZATION
                  if (q != d)
                    printf("         Adjusted d from %17.14e to %17.14e\n",q,d);
                  printf("         DERIVATIVE STEP: %17.14e\n",d);
#endif
                }
            }
        }

      if (bisect)                   //  Bisection based on derivative dx
        { e = c-x;
          if (dx >= 0 || e < rel)   //  Careful do not bisect something too small!
            e = a-x;
          d = 0.5*e;
          u = x+d;
#ifdef SHOW_MINIMIZATION
          printf("         BISECTION STEP: %17.14e\n",d);
#endif
        }

      fu = (*f)(u,pack);   //  Update bracket [a,x,c], and 3 minimums, x,y,z, and their values
      du = (*df)(u,pack);  //    fx,fy,fz and derivatives dx,dy,dz with respect to the new point u.
#ifdef SHOW_MINIMIZATION
      printf("         u = %17.14e, fu = %17.14e, du = %17.14e\n",u,fu,du);
#endif
      if (fu <= fx)
        { if (u >= x)
            a = x;
          else
            c = x;
          z = y;  fz = fy;  dz = dy;
          y = x;  fy = fx;  dy = dx;
          x = u;  fx = fu;  dx = du;
#ifdef SHOW_MINIMIZATION
          printf("         Shift z <- y <- x <- u\n");
#endif
        }
      else
        { if (u < x)
            a = u;
          else
            c = u;
          if (iter <= 1 || fu <= fy)
            { z = y;  fz = fy;  dz = dy;
              y = u;  fy = fu;  dy = du;
#ifdef SHOW_MINIMIZATION
              printf("         Shift z <- y <- u\n");
#endif
            }
          else if (iter <= 2 || fu <= fz)
            { z = u;  fz = fu;  dz = du;
#ifdef SHOW_MINIMIZATION
              printf("         Shift z <- u\n");
#endif
            }
        }
    }

  fprintf(stderr,"Warning: Too many iterations (%d) in Minimize_Fct (with derivative)\n",ITMAX);

  *xmin = x;
  return (fx);
}

double Minimize_Fct(Minimum_Bundle *brack, double (*f)(double), double (*df)(double))
{ Pack1  pack;
  double a;

  pack.f  = f;
  pack.df = df;
  if (df == NULL)
    min_fct_nod(brack,f1,&a,&pack);
  else
    min_fct_wd(brack,f1,df1,&a,&pack);
  return (a);
}

/* LINE MINIMIZATION ROUTINES:

     init_line_min:   Setup storage and global vars for dimension, function, and gradient thereof
     line_min_nod:    Do a line minimization without derivative
     line_min_wd:     Do a line minimization with derivative
     f1dim:           Evaluate function at scalar position along current line
     df1dim:          Evaluate derivative at scalar position along current line in direction of said
*/

#ifdef SHOW_MULTI_MIN

static void print_vector(Dimn_Type n, double *vec)
{ Dimn_Type j;

  printf("(%.14g",vec[0]);
  for (j = 1; j < n; j++)
    printf(",%.14g",vec[j]);
  printf(")");
}

#endif

//  Global vars for tricking 1d routines into minimizing along a line

typedef struct
  { double  (*f)(double *);
    double *(*df)(double *, double *);
    int     n;
    double *pl;
    double *gl;
    double *dir;
    double *pc;
  } PackN;

static double f1dim(double x, void *pack)     // evaluate pack->f at pack->pc + x*pack->dir
{ PackN *p = (PackN *) pack;
  int    i;

  for (i = 0; i < p->n; i++)
    p->pl[i] = p->pc[i] + x*p->dir[i];
  return ((*p->f)(p->pl));
}

static double df1dim(double x, void *pack)   // evaluate d(pack->f)/d(pack->dir)
{ PackN  *p = (PackN *) pack;                //     at pack->pc + x*pack->dir
  double *gd, df;
  int     i;

  (void) x;                        //  x is ignored, always the same as last call to f1dim!
  gd = (*p->df)(p->pl,p->gl);
  df = 0.;
  for (i = 0; i < p->n; i++)
    df += gd[i]*p->dir[i];
  return (df);
}

/* MULTI-DIMENSIONAL MINIMIZATION:

     Powell_Minimizer:        find a multi-dimensional min using line minimization on a set of
                                direction vectors that are evolved to be mutually conjugate
     Polak_Rabier_Minimizer:  find a multi-dimensional min using conjugate gradient descent
*/

Double_Vector *Powell_Minimizer(Double_Vector *R(M(xinit)), double step, double (*f)(double *))
{ double  Fvec[ 40],  *vec;
  double  Fmat[100],  *mat;
  double *Fdir[ 10], **dir;
  PackN   pack;

  double   *av, *pi, *pe, *pl;
  double   *pc;
  double    fi,  fc,  fe;
  int       iter;
  Dimn_Type n;

  n = xinit->dims[0];
  if (xinit->type != FLOAT64_TYPE || xinit->ndims != 1)
    { fprintf(stderr,"xinit is not a 1D double array (Powell_Minimizer)\n");
      exit (1);
    }

  if (n > 10)
    { vec = (double *) Guarded_Malloc((sizeof(double)*((size_t) (4+n)) + sizeof(double *)) *
                                        ((size_t) n),"Powell_Minimizer");
      mat = vec + 4*n;
      dir = (double **) (mat + n*n);
    }
  else
    { vec = Fvec;
      mat = Fmat;
      dir = Fdir;
    }
  pi = vec;
  pe = pi+n;
  av = pe+n;
  pl = av+n;

  { double   *d;       //   make a matrix of initial direction vectors of size step
    Indx_Type i, s;

    s = n*n;
    d = mat;
    for (i = 0; i < s; i++)
      d[i] = 0.;
    for (i = 0; i < s; i += n+1)
      d[i] = step;
    for (i = 0; i < n; i++)
      { dir[i] = d;
        d     += n;
      }
  }

  pc = AFLOAT64(xinit);
  fc = (*f)(pc);

#ifdef SHOW_MULTI_MIN
  printf("   FC");
  print_vector(n,pc);
  printf(" = %.14g\n",fc);
#endif

  pack.f  = f;
  pack.n  = n;
  pack.pc = pc;
  pack.pl = pl;

  for (iter = 1; iter <= ITMAX; iter++)
    { double    dmax;
      Dimn_Type imax;
      Dimn_Type i;

      for (i = 0; i < n; i++)
        pi[i] = pc[i];
      fi = fc;
#ifdef SHOW_MULTI_MIN
      printf("\n");
#endif

      imax = 0;
      dmax = 0.0;
      for (i = 0; i < n; i++)
        { double         fn, xmin, *e;
          Minimum_Bundle brack;
          Dimn_Type      j;

          pack.dir = e = dir[i];
          find_min_bracket(&brack,0.0,0.1,f1dim,&pack);     //  This could fail!
          fn = min_fct_nod(&brack,f1dim,&xmin,&pack);
          for (j = 0; j < n; j++)
            pc[j] = pc[j] + xmin*e[j];

#ifdef SHOW_MULTI_MIN
          printf("  MIN %d ",i);
          print_vector(n,dir[i]);
          printf(" ->  ");
          print_vector(n,pc);
          printf(" f = %.14g\n",fn);
#endif
          if (fc-fn > dmax)
            { dmax = fc-fn;
              imax = i;
            }
          fc = fn;
        }
#ifdef SHOW_MULTI_MIN
      printf("  dmax = %.14g(%d)\n",dmax,imax);
#endif

      if (fi-fc <= TOL*fabs(fc)+TINY)
        { if (n > 10)
            free(vec);
          return (xinit); 
        }

      for (i = 0; i < n; i++)
        { pe[i] = 2.0*pc[i] - pi[i]; 
          av[i] = pc[i] - pi[i];
        }

      fe = (*f)(pe);
#ifdef SHOW_MULTI_MIN
      printf("   FE");
      print_vector(n,pe);
      printf(" = %.14g\n",fe);
#endif
      if (fe < fi && dmax > (n+1.0)*(fi-fc)/(2.0*n) && (fc-fe) < 0.25*(fi-fc))
        { double         *tmp, xmin;
          Minimum_Bundle brack;
          Dimn_Type      j;

          pack.dir = av;
          find_min_bracket(&brack,0.0,0.1,f1dim,&pack);     //  This could fail!
          fc = min_fct_nod(&brack,f1dim,&xmin,&pack);
          for (j = 0; j < n; j++)
            pc[j] = pc[j] + xmin*av[j];

#ifdef SHOW_MULTI_MIN
          printf("  AV_MIN ");
          print_vector(n,av);
          printf(" ->  ");
          print_vector(n,pc);
          printf(" f = %.14g\n",fc);
#endif
          tmp       = dir[imax];
          dir[imax] = dir[n-1];
          dir[n-1]  = av;
          av        = tmp;
        }
    }

  fprintf(stderr,"Warning: Too many iterations (%d) in Powell_Minimizer\n",ITMAX);
  if (n > 10)
    free(vec);
  return (xinit);
}

Double_Vector *Polak_Rabier_Minimizer(Double_Vector *R(M(xinit)),
                                      double (*f) (double *x),
                                      double *(*df)(double *, double *R(O(g))))
{ double Fh[40], *h, *g, *pl, *gl;
  PackN  pack;

  double   *pc;
  double    fc;
  double   *dc;
  int       iter;
  Dimn_Type j, n;

  n = xinit->dims[0];
  if (xinit->type != FLOAT64_TYPE || xinit->ndims != 1)
    { fprintf(stderr,"xinit is not a 1D double array (Powell_Minimizer)\n");
      exit (1);
    }

  if (n > 10)
    h = (double *) Guarded_Malloc(sizeof(double)*4*((size_t) n),"Polak_Rabier_Minimizer");
  else
    h = Fh;
  g  = h+n;
  pl = g+n;
  gl = pl+n;

  pc = AFLOAT64(xinit);
  fc = (*f)(pc);
  dc = (*df)(pc,gl);
  for (j = 0; j < n; j++)
    h[j] = g[j] = -dc[j];

#ifdef SHOW_MULTI_MIN
  printf("   FC");
  print_vector(n,pc);
  printf(" = %.14g (",fc);
  print_vector(n,dc);
  printf("\n");
#endif

  pack.f   = f;
  pack.df  = df;
  pack.n   = n;
  pack.pc  = pc;
  pack.pl  = pl;
  pack.gl  = gl;
  pack.dir = h;

  for (iter = 1; iter <= ITMAX; iter++)
    { double         fn, xmin;
      Minimum_Bundle brack;
      Dimn_Type      j;
      double         num, den, gam;

      find_min_bracket(&brack,0.0,0.1,f1dim,&pack);     //  This could fail!
      fn = min_fct_wd(&brack,f1dim,df1dim,&xmin,&pack);
      for (j = 0; j < n; j++)
        pc[j] = pc[j] + xmin*h[j];

#ifdef SHOW_MULTI_MIN
      printf("  MIN %d ",iter);
      print_vector(n,h);
      printf(" ->  ");
      print_vector(n,pc);
      printf(" f = %.14g\n",fn);
#endif

      if (fn-fc <= TOL*fabs(fc)+TINY)
        { if (n > 10)
            free(h);
          return (xinit); 
        }

      fc = fn;
      dc = (*df)(pc,gl);

      num = den = 0.;
      for (j = 0; j < n; j++)
        { num += g[j] * g[j];
          den += h[j] * (h[j] + g[j]);
        }
      if (den == 0.0)
        { if (n > 10)
            free(h);
          return (xinit);
        }
      gam = num/den;
      for (j = 0; j < n; j++)
        { g[j] = -dc[j];
          h[j] = g[j] + gam*h[j];
        }
    }

  fprintf(stderr,"Warning: Too many iterations (%d) in Polak_Ribier_Minimzer\n",ITMAX);
  if (n > 10)
    free(h);
  return (xinit);
}
