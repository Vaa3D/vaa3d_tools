/**@file tz_3dgeom.h
 * @brief 3d transformation
 * @author Ting Zhao
 * @date 31-Oct-2007
 */

#ifndef _TZ_3DGEOM_H_
#define _TZ_3DGEOM_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup 3dgeom_
 * @{
 */

/**@addtogroup transform_3d_ 3d transformation (tz_3dgeom.h)
 *
 * @{
 */

/**@brief 3d linear transformation.
 *
 * Transform_3d() applies linear transformation on 3D points. The 
 * transformation is defined by <t>, which is an array with  9 elements 
 * corresponding to a 3x3 matrix:
 \verbatim
   t[0] t[1] t[2]
   t[3] t[4] t[5]
   t[6] t[7] t[8]
 \endverbatim
 * <in> provides points to transform. Its every 3 elements represent the 
 * coordinates of a 3D point. So the kth point has coordinates 
 * (in[3*k], in[3*k+1], in[3*k+2]), where k should be a non-negative number 
 * and less than the number of points <n>.
 * The results will be stored in the array <out>, which has the same alignment
 * as <in>.
 * If <out> is the same pointer as <in>, it does in-place transformation. If
 * <out> is NULL, a new pointer for the result will be returned; Otherwise the
 * returned pointer is the same is <out>.
 */
double* Transform_3d(double *in, double *out, int n, double *t);

/**@brief 3d rotation around X and then Z.
 *
 * Rotate_XZ() rotates 3D points. <in>, <out> and <n> have the same meaning as
 * those in Transform_3d(). <theta> is the angle around X axis and <psi> is
 * the angle around Z axis. See the M documents for more details about the
 * rotation. The function could reverse the rotation if <inverse> is not 0.
 * For example, Rotate_XZ(in, in, n, theta psi, 0) and then
 * Rotate_XZ(in, in, n, theta psi, 1) will have no effects on <in> if there are
 * no computing rounding errors.
 * <in> and <out> could be the same pointer for in-place transformation.
 */
double* Rotate_XZ(double *in, double *out, int n, double theta, double psi, 
		  int inverse);

/**@brief 3d rotation around Z.
 *
 * Rotate_Z() rotates points around Z with the angle <alpha>. 
 */
double* Rotate_Z(double *in, double *out, int n, double alpha, int inverse);

/**@brief Scale XY dimensions.
 *
 */
double* Scale_XY(double *in, double *out, int n, double rx, double ry,
		 int inverse);


/**@brief Scale X and rotate around Z.
 */
double* Scale_X_Rotate_Z(double *in, double *out, int n, 
			 double s, double alpha, int inverse);

/**@}*/

/**@}*/


__END_DECLS

#endif
