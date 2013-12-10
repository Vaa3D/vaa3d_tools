/**@file tz_coordinate_3d.h
 * @brief 3d coordinate
 * @author Ting Zhao
 * @date 02-Mar-2008 
 */

#ifndef _TZ_COORDINATE_3D_H_
#define _TZ_COORDINATE_3D_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup 3d_coordinate_ 3D coordinate (tz_coordinate_3d.h)
 * @{
 */

/**@brief Data type of 3D coordinates.
 * 
 * 3D coordinate data type. It can be accessed by array indices 0, 1, 2,
 * corresponding to x, y, z respectively. Each of its element has double 
 * precision, so it can be casted to a double pointer type.
 */
typedef double coordinate_3d_t[3];
typedef double *coordinate_3d_tp;

/**@brief Print coordinates of a 3D point.
 *
 * Print_Coordinate_3d() prints a point with coordinates \a coord to the screen.
 */
void Print_Coordinate_3d(const coordinate_3d_t coord);

/**@brief Set 3D coordinates.
 *
 * Set_Coordinate_3d() sets the coordinates of a point to (<x>, <y>, <z>).
 */
void Set_Coordinate_3d(coordinate_3d_t coord, double x, double y, double z);

/**@brief Copy coordinates.
 *
 * Coordinate_3d_Copy copies the values of <res> to <dst>.
 */
void Coordinate_3d_Copy(coordinate_3d_t dst, const coordinate_3d_t res);

/**@brief Cast coordinates to double array.
 *
 * Coordinate_3d_Double_Array() casts an array of coordinates to a double array.
 */
double* Coordinate_3d_Double_Array(coordinate_3d_t *coord);

/**@brief Add two coordinates.
 */
void Coordinate_3d_Add(const coordinate_3d_t coord1, 
		       const coordinate_3d_t coord2, 
		       coordinate_3d_t coord3);

/**@brief Subtract two coordinates.
 *
 * Coordinate_3d_Sub() subtracts <coord2> from <coord1>.
 */
void Coordinate_3d_Sub(const coordinate_3d_t coord1, 
		       const coordinate_3d_t coord2, 
		       coordinate_3d_t coord3);

/**@brief Reverses the signs of the coordinates.
 *
 * Coordinate_3d_Neg() reverses the signs of the coordinates.
 */
void Coordinate_3d_Neg(const coordinate_3d_t coord1, 
		       coordinate_3d_t coord2);

/*@brief Norm of a point.
 *
 * Coordinate_3d_Norm() returns the norm of <coord>.
 */
double Coordinate_3d_Norm(const coordinate_3d_t coord);

/*
 * Coordinate_3d_Unitize() scales a non-zero <coord> so that its norm is 1. 
 * It does not affect (0, 0, 0).
 */
void Coordinate_3d_Unitize(coordinate_3d_t coord);

/*
 * Coordinate_3d_Distance() calcualtes the distance between two coordinates
 * <coord1> and <coord2>.
 */
double Coordinate_3d_Distance(const coordinate_3d_t coord1, 
			      const coordinate_3d_t coord2);

/*
 * Coordinate_3d_Scale() scales <coord> by multiplying it with <scale>. 
 */
void Coordinate_3d_Scale(coordinate_3d_t coord, double scale);

/*
 * Coordinate_3d_Dot() returns the dot product of <coord1> and <coord2>.
 */
double Coordinate_3d_Dot(const coordinate_3d_t coord1,
			 const coordinate_3d_t coord2);

/*
 * Coordinate_3d_Cross() calculates the cross product of <coord1> and <coord2>.
 * The result is stored in <result>, which is not required to be initialized
 * before passing to the function.
 */
void Coordinate_3d_Cross(const coordinate_3d_t coord1, 
			 const coordinate_3d_t coord2,
			 coordinate_3d_t result);

/**@brief Normalized dot product.
 *
 * Coordinate_3d_Normalized_Dot() returns the dot product between the unitized
 * <coord1> and unitized <coord2>.
 */
double Coordinate_3d_Normalized_Dot(const coordinate_3d_t coord1,
				    const coordinate_3d_t coord2);

/**@brief Square length of a vector.
 * Coordinate_3d_Length_Square() returns the square length of the vector
 * corresponding to the coordinates <coord>.
 */
double Coordinate_3d_Length_Square(const coordinate_3d_t coord);

/**@brief The angle between two vectors.
 *
 * Coordinate_3d_Angle2() returns the angle between the two vectors defined by
 * <coord1> and <coord2>. The value is in the range [0, pi].
 */
double Coordinate_3d_Angle2(const coordinate_3d_t coord1, 
			    const coordinate_3d_t coord2);

/* cos(coord1-->coord2-->coord3) */
double Coordinate_3d_Cos3(const coordinate_3d_t coord1, 
			  const coordinate_3d_t coord2,
			  const coordinate_3d_t coord3);

/* cos(coord1-->coord2, coord3-->coord4) */
double Coordinate_3d_Cos4(const coordinate_3d_t coord1, 
			  const coordinate_3d_t coord2,
			  const coordinate_3d_t coord3,
			  const coordinate_3d_t coord4);

/**@}*/

__END_DECLS

#endif
