/**@file tz_xz_orientation.h
 * @brief Xz orientation routines.
 * @author Ting Zhao
 * @date 19-Mar-2008
 */

#ifndef _TZ_XZ_ORIENTATION_H_
#define _TZ_XZ_ORIENTATION_H_

#include "tz_cdefs.h"
#include "tz_coordinate_3d.h"

__BEGIN_DECLS
/**@addtogroup 3dgeom_ 3D Geometry
 * @{
 */

/**@addtogroup xz_ort_ XZ orientation (tz_xz_orientation.h)
 * @{
 */

/**@typedef double xz_orientation_t[2]
 * 
 * @brief Xz orientation type.
 *
 * xz_orientation_t is a type for defining an orientation in the 3D coordinate
 * system.  It is represented by the rotation of the vector [0, 0, 1] around X 
 * and Z axes. Any instance with this type can be accessed by index 0 and 1, 
 * corresponding to the rotating angle around X axis and Z axis respectively. 
 * The rotations are in the counterclockwise direction and have the unit 
 * radian.
 * Each of the two entries has a double precision, so the instance can be 
 * casted to a double array.
 */
typedef double xz_orientation_t[2];

#define Xz_Orientation_t xz_orientation_t

/**@defgroup Print_Xz_Orientation XZ orientation I/O
 * @{
 */

/**@brief Prints an orientation.
 *
 * Print_Xz_Orientation() prints \a ort to the standard output stream.
 */
void Print_Xz_Orientation(const xz_orientation_t ort);

/*@}*/

/**@addtogroup Set_Xz_Orientation XZ orientation initialization
 * @{
 */
/** @brief Set an orientation.
 *
 *  Set_Xz_Orientation() sets <ort> by <theta>, the angle around X, and <psi>,
 *  the angle around Z. The unit is radian.
 */
void Set_Xz_Orientation(xz_orientation_t ort, double theta, double psi);
/*@}*/

/**@addtogroup Xz_Orientation_Dot */
/* @{ */
/** @brief Dot product.
 *
 *  Xz_Orientation_Dot() returns the dot product of two unit vectors, which are
 *  in the orientations <ort1> and <ort2> respecitvely.
 */
double Xz_Orientation_Dot(const xz_orientation_t ort1, 
			  const xz_orientation_t ort2);
/* @} */

/**@addtogroup Xz_Orientation_Dot */
/* @{ */
/**@brief Cross product.
 *
 * Xz_Orientation_Cross() calculates their cross products and store the result
 * in <coord>.
 */
void Xz_Orientation_Cross(const xz_orientation_t ort1, 
			  const xz_orientation_t ort2,
			  coordinate_3d_t coord);
/**@} */

/**@}*/
/**@}*/

__END_DECLS

#endif
