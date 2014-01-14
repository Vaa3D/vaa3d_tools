/**@file tz_geo3d_utils.h
 * @brief Utilities for 3d geometry
 * @author Ting Zhao
 * @date 17-Jan-2008
 */

#ifndef _TZ_GEO3D_UTILS_H_
#define _TZ_GEO3D_UTILS_H_ 

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup 3dgeom_
 * @{
 */

/**@addtogroup geo3d_util_ Some useful utilities (tz_geo3d_utils.h)
 * @{
 */

/**@brief Point translation.
 *
 * Geo3d_Translate_Coordinate() translates the coordinate (<x>, <y>, <z>) by
 * (<dx>, <dy>, <dz>). Geo3d_Coordinate_Offset() calculates the translation
 * offset between two coordinates (<x1>, <y1>, <z1>) and (<x2>, <y2>, <z2>) and
 * store the result in <dx>, <dy> and <dz>.
 */
void Geo3d_Translate_Coordinate(double *x, double *y, double *z, 
				 double dx, double dy, double dz);

/**@brief Points offset.
 *
 * Geo3d_Coordinate_Offset() calculates the offset from (<x1>, <y1>, <z1>) to
 * (<x2>, <y2>, <z2>) and stores the result in (<dx>, <dy>, <dz>).
 */
void Geo3d_Coordinate_Offset(double x1, double y1, double z1,
			      double x2, double y2, double z2,
			      double *dx, double *dy, double *dz);

/**@brief Point rotation.
 *
 * Geo3d_Rotate_Coordinate() rotates the coordinate (<x>, <y>, <z>) by the
 * angle <theta> around X axis and the angle <psi> around Z axis if <reverse>
 * is FALSE. The unit is radian. If <reverse> is TRUE, it reverse the rotation.
 */
void Geo3d_Rotate_Coordinate(double *x, double *y, double *z,
			     double theta, double psi, BOOL reverse);
			     
/**@brief Orientation to a vector.
 *
 * Geo3d_Orientation_Normal() turns an orientation to a normal vector. <theta>
 * is the radian around X axis and <psi> is the radian around Z axis. The
 * result is stored in <x>, <y> and <z>.
 */
void Geo3d_Orientation_Normal(double theta, double psi, 
			      double *x, double *y, double *z);

/**@brief Orientation of a normal vector.
 *
 * Geo3d_Normal_Orientation() turns a normal vector to an orientation. The
 * result is stored in <theta> and <psi>.
 */
void Geo3d_Normal_Orientation(double x, double y, double z, 
			      double *theta, double *psi);

/**@brief Orientation of a vector.
 *
 * Geo3d_Coord_Orientation() turns calculates the orientaion of the vector
 * (<x>, <y>, <z>) and stores the result in <theta> and <psi>.
 */
void Geo3d_Coord_Orientation(double x, double y, double z, 
			      double *theta, double *psi);

/**@brief Orientation rotation.
 *
 * Geo3d_Rotate_Orientation() rotates the orientation (<theta>, <psi>) by
 * <rtheta> and <rpsi>.
 */
void Geo3d_Rotate_Orientation(double rtheta, double rpsi, 
			      double *theta, double *psi);

/**@brief Dot product.
 *
 * Geo3d_Dot_Product() returns the dot product of (<x1>, <y1>, <z1>) and
 * (<x2>, <y2>, <z2>)
 */
double Geo3d_Dot_Product(double x1, double y1, double z1,
			 double x2, double y2, double z2);

/**@brief Cross product.
 *
 * Geo3d_Cross_Product() calculates (<x1>, <y1>, <z1>) x (<x2>, <y2>, <z2>) and
 * stores the result in (<x>, <y>, <z>).
 */
void Geo3d_Cross_Product(double x1, double y1, double z1,
			 double x2, double y2, double z2,
			 double *x, double *y, double *z);

/**@brief Square distance to the origin.
 */
double Geo3d_Orgdist_Sqr(double x, double y, double z);

/**@brief Distance to the origin.
 */
double Geo3d_Orgdist(double x, double y, double z);

/**@brief Distance square between two points.
 */
double Geo3d_Dist_Sqr(double x1, double y1, double z1,
		      double x2, double y2, double z2);

/**@brief Distance between two points.
 */
double Geo3d_Dist(double x1, double y1, double z1,
		  double x2, double y2, double z2);

/**@brief Angle between two vectors.
 *
 * Geo3d_Angled2() returns the angle between (<x1>, <y1>, <z1>) and
 * (<x2>, <y2>, <z2>). The unit is radian. The result is in [0, PI].
 */
double Geo3d_Angle2(double x1, double y1, double z1,
		    double x2, double y2, double z2);

/**@brief Distance between two lines */
double Geo3d_Line_Line_Dist(double line1_start[], double line1_end[],
			    double line2_start[], double line2_end[]);

/* obsolete */
double Geo3d_Lineseg_Dist2(double line1_start[], double line1_end[],
			   double line2_start[], double line2_end[],
			   double *intersect1, double *intersect2, int *cond);

/**@brief Distance between two line segments. 
 * 
 * Geo3d_Lineseg_Lineseg_Dist() return the distances between two lines that 
 * have  length greater than 0, i.e. neither of them collapses into one point. 
 * The relation between the two lines is also examined and stored in \a cond:
 *   0: i-i; 1: i-e; 2: e-i; 3: i-s; 4: e-s; 5: min(s-i, i-s); 
 *   6: min(e-i, i-s); 7: min(s-i, i-e); 8: min(e-i, i-e); 9: parallel
 *
 * It returns a negative number if the calculation is failed.
 */
double Geo3d_Lineseg_Lineseg_Dist(double line1_start[], double line1_end[],
				  double line2_start[], double line2_end[],
				  double *intersect1, double *intersect2, 
				  int *cond);

/**@brief Distance between a point and a line segment.
 *
 * Geo3d_Point_Lineseg_Dist() returns the distance between \a point and 
 * the line segment from \a line_start to \a line_end. \a lamda indicates the
 * position of the closest point on the line segment and its value belongs 
 * to [0, 1]. You can just pass NULL if you do not need \a lamda.
 */
double Geo3d_Point_Lineseg_Dist(const double *point, const double *line_start, 
				const double *line_end, double *lamda);

/**@brief Break point of a line segment.
 *
 * Geo3d_Lineseg_Break() calculates the break point of the line segment 
 * (<line_start>, <line_end>) at <lambda> and stores the result in <point>.
 */
void Geo3d_Lineseg_Break(const double *line_start, const double *line_end,
			 double lambda, double *point);

/**@brief Orientation of a covariance matrix.
 */
void Geo3d_Cov_Orientation(double *cov, double *vec, double *ext);

/**@}*/
/**@}*/

__END_DECLS

#endif
