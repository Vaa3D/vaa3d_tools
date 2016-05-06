#ifndef RK4_H
#define RK4_H 
#include "stackutil.h"

Point rk4(Point startpoint, double* grad, V3DLONG* gradientArraySize, unsigned char stepsize);

#endif