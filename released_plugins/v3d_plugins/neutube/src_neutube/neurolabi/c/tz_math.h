/**@file tz_math.h
 * @brief basic math routines
 * @author Ting Zhao
 * @date 08-Nov-2008
 */

#ifndef _TZ_MATH_H_
#define _TZ_MATH_H_

#include "tz_cdefs.h"
#include "tz_math_lib.h"

__BEGIN_DECLS

/**@addtogroup general_math_ General math functions
 * @{
 */

/*
 * iround() round x to the nearest integer. Its behavior is undefined when x
 * is beyond the integer range.
 */
int iround(double x);

/**@brief Quantile of normal distribution
 *
 * x has to be between (0, 1)
 */
double Normcdf_Quantile(double x);

/**@brief Compare two float numbers.
 *
 * Compare_Float() returns 0 if x and y are equal given the error torelance
 * <eps>. It returns 1 if x greater than y and -1 if x is smaller than y.
 */
int Compare_Float(double x, double y, double eps);

double Cube_Root(double x);

/* solve equation of ax^3+bx^2+cx+d=0 */
int Solve_Cubic(double a, double b, double c, double d, double *sol);

/*
 * Polyeval returns c[0] + c[1] * x + ... + c[order] * x^order
 */
double Polyeval(double *c, int order, double x);

/* calculate eigenvalues of 3x3 symmetric matrix 
 *  a d e
 *  d b f
 *  e f c
 * It returns a non-zero value when the calculation failed and result is set
 * to all 0. The result is stored in <result> in the descending order.
 */
int Matrix_Eigen_Value_Cs(double a, double b, double c, double d, double e,
			  double f, double *result);

/**@brief Eigenvector of a 3x3 symmetric matrix.
 *
 * Matrix_Eigen_Vector_Cs() calculates the the eigenvector of the one with the
 * value <r>. The result is stored in <result>, which is normalized.
 */
void Matrix_Eigen_Vector_Cs(double a, double b, double c, double d, double e,
    double f, double r, double *result);

/**@}*/

__END_DECLS

#endif
