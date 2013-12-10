/* testhistogram.c
 *
 * 25-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_int_histogram.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
#if 0
  int hist[]= {10, 0, 2, 3, 10, 2, 3, 4, 1, 1, 2, 1};
  
  Print_Int_Histogram(hist);
  printf("slope: %g\n", Int_Histogram_Slope(hist, 0, 2));
#endif

#if 0
  int hist[]= {10, 0, 2, 3, 10, 2, 3, 4, 1, 1, 2, 1};
  
  Print_Int_Histogram(hist);

  int *map = Int_Histogram_Equalize(hist, 0, 100);
  Print_Int_Histogram(map);
#endif

#if 1
  int hist[]= {10, 0, 2, 3, 10, 2, 3, 4, 1, 1, 2, 10};
  
  Print_Int_Histogram(hist);

  int *map = Int_Histogram_Equal_Info_Map(hist, 5, NULL);

  iarray_print2(map, Int_Histogram_Length(hist), 1);
#endif

  return 0;
}
