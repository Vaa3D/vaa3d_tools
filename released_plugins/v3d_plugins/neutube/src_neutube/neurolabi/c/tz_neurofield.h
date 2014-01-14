/**@file tz_neurofield.h
 * @brief *neuron receptor field (obsolete)
 * @author Ting Zhao
 * @date 17-Jan-2008
 */

#ifndef _TZ_NEUROFIELD_H_
#define _TZ_NEUROFIELD_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#define NEUROFIELD 5

#if NEUROFIELD == 1
#define Neurofield Neurofield1
#endif

#if NEUROFIELD == 2
#define Neurofield Neurofield2
#endif

#if NEUROFIELD == 3
#define Neurofield Neurofield3
#endif

#if NEUROFIELD == 4
#define Neurofield Neurofield4
#endif

#if NEUROFIELD == 5
#define Neurofield Neurofield5
#endif

double Neurofield1(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield2(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield3(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield4(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield5(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield6(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neurofield7(double coef, double base, 
		   double x, double y, double z,
		   double z_min, double z_max);
double Neuroseg_Dist_Field(double coef, double base, 
			   double x, double y, double z,
			   double z_min, double z_max);

inline static double Neurofield_S1(double x, double y)
{
  double t = x * x + y * y;

  double value = (1 - t) * exp(-t);

  if (value < 0.0) {
    value *= 2.0;
  }

  return value;
}

inline static double Neurofield_S2(double x, double y)
{
  double t = x * x + y * y;
  return exp(-t) - exp(-1.0);
}

inline static double Neurofield_S3(double x, double y)
{
  double t = x * x + y * y;
  
  if (t > 1.0) {
    return -1.0;
  } else {
    return 1.0;
  }
}

/* for testing */
inline static double Neurofield_T(double x, double y)
{
  double t = x * x + y * y;
  
  if (t > 1.0) {
    return 1.0;
  } else {
    return 1.0;
  }
}

/* For special purpose */
#define Neurofield_Rpi Neurofield_T

__END_DECLS

#endif
