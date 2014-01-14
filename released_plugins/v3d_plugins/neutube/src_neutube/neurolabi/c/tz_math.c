#include <math.h>
#include "tz_math.h"
#include "tz_utilities.h"
#include "tz_coordinate_3d.h"

int iround(double x)
{
#if defined HAVE_LRINT
  return (int) lrint(x);
#elif defined HAVE_ROUND
  return (int) round(x);
#else
  return (x > 0.0) ? ((int) (x + 0.5)) : ((int) (x - 0.5));
#endif
}

/* A better approximation from ihttp://arxiv.org/abs/1002.0567 */
double Normcdf_Quantile(double x)
{
  if ((x <= 0.0) || (x >= 1.0)) {
    return NaN;
  }

  
  double s = 1.0;

  if (x > 0.5) {
    x = 1.0 - x;
    s = -1.0;
  }

  double y = 0.0;

  if (x >= 0.0465) {
    double q = x - 0.5;
    double r = q * q;
    //const static double a0 = 0.389422403767615;
    //const static double a1 = -1.699385796345221;
    const static double a2 = 1.246899760652504;
    const static double a0p = 0.195740115269792;
    const static double a1p = -0.652871358365296;
    const static double b0 = 0.155331081623168;
    const static double b1 = -0.839293158122257;

    y = q * (a2 + (a1p * r + a0p) / (r * r + b1 * r + b0));
  } else {
    double r = sqrt((-2.0) * log(x));
    
    //const static double c0 = 0.155331081623168;
    //const static double c1 = -2.793522347562718412;
    //const static double c2 = -8.731478129786263127;
    const static double c3 = -1.000182518730158122;
    const static double c0p = 16.682320830719986527;
    const static double c1p = 4.120411523939115059;
    const static double c2p = 0.029814187308200211;
    const static double d0 = 7.173787663925508066;
    const static double d1 = 8.759693508958633869;

    y = c3 * r + c2p + (c1p * r + c0p) / (r * r + d1 * r + d0);
  }

  return y * s;
}

/*
double Normcdf_Quantile(double x)
{
  if ((x <= 0.0) || (x >= 1.0)) {
    return NaN;
  }

  const static double a0 = 2.515517;
  const static double a1 = 0.802853;
  const static double a2 = 0.010328;
  const static double b0 = 1.0;
  const static double b1 = 1.432788;
  const static double b2 = 0.189269;
  const static double b3 = 0.001308;

  double s = 1.0;

  if (x > 0.5) {
    x = 1.0 - x;
    s = -1.0;
  }

  double w = sqrt((-2.0) * log(x));

  x = -w + (a0 + a1 * w + a2 * w * w) /
    (b0 + b1 * w + b2 * w * w + b3 * w * w * w);
  x *= s;

  return x;
}
*/

int Compare_Float(double x, double y, double eps)
{
  int k1, k2;
  frexp(x, &k1);
  frexp(y, &k2);
  int k = imax2(k1, k2);
  if (k > 0) {
    eps *= (double) (2 << (k - 1));
  }
  if (fabs(x - y) < eps) {
    return 0;
  } else if (x < y) {
    return -1;
  } else {
    return 1;
  }
}

/*reference: www.worldserver.com/turk/computergraphics/CubeRoot.pdf*/
double Cube_Root(double x)
{
  if (x == 0.0) {
    return 0.0;
  }

  double fr, r;
  int ex, shx;
  
  fr = frexp(fabs(x), &ex);
  shx = ex % 3;
  if (shx > 0) {
    shx -= 3;
  }

  ex = (ex - shx) / 3;
  fr = ldexp(fr, shx);

  fr = ((((45.2548339756803022511987494 * fr + 
	   192.2798368355061050458134625) * fr + 
	  119.1654824285581628956914143) * fr + 
	 13.43250139086239872172837314) * fr + 
	0.1636161226585754240958355063) / 
    ((((14.80884093219134573786480845 * fr + 
	151.9714051044435648658557668) * fr + 
       168.5254414101568283957668343) * fr + 
      33.9905941350215598754191872) * fr + 1.0); 

  r = ldexp(fr, ex);

  if (x < 0.0) {
    r = -r;
  }

  return r;
}

#define SOLVE_CUBIC_MIND 1e-5
int Solve_Cubic(double a, double b, double c, double d, double *sol)
{
  double u = b / 3.0 / a;  
  double q = c / 3.0 / a - u * u; //(3.0 * a * c - b * b) / 9.0 / a / a;
  double r = (9.0 * a * b * c - 27.0 * a * a * d - 2.0 * b * b * b) /
    54.0 / a / a / a;
  double delta = q * q * q + r * r;

  if (delta > 0.0) {
    if (delta < SOLVE_CUBIC_MIND) {
      delta = 0.0;
    } else {
      double delta_2 = sqrt(delta);
      double s = Cube_Root(r + delta_2);
      double t = Cube_Root(r - delta_2);
      sol[0] = s + t - u;
      sol[1] = 0.0;
      sol[2] = 0.0;
      return 1;
    }
  }

  if (delta < 0) {
    double rho = sqrt(r * r - delta);
    double theta = acos(r / rho) / 3.0;
    rho = Cube_Root(rho);
    double st = rho * cos(theta);
    double s_t = sqrt(3.0) * rho * sin(theta);
    sol[0] =  2.0 * st - u;
    sol[1] = -st - u - s_t;
    sol[2] = -st - u + s_t;
    return 3;
  }
  
  if (delta == 0) {
    sol[0] = Cube_Root(r) * 2.0 - u;
    sol[1] = -Cube_Root(r) - u; 
    sol[2] = sol[1];
    return 2;
  }

  return 0;
}

double Polyeval(double *c, int order, double x)
{
  if (order < 0) {
    return 1.0;
  } else if (order == 0) {
    return c[0];
  }

  int i;
  double y = c[order];
  for (i = order - 1; i >= 0; i--) {
    y = x * y + c[i];
  }

  return y;
}

int Matrix_Eigen_Value_Cs(double a, double b, double c, double d, double e,
			  double f, double *result)
{
  double coeff[3];
  coeff[0] = -a - b - c;
  coeff[1] = a * b + b * c + a * c - e * e - d * d - f * f;
  coeff[2] = -a * b * c - 2.0 * d * e * f + a * f * f + c * d * d + b * e * e;
  if (Solve_Cubic(1.0, coeff[0], coeff[1], coeff[2], result) > 0){
    double tmp;
    if (result[0] < result[1]) {
      SWAP2(result[0], result[1], tmp);
      if (result[0] < result[2]) {
	SWAP2(result[0], result[2], tmp);
      }
    }
    if (result[1] < result[2]) {
      SWAP2(result[1], result[2], tmp);
    }
  } else {
    result[0] = 0.0;
    result[1] = 0.0;
    result[2] = 0.0;
    return 1;
  }

  return 0;
}

#define MATRIX_EIGEN_VECTOR_CS_EPS 0.00001
void Matrix_Eigen_Vector_Cs(double a, double b, double c, double d, double e,
    double f, double r, double *result)
{
  //set to 0 if the eigen value is too small
  if (r <= MATRIX_EIGEN_VECTOR_CS_EPS) {    
    result[0] = 0.0;
    result[1] = 0.0;
    result[2] = 0.0;

    return;
  }

  if ((fabs(a) < MATRIX_EIGEN_VECTOR_CS_EPS) && 
      (fabs(b) < MATRIX_EIGEN_VECTOR_CS_EPS)) {
    result[0] = 0.0;
    result[1] = 0.0;
    result[2] = 1.0;
  } else if ((fabs(a) < MATRIX_EIGEN_VECTOR_CS_EPS) && 
      (fabs(c) < MATRIX_EIGEN_VECTOR_CS_EPS)) {
    result[0] = 0.0;
    result[1] = 1.0;
    result[2] = 0.0;
  } else if ((fabs(b) < MATRIX_EIGEN_VECTOR_CS_EPS) && 
      (fabs(c) < MATRIX_EIGEN_VECTOR_CS_EPS)) {
    result[0] = 1.0;
    result[1] = 0.0;
    result[2] = 0.0;
  } else if (fabs(c) < MATRIX_EIGEN_VECTOR_CS_EPS) {
    result[0] = -d;
    result[1] = a - r;
    result[2] = 0.0;
  } else if (fabs(a) < MATRIX_EIGEN_VECTOR_CS_EPS) {
    result[0] = 0.0;
    result[1] = -f;
    result[2] = b - r;
  } else if (fabs(b) < MATRIX_EIGEN_VECTOR_CS_EPS) {
    result[0] = -d;
    result[1] = 0.0;
    result[2] = a - r;
  } else {
    result[0] = d * f - e * (b - r);
    result[1] = d * e - f * (a - r);
    result[2] = (a - r) * (b - r) - d * d;
  }

  Coordinate_3d_Unitize(result);
}
