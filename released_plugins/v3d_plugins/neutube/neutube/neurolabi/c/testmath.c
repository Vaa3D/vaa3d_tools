/**@file testmath.c
 * @author Ting Zhao
 * @date 22-Jul-2010
 */

#include "tz_utilities.h"
#include "tz_math.h"
#include "tz_darray.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  double a = 0;
  double b = 1;
  double c = 1;
  double d = 1;
  double e = -1;
  double f = 0;

  double result[3];
  Matrix_Eigen_Value_Cs(a, b, c, d, e, f, result);
  darray_print2(result, 3, 1);

  double vec[3];
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[0], vec);
  darray_print2(vec, 3, 1);
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[1], vec);
  darray_print2(vec, 3, 1);
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[2], vec);
  darray_print2(vec, 3, 1);
#endif

#if 0
  /** result:
   * eigenvalues:
   9.4139  1.2601  0.0260  

   * eigen vectors:
   [0.3068  0.4831  0.8200]

   [0.6917  0.4786  -0.5408]

   [0.6537  -0.7331  0.1874]
   */
  double a = 1.5;
  double b = 2.5;
  double c = 6.7;
  double d = 1.8;
  double e = 1.9;
  double f = 3.4;
  double result[3];
  Matrix_Eigen_Value_Cs(a, b, c, d, e, f, result);
  darray_print2(result, 3, 1);

  double vec[3];
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[0], vec);
  darray_print2(vec, 3, 1);
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[1], vec);
  darray_print2(vec, 3, 1);
  Matrix_Eigen_Vector_Cs(a, b, c, d, e, f, result[2], vec);
  darray_print2(vec, 3, 1);
#endif

#if 1
  double x;
  for (x = 0.0; x < 1.0; x += 0.01) {
    double y = Normcdf_Quantile(x);
    printf("f(%g) = %g\n", x, y);
  }
#endif

  return 0;
}
