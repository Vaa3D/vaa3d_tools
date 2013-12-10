/*****************************************************************************************\
*                                                                                         *
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "utilities.h"
#include "lu_matrix.h"

#define TINY 1.0e-20

static inline double pivot_check(double pivot)
{ if (pivot == 0.)
    fprintf(stderr,"Singular Matrix!\n");
  else if (pivot <= TINY)
    { fprintf(stderr,"Warning: pivot magnitude is less than %g, setting to this value\n",TINY);
      if (pivot < 0.)
        pivot = -TINY;
      else
        pivot = TINY;
    }
  return (pivot);
}

//  Solve the tridiagonal system [ a, b, c] x = v returning the answer in v.
//    [ a, b, c] is unmodified and can be reused.  A non-zero value is returned
//    if the matrix is singular or an underflow of the pivot occured.  Handles
//    the circular case where a[0] != 0 or c[n-1] != 0

int TDG_Solve(int n, double *a, double *b, double *c, double *v)
{ static double *w = NULL;
  static int     wmax = 0;

  double p;
  int    i, circular;
  double cnr, vnr, bnr, anr;
  double wm1, vm1;

  if (n >= wmax)
    { wmax = 1.2*n + 100;
      w    = (double *) Guarded_Realloc(w,sizeof(double)*wmax,Program_Name());
    }

  circular = (a[0] != 0. || c[n-1] != 0.);

  p = pivot_check(b[0]);
  if (p == 0.)
    return (1);

  w[0] = wm1 = c[0] / p;
  v[0] = vm1 = v[0] / p;
  if (circular)
    { anr  = a[0] / p;
      cnr  = c[n-1];
      vnr  = v[n-1];
      bnr  = b[n-1] - anr * cnr; 
    }

  for (i = 1; i < n-1; i++)
    { double ai = a[i];

      p = pivot_check(b[i] - ai*wm1);
      if (p == 0.)
        return (1);

      if (circular)
        { vnr -= cnr*vnr;
          cnr *= -wm1;
        }

      w[i] = wm1 = c[i] / p;
      v[i] = vm1 = (v[i] - ai*vm1) / p;
    }

  if (circular)
    { cnr += a[n-1];
      p = pivot_check(bnr - cnr*wm1);
      if (p == 0.)
        return (1);
      v[n-1] = vm1 = (vnr - cnr*vm1) / p;
    }
  else
    { double ai = a[n-1];
      p = pivot_check(b[n-1] - ai*wm1);
      if (p == 0.)
        return (1);
      v[n-1] = vm1 = (v[n-1] - ai*vm1) / p;
    }

  for (i = n-2; i >= 0; i--)
    vm1 = (v[i] -= w[i]*vm1);

  if (circular)
    v[0] -= anr * v[n-1];

  return (0);
}

/*
    Given control points v1, v2, ... vn, compute the Bezier displacements for cubic
    splines between them such that (a) the points are interpolated, and (b) 1st and
    2nd order continuous.  If the curve is to begin at v1 and end at vn then these
    are the solution [d] to the tridiagonal system at left, and if the curve is to be
    closed then they are the solution to the system at right.

       | 2 1        | d1     v1-v0         | 4 1     1 |  d1     v1-vn
       | 1 4 1      | d2     v2-v0         | 1 4 1     |  d2     v2-v0
       |   1 4 1    | d3     v3-v1         |   1 4 1   |  d3     v3-v1
       |     .....  |      =               |     ..... |      =
       |      1 4 1 | dn-1   vn-vn-2       |     1 4 1 | dn-1   vn-vn-2
       |        1 2 | dn     vn-vn-1       | 1     1 4 | dn     v1-vn-1

    We use a tailored version of the routine TDG solve to return the desired displacements
    in the array v.
*/

void Cubic_Spline(int n, double *v, int circular)
{ static double *w = NULL;
  static int     wmax = 0;

  double p;
  int    i;
  double cnr, vnr;
  double wm1, vm1;

  if (n >= wmax)
    { wmax = 1.2*n + 100;
      w    = (double *) Guarded_Realloc(w,sizeof(double)*wmax,Program_Name());
    }

  { double v0 = v[0];

    if (circular)
      v[0] = v[1] - v[n-1];
    else
      v[0] = v[1] - v0;
  
    vm1 = v[0];
    for (i = 1; i < n-1; i++)
      { double vi = v[i];
        v[i] = v[i+1] - vm1;
        vm1  = vi;
      }
  
    if (circular)
      v[n-1] = v0-vm1;
    else
      v[n-1] -= vm1;
  }

  if (circular)
    { w[0] = wm1 = .25;
      v[0] = vm1 = .25 * v[0];
      cnr  = 1.;
      vnr  = v[n-1];
    }
  else
    { w[0] = wm1 = .5;
      v[0] = vm1 = .5 * v[0];
    }

  for (i = 1; i < n-1; i++)
    { if (circular)
        { vnr -= cnr*vnr;
          cnr *= -wm1;
        }

      w[i] = wm1 = 1. / (4. - wm1);
      v[i] = vm1 = (v[i] - vm1) * wm1;
    }

  if (circular)
    { cnr += 1.;
      v[n-1] = vm1 = (vnr - cnr*vm1) / (3.75 - cnr*wm1);
    }
  else
    v[n-1] = vm1 = (v[n-1] - vm1) / (2. - wm1);

  for (i = n-2; i >= 0; i--)
    vm1 = (v[i] -= w[i]*vm1);

  if (circular)
    v[0] -= .25 * v[n-1];
}


//  L.U Decompose square matrix a of dimension n in place with partial pivoting, where
//    p is the identity permutation on input, and the permutation of rows upon return.
//    The diagonal of L is 1, and +/-1 is returned depending on the even/odd # of row
//    permutations (needed for determinant calculation).

int LU_Decompose(int n, double **a, int *p)
{ static double *v = NULL;
  static int     vmax = 0;

  int     i, j;
  int     sign;

  if (n >= vmax)
    { vmax = 1.2*n + 100;
      v    = (double *) Guarded_Realloc(v,sizeof(double)*vmax,Program_Name());
    }

  sign = 1;
  for (i = 0; i < n; i++)  // Find the scale factors for each row in v.
    { double b, f, *r;

      r = a[i];
      b = 0.;
      for (j = 0; j < n; j++)
        { f = fabs(r[j]);
          if (f > b)
            b = f;
        }
      if (b == 0.0)
        { fprintf(stderr,"Singular Matrix!\n");
          exit (1);
        }
      v[i] = 1./b;
    }

  for (j = 0; j < n; j++)      //  For each column
    { double b, s, *r;
      int    k, w;

      for (i = 0; i < j; i++)    // Determine U
        { r = a[i];
          s = r[j];
          for (k = 0; k < i; k++)
            s -= r[k]*a[k][j];
          r[j] = s;
        }

      b = -1.;
      for (i = j; i < n; i++)      // Determine L without dividing by pivot, in order to
        { r = a[i];                //   determine who the pivot should be.
          s = r[j];
          for (k = 0; k < j; k++)
            s -= r[k]*a[k][j];
          r[j] = s;

          s = v[i]*fabs(s);        // Update best pivot seen thus far
          if (s > b)
            { b = s;
              w = i;
            }
        }

      if (w != j)                  // Pivot if necessary
        { r    = a[w];
          a[w] = a[j];
          a[j] = r;
          k    = p[w];
          p[w] = p[j];
          p[j] = k;
          sign = -sign;
          v[w] = v[j];
        }

      if (fabs(a[j][j]) < TINY)    // Complete column of L by dividing by selected pivot
        { fprintf(stderr,"Warning: pivot magnitude is less than %g, setting to this value\n",TINY);
          if (a[j][j] < 0.)
            a[j][j] = -TINY;
          else
            a[j][j] = TINY;
        }
      b = 1./a[j][j];
      for (i = j+1; i < n; i++)
        a[i][j] *= b;
    }

#ifdef DEBUG_LU
  { int i, j;

    printf("\nLU Decomposition\n");
    for (i = 0; i < n; i++)
      { printf("  %2d: ",p[i]);
        for (j = 0; j < n; j++)
          printf(" %8g",a[i][j]);
        printf("\n");
      }
  }
#endif

  return (sign);
}

void Show_LU_Product(FILE *file, int n, double **a, int *p)
{ int    i, j, k;
  double u;

  fprintf(file,"\nLU Product:\n");
  for (i = 0; i < n; i++)
    { for (j = 0; j < i; j++)
        { u = 0.;
          for (k = 0; k <= j; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      for (j = i; j < n; j++)
        { u = a[i][j];
          for (k = 0; k < i; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      fprintf(file,"\n");
    }
}

//  Given square matrix a of size n in LU-form, permuted row-wise by p, solve in-place
//    in vector b the sytem of equations a.x = p(b)

void LU_Solve(int n, double **a, double *b, int *p)
{ static double *x = NULL;
  static int     xmax = 0;

  int    i, j;
  double s, *r;

  if (n >= xmax)
    { xmax = 1.2*n + 100;
      x    = (double *) Guarded_Realloc(x,sizeof(double)*xmax,Program_Name());
    }

  for (i = 0; i < n; i++)
    { r = a[i];
      s = b[p[i]];
      for (j = 0; j < i; j++)
        s -= r[j] * x[j];
      x[i] = s;
    }

  for (i = n-1; i >= 0; i--)
    { r = a[i];
      s = x[i]; 
      for (j = i+1; j < n; j++)
        s -= r[j] * b[j];
      b[i] = s/r[i];
    }
}

double LU_Determinant(int n, double **a, int sign)
{ int    i;
  double det;

  det = sign;
  for (i = 0; i < n; i++)
    det *= a[i][i];
  return (det);
}
