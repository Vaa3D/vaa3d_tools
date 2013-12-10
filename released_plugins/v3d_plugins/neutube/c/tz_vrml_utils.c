/* tz_vrml_utils.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_geo3d_utils.h"
#include "tz_vrml_utils.h"


/**
 *  @param x x coordinate
 *  @param y y coordinate
 *  @param z z coordinate
 *  @param theta pointer to store resulted theta
 *  @param psi pointer to store resulted psi
 *  @return no return
 *  @note null
 */
void Vrml_Vector_Orientation(double x, double y, double z, 
			     double *theta, double *psi)
{
  double length = sqrt(x * x + y * y + z * z);
  if (length < VRML_MIN_SCALE) {
    *theta = 0;
    *psi = 0;
  } else {
    x /= length;
    y /= length;
    z /= length;
    Geo3d_Normal_Orientation(z, x, y, theta, psi);
  }
}
