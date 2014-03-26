/* tz_xz_orientation.c
 *
 * 19-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_geo3d_utils.h"
#include "tz_xz_orientation.h"

void Print_Xz_Orientation(const xz_orientation_t ort)
{
  printf("(%g, %g)", ort[0], ort[1]);
}

void Set_Xz_Orientation(xz_orientation_t ort, double theta, double psi)
{
  ort[0] = theta;
  ort[1] = psi;
}

double Xz_Orientation_Dot(const xz_orientation_t ort1, 
			  const xz_orientation_t ort2)
{
  coordinate_3d_t normal1;
  coordinate_3d_t normal2;
  Geo3d_Orientation_Normal(ort1[0], ort1[1], normal1, normal1 + 1, normal1 + 2);
  Geo3d_Orientation_Normal(ort2[0], ort2[1], normal2, normal2 + 1, normal2 + 2);
  //Print_Coordinate_3d(normal1);
  //Print_Coordinate_3d(normal2);

  return Coordinate_3d_Dot(normal1, normal2);
}


void Xz_Orientation_Cross(const xz_orientation_t ort1, 
			  const xz_orientation_t ort2,
			  coordinate_3d_t coord)
{
  coordinate_3d_t normal1;
  coordinate_3d_t normal2;
  Geo3d_Orientation_Normal(ort1[0], ort1[1], normal1, normal1 + 1, normal1 + 2);
  Geo3d_Orientation_Normal(ort2[0], ort2[1], normal2, normal2 + 1, normal2 + 2);
  Coordinate_3d_Cross(normal1, normal2, coord);
}
