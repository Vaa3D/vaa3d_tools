#include "rk4.h"
#include "math.h"
#include <iostream>

/* RK4 is a function which performs one step of the Runge-Kutta 4 ray tracing
 *
 * EndPoint = RK4(StartPoint, GradientVolume, StepSize);
 *
 * inputs :
 *      StartPoint: 2D or 3D location in vectorfield
 *      GradientVolume: Vectorfield
 *      Stepsize : The stepsize
 *
 * outputs :
 *      EndPoint : The new location (zero if outside image)
 *
 * Function is written by D.Kroon University of Twente (July 2008)
 * Adapted by Siqi University of Sydney (Sep 2015)
 */

using namespace std;
__inline long mindex3(long x, long y, long z, long sizx, long sizy) {
  return z * sizy * sizx + y * sizx + x;
}

__inline bool checkBounds3d(double *point, long *dims) {
  if ((point[0] < 0) || (point[1] < 0) || (point[2] < 0) ||
      (point[0] > (dims[0] - 1)) || (point[1] > (dims[1] - 1)) ||
      (point[2] > (dims[2] - 1))) {
    return false;
  }
  return true;
}

__inline double norm2(double *a) { return sqrt(a[0] * a[0] + a[1] * a[1]); }
__inline double norm3(double *a) {
  return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

__inline void interpgrad3d(double *Ireturn, double *I, long *dims,
                           double *point) {
  /*  Linear interpolation variables */
  long xBas0, xBas1, yBas0, yBas1, zBas0, zBas1;
  double perc[8];
  double xCom, yCom, zCom;
  double xComi, yComi, zComi;
  double fTlocalx, fTlocaly, fTlocalz;
  long f0, f1;
  long index[8];
  double temp;

  fTlocalx = floor(point[0]);
  fTlocaly = floor(point[1]);
  fTlocalz = floor(point[2]);
  xBas0 = (long)fTlocalx;
  yBas0 = (long)fTlocaly;
  zBas0 = (long)fTlocalz;
  xBas1 = xBas0 + 1;
  yBas1 = yBas0 + 1;
  zBas1 = zBas0 + 1;

  /* Linear interpolation constants (percentages) */
  xCom = point[0] - fTlocalx;
  yCom = point[1] - fTlocaly;
  zCom = point[2] - fTlocalz;
  xComi = (1 - xCom);
  yComi = (1 - yCom);
  zComi = (1 - zCom);
  perc[0] = xComi * yComi;
  perc[1] = perc[0] * zCom;
  perc[0] = perc[0] * zComi;
  perc[2] = xComi * yCom;
  perc[3] = perc[2] * zCom;
  perc[2] = perc[2] * zComi;
  perc[4] = xCom * yComi;
  perc[5] = perc[4] * zCom;
  perc[4] = perc[4] * zComi;
  perc[6] = xCom * yCom;
  perc[7] = perc[6] * zCom;
  perc[6] = perc[6] * zComi;

  /* Stick to boundary */
  if (xBas0 < 0) {
    xBas0 = 0;
    if (xBas1 < 0) {
      xBas1 = 0;
    }
  }
  if (yBas0 < 0) {
    yBas0 = 0;
    if (yBas1 < 0) {
      yBas1 = 0;
    }
  }
  if (zBas0 < 0) {
    zBas0 = 0;
    if (zBas1 < 0) {
      zBas1 = 0;
    }
  }
  if (xBas1 > (dims[0] - 1)) {
    xBas1 = dims[0] - 1;
    if (xBas0 > (dims[0] - 1)) {
      xBas0 = dims[0] - 1;
    }
  }
  if (yBas1 > (dims[1] - 1)) {
    yBas1 = dims[1] - 1;
    if (yBas0 > (dims[1] - 1)) {
      yBas0 = dims[1] - 1;
    }
  }
  if (zBas1 > (dims[2] - 1)) {
    zBas1 = dims[2] - 1;
    if (zBas0 > (dims[2] - 1)) {
      zBas0 = dims[2] - 1;
    }
  }

  /*Get the neighbour intensities */
  index[0] = mindex3(xBas0, yBas0, zBas0, dims[0], dims[1]);
  index[1] = mindex3(xBas0, yBas0, zBas1, dims[0], dims[1]);
  index[2] = mindex3(xBas0, yBas1, zBas0, dims[0], dims[1]);
  index[3] = mindex3(xBas0, yBas1, zBas1, dims[0], dims[1]);
  index[4] = mindex3(xBas1, yBas0, zBas0, dims[0], dims[1]);
  index[5] = mindex3(xBas1, yBas0, zBas1, dims[0], dims[1]);
  index[6] = mindex3(xBas1, yBas1, zBas0, dims[0], dims[1]);
  index[7] = mindex3(xBas1, yBas1, zBas1, dims[0], dims[1]);
  f0 = dims[0] * dims[1] * dims[2];
  f1 = f0 + f0;

  /*the interpolated color */
  temp = I[index[0]] * perc[0] + I[index[1]] * perc[1] + I[index[2]] * perc[2] +
         I[index[3]] * perc[3];
  Ireturn[0] = temp + I[index[4]] * perc[4] + I[index[5]] * perc[5] +
               I[index[6]] * perc[6] + I[index[7]] * perc[7];
  temp = I[index[0] + f0] * perc[0] + I[index[1] + f0] * perc[1] +
         I[index[2] + f0] * perc[2] + I[index[3] + f0] * perc[3];
  Ireturn[1] = temp + I[index[4] + f0] * perc[4] + I[index[5] + f0] * perc[5] +
               I[index[6] + f0] * perc[6] + I[index[7] + f0] * perc[7];
  temp = I[index[0] + f1] * perc[0] + I[index[1] + f1] * perc[1] +
         I[index[2] + f1] * perc[2] + I[index[3] + f1] * perc[3];
  Ireturn[2] = temp + I[index[4] + f1] * perc[4] + I[index[5] + f1] * perc[5] +
               I[index[6] + f1] * perc[6] + I[index[7] + f1] * perc[7];
}

bool RK4STEP_3D(double *grad, long *dims,
                double *srcpt, double *endpt, double stepsize) {
  double k1[3], k2[3], k3[3], k4[3];
  double tp[3];
  double tempnorm;

  /*Calculate k1 */
  interpgrad3d(k1, grad, dims, srcpt);
  tempnorm = norm3(k1);
  k1[0] = k1[0] * stepsize / tempnorm;
  k1[1] = k1[1] * stepsize / tempnorm;
  k1[2] = k1[2] * stepsize / tempnorm;

  tp[0] = srcpt[0] - k1[0] * 0.5;
  tp[1] = srcpt[1] - k1[1] * 0.5;
  tp[2] = srcpt[2] - k1[2] * 0.5;

  /*Check the if are still inside the domain */
  if (!checkBounds3d(tp, dims))
    return false;

  /*Calculate k2 */
  interpgrad3d(k2, grad, dims, tp);
  tempnorm = norm3(k2);
  k2[0] = k2[0] * stepsize / tempnorm;
  k2[1] = k2[1] * stepsize / tempnorm;
  k2[2] = k2[2] * stepsize / tempnorm;

  tp[0] = srcpt[0] - k2[0] * 0.5;
  tp[1] = srcpt[1] - k2[1] * 0.5;
  tp[2] = srcpt[2] - k2[2] * 0.5;

  /*Check the if are still inside the domain */
  if (!checkBounds3d(tp, dims))
    return false;

  /*Calculate k3 */
  interpgrad3d(k3, grad, dims, tp);
  tempnorm = norm3(k3);
  k3[0] = k3[0] * stepsize / tempnorm;
  k3[1] = k3[1] * stepsize / tempnorm;
  k3[2] = k3[2] * stepsize / tempnorm;

  tp[0] = srcpt[0] - k3[0];
  tp[1] = srcpt[1] - k3[1];
  tp[2] = srcpt[2] - k3[2];

  /*Check the if are still inside the domain */
  if (!checkBounds3d(tp, dims))
    return false;

  /*Calculate k4 */
  interpgrad3d(k4, grad, dims, tp);
  tempnorm = norm3(k4);
  k4[0] = k4[0] * stepsize / tempnorm;
  k4[1] = k4[1] * stepsize / tempnorm;
  k4[2] = k4[2] * stepsize / tempnorm;

  /*Calculate final point */
  endpt[0] =
      srcpt[0] - (k1[0] + k2[0] * 2.0 + k3[0] * 2.0 + k4[0]) / 6.0;
  endpt[1] =
      srcpt[1] - (k1[1] + k2[1] * 2.0 + k3[1] * 2.0 + k4[1]) / 6.0;
  endpt[2] =
      srcpt[2] - (k1[2] + k2[2] * 2.0 + k3[2] * 2.0 + k4[2]) / 6.0;

  /*Check the if are still inside the domain */
  if (!checkBounds3d(endpt, dims))
    return false;

  return true;
}

double *rk4(double *srcpt, double *grad, V3DLONG *dims,
            double stepsize) {
  double *endpt = new double[3];

  /*Perform the RK4 raytracing step */
  if (!RK4STEP_3D(grad, dims, srcpt, endpt, stepsize)) {
    endpt[0] = -1; endpt[1] = -1;endpt[2] = -1;}

  return endpt;
}
