/* tz_geo3d_vector.c
 *
 * 02-Jan-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tz_error.h"
#include "tz_geometry.h"
#include "tz_geo3d_utils.h"
#include "tz_geo3d_vector.h"

void Set_Geo3d_Vector(Geo3d_Vector *v, double x, double y, double z)
{
  v->x = x;
  v->y = y;
  v->z = z;
}

void Copy_Geo3d_Vector(Geo3d_Vector *v1, const Geo3d_Vector *v2)
{
  if (v1 != v2) {
    memcpy(v1, v2, sizeof(*v1));
  }
}

double Geo3d_Vector_Length_Square(const Geo3d_Vector *v)
{
  return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

double Geo3d_Vector_Length(const Geo3d_Vector *v)
{
  return sqrt(Geo3d_Vector_Length_Square(v));
}

void Geo3d_Vector_Orientation(const Geo3d_Vector *v, 
			      double *theta, double *psi)
{
  double length = Geo3d_Vector_Length_Square(v);
  if (length == 0.0) {
    *theta = 0.0;
    *psi = 0.0;
    return;
  }

  length = sqrt(length);

  double x = v->x / length;
  double y = v->y / length;
  double z = v->z / length;
  
  if (z > 1.0) {
    z = 1.0;
  }

  Geo3d_Normal_Orientation(x, y, z, theta, psi);
}

void Geo3d_Vector_Normalize(Geo3d_Vector *v)
{
  double length = Geo3d_Vector_Length(v);
  if (length != 0.0) {
    v->x /= length;
    v->y /= length;
    v->z /= length;
  }
}

void Print_Geo3d_Vector(const Geo3d_Vector *v)
{
  printf("(%g, %g, %g)\n", v->x, v->z, v->z);
}

double Geo3d_Vector_Distance(const Geo3d_Vector *v1, const Geo3d_Vector *v2)
{
  double dx = v1->x - v2->x;
  double dy = v1->y - v2->y;
  double dz = v1->z - v2->z;

  return sqrt(dx * dx + dy * dy + dz * dz);
}

/* Geo3d_Vector_Dot(): The dot product between v1 and v2.
 * 
 * Args: v1 - the first vector;
 *       v2 - the second vector.
 *
 * Return: the dot product.
 */
double Geo3d_Vector_Dot(const Geo3d_Vector *v1, const Geo3d_Vector *v2)
{
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

/* Geo3d_Vector_Angle2(): The angle between two vectors.
 *
 * Args: v1 - the first vector;
 *       v2 - the second vector;
 *
 * Return: the dot product.
 *
 * Note: the angle between a zero vector and any vector is 0.
 */
double Geo3d_Vector_Angle2(const Geo3d_Vector *v1, const Geo3d_Vector *v2)
{
  double d1 = Geo3d_Vector_Length_Square(v1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Geo3d_Vector_Length_Square(v2);
  if (d2 == 0.0) {
    return 0.0;
  }

  double d12 = Geo3d_Vector_Dot(v1, v2) / sqrt(d1 * d2);
  
  ASSERT(fabs(round(d12)) <= 1.0, "Invalid dot product.");

  /* to avoid invalid value caused by rounding error */
  d12 = (fabs(d12) > 1.0) ? round(d12) : d12;
  
  return acos(d12);
}

