/*****************************************************************************************\
*                                                                                         *
*  Function minimization routines for 1 and multiple dimensions                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  May 2007                                                                      *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "utilities.h"
#include "fct_min.h"

#undef  SHOW_MINIMIZATION
#undef  SHOW_MULTI_MIN
#undef  TEST_DRIVER
#undef  FUNC3

#define ITMAX  1000                // Maximum # of iterations
#define CGOLD  0.381966011250105   // The Golden Ratio (to DBL_DIG = 15 digits)
#define TINY   1e-20               // Minimum fractional accuracy
#define RND    1.00000001          // Rounding error in a half precision add
#define TOL     .00000001          // For doubles, cannot generally hope for more
                                   //   than half-precision = 7.5 digits

/* ONE-DIMENSIONAL MINIMIZATION:

     Find_Min_Bracket:  find a bracket enclosing a minimum
     Min_Fct_NoD:       find a minimum using parabolic acceleration of Golden sections
     Min_Fct_wD:        find a minimum using derivative driven bisection
     Minimize_Function: general entry point for finding a minimum
*/

Min_Bracket *Find_Min_Bracket(double p, double s, double (*f)(double))
{ static Min_Bracket Brack;

  double  a,  b,  c,  u;
  double fa, fb, fc, fu;
  double gstep, gratio;
  int    swap, iter;

  a  = p;
  b  = p + s;
  fa = (*f)(a);             //  Swap a and b if necessary so that b is downhill of a
  fb = (*f)(b);
  if (fb >= fa)
    { u = a;  fu = fa;
      a = b;  fa = fb;
      b = u;  fb = fu;
    }
  swap = (b < a);           //  Have to reverse a,b,c in final answer?

  gratio = 2.-CGOLD;
  gstep  = gratio * (b-a);  //  Always keep track of the last (and largest Golden ratio step)

  c  = b + gstep;           //  Take a first step
  fc = (*f)(c);

#ifdef SHOW_MINIMIZATION
  printf("\n  0: % 17.14e   % 17.14e   % 17.14e\n",a,b,c);
  printf("     % 17.14e > % 17.14e > % 17.14e\n",fa,fb,fc);
#endif

  for (iter = 1; iter < 100; iter++)

    if (fc > fb)                    //  If f(a) > f(b) < f(c) then your done
      { if (swap)
          { Brack.lft = c;
            Brack.rgt = a;
          }
        else
          { Brack.lft = a;
            Brack.rgt = c;
          }
        Brack.mid = b;
        return (&Brack);
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
            fu = (*f)(u);
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
                    fc = (*f)(c);            //   too big a step size
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
            fc = (*f)(c);
#ifdef SHOW_MINIMIZATION
            printf("         Gold: c = %17.14e, fc = %17.14e\n",c,fc);
#endif
          }
      }

  fprintf(stderr,"Warning: Find_Min_Bracket did not find a bracket in %d iteratios\n",ITMAX);

  return (NULL);
}

static double Min_Fct_NoD(Min_Bracket *brack, double (*f)(double), double *xmin)
{
  double  a, c;       // Current bracket [a,c]
  double  x,  y,  z;  // Lowest f(x) < f(y) < f(z) with x in [a,c]
  double fx, fy, fz;  // function values at x, y, and z
  double  d, e;       // step in iteration (d), and the one before (e)
  int     iter;

  a = brack->lft;
  c = brack->rgt;
  x  = y  = z  = brack->mid;  // 1st two iterations will be Golden ratio divisions, so y and z
  fx = fy = fz = (*f)(x);     //   will not be used, nor will e or d, until they have been set

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

      fu = (*f)(u);
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

static double Min_Fct_wD(Min_Bracket *brack,
                         double (*f)(double), double (*df)(double), double *xmin)
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
  x  = y  = z  = brack->mid;  // 1st two iterations will be Golden ratio divisions, so y and z
  fx = fy = fz = (*f)(x);     //   will not be used, nor will e or d, until they have been set
  dx = dy = dz = (*df)(x);
  fatstep = 2.0*(c-a);

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

      fu = (*f)(u);      //  Update bracket [a,x,c], and 3 minimums, x,y,z, and their values
      du = (*df)(u);     //    fx,fy,fz and derivatives dx,dy,dz with respect to the new point u.
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

double Minimize_Fct(Min_Bracket *brack, double (*f)(double), double (*df)(double), double *xmin)
{ double a;

  if (df == NULL)
    a = Min_Fct_NoD(brack,f,xmin);
  else
    a = Min_Fct_wD(brack,f,df,xmin);
  return (a);
}

/* LINE MINIMIZATION ROUTINES:

     init_line_min:   Setup storage and global vars for dimension, function, and gradient thereof
     line_min_nod:    Do a line minimization without derivative
     line_min_wd:     Do a line minimization with derivative
     f1dim:           Evaluate function at scalar position along current line
     df1dim:          Evaluate derivative at scalar position along current line in direction of said
*/

static void print_vector(int n, double *vec)
{ int j;

  printf("(%.14g",vec[0]);
  for (j = 1; j < n; j++)
    printf(",%.14g",vec[j]);
  printf(")");
}

//  Global vars for tricking 1d routines into minimizing along a line

static int     MFN_n;                  // Dimension of the function
static double  (*MFN_f)(double *);     // N-dimensional function
static double *(*MFN_df)(double *);    // N-dimensional gradiant

static double *MFN_pc;                 // Start & end point of current line minimization
static double *MFN_dir;                // Current line direction
static double *MFN_pe = NULL;          // Space for current line point to evaluate at

static void init_line_min(int n, double (*f)(double *), double *(*df)(double *))
{ static int nmax = 0;

  if (n >= nmax)
    { nmax = 1.2*n + 10;
      MFN_pe = (double *)  Guarded_Realloc(MFN_pe,sizeof(double)*nmax,Program_Name());
    }
  MFN_n  = n;
  MFN_f  = f;
  MFN_df = df;
}

static double f1dim(double x)     // evaluate MFN_f at MFN_pc + x*MFN_dir
{ int i;

  for (i = 0; i < MFN_n; i++)
    MFN_pe[i] = MFN_pc[i] + x*MFN_dir[i];
  return ((*MFN_f)(MFN_pe));
}

static double df1dim(double x)     // evaluate d(MFN_f)/d(MFN_dir) at MFN_pc + x*MFN_dir
{ double *gd, df;
  int     i;

  gd = (*MFN_df)(MFN_pe);     //  x is ignored, always the same as last call to f1dim!
  for (i = 0; i < MFN_n; i++)
    df += gd[i]*MFN_dir[i];
  return (df);
}

static inline double line_min_nod(double *pc, double *e)
{ double       xmin, fp;
  Min_Bracket *brack;
  int          i;

  MFN_dir = e;
  MFN_pc  = pc;
  brack   = Find_Min_Bracket(0.0,0.1,f1dim);     //  This could fail!
  fp      = Min_Fct_NoD(brack,f1dim,&xmin);
  for (i = 0; i < MFN_n; i++)
    MFN_pc[i] = MFN_pc[i] + xmin*e[i];
  return (fp);
}

static inline double line_min_wd(double *pc, double *e)
{ double       xmin, fp;
  Min_Bracket *brack;
  int          i;

  MFN_dir = e;
  MFN_pc  = pc;
  brack   = Find_Min_Bracket(0.0,0.1,f1dim);     //  This could fail!
  fp      = Min_Fct_wD(brack,f1dim,df1dim,&xmin);
  for (i = 0; i < MFN_n; i++)
    MFN_pc[i] = MFN_pc[i] + xmin*e[i];
  return (fp);
}

/* MULTI-DIMENSIONAL MINIMIZATION:

     Make_Orthogonal_Step:    make a matrix of initial direction vectors for Powell Minimizer
     Powell_Minimizer:        find a multi-dimensional min using line minimization on a set of
                                direction vectors that are evolved to be mutually conjugate
     Polak_Rabier_Minimizer:  find a multi-dimensional min using conjugate gradient descent
*/

double Powell_Minimizer(int n, double *xin, double **step, double (*f)(double *))
{ static int     nmax = 0;
  static double  *vec = NULL;
  static double **dir = NULL;
  static double  *av, *pi, *pe;

  double *pc;
  double  fi,  fc,  fe;
  int     iter;

  if (n >= nmax)
    { nmax = 1.2*n + 10;
      vec = (double *)  Guarded_Realloc(vec,sizeof(double)*3*nmax,Program_Name());
      dir = (double **) Guarded_Realloc(dir,sizeof(double *)*nmax,Program_Name());
      pi     = vec;
      pe     = vec+nmax;
      av     = vec+2*nmax;
    }

  { int i;
    for (i = 0; i < nmax; i++)
      dir[i] = step[i];
  }

  init_line_min(n,f,NULL);

  pc = xin;
  fc = (*f)(pc);
#ifdef SHOW_MULTI_MIN
  printf("   FC");
  print_vector(n,pc);
  printf(" = %.14g\n",fc);
#endif

  for (iter = 1; iter <= ITMAX; iter++)
    { double dmax;
      int    imax;
      int    i;

      for (i = 0; i < n; i++)
        pi[i] = pc[i];
      fi = fc;
#ifdef SHOW_MULTI_MIN
      printf("\n");
#endif

      imax = 0;
      dmax = 0.0;
      for (i = 0; i < n; i++)
        { double fn;

          fn = line_min_nod(pc,dir[i]);
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
        return (fc); 

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
        { double *tmp;
          fc = line_min_nod(pc,av);
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
  return (fc);
}

double **Make_Orthogonal_Step(int n, double step)
{ static int     nmax = 0;
  static double  *vec = NULL;
  static double **dir = NULL;

  if (n >= nmax)
    { nmax = 1.2*n + 10;
      vec = (double *)  Guarded_Realloc(vec,sizeof(double)*nmax*nmax,Program_Name());
      dir = (double **) Guarded_Realloc(dir,sizeof(double *)*nmax,Program_Name());
    }

  { int i;

    for (i = n*n-1; i >= 0; i--)
      vec[i] = 0.0;
    for (i = n-1; i >= 0; i--)
      { dir[i]    = vec + i*n;
        dir[i][i] = step;
      }
  }

  return (dir);
}

double Polak_Rabier_Minimizer(int n, double *xin, double (*f)(double *), double *(*df)(double *))
{ static int      nmax = 0;
  static double  *g, *h = NULL;

  double *pc;
  double  fc;
  double *dc;
  int     iter, j;

  if (n >= nmax)
    { nmax = 1.2*n + 10;
      h = (double *) Guarded_Realloc(h,sizeof(double)*2*nmax,Program_Name());
      g = h+nmax;
    }

  init_line_min(n,f,df);

  pc = xin;
  fc = (*f)(pc);
  dc = (*df)(pc);
  for (j = 0; j < n; j++)
    h[j] = g[j] = -dc[j];
#ifdef SHOW_MULTI_MIN
  printf("   FC");
  print_vector(n,pc);
  printf(" = %.14g (",fc);
  print_vector(n,dc);
  printf("\n");
#endif

  for (iter = 1; iter <= ITMAX; iter++)
    { double fn;
      double num, den, gam;

      fn = line_min_wd(pc,h);
#ifdef SHOW_MULTI_MIN
      printf("  MIN %d ",iter);
      print_vector(n,h);
      printf(" ->  ");
      print_vector(n,pc);
      printf(" f = %.14g\n",fn);
#endif

      if (fn-fc <= TOL*fabs(fc)+TINY)
        return (fn); 

      fc = fn;
      dc = (*df)(pc);

      num = den = 0.;
      for (j = 0; j < n; j++)
        { num += g[j] * g[j];
          den += h[j] * (h[j] + g[j]);
        }
      if (den == 0.0)
        return (fn);
      gam = num/den;
      for (j = 0; j < n; j++)
        { g[j] = -dc[j];
          h[j] = g[j] + gam*h[j];
        }
    }

  fprintf(stderr,"Warning: Too many iterations (%d) in Polak_Ribier_Minimzer\n",ITMAX);
  return (fc);
}


#ifdef TEST_DRIVER

double poly(double x)
{ return ((x+1.0)*x*(x-1.0)); }

double dpoly(double x)
{ return (3.0*x*x - 1.0); }

double poly3(double *x)
// { return (x[0]*x[0] + x[1]*x[1] + x[2]*x[2]); }
{ return ((x[0] + x[1] - 2.0)*(x[0] + x[1] - 2.0) + (x[0] - x[1] + 3.0)*(x[0] - x[1] + 3.0)); }

double *dpoly3(double *x)
{ static double dx[2];
 
  dx[0] = 2.0*(x[0] + x[1] - 2.0) + 2.0*(x[0] - x[1] + 3.0);
  dx[1] = 2.0*(x[0] + x[1] - 2.0) - 2.0*(x[0] - x[1] + 3.0);
  return (dx);
}

#ifdef FUNC1

int main(int argc, char *argv[])
{ Min_Bracket _brack, *brack = &_brack;
  double       fmin, xmin;

  printf("\n\nFind_Min_Bracket:\n");
  brack = Find_Min_Bracket(80.,79.9,poly);
  if (brack != NULL)
    printf("Bracket: [%17.14e,%17.14e,%17.14e]\n",brack->lft,brack->mid,brack->rgt);
  else
    printf("No bracket found\n");

  printf("\n\nMinimize_Fct:\n");
  fmin = Minimize_Fct(brack,poly,dpoly,&xmin);
  printf("Soltn: min of %17.14e at x=%17.14e\n",fmin,xmin);
  fmin = Minimize_Fct(brack,poly,NULL,&xmin);
  printf("Soltn: min of %17.14e at x=%17.14e\n",fmin,xmin);

  exit (0);
}

#endif

#ifdef FUNC3

int main(int argc, char *argv[])
{ double pi[2];
  double **step;
  double fmin;

  printf("\n\nMinimize_Fct:\n");
  pi[0] = 1.0;
  pi[1] = 1.0;
  step  = Make_Orthogonal_Step(2,.1);
  fmin  = Powell_Minimizer(2,pi,step,poly3);
  printf("Soltn: min of %17.14e at x=(%17.14e,%17.14e,%17.14e)\n",fmin,pi[0],pi[1],pi[2]);

  pi[0] = 1.0;
  pi[1] = 1.0;
  fmin  = Polak_Rabier_Minimizer(2,pi,poly3,dpoly3);
  printf("Soltn: min of %17.14e at x=(%17.14e,%17.14e,%17.14e)\n",fmin,pi[0],pi[1],pi[2]);

  exit (0);
}

#endif

#endif
