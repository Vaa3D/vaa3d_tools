/**@file tz_geoangle_utils.h
 * @brief utilities for angle
 * @author Ting Zhao
 * @date 25-Jan-2008
 */

#ifndef _TZ_GEOANGLE_UTILS_H_
#define _TZ_GEOANGLE_UTILS_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#define GEOANGLE_COMPARE_EPS 0.00001

/**@brief Normalize a radian.
 *
 * Normalize_Radian() normalizes a radian to [0, 2PI);
 */
double Normalize_Radian(double r);

/**@brief Normalize radian difference.
 *
 * Radian_Normdiff() calculates normalized difference between two radians,
 * which is between [0, PI].
 */
double Radian_Normdiff(double r1, double r2);

__END_DECLS

#endif 
