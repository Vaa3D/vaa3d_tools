/**file testrand.c
 * @author Ting Zhao
 * @date 15-Apr-2010
 */

#include <time.h>
#include <unistd.h>
#include "tz_utilities.h"
#include "tz_random.h"
#include "tz_iarray.h"
#include "tz_darray.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  Random_Seed(time(NULL) - getpid());
  int *array = Unifrnd_Int_Array(100, 100, NULL);
  iarray_print2(array, 10, 10);

  double *array2 = Unifrnd_Double_Array(100, NULL);
  darray_print2(array2, 10, 10);
#endif

#if 0
  Random_Seed(time(NULL) - getpid());
  int array[100];
  int i;
  for (i = 0; i < 100; i++) {
    array[i] = Poissonrnd(255.0);
  }
  //printf("%g\n", iarray_mean_d(array, 100));
  iarray_print2(array, 20, 5);
#endif

#if 1
  Random_Seed(time(NULL) - getpid());
  double array[1000];
  int i;
  for (i = 0; i < 1000; i++) {
    array[i] = Normrnd(10.0, 3.0);
  }

  darray_write("../data/test.dat", array, 1000);
#endif

  return 0;
}
