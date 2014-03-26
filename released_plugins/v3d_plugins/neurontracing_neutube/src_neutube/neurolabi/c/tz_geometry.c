/* tz_geometry.c
 *
 * 20-Dec-2007 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_geometry.h"

const static Sphere_Point_30[90] = {
  0.7474,0.1036,-0.6563
    ,0.8032,-0.0633,0.5923
    ,0.1574,0.9494,-0.2718
    ,0.7123,0.6508,-0.2628
    ,0.2609,0.5328,-0.8050
    ,-0.3124,0.1116,0.9434
    ,-0.4028,0.7157,-0.5705
    ,-0.4446,0.6382,0.6285
    ,0.2634,0.4699,0.8425
    ,-0.2880,0.1809,-0.9404
    ,-0.8227,0.2203,-0.5241
    ,-0.4445,0.8947,0.0450
    ,0.1643,0.9118,0.3763
    ,-0.8981,0.4311,0.0871
    ,-0.5491,-0.3812,-0.7438
    ,-0.9555,-0.2616,-0.1366
    ,-0.8275,0.0017,0.5614
    ,-0.7223,-0.5987,0.3462
    ,0.7339,0.5639,0.3786
    ,0.2220,-0.1970,-0.9549
    ,0.0138,-0.7462,-0.6656
    ,-0.5071,-0.8196,-0.2665
    ,-0.1574,-0.9494,0.2718
    ,-0.2609,-0.5328,0.8050
    ,0.8346,-0.5399,0.1089
    ,0.6691,-0.5211,-0.5299
    ,0.9965,0.0660,-0.0503
    ,0.3302,-0.9360,-0.1219
    ,0.4028,-0.7157,0.5705
    ,0.2881,-0.1811,0.9403
};

/*
 * Vector_Angle(): Calculate the angle of a vector in 2D space. 
 *
 * Args: x - x coordinate;
 *       y - y coordinate.
 *
 * Return: the angle, which belongs to [0, 2*pi).
 */
double Vector_Angle(double x, double y)
{
  if ((x == 0.0) && (y == 0.0)) {
    return 0.0;
  }

  double angle;

  if (x == 0.0) {
    angle = TZ_PI_2;
    if (y < 0.0) {
      angle += TZ_PI;
    }
  } else {
    angle = atan(y / x);
    if (x < 0.0) {
      angle += TZ_PI;
    }
  }

  if (angle < 0.0) {
    angle += TZ_2PI;
  }

  /*
  if (x < 0.0) {
    angle += TZ_PI;
  } else if (y < 0.0) {
    angle += TZ_2PI;
  }
  */
  return  angle;
}

static 
double DistancePointEllipseSpecial(double dU, double dV, double dA, 
				   double dB, double dEpsilon, int iMax, 
				   //int riIFinal, 
				   double *rdX, double *rdY) 
{ 
  // initial guess 
  double dT = dB*(dV - dB); 
  // Newton's method 
  int i; 
  for (i = 0; i < iMax; i++) { 
    double dTpASqr = dT + dA*dA; 
    double dTpBSqr = dT + dB*dB; 
    double dInvTpASqr = 1.0/dTpASqr; 
    
    double dInvTpBSqr = 1.0/dTpBSqr; 
    double dXDivA = dA*dU*dInvTpASqr; 
    double dYDivB = dB*dV*dInvTpBSqr; 
    double dXDivASqr = dXDivA*dXDivA; 
    double dYDivBSqr = dYDivB*dYDivB; 
    double dF = dXDivASqr + dYDivBSqr - 1.0; 
    if (dF < dEpsilon) { 
      // F(t0) is close enough to zero, terminate the iteration 
      *rdX = dXDivA*dA; 
      *rdY = dYDivB*dB; 
      //riIFinal = i; 
      break; 
    } 
    double dFDer = 2.0*(dXDivASqr*dInvTpASqr + dYDivBSqr*dInvTpBSqr); 
    double dRatio = dF/dFDer; 
    if (dRatio < dEpsilon) { 
      // t1-t0 is close enough to zero, terminate the iteration 
      *rdX = dXDivA*dA; 
      *rdY = dYDivB*dB; 
      //riIFinal = i; 
      break; 
    } 
    dT += dRatio; 
  } 
  /*
  if (i == iMax) { 
    // method failed to converge, let caller know 
    riIFinal = -1; 
    return -FLT_MAX; 
  } 
  */
  double dDelta0 = *rdX - dU, dDelta1 = *rdY - dV; 
  return sqrt(dDelta0*dDelta0 + dDelta1*dDelta1); 
} 

/* from www.geometrictools.com (by David Eberly) */
double
Ellipse_Point_Distance(double dU, double dV, /* test point (u,v) */
		       double dA, double dB, /* ellipse (x/a)^2 + (y/b)^2 = 1 */
		       double *rdX, double *rdY)
{ 
  double dEpsilon = 1e-5; /* zero tolerance for Newton's method */
  int iMax = 100; /* maximum iterations in Newton's method */

  //int riIFinal;

  double tmp_rdX, tmp_rdY;
  if (rdX == NULL) {
    rdX = &tmp_rdX;
  }
  if (rdY == NULL) {
    rdY = &tmp_rdY;
  }

  // special case of circle 
  if (fabs(dA-dB) < dEpsilon) { 
    double dLength = sqrt(dU*dU+dV*dV); 
    *rdX = dA / dLength * dU;
    *rdY = dA / dLength * dV;
    return fabs(dLength - dA); 
  } 
  // reflect U = -U if necessary, clamp to zero if necessary 
  BOOL bXReflect; 
  if (dU > dEpsilon) { 
    bXReflect = FALSE; 
  } else if (dU < -dEpsilon) { 
    bXReflect = TRUE; 
    dU = -dU; 
  } else { 
    bXReflect = FALSE; 
    dU = 0.0; 
  } 
  // reflect V = -V if necessary, clamp to zero if necessary 
  BOOL bYReflect; 
  if (dV > dEpsilon) { 
    bYReflect = FALSE; 
  } else if (dV < -dEpsilon) { 
    bYReflect = TRUE; 
    dV = -dV; 
  } else { 
    bYReflect = FALSE; 
    dV = 0.0; 
  } 
  // transpose if necessary 
  double dSave; 
  BOOL bTranspose; 
  if (dA >= dB) { 
    bTranspose = FALSE; 
  } else { 
    bTranspose = TRUE; 
    dSave = dA; 
    dA = dB; 
    dB = dSave; 
    dSave = dU; 
    dU = dV; 
    dV = dSave; 
  } 
  double dDistance; 
  if (dU != 0.0) { 
    if (dV != 0.0) { 
      dDistance = DistancePointEllipseSpecial(dU,dV,dA,dB,dEpsilon,iMax, 
					      /*riIFinal,*/rdX,rdY); 
    } else { 
      double dBSqr = dB*dB; 
      if (dU < dA - dBSqr/dA) { 
	double dASqr = dA*dA; 
	*rdX = dASqr*dU/(dASqr-dBSqr);
	double dXDivA = *rdX/dA;
	*rdY = dB*sqrt(fabs(1.0-dXDivA*dXDivA)); 
	double dXDelta = *rdX - dU; 
	dDistance = sqrt(dXDelta*dXDelta+(*rdY) * (*rdY)); 
	//riIFinal = 0; 
      } else { 
	dDistance = fabs(dU - dA); 
	*rdX = dA; 
	*rdY = 0.0; 
	//riIFinal = 0; 
      } 
    } 
  } else { 
    dDistance = fabs(dV - dB); 
    *rdX = 0.0; 
    *rdY = dB; 
    //riIFinal = 0; 
  } 
  if (bTranspose) { 
    dSave = *rdX; 
    *rdX = *rdY; 
    *rdY = dSave; 
  } 
  if (bYReflect) { 
    *rdY = -(*rdY); 
  } 
  if (bXReflect) { 
    *rdX = -(*rdX); 
  } 

  return dDistance; 
}

BOOL Point_In_Ellipse(double x, double y, double a, double b)
{
  x /= a;
  y /= b;
  
  return (x * x + y * y <= 1.0);
}

#define COORDINATE_EPS 0.00001
double Vector_Angle2(double x1, double y1, double x2, double y2, 
    BOOL counter_clockwise)
{
  if ((fabs(x1) < COORDINATE_EPS && fabs(y1) < COORDINATE_EPS) ||
      (fabs(x2) < COORDINATE_EPS && fabs(y2) < COORDINATE_EPS)) {
    return 0.0;
  }

  double angle1 = Vector_Angle(x1, y1);
  double angle2 = Vector_Angle(x2, y2);
  
  double angle = 0.0;
  angle = angle2 - angle1;
  if (angle < 0) {
    angle += TZ_2PI;
  }

  if (counter_clockwise == FALSE) {
    angle = TZ_2PI - angle;
  }

  return angle;
}

const double* Get_Uniform_Sphere_Point(int *n)
{
  if (n != NULL) {
    *n = 30;
  }

  return Sphere_Point_30;
}
