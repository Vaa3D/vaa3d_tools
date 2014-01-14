/**@file tz_geometry.h
 * @brief 2d geometry
 * @author Ting Zhao
 * @brief 20-Dec-2007
 */

#ifndef _TZ_GEOMETRY_H_
#define _TZ_GEOMETRY_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@brief Angle of a 2D vector.
 *
 * Vector_Angle() calculates the angle of the vector (<x>, <y>). The unit is
 * radian and the range is [0, 2*pi). It returns 0 for the origin.
 */
double Vector_Angle(double x, double y);

/**@brief Angle between two vectors.
 *
 * Vector_Angle2() returns the angle by which (<x1>, <y1>) rotates to 
 * (<x2>, <y2>). The rotation is counter-clockwise if counter_clockwise is TRUE,
 * otherwise it is clockwise.
 */
double Vector_Angle2(double x1, double y1, double x2, double y2, 
    BOOL counter_clockwise);

/**@brief Distance from a point to an ellipse. 
 * 
 * Ellipse_Point_Distance() returns the shortest distance between the point 
 * (<x>, <y>) and the ellipse, which is centered at the origin and has the 
 * major axis <a> and minor axis <b>. The closest point on the ellipse is 
 * in (<ex>, <ey>) if they are not NULL.
 */
double Ellipse_Point_Distance(double x, double y, double a, double b, 
			      double *ex, double *ey);

/**@brief Test if a point is in ellipse.
 *
 * Point_In_Ellipse() returns TRUE if (<x>, <y>) is in the ellipse
 *  $\frac{x^2}{a^2} + \frac{y^2}{b^2} = 1$.
 */
BOOL Point_In_Ellipse(double x, double y, double a, double b);

/**@brief Get uniformly distributed sphere point.
 *
 * n stores the number of points returned if it is not NULL.
 */
const double* Get_Uniform_Sphere_Point(int *n);

__END_DECLS

#endif
