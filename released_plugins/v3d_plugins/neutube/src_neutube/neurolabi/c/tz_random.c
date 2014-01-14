/* tz_random.c
 * 
 * 14-Oct-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_math.h"
#include "tz_random.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_iarray.h"

#if EXTLIB_EXIST(GSL)
#  include "gsl/gsl_rng.h"
#  include "gsl/gsl_randist.h"
#else
#  include "private/tzp_random.c"
#endif

#if EXTLIB_EXIST(GSL)
static gsl_rng *Random_Generator = NULL;

static void gsl_rng_init()
{
  if (Random_Generator == NULL) {
    Random_Generator = gsl_rng_alloc(gsl_rng_default);
    gsl_rng_set(Random_Generator, 0);
  }
}
#endif

void Random_Seed(unsigned long int seed)
{
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  gsl_rng_set(Random_Generator, seed);
#else
  init_genrand(seed);
#endif
}


double Unifrnd()
{
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  return gsl_rng_uniform(Random_Generator);
#else
  return genrand_real1();
#endif  
}

int Unifrnd_Int(int range)
{
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  return gsl_rng_uniform_int(Random_Generator, range);
#else
  return genrand_int32() % (range + 1);
#endif    
}

unsigned int Poissonrnd(double mu)
{
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  return gsl_ran_poisson(Random_Generator, mu);
#else
  /*
  TZ_ERROR(ERROR_NA_FUNC);
  return 0.0;
*/

  double L = exp(-mu);
  unsigned int k = 0;
  double p = 1;

  do {
    k++;
    p *= Unifrnd();
  } while (p > L);

  return k - 1;
#endif  
}

double* Unifrnd_Double_Array(int length, double *array)
{
  if (length <= 0) {
    return NULL;
  }
  
  if (array == NULL) {
    array = darray_malloc(length);
  }

  int i;
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  for (i = 0; i < length; i++) {
    array[i] = gsl_rng_uniform(Random_Generator);
  }
#else
  for (i = 0; i < length; i++) {
    array[i] = Unifrnd();
  }  
#endif

  return array;
}

int* Unifrnd_Int_Array(int range, int length, int *array)
{
  if (length <= 0) {
    return NULL;
  }
  
  if (array == NULL) {
    array = iarray_malloc(length);
  }

  int i;
#if EXTLIB_EXIST(GSL)
  gsl_rng_init();
  for (i = 0; i < length; i++) {
    array[i] = gsl_rng_uniform_int(Random_Generator, range);
  }
#else
  for (i = 0; i < length; i++) {
    array[i] = Unifrnd_Int(range);
  }
#endif

  return array; 
}

double Normrnd_Std()
{
  double x = Unifrnd();

  x = Normcdf_Quantile(x);

  if (isnan(x)) {
    x = Normrnd_Std();
  }

  return x;
}

double Normrnd(double mu, double sigma)
{
  return Normrnd_Std() * sigma + mu;
}

int Bernrnd(double p)
{
  double x = Unifrnd();

  return x < p;
}
