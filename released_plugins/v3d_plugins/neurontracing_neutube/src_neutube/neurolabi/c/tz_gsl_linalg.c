/* tz_gsl_linalg.h
 *
 * 01-Feb-2008 Initial write: Ting Zhao
 */
#include "tz_gsl_linalg.h"
#ifdef HAVE_LIBGSL
#  include <gsl/gsl_permutation.h>
#  include <gsl/gsl_blas.h>
#endif
#include "tz_error.h"

INIT_EXCEPTION

/* gsl_linalg_invert(): invert a matrix.
 * 
 * Args: m - the matrix to invert;
 *       inverse - the inverse of m.
 *
 * Return: NULL.
 */
#ifdef HAVE_LIBGSL
void Gsl_Linalg_Invert(gsl_matrix *m, gsl_matrix *inverse)
{
  gsl_permutation *p = gsl_permutation_alloc(m->size2);
  gsl_permutation_init(p);

  int s;
  gsl_linalg_LU_decomp(m, p, &s);
  
  gsl_linalg_LU_invert(m, p, inverse);

  gsl_permutation_free(p);
}
#endif

/* 
 * under defelopment
 */
#ifdef HAVE_LIBGSL
void Gsl_Matrix_Sycom(CBLAS_UPLO_t sy_uplo_option, gsl_matrix *m)
{
  //int i, j;

  switch (sy_uplo_option) {
  case CblasUpper:

    
    break;
  default:
    break;
  }
}
#endif

void Mat3d_Eigen_Coef(double a, double b, double c, double d, double e,
		      double f, double *coef)
{
  coef[0] = - a - b - c;
  double e2 = e * e;
  double d2 = d * d;
  double f2 = f * f;
  coef[1] = a * (b + c) + b * c + d2 + e2 + f2;
  coef[2] = a * f2 + b * e2 + c * d2 - a * b * c + 2.0 * d * e * f;
}
