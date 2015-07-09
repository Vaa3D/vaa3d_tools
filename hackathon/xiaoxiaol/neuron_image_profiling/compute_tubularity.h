#ifndef __COMPUTE_TUBULARITY_H__
#define __COMPUTE_TUBULARITY_H__

#include <v3d_interface.h>


double compute_anisotropy_sphere(const unsigned char * data1d, V3DLONG N, V3DLONG M, V3DLONG P, V3DLONG c,
                                                  V3DLONG x0, V3DLONG y0, V3DLONG z0,
                                                  V3DLONG radius);


#endif // __COMPUTE_TUBULARITY_H__

