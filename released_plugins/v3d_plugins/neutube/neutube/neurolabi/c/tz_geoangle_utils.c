/* tz_geoangle_utils.c
 *
 * 25-Jan-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_constant.h"
#include "tz_geoangle_utils.h"

/* Normalize_Radian(): Normalize a radian to [0, 2*pi).
 *
 * Depedence: tz_constant.h.
 */
double Normalize_Radian(double r)
{
  double norm_r = r;
  
  if ((r < 0.0) || (r >= TZ_2PI)) {
    norm_r = r - floor(r / TZ_2PI) * 2.0 * TZ_PI;
  }

  return norm_r;
}

/* Randian_Normdiff(): Normalized difference between two radians.
 *
 * Dependence: tz_constant.h.
 */
double Radian_Normdiff(double r1, double r2)
{
  double diff = fabs(Normalize_Radian(r1) - Normalize_Radian(r2));

  if (diff > TZ_PI) {
    diff = TZ_2PI - diff;
  }

  return diff;
}
