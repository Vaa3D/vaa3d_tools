/* tz_3dgeom_utils.c
 *
 * 17-Jan-2008 Initial write: Ting Zhao
 */
#include <stdio.h>
#include <math.h>
#include "tz_geo3d_utils.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_constant.h"
#include "tz_geoangle_utils.h"
#include "tz_geometry.h"
#include "tz_3dgeom.h"
#include "tz_darray.h"
#include "tz_math.h"

#define TZ_DIST_3D_EPS 0.0000000000001 /* rounding torelance */

/* Geo3d_Translate_Coordinate(): 3D coordinate translation.
 * 
 * Args: x - x coordinate to translate. It stores the result after return;
 *       y - y coordinate to translate. It stores the result after return;
 *       z - z coordinate to translate. It stores the result after return;
 *       dx - x axis translation;
 *       dy - y axis translation;
 *       dz - z axis translation.
 *
 * Return: void.
 */
void Geo3d_Translate_Coordinate(double *x, double *y, double *z, 
				double dx, double dy, double dz)
{
  *x += dx;
  *y += dy;
  *z += dz;
}

/* Geo3d_Coordinate_Offset(): 3D coordinates offset.
 * 
 * Args: x1, y1, z1 - start coordinate;
 *       x2, y2, z2 - end coordinate;
 *       dx, dy, dz - store the result.
 *
 * Return: void.
 */
void Geo3d_Coordinate_Offset(double x1, double y1, double z1,
			     double x2, double y2, double z2,
			     double *dx, double *dy, double *dz)
{
  *dx = x2 - x1;
  *dy = y2 - y1;
  *dz = z2 - z1;
}

/*
 * Geo3d_Rotate_Coordinate(): 3D  rotation.
 * 
 * Args: x, y, z - the point to be rotated;
 *       theta - angle around the X axis;
 *       psi - angle around the Z axis;
 *       reverse - reverse the rotation or not.
 *
 * Return: void.
 */
void Geo3d_Rotate_Coordinate(double *x, double *y, double *z,
			     double theta, double psi, BOOL reverse)
{
  double p[3] = {*x, *y, *z};
  Rotate_XZ(p, p, 1, theta, psi, reverse);
  *x = p[0];
  *y = p[1];
  *z = p[2];
}

/* Geo3d_Orientation_Normal(): calculate the normal vector at a certain 
 *                             direction.
 *
 * Args: theta - radian around X axis;
 *       psi - radian around Z axis;
 *       x - resulted x coordinate;
 *       y - resulted y coordinate;
 *       z - resulted z coordinate.
 *
 * Return: void.
 */
void Geo3d_Orientation_Normal(double theta, double psi, 
			      double *x, double *y, double *z)
{
  /* rotaion of the vector (0, 0, 1) */
  double sin_theta = sin(theta);
  *x = sin_theta * sin(psi);
  *y = -sin_theta * cos(psi);
  //*z = cos(theta);
  *z = sqrt(1.0 - sin_theta * sin_theta);
}

void Geo3d_Normal_Orientation(double x, double y, double z, 
			      double *theta, double *psi)
{
  /*
  *theta = acos(z);
  *psi = Vector_Angle(x, y) + TZ_PI_2;  
  */
  //*psi = Vector_Angle(x, y) + TZ_PI_2 * 3.0;  
  
  /* The range of theta will be [-pi, pi] and 
   * the range of psi will be [-pi/2, pi/2]. */
  *theta = acos(z);

  if (*theta < GEOANGLE_COMPARE_EPS) {
    *psi = 0.0;
  } else {
    if (y >= 0.0) {
      *theta = -*theta;
      *psi = Vector_Angle(x, y) - TZ_PI_2; 
    } else {
      *psi = Vector_Angle(x, y) - TZ_PI_2 * 3.0; 
    }
  }
}

void Geo3d_Coord_Orientation(double x, double y, double z, 
			      double *theta, double *psi)
{
  double r = Geo3d_Orgdist(x, y, z);
  if (r > TZ_DIST_3D_EPS) {
    x /= r;
    y /= r;
    z /= r;
    Geo3d_Normal_Orientation(x, y, z, theta, psi);
  } else {
    *theta = 0.0;
    *psi = 0.0;
  }
}

void Geo3d_Rotate_Orientation(double rtheta, double rpsi, 
			      double *theta, double *psi)
{
  double coord[3];
  Geo3d_Orientation_Normal(*theta, *psi, coord, coord + 1, coord + 2);
  Rotate_XZ(coord, coord, 1, rtheta, rpsi, 0);
  Geo3d_Normal_Orientation(coord[0], coord[1], coord[2], theta, psi);
}

double Geo3d_Dot_Product(double x1, double y1, double z1,
			 double x2, double y2, double z2)
{
  return x1 * x2 + y1 * y2 + z1 * z2;
}

void Geo3d_Cross_Product(double x1, double y1, double z1,
			 double x2, double y2, double z2,
			 double *x, double *y, double *z)
{
  *x = y1 * z2 - y2 * z1;
  *y = x2 * z1 - x1 * z2;
  *z = x1 * y2 - x2 * y1;
}

double Geo3d_Orgdist_Sqr(double x, double y, double z)
{
  return x * x + y * y + z * z;
}

double Geo3d_Orgdist(double x, double y, double z)
{
  return sqrt(x * x + y * y + z * z);
}

double Geo3d_Dist_Sqr(double x1, double y1, double z1,
		      double x2, double y2, double z2)
{
  double dx = x1 - x2;
  double dy = y1 - y2;
  double dz = z1 - z2;

  return dx * dx + dy * dy + dz * dz;
}

double Geo3d_Dist(double x1, double y1, double z1,
		  double x2, double y2, double z2)
{
  double dx = x1 - x2;
  double dy = y1 - y2;
  double dz = z1 - z2;

  return sqrt(dx * dx + dy * dy + dz * dz); 
}

double Geo3d_Angle2(double x1, double y1, double z1,
		    double x2, double y2, double z2)
{
  double d1 = Geo3d_Orgdist_Sqr(x1, y1, z1);
  if (d1 == 0.0) {
    return 0.0;
  }

  double d2 = Geo3d_Orgdist_Sqr(x2, y2, z2);
  if (d2 == 0.0) {
    return 0.0;
  }

  double d12 = Geo3d_Dot_Product(x1, y1, z1, x2, y2, z2) / sqrt(d1 * d2);
  
  /* to avoid invalid value caused by rounding error */
  d12 = (fabs(d12) > 1.0) ? round(d12) : d12;
  
  return acos(d12);  
} 

void Geo3d_Lineseg_Break(const double *line_start, const double *line_end,
			 double lambda, double *point)
{
  int i;
  for (i = 0; i < 3; i++) {
    point[i] = (1 - lambda) * line_start[i] + lambda * line_end[i];
  }
}

#define GEO3D_LINESEG_DIST2_EPS TZ_DIST_3D_EPS

/*
 * reference: http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
 */
static int geo3d_line_line_dist(const double *line1_start, 
				const double *line1_end,
				const double *line2_start, 
				const double *line2_end,
				double *dc1, double *dc2,
				double *intersect1, double *intersect2,
				double eps)
{
  double ds[3];  /*13*/
  double de[3];  /*24*/

  int i;
  for (i = 0; i < 3; i++) {
    dc1[i] = line1_end[i] - line1_start[i]; /*21*/
    dc2[i] = line2_end[i] - line2_start[i]; /*43*/
    ds[i] = line1_start[i] - line2_start[i];
    de[i] = line1_end[i] - line2_end[i];
  }

  double d2121 = dc1[0] * dc1[0] + dc1[1] * dc1[1] + dc1[2] * dc1[2];
  double d4343 = dc2[0] * dc2[0] + dc2[1] * dc2[1] + dc2[2] * dc2[2];

  if (d2121 < eps) {
    if (d4343 < eps) {
      return -3;
    }

    return -1;
  }

  if (d4343 < eps) {
    return -2;
  }
  
  double d4321 = dc2[0] * dc1[0] + dc2[1] * dc1[1] + dc2[2] * dc1[2];

  double denom = d2121 * d4343 - d4321 * d4321;

  double d1343 = ds[0] * dc2[0] + ds[1] * dc2[1] + ds[2] * dc2[2];
  double d1321 = ds[0] * dc1[0] + ds[1] * dc1[1] + ds[2] * dc1[2];

  double numer = d1343 * d4321 - d1321 * d4343;

  /* parallel */
  if (denom <= eps) {
    return 0;
  }

  *intersect1 = numer / denom;
  *intersect2 = (d1343 + d4321 * (*intersect1)) / d4343;

  return 1;
}


static int geo3d_point_line_dist(const double *point, 
				 const double *line_start,
				 const double *line_end, 			
				 double *lamda, double eps)
{
  double line_vec[3];
  double point_vec[3];
  int i;
  for (i = 0; i < 3; i++) {
    line_vec[i] = line_end[i] - line_start[i];
    point_vec[i] = point[i] - line_start[i];
  }

  double length_sqr = Geo3d_Orgdist_Sqr(line_vec[0], line_vec[1], line_vec[2]);

  if (length_sqr < eps) {
    return 0;
  }

  /* projection from point to line */
  double dot = point_vec[0] * line_vec[0] + point_vec[1] * line_vec[1] +
    point_vec[2] * line_vec[2];

  *lamda = dot / length_sqr;

  return 1;
} 


double Geo3d_Line_Line_Dist(double line1_start[], double line1_end[],
			    double line2_start[], double line2_end[])
{
  double intersect1, intersect2;
  double dc1[3], dc2[3];
  
  double lamda;

  switch (geo3d_line_line_dist(line1_start, line1_end, line2_start, line2_end,
			       dc1, dc2, &intersect1, &intersect2, 
			       TZ_DIST_3D_EPS)) {
  case -3: /* point to point */
    return Geo3d_Dist(line1_start[0], line1_start[1], line1_start[2],
		      line2_start[0], line2_start[1], line2_start[2]);
  case -1: /* point to line2 */
    geo3d_point_line_dist(line1_start, line2_start, line2_end, &lamda, 
			  TZ_DIST_3D_EPS);
    return Geo3d_Dist(line1_start[0], line1_start[1], line1_start[2],
		      line2_start[0] + lamda * dc2[0],
		      line2_start[1] + lamda * dc2[1],
		      line2_start[2] + lamda * dc2[2]);
  case -2: /* point to line 1 */
    geo3d_point_line_dist(line2_start, line1_start, line1_end, &lamda, 
			  TZ_DIST_3D_EPS);
    return Geo3d_Dist(line2_start[0], line2_start[1], line2_start[2],
		      line1_start[0] + lamda * dc1[0],
		      line1_start[1] + lamda * dc1[1],
		      line1_start[2] + lamda * dc1[2]);
  case 0: /* parallel */
    geo3d_point_line_dist(line1_start, line2_start, line2_end, &lamda, 
			  TZ_DIST_3D_EPS);
    return Geo3d_Dist(line1_start[0], line1_start[1], line1_start[2],
		      line2_start[0] + lamda * dc2[0],
		      line2_start[1] + lamda * dc2[1],
		      line2_start[2] + lamda * dc2[2]);
  default: /* line to line */
    return sqrt(Geo3d_Dist_Sqr(line1_start[0] + intersect1 * dc1[0],
			       line1_start[1] + intersect1 * dc1[1],
			       line1_start[2] + intersect1 * dc1[2],
			       line2_start[0] + intersect2 * dc2[0],
			       line2_start[1] + intersect2 * dc2[1],
			       line2_start[2] + intersect2 * dc2[2]));
  }
}

double Geo3d_Lineseg_Lineseg_Dist(double line1_start[], double line1_end[],
				  double line2_start[], double line2_end[],
				  double *intersect1, double *intersect2,
				  int *cond)
{
  double dc1[3]; /*21*/
  double dc2[3]; /*43*/
  
  double d1, d2;

  switch (geo3d_line_line_dist(line1_start, line1_end, line2_start, line2_end,
			       dc1, dc2, intersect1, intersect2, 
			       TZ_DIST_3D_EPS)) {
  case 0: /* parallel */
    *cond = 9;
    d1 = Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end, 
				  intersect1);
    d2 = Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				  intersect2);
    if (d1 <= d2) {
      *intersect2 = *intersect1;
      *intersect1 = 1.0;
      return d1;
    } else {
      *intersect1 = 0.0;
      return d2;
    }
  case -1:
    *intersect1 = 0.0;
    *cond = 10;
    return Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end, 
				    intersect2);
  case -2:
    *intersect2= 0.0;
    *cond = 10;
    return Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end, 
				    intersect1);
  case -3:
    *cond = 10;
    *intersect1 = 0.0;
    *intersect2 = 0.0;
    return Geo3d_Dist(line1_start[0], line1_start[1], line1_start[2],
		      line2_start[0], line2_start[1], line2_start[2]);
  default:
#if defined BREAK_IS_IN_RANGE
#  undef BREAK_IS_IN_RANGE
#endif
#define BREAK_IS_IN_RANGE(mu) ((mu >= 0.0) && (mu <= 1.0))

    if (BREAK_IS_IN_RANGE(*intersect1) && BREAK_IS_IN_RANGE(*intersect2)) {
      *cond = 0;
      return sqrt(Geo3d_Dist_Sqr(line1_start[0] + *intersect1 * dc1[0],
				 line1_start[1] + *intersect1 * dc1[1],
				 line1_start[2] + *intersect1 * dc1[2],
				 line2_start[0] + *intersect2 * dc2[0],
				 line2_start[1] + *intersect2 * dc2[1],
				 line2_start[2] + *intersect2 * dc2[2]));
    } else if ((*intersect1 < 0.0) && BREAK_IS_IN_RANGE(*intersect2)) {
      *cond = 1;
      *intersect1 = 0.0;
      return Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				      intersect2);
    } else if ((*intersect1 > 0.0) && BREAK_IS_IN_RANGE(*intersect2)) {
      *cond = 2;
      *intersect1 = 1.0;
      return Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				      intersect2);
    } else if ((*intersect2 < 0.0) && BREAK_IS_IN_RANGE(*intersect1)) {
      *cond = 3;
      *intersect2 = 0.0;
      return Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				      intersect1);
    } else if ((*intersect2 > 1.0) && BREAK_IS_IN_RANGE(*intersect1)) {
      *cond = 4;
      *intersect2 = 1.0;
      return Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				      intersect1);
    } else if ((*intersect1 < 0.0) && (*intersect2 < 0.0)) {
      *cond = 5;
      d1 = Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end, 
				    intersect2);
      d2 = Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				    intersect1);
      if (d1 <= d2) {
	*intersect1 = 0.0;
	return d1;
      } else {
	*intersect2 = 0.0;
	return d2;
      }
    } else if ((*intersect1 > 1.0) && (*intersect2 < 0.0)) {
      *cond = 6;
      d1 = Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				    intersect2);
      d2 = Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				    intersect1);
      if (d1 <= d2) {
	*intersect1 = 1.0;
	return d1;
      } else {
	*intersect2 = 0.0;
	return d2;
      }
    } else if ((*intersect1 < 0.0) && (*intersect2 > 1.0)) {
      *cond = 7;
      d1 = Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				    intersect2);
      d2 = Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				    intersect1);
      if (d1 <= d2) {
	*intersect1 = 0.0;
	return d1;
      } else {
	*intersect2 = 1.0;
	return d2;
      }
    } else if ((*intersect1 > 1.0) && (*intersect2 > 1.0)) {
      *cond = 8;
      d1 = Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				    intersect2);
      d2 = Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				    intersect1);
      if (d1<= d2) {
	*intersect1 = 1.0;
	return d1;
      } else {
	*intersect2 = 1.0;
	return d2;
      }
    }
  }

  return -1.0;
}


double Geo3d_Lineseg_Dist2(double line1_start[], double line1_end[],
			   double line2_start[], double line2_end[],
			   double *intersect1, double *intersect2,
			   int *cond)
{
  double dc1[3]; /*21*/
  double dc2[3]; /*43*/
  double ds[3];  /*13*/
  double de[3];  /*24*/

  int i;
  for (i = 0; i < 3; i++) {
    dc1[i] = line1_end[i] - line1_start[i];
    dc2[i] = line2_end[i] - line2_start[i];
    ds[i] = line1_start[i] - line2_start[i];
    de[i] = line1_end[i] - line2_end[i];
  }

  double d2121 = dc1[0] * dc1[0] + dc1[1] * dc1[1] + dc1[2] * dc1[2];
  double d4343 = dc2[0] * dc2[0] + dc2[1] * dc2[1] + dc2[2] * dc2[2];

  ASSERT(d2121 > GEO3D_LINESEG_DIST2_EPS, "invalid length");
  ASSERT(d4343 > GEO3D_LINESEG_DIST2_EPS, "invalid length");
  
  double d4321 = dc2[0] * dc1[0] + dc2[1] * dc1[1] + dc2[2] * dc1[2];

  double denom = d2121 * d4343 - d4321 * d4321;

  double d1343 = ds[0] * dc2[0] + ds[1] * dc2[1] + ds[2] * dc2[2];
  double d1321 = ds[0] * dc1[0] + ds[1] * dc1[1] + ds[2] * dc1[2];

  double numer = d1343 * d4321 - d1321 * d4343;

  if (denom <= GEO3D_LINESEG_DIST2_EPS) {
    *cond = 9;
    return 
      dmin2(Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end, 
				     intersect1),
	    Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				     intersect2)
	    );
  }

  *intersect1 = numer / denom;
  *intersect2 = (d1343 + d4321 * (*intersect1)) / d4343;

#if defined BREAK_IS_IN_RANGE
#  undef BREAK_IS_IN_RANGE
#endif
#define BREAK_IS_IN_RANGE(mu) ((mu >= 0.0) && (mu <= 1.0))


  if (BREAK_IS_IN_RANGE(*intersect1) && BREAK_IS_IN_RANGE(*intersect2)) {
    *cond = 0;
    return sqrt(Geo3d_Dist_Sqr(line1_start[0] + *intersect1 * dc1[0],
			       line1_start[1] + *intersect1 * dc1[1],
			       line1_start[2] + *intersect1 * dc1[2],
			       line2_start[0] + *intersect2 * dc2[0],
			       line2_start[1] + *intersect2 * dc2[1],
			       line2_start[2] + *intersect2 * dc2[2]));
  } else if ((*intersect1 < 0.0) && BREAK_IS_IN_RANGE(*intersect2)) {
    *cond = 1;
    return Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				    intersect2);
  } else if ((*intersect1 > 0.0) && BREAK_IS_IN_RANGE(*intersect2)) {
    *cond = 2;
    return Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				    intersect2);
  } else if ((*intersect2 < 0.0) && BREAK_IS_IN_RANGE(*intersect1)) {
    *cond = 3;
    return Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				    intersect1);
  } else if ((*intersect2 > 1.0) && BREAK_IS_IN_RANGE(*intersect1)) {
    *cond = 4;
    return Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				    intersect1);
  } else if ((*intersect1 < 0.0) && (*intersect2 < 0.0)) {
    *cond = 5;
    return 
      dmin2(Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end, 
				     intersect2),
	    Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				     intersect1)
	    );
  } else if ((*intersect1 > 1.0) && (*intersect2 < 0.0)) {
    *cond = 6;
    return 
      dmin2(Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				     intersect2),
	    Geo3d_Point_Lineseg_Dist(line2_start, line1_start, line1_end,
				     intersect1)
	    );
  } else if ((*intersect1 < 0.0) && (*intersect2 > 1.0)) {
    *cond = 7;
    return
      dmin2(Geo3d_Point_Lineseg_Dist(line1_start, line2_start, line2_end,
				     intersect2),
	    Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				     intersect1)
	    );
  } else if ((*intersect1 > 1.0) && (*intersect2 > 1.0)) {
    *cond = 8;
    return
      dmin2(Geo3d_Point_Lineseg_Dist(line1_end, line2_start, line2_end,
				     intersect2),
	    Geo3d_Point_Lineseg_Dist(line2_end, line1_start, line1_end,
				     intersect1)
	    );
  }

  return -1.0;
}

#define GEO3D_POINT_LINESEG_EPS 0.0000000000001
double Geo3d_Point_Lineseg_Dist(const double *point, const double *line_start, 
				const double *line_end, double *lamda)
{
  double tmp_lamda;
  int status = geo3d_point_line_dist(point, line_start, line_end, &tmp_lamda, 
				     GEO3D_POINT_LINESEG_EPS);
  
  double dist;
  if (status == 0) {
    dist = Geo3d_Dist(line_start[0], line_start[1], line_start[2],
		      point[0], point[1], point[2]);
  } else {
    if ((tmp_lamda >= 0.0) && (tmp_lamda <= 1.0)) {
      dist = 
	Geo3d_Dist(point[0], point[1], point[2],
		   (1.0 - tmp_lamda) * line_start[0] + tmp_lamda * line_end[0],
		   (1.0 - tmp_lamda) * line_start[1] + tmp_lamda * line_end[1],
		   (1.0 - tmp_lamda) * line_start[2] + tmp_lamda * line_end[2]);
    } else if (tmp_lamda < 0.0) {
      tmp_lamda = 0.0;
      dist = sqrt(Geo3d_Dist_Sqr(line_start[0], line_start[1], line_start[2],
				 point[0], point[1], point[2]));
    } else {
      tmp_lamda = 1.0;
      dist = sqrt(Geo3d_Dist_Sqr(line_end[0], line_end[1], line_end[2],
				 point[0], point[1], point[2]));
    }
  }

  /*
  double line_vec[3];
  double point_vec[3];
  int i;
  for (i = 0; i < 3; i++) {
    line_vec[i] = line_end[i] - line_start[i];
    point_vec[i] = point[i] - line_start[i];
  }
  double length_sqr = Geo3d_Orgdist_Sqr(line_vec[0], line_vec[1], line_vec[2]);

  double tmp_lamda = 0.0;
  double dist = -1.0;

  if (length_sqr <= GEO3D_POINT_LINESEG_EPS) {
    dist = sqrt(Geo3d_Dist_Sqr(line_start[0], line_start[1], line_start[2],
			       point[0], point[1], point[2]));
  } else {
    double dot = point_vec[0] * line_vec[0] + point_vec[1] * line_vec[1] +
      point_vec[2] * line_vec[2];

    double point_vec_lensqr = Geo3d_Orgdist_Sqr(point_vec[0], point_vec[1], 
					      point_vec[2]);
    
    if (point_vec_lensqr <= GEO3D_POINT_LINESEG_EPS) {
      dist = 0.0;
    } else {
      tmp_lamda = dot / length_sqr;
      if ((tmp_lamda >= 0.0) && (tmp_lamda <= 1.0)) {
	dist = sqrt(point_vec_lensqr - dot * dot / length_sqr);
      } else if (tmp_lamda < 0.0) {
	dist = sqrt(Geo3d_Dist_Sqr(line_start[0], line_start[1], line_start[2],
				   point[0], point[1], point[2]));
      } else {
	dist = sqrt(Geo3d_Dist_Sqr(line_end[0], line_end[1], line_end[2],
				   point[0], point[1], point[2]));
      }
    }
  }
  */
  if (lamda != NULL) {
    *lamda = tmp_lamda;
  }

  return dist;
}


void Geo3d_Cov_Orientation(double *cov, double *vec, double *ext)
{
#ifdef HAVE_LIBGSL
  gsl_matrix_view gmv = gsl_matrix_view_array(cov, 3, 3);
  
  gsl_vector *eval = gsl_vector_alloc(3);
  gsl_matrix *evec = gsl_matrix_alloc(3,3);
  gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(3);

  gsl_eigen_symmv(&(gmv.matrix), eval, evec, w);
  gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_DESC);

  if(ext != NULL) {
    darraycpy(ext, gsl_vector_const_ptr(eval, 0), 0, 3);
  }

  gsl_vector_view v = gsl_vector_view_array(vec, 3);
  gsl_matrix_get_col(&(v.vector), evec, 0);

  gsl_vector_free(eval);
  gsl_matrix_free(evec);
  gsl_eigen_symmv_free(w); 
#else
  double eigv[3];
  Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], eigv);
  Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
      eigv[0], vec);
  if(ext != NULL) {
    darraycpy(ext, eigv, 0, 3);
  }
#endif
}
