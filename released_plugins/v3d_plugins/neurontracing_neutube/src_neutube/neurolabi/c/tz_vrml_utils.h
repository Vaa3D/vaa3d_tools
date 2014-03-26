/** @file tz_vrml_utils.h
 *  @brief Vrml utilities.
 *  @author Ting Zhao 
 *  @date 06-May-2008
 */

#ifndef _TZ_VRML_UTILS_H_
#define _TZ_VRML_UTILS_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/** @brief Minimal scale for VRML system
 */
#define VRML_MIN_SCALE 1e-10 

/** @brief Calculates vector orientation for VRML system.
 *
 *  Vrml_Vector_Orientation() calculates the XZ orienation for the vector 
 *  (\a x, \a y, \a z) and stores the result in (* \a theta, * \a psi).
 *  For vrml system, theta is around Z and psi is aournd Y (from z to x).
 */
void Vrml_Vector_Orientation(double x, double y, double z, 
			     double *theta, double *psi);


__END_DECLS

#endif
