/* tz_coordinate_3d.c
 * 
 * 02-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tz_geo3d_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_error.h"

void Print_Coordinate_3d(const coordinate_3d_t coord)
{
  printf("%g, %g, %g\n", coord[0], coord[1], coord[2]);
}

void Set_Coordinate_3d(coordinate_3d_t coord, double x, double y, double z)
{
  coord[0] = x;
  coord[1] = y;
  coord[2] = z;
}

void Coordinate_3d_Copy(coordinate_3d_t dst, const coordinate_3d_t res)
{
  memcpy(dst, res, sizeof(coordinate_3d_t));
}

double* Coordinate_3d_Double_Array(coordinate_3d_t *coord)
{
  return coord[0];
}


void Coordinate_3d_Add(const coordinate_3d_t coord1, 
		       const coordinate_3d_t coord2, 
		       coordinate_3d_t coord3)
{
  coord3[0] = coord1[0] + coord2[0];
  coord3[1] = coord1[1] + coord2[1];
  coord3[2] = coord1[2] + coord2[2];
}

void Coordinate_3d_Sub(const coordinate_3d_t coord1, 
		       const coordinate_3d_t coord2, 
		       coordinate_3d_t coord3)
{
  coord3[0] = coord1[0] - coord2[0];
  coord3[1] = coord1[1] - coord2[1];
  coord3[2] = coord1[2] - coord2[2];  
}

void Coordinate_3d_Neg(const coordinate_3d_t coord1, 
		       coordinate_3d_t coord2)
{
  coord2[0] = -coord1[0];
  coord2[1] = -coord1[1];
  coord2[2] = -coord1[2];
}

double Coordinate_3d_Norm(const coordinate_3d_t coord)
{
  return sqrt(coord[0] * coord[0] + coord[1] * coord[1] + coord[2] * coord[2]);
}

void Coordinate_3d_Unitize(coordinate_3d_t coord)
{
  double norm = Coordinate_3d_Norm(coord);
  if (norm != 0.0) {
    coord[0] /= norm;
    coord[1] /= norm;
    coord[2] /= norm;
  }
}

double Coordinate_3d_Distance(const coordinate_3d_t coord1, 
			      const coordinate_3d_t coord2)
{
  coordinate_3d_t offset;
  Geo3d_Coordinate_Offset(coord1[0], coord1[1], coord1[2],
			  coord2[0], coord2[1], coord2[2],
			  offset, offset + 1, offset + 2);

  return Coordinate_3d_Norm(offset);
}

void Coordinate_3d_Scale(coordinate_3d_t coord, double scale)
{
  coord[0] *= scale;
  coord[1] *= scale;
  coord[2] *= scale;
}

void Coordinate_3d_Cross(const coordinate_3d_t coord1, 
			 const coordinate_3d_t coord2,
			 coordinate_3d_t result)
{
  Geo3d_Cross_Product(coord1[0], coord1[1], coord1[2],
		      coord2[0], coord2[1], coord2[2],
		      result, result + 1, result + 2);
}

double Coordinate_3d_Dot(const coordinate_3d_t coord1,
			 const coordinate_3d_t coord2)
{
  return coord1[0] * coord2[0] + coord1[1] * coord2[1] + coord1[2] * coord2[2];
}

double Coordinate_3d_Normalized_Dot(const coordinate_3d_t coord1,
				    const coordinate_3d_t coord2)
{
  double d1 = Coordinate_3d_Length_Square(coord1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Coordinate_3d_Length_Square(coord2);
  if (d2 == 0.0) {
    return 0.0;
  }

  return Coordinate_3d_Dot(coord1, coord2) / sqrt(d1 * d2);
}

double Coordinate_3d_Length_Square(const coordinate_3d_t coord)
{
  return coord[0] * coord[0] + coord[1] * coord[1] + coord[2] * coord[2];
}

double Coordinate_3d_Angle2(const coordinate_3d_t coord1, 
			    const coordinate_3d_t coord2)
{
  double d1 = Coordinate_3d_Length_Square(coord1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Coordinate_3d_Length_Square(coord2);
  if (d2 == 0.0) {
    return 0.0;
  }

  double d12 = Coordinate_3d_Dot(coord1, coord2) / sqrt(d1 * d2);
  
  ASSERT(fabs(round(d12)) <= 1.0, "Invalid dot product.");

  /* to avoid invalid value caused by rounding error */
  d12 = (fabs(d12) > 1.0) ? round(d12) : d12;
  
  return acos(d12);
}

double Coordinate_3d_Cos3(const coordinate_3d_t coord1, 
			  const coordinate_3d_t coord2,
			  const coordinate_3d_t coord3)
{
  coordinate_3d_t vec1;
  coordinate_3d_t vec2;

  Coordinate_3d_Sub((double*) coord1, (double*) coord2, vec1);
  Coordinate_3d_Sub((double*) coord2, (double*) coord3, vec2);
  
  double d1 = Coordinate_3d_Length_Square(vec1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Coordinate_3d_Length_Square(vec2);
  if (d2 == 0.0) {
    return 0.0;
  }

  return Coordinate_3d_Dot(vec1, vec2) / sqrt(d1 * d2);
}

double Coordinate_3d_Cos4(const coordinate_3d_t coord1, 
			  const coordinate_3d_t coord2,
			  const coordinate_3d_t coord3,
			  const coordinate_3d_t coord4)
{
  coordinate_3d_t vec1;
  coordinate_3d_t vec2;

  Coordinate_3d_Sub((double*) coord1, (double*) coord2, vec1);
  Coordinate_3d_Sub((double*) coord3, (double*) coord4, vec2);
  
  double d1 = Coordinate_3d_Length_Square(vec1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Coordinate_3d_Length_Square(vec2);
  if (d2 == 0.0) {
    return 0.0;
  }

  return Coordinate_3d_Dot(vec1, vec2) / sqrt(d1 * d2);  
}
