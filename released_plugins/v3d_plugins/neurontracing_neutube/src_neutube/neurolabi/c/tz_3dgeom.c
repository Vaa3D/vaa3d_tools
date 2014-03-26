/* tz_3dgeom.c
 *
 * 31-Oct-2007 Initial write: Ting Zhao
 */

#include <math.h>
#include <string.h>
#include "tz_darray.h"
#include "tz_3dgeom.h"
#include "tz_constant.h"
#include "tz_geoangle_utils.h"

static double Ar[9]; /* rotation matrix. row-major order*/

/* init_a_rxz(): build XZ rotation matrix
 *
 * Args: theta - angle about x
 *       psi - angle about z
 *       inverse - inverse rotation if it is not 0
 *
 * Return: rotation matrix
 */
static double* init_a_rxz(double theta, double psi, int inverse)
{
  if (inverse != 0) {
    Ar[0] = cos(psi);
    Ar[1] = sin(psi);
    Ar[2] = 0;
    Ar[5] = sin(theta);
    Ar[8] = cos(theta);
    Ar[3] = - Ar[8] * Ar[1];
    Ar[4] = Ar[8] * Ar[0];
    Ar[6] = Ar[5] * Ar[1];
    Ar[7] = -Ar[5] * Ar[0];
  } else {
    Ar[0] = cos(psi);
    Ar[3] = sin(psi);
    Ar[7] = sin(theta);
    Ar[8] = cos(theta);
    Ar[1] = - Ar[8] * Ar[3];
    Ar[2] = Ar[7] * Ar[3];
    Ar[4] = Ar[8] * Ar[0];
    Ar[5] = - Ar[7] * Ar[0];
    Ar[6] = 0;
  }

  return Ar;
}

/* Transform_3d(): 3d transformation
 *
 * Args: in - input point array of 3xn matrix
 *       out - array to restore transformed points
 *       n - number of points
 *       A - transformation matrix   
 *
 * Return: transformed array.    
 */
double* Transform_3d(double *in, double *out, int n, double *a)
{
  if (out == NULL) {
    out = darray_malloc(n * 3);
  }

  double result[3]; /* array to store temp result */
  int offset = 0;
  int i;
  
  for (i = 0; i < n; i++) {
    result[0] = a[0] * in[offset] + a[1] * in[offset + 1] + 
      a[2] * in[offset + 2];
    result[1] = a[3] * in[offset] + a[4] * in[offset + 1] + 
      a[5] * in[offset + 2];
    result[2] = a[6] * in[offset] + a[7] * in[offset + 1] + 
      a[8] * in[offset + 2];
    out[offset] = result[0];
    out[offset + 1] = result[1];
    out[offset + 2] = result[2];

    offset += 3;
  }

  return out;
}

/* Rotate_XZ(): 3D rotation about X and Z axes
 *
 * Args: in - input point array (3 x n numbers)
 *       out - output point array (should be as long as in to hold the results)
 *       n - number of points
 *       theta - angle about x
 *       psi - angle about z
 *
 * Return: the new point array.
 */
double* Rotate_XZ(double *in, double *out, int n, double theta, double psi, 
		  int inverse)
{ 
  /*
  if ((theta < 0.0) || (theta >= TZ_2PI)) {
    theta = theta - floor(theta / TZ_2PI) * 2.0 * TZ_PI;
  }
  if ((psi < 0.0) || (psi >= TZ_2PI)) {
    psi = psi - floor(psi / TZ_2PI) * 2.0 * TZ_PI;
  }

  Ar[0] = cos(theta);
  if ((Ar[0] >= 1.0) || (Ar[0] <= -1.0)) {
    Ar[1] = 1.0;
  } else {
    Ar[1] = sqrt(1.0 - Ar[0] * Ar[0]);
    if (theta > TZ_PI) {
      Ar[1] = -Ar[1];
    }
  }
  //printf("%g\n", Ar[1] - sin(theta));
  Ar[2] = cos(psi);
  if ((Ar[2] >= 1.0) || (Ar[2] <= -1.0)) {
    Ar[3] = 1.0;
  } else {
    Ar[3] = sqrt(1.0 - Ar[2] * Ar[2]);
    if (psi > TZ_PI) {
      Ar[3] = -Ar[3];
    }
  }
  //printf("%g\n", Ar[3] - sin(psi));
  */
  
  // use local variable to make this function reentrant
  double Ar[4];

  Ar[0] = cos(theta);
  Ar[1] = sin(theta);  
  Ar[2] = cos(psi);
  Ar[3] = sin(psi);
  
  int i;
  int offset = 0;
  double result[3];
  double *iny = in + 1;
  double *inz = in + 2;
  if (inverse == 0) {
    for (i = 0; i < n; i++) {
      result[2] = Ar[1] * iny[offset] + Ar[0] * inz[offset];  
      result[0] = inz[offset] * Ar[1] - iny[offset] * Ar[0];
      result[1] = in[offset] * Ar[3] - result[0] * Ar[2];
      result[0] = in[offset] * Ar[2] + result[0] * Ar[3];
      memcpy(out + offset, result, 24);
      offset += 3;
    }
  } else {
    for (i = 0; i < n; i++) {
      result[0] = Ar[2] * in[offset] + Ar[3] * iny[offset];
      result[1] = iny[offset] * Ar[2] - in[offset] * Ar[3];
      result[2] = inz[offset] * Ar[0] - result[1] * Ar[1];
      result[1] = inz[offset] * Ar[1] + result[1] * Ar[0];
      memcpy(out + offset, result, 24);
      offset += 3;
    }
  }
  
  return out;
}

/* for testing */
double* Rotate_XZ2(double *in, double *out, int n, double theta, double psi, 
		  int inverse)
{
  double *a = init_a_rxz(theta, psi, inverse);

  return Transform_3d(in, out, n, a);
}

double* Rotate_Z(double *in, double *out, int n, double alpha, int inverse)
{
  double *orgout = out;

  if (alpha != 0.0) {
    int i;
    double cos_a = cos(alpha);
    double sin_a = sin(alpha);
    
    if (inverse == 0) {
      for (i = 0; i < n; i++) {
	double tmp = in[0] * cos_a - in[1] * sin_a;
	out[1] = in[0] *sin_a + in[1] * cos_a;
	out[0] = tmp;
	out[2] = in[2];
	in += 3;
	out += 3;
      }
    } else { /* inverse transformation */
      for (i = 0; i < n; i++) {
	double tmp = in[0] * cos_a + in[1] * sin_a;
	out[1] = -in[0] * sin_a + in[1] * cos_a;
	out[0] = tmp;
	out[2] = in[2];
	in += 3;
	out += 3;
      }
    }
  } else {
    if (in != out) {
      out[0] = in[0];
      out[1] = in[1];
      out[2] = in[2];
    }
  }

  return orgout;  
}

double* Scale_XY(double *in, double *out, int n, double rx, double ry,
		 int inverse)
{
  int i;
  double *iny = in + 1;
  double *outy = out + 1;
  int offset = 0;
  if (inverse == 0) {
    for (i = 0; i < n; i++) {
      out[offset] = in[offset] * rx;
      outy[offset] *= iny[offset] * ry;
      offset += 3;
    }
  } else {
    for (i = 0; i < n; i++) {
      out[offset] = in[offset] / rx;
      outy[offset] *= iny[offset] / ry;
      offset += 3;
    }
  }

  return out;
}

double* Scale_X_Rotate_Z(double *in, double *out, int n, 
			 double s, double alpha, int inverse)
{
  int i;
  double *orgout = out;
  double cos_a = cos(alpha);
  double sin_a = sin(alpha);

  if (inverse == 0) {
    for (i = 0; i < n; i++) {
      double tmp = in[0] * s * cos_a - in[1] * sin_a;
      out[1] = in[0] * s *sin_a + in[1] * cos_a;
      out[0] = tmp;
      out[2] = in[2];
      in += 3;
      out += 3;
    }
  } else { /* inverse transformation */
    for (i = 0; i < n; i++) {
      double tmp = (in[0] * cos_a + in[1] * sin_a) / s;
      out[1] = -in[0] * sin_a + in[1] * cos_a;
      out[0] = tmp;
      out[2] = in[2];
      in += 3;
      out += 3;
    }
  }

  return orgout;
}

double* Scale_Y_Rotate_Z(double *in, double *out, int n, 
			 double s, double alpha, int inverse)
{
  int i;
  double *orgout = out;
  double cos_a = cos(alpha);
  double sin_a = sin(alpha);

  if (inverse == 0) {
    for (i = 0; i < n; i++) {
      double tmp = in[0] * cos_a - s * in[1] * sin_a;
      out[1] = in[0] *sin_a + s * in[1] * cos_a;
      out[0] = tmp;
      out[2] = in[2];
      in += 3;
      out += 3;
    }
  } else { /* inverse transformation */
    for (i = 0; i < n; i++) {
      double tmp = in[0] * cos_a + in[1] * sin_a;
      out[1] = (-in[0] * sin_a + in[1] * cos_a) / s;
      out[0] = tmp;
      out[2] = in[2];
      in += 3;
      out += 3;
    }
  }

  return orgout;
}
