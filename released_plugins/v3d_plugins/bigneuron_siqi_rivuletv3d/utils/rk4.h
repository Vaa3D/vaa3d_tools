#ifndef RK4_H
#define RK4_H 

#include "stackutil.h"
#include "rivulet.h"
using namespace rivulet;

Point<float> rk4(Point<float> startpoint, double* grad, V3DLONG* gradientArraySize, unsigned char stepsize);
 __inline double norm3(double *a);
#endif