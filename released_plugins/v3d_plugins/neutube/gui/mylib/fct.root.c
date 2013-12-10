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
*  Root finding of 1- and multi-dimensional functions                                     *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
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
#include "fct.root.h"

#undef  SHOW_ROOT
#undef  TEST_DRIVER

#define ITMAX  100                 // Maximum # of iterations
#define CGOLD  1.618033988749895   // 1 + the Golden Ratio (to DBL_DIG = 15 digits)
#define TINY   1e-30               // Minimum fractional accuracy
#define EPS    1e-15               // For doubles, cannot generally hope for more
                                   //   than 15 digits

/* ONE-DIMENSIONAL ROOT FINDING:

     Find_Root_Bracket:    find a bracket enclosing a root
     Fct_Root_NoD:         find a root using parabolic acceleration of Golden sections
     Fct_Root_wD:          find a root using derivative driven bisection
     Find_Function_Root:   general entry point for finding a root
*/

Root_Bundle *Find_Root_Bracket(Root_Bundle *brack, double (*f)(double))
{ double  a,  b;
  double fa, fb;
  int    iter;

  a = brack->lft;
  b = brack->rgt;
  if (a == b) b = a+EPS;
  fa = (*f)(a);
  fb = (*f)(b);

#ifdef SHOW_ROOT
  printf("\n  0: % 17.14e   % 17.14e\n",a,b);
  printf("     % 17.14e   % 17.14e\n",fa,fb);
#endif

  for (iter = 1; iter < ITMAX; iter++)
    { if ((fa < 0.) != (fb < 0.))
        { brack->lft = a;
          brack->rgt = b;
          return (brack);
        }
      if (fabs(fa) < fabs(fb))
        { a += CGOLD * (a-b);
          fa = (*f)(a);
        }
      else
        { b += CGOLD * (b-a);
          fb = (*f)(b);
        }

#ifdef SHOW_ROOT
  printf("\n %2d: % 17.14e   % 17.14e\n",iter,a,b);
  printf("     % 17.14e   % 17.14e\n",fa,fb);
#endif

    }

  return (NULL);
}

static double Fct_Root_NoD(Root_Bundle *brack, double (*f)(double))
{ double  a,  b,  c;
  double fa, fb, fc;
  double  d,  e;
  int    iter;

  // b and c always bracket a root, b is most recent iterate, and a is the previous
  //   iterate.  d is the last step, and e the step before that (unless getting started
  //   or a iterate swap (F.1) takes place.

  a  = c  = brack->lft;
  b  =      brack->rgt;
  fa = fc = (*f)(a);
  fb =      (*f)(b);
  e = d = c-b;

  for (iter = 1; iter <= ITMAX; iter++)
    { double rel, cmb;

#ifdef SHOW_ROOT
      printf("\n%3d: [ % 17.14e , % 17.14e ] % 17.14e\n",iter,b,c,a);
      printf("           % 17.14e   % 17.14e   % 17.14e\n",fb,fc,fa);
#endif

      rel = EPS*fabs(b) + TINY;         //  Absolute error

      cmb = c-b;                        //  Bracket interval size

      if (fabs(fc) < fabs(fb))         //  c is a better choice of next iterate then b (F.1)
        { a  =  b;  b =  c;  c =  a;   //  generally implies quad step was poor
          fa = fb; fb = fc; fc = fa;
          e = d = cmb = -cmb;
#ifdef SHOW_ROOT
          printf("     Swap iterator (b and c)\n");
#endif
        }

      if (fabs(.5*cmb) <= rel || fb == 0.0)  //  Termination: at 0 or step size as small as can be
        return (b);

      { double p, q, s;

        s = fb/fa;
        if (a == c)       //  Secant method if points adjusted in F.1 or F.2
          { p = cmb*s;
            q = s-1.0;
#ifdef SHOW_ROOT
            printf("     Computing secant step %17.14e / %17.14e\n",p,q);
#endif
          }
        else              //  Else try quadratic estimate (p/q)
          { double t, r;
            t = fa/fc;
            r = fb/fc;
            p = s*(t*(r-t)*cmb - (1.0-r)*(b-a));
            q = (t-1.0)*(r-1.0)*(s-1.0);
#ifdef SHOW_ROOT
	    printf("     Computing quadratic step %17.14e / %17.14e\n",p,q);
#endif
          }
        if (p < 0.0)       //  Make p positive
          { q = -q;
            p = -p;
          }
        if (p < fabs(0.5*e*q) && p < .75*cmb*q)   // Converging (new step < .5 * step 2 before)
          { e = d;                                //     and new point between b & (b+3c)/4
            d = p/q;                              // (also catches degeneracies (q == 0))
#ifdef SHOW_ROOT
	    printf("     Taking step %17.14e , %17.14e [%17.14e]\n",d,e,rel);
#endif
          }
        else                                     //  No? then just bisect
          { e = d;
            d = .5*cmb;
#ifdef SHOW_ROOT
	    printf("     Bisecting %17.14e , %17.14e [%17.14e]\n",d,e,rel);
#endif
          }
      }

      a  = b;               // Take the step, but rel at the smallest
      fa = fb;
      if (fabs(d) > rel)
        b += d;
      else if (d > 0)
        b += rel;
      else
        b -= rel;
      fb = (*f)(b);

      if ((fb < 0.) == (fc < 0.))    //  b and c are not bracketing => a and b are (F.2)
        { c  = a;
          fc = fa;
#ifdef SHOW_ROOT
	  printf("     Swap sign of c\n");
#endif
        }
    }

  fprintf(stderr,"Error: Did not find a root in %d iterations of Find_Function_Root ",ITMAX);
  fprintf(stderr,"(no derivative)\n");
  exit (1);
}

static double Fct_Root_wD(Root_Bundle *brack, double (*f)(double), double (*df)(double))
{ double  a,  b,  c;
  double fa, fb, fc;
  double dc;
  double  d,  e;
  int    iter;

  a  = brack->lft;
  b  = brack->rgt;

  fa = (*f)(a);
  fb = (*f)(b);
  if (fa == 0.0) return (a);
  if (fb == 0.0) return (b);

  if (fa > 0.0)
    { double x;
      x = a; a = b; b = x;
      fa = fb;
    }

  c  = a;
  fc = (*f)(c);
  dc = (*df)(c);
  e = d = fabs(b-a);

  for (iter = 1; iter <= ITMAX; iter++)
    {
#ifdef SHOW_ROOT
      printf("\n%3d: [ % 17.14e , % 17.14e, % 17.14e ]\n",iter,a,c,b);
      printf("           % 17.14e   % 17.14e\n",fc,dc);
#endif

      if ( ((c-b)*dc < fc) == ((c-a)*dc < fc) || fabs(fc) > fabs(0.5*e*dc))
        { e  = d;
          d  = 0.5*(b-a);
          c  = a+d;
#ifdef SHOW_ROOT
	  printf("     Taking bisection step %17.14e , %17.14e [%17.14e]\n",d,e,EPS*fabs(c)+TINY);
#endif
        }
      else
        { e  = d;
          d  = fc/dc;
          c -= d;
#ifdef SHOW_ROOT
	  printf("     Taking Newton step %17.14e , %17.14e [%17.14e]\n",d,e,EPS*fabs(c)+TINY);
#endif
        }

      if (fabs(d) < EPS*fabs(c) + TINY) return (c);

      fc = (*f)(c);
      dc = (*df)(c);

      if (fc == 0.0)
        return (c);
      else if (fc < 0.0)
        a = c;
      else
        b = c;
    }

  fprintf(stderr,"Error: Did not find a root in %d iterations of Find_Function_Root ",ITMAX);
  fprintf(stderr,"(with derivative)\n");
  exit (1);
}

double Find_Function_Root(Root_Bundle *brack, double (*f)(double), double (*df)(double))
{ if (df == NULL)
    return (Fct_Root_NoD(brack,f));
  else
    return (Fct_Root_wD(brack,f,df));
}


#ifdef TEST_DRIVER

double poly(double x)
{ return ((x-1.0)*(x-1.0)*(x+3.0)); }

double dpoly(double x)
{ return ((x-1.0)*(2.0*(x+3.0) + (x-1.0))); }

double poly3(double *x)
// { return (x[0]*x[0] + x[1]*x[1] + x[2]*x[2]); }
{ return ((x[0] + x[1] - 2.0)*(x[0] + x[1] - 2.0) + (x[0] - x[1] + 3.0)*(x[0] - x[1] + 3.0)); }

double *dpoly3(double *x)
{ static double dx[2];
 
  dx[0] = 2.0*(x[0] + x[1] - 2.0) + 2.0*(x[0] - x[1] + 3.0);
  dx[1] = 2.0*(x[0] + x[1] - 2.0) - 2.0*(x[0] - x[1] + 3.0);
  return (dx);
}

int main(int argc, char *argv[])
{ Root_Bundle _brack, *brack = &_brack;
  double       root;

  (void) argc;
  (void) argv;

  printf("\n\nFind_Min_Bracket:\n");
  brack->lft = -1.0;
  brack->rgt =  0.0;
  brack = Find_Root_Bracket(brack,poly);
  if (brack != NULL)
    printf("Bracket: [%17.14e,%17.14e]\n",brack->lft,brack->rgt);
  else
    printf("No bracket found\n");

  printf("\n\nFind_Root:\n");
  root = Find_Function_Root(brack,poly,NULL);
  printf("Soltn: root at %17.14e f(said) = %17.14e\n",root,poly(root));

  printf("\n\nFind_Root (wd):\n");
  root = Find_Function_Root(brack,poly,dpoly);
  printf("Soltn: root at %17.14e f(said) = %17.14e\n",root,poly(root));

  exit (0);
}

#endif
