/**@file tz_gsl_linalg.h
 * @brief wrappers for gsl linear algebra
 * @author Ting Zhao
 * @date 01-Feb-2008
 */

#ifndef _TZ_GSL_LINALG_H_
#define _TZ_GSL_LINALG_H_

#include "tz_cdefs.h"

#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_linalg.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif

__BEGIN_DECLS

#ifdef HAVE_LIBGSL
/*
 * Gsl_Linalg_Invert() calculates the inverse of <m> and stores the result in
 * <inverse>. No aliasing between <m> and <inverse> is allowed. Note that <m>
 * is modified in the function.
 */
void Gsl_Linalg_Invert(gsl_matrix *m, gsl_matrix *inverse);
#endif

void Mat3d_Eigen_Coef(double a, double b, double c, double d, double e,
		      double f, double *coef);

void Solve_Cubic_Equation(double a, double b, double c, double *roots);

__END_DECLS

#endif
