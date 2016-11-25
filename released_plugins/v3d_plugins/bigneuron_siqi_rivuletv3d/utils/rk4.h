#ifndef RK4_H
#define RK4_H 

#include "stackutil.h"

double* rk4(double* startpoint, double* grad, V3DLONG* gradientArraySize, unsigned char stepsize);
 __inline double norm3(double *a);
#endif