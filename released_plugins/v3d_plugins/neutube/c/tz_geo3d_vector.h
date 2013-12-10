/**@file tz_geo3d_vector.h
 * @brief 3d vector
 * @author Ting Zhao
 * @date 02-Jan-2008
 */

#ifndef _TZ_GEO3D_VECTOR_H_
#define _TZ_GEO3D_VECTOR_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/* No construction is required for Geo3d_Vector */

typedef struct tagGeo3d_Vector {
  double x;
  double y;
  double z;
} Geo3d_Vector;

typedef Geo3d_Vector* Geo3d_Vector_P;

/*
 * Set_Geo3d_Vector() initializes a 3D vector by the coordinate (<x>, <y>, <z>).
 */
void Set_Geo3d_Vector(Geo3d_Vector *v, double x, double y, double z);

/*
 * Copy_Geo3d_Vector() copies the vector <v2> to the vector <v1>.
 */
void Copy_Geo3d_Vector(Geo3d_Vector *v1, const Geo3d_Vector *v2);

/*
 * Geo3d_Vector_Length_Square() returns the square of the length of the vector
 * <v>. Geo3d_Vector_Length() returns the length of the vector <v>.
 */
double Geo3d_Vector_Length_Square(const Geo3d_Vector *v);
double Geo3d_Vector_Length(const Geo3d_Vector *v);

/*
 * Geo3d_Vector_Orientation() calculates the orientation of a vector. The
 * orientation is represented by <theta> and <psi>, which are angles of 
 * rotation from the vector (1, 0, 0) to <v>. The unit is radian.
 */
void Geo3d_Vector_Orientation(const Geo3d_Vector *v, 
			      double *theta, double *psi);

/*
 * Geo3d_Vector_Normalize() normalizes a vector to length 1. A zero vector
 * will be remained as itself.
 */
void Geo3d_Vector_Normalize(Geo3d_Vector *v);

/*
 * Geo3d_Vector_Distance() returns the distance between the vectors <v1> and 
 * <v2>.
 */
double Geo3d_Vector_Distance(const Geo3d_Vector *v1, const Geo3d_Vector *v2);

/*
 * Print the vector <v> to the screen.
 */
void Print_Geo3d_Vector(const Geo3d_Vector *v);

/*
 * Geo3d_Vector_Dot() calculates the dot product between v1 and v2.
 */
double Geo3d_Vector_Dot(const Geo3d_Vector *v1, const Geo3d_Vector *v2);

/*
 * Geo3d_Vector_Angle2() returns the angle between v1 and v2. The unit of the
 * returned value is radian and the range is [0, PI]. It returns 0 if one of
 * the vectors is a zero vector.
 */
double Geo3d_Vector_Angle2(const Geo3d_Vector *v1, const Geo3d_Vector *v2);

__END_DECLS

#endif
