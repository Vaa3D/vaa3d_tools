/* testbitmask.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_error.h"
#include "tz_gsl_linalg.h"

INIT_EXCEPTION_MAIN(e)

int main()
{
  double d[] = {0.9561, 0.9023, 0.3696, 0.0871, 0.4159,
		0.6842, 0.6389, 0.0851, 0.3949, 0.2790,
		0.9572, 0.6953, 0.5239, 0.3074, 0.9671,
		0.2132, 0.1105, 0.6237, 0.5497, 0.5662,
		0.3049, 0.6658, 0.0298, 0.9852, 0.0655};

  gsl_matrix_view m_view = gsl_matrix_view_array(d, 5, 5);
  gsl_matrix *inv = gsl_matrix_alloc(5, 5);
  Gsl_Linalg_Invert(&(m_view.matrix), inv);

  darray_print2(m_view.matrix.data, 5, 5);
  darray_print2(inv->data, 5, 5);
 
  return 0;
}
