/* tz_neurofield.c
 *
 * 17-Jan-2008 Initial write: Ting Zhao
 */

#include <math.h>

#include "tz_error.h"
#ifdef HAVE_LIBGSL
#  include <gsl/gsl_cdf.h>
#endif
#include "tz_neurofield.h"

/* Neurofield1(): sample neuron field.
 *
 * Args: coef - expanding coefficient;
 *       base - radius at z=0;
 *       x - x coordinate;
 *       y - y coordinate;
 *       z - z coordinate.
 *       z_min - minimal z coordinate;
 *       z_max - maximal z coordinate.
 *
 * Return: the field value.
 *
 * Note: The function is 
 */
double Neurofield1(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma2 = coef * z + base;
						
#if 0
  printf("%g\n", sigma2);
#endif

  sigma2 = sigma2 * sigma2;
  
  double t = (x * x + y * y) / sigma2;
  
  return (1 - t) * exp(-t) / sigma2;
}

double Neurofield5(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma = coef * z + base;
						
#if 0
  printf("%g\n", sigma);
#endif

  double sigma2 = sigma * sigma;
  
  double t = (x * x + y * y) / sigma2;
  
  double value = (1 - t) * exp(-t) / sigma2;

  /* Give positive part more weight when the filter is thicker */
  if (value > 0.0) {
    value *= sigma;
  }

  return value;
}

/* neurofield2(): calculate neuron field.
 *
 * Args: coef - expanding coefficient;
 *       base - radius at z=0;
 *       x - x coordinate;
 *       y - y coordinate;
 *       z - z coordinate.
 *
 * Return: the field value.
 */
double Neurofield2(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma2 = coef * z + base;
						
#if 0
  printf("%g\n", sigma2);
#endif

  sigma2 = sigma2 * sigma2;
  
  double t = (x * x + y * y) ;
  
  if (t < sigma2) {
    return 1.0;
  } else if (t < sigma2 * 2.25) {
    return -0.5;
  } else {
    return 0;
  }
}


/* neurofield3(): calculate neuron field.
 *
 * Args: coef - expanding coefficient;
 *       base - radius at z=0;
 *       x - x coordinate;
 *       y - y coordinate;
 *       z - z coordinate.
 *
 * Return: the field value.
 */
#define PSF_SIGMA 1.0
double Neurofield3(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
#ifdef HAVE_LIBGSL
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma2 = coef * z + base;

  double t = sqrt(x * x + y * y);
  
  double g = 0.0;

  if (t < sigma2) {
    g = (gsl_cdf_gaussian_P(sigma2 - t, PSF_SIGMA) - 
	 gsl_cdf_gaussian_P(- sigma2 - t, PSF_SIGMA));
  } else if (t < sigma2 + 2.0) {
    g = -0.2;
  }

  return g;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return 0.0;
#endif
}
#undef PSF_SIGMA

double Neurofield4(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma2 = coef * z + base;
  sigma2 = sigma2 * sigma2;
  
  double d2 = (x * x + y * y);

  double t =  d2 / sigma2;

  if (t <= 1) {
    return exp(-t);
  } else {
    return - exp(sigma2 - d2) / 2.0;
  }
}

double Neurofield7(double coef, double base, 
			  double x, double y, double z,
			  double z_min, double z_max)
{
  if (z < z_min) {
    return 0;
  }

  if (z > z_max) {
    return 0;
  }

  double sigma2 = coef * z + base;
						
#if 0
  printf("%g\n", sigma2);
#endif

  sigma2 = sigma2 * sigma2;
  
  double t = (x * x + y * y) / sigma2;

  
  /* 0.01 is for relaxation */
  if (t > 1.01) {
    return 0.0;
  }
  
  
 return (1 - t) * exp(-t) / sigma2 + 0.1;
}

double Neuroseg_Dist_Field(double coef, double base, 
				  double x, double y, double z,
				  double z_min, double z_max)
{
  if (z < z_min) {
    return 2.0;
  }

  if (z > z_max) {
    return 2.0;
  }

  double sigma2 = coef * z + base;
  sigma2 = sigma2 * sigma2;
  
  double d2 = (x * x + y * y);

  double t =  d2 / sigma2;

  return sqrt(t);
}
