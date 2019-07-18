#ifndef BRAINVESSELCPR_FILTER_H
#define BRAINVESSELCPR_FILTER_H

#include "v3d_message.h"
#include <v3d_interface.h>
#include "v3d_message.h"

void gaussian_filter(unsigned short int * data1d, V3DLONG *in_sz, unsigned int Wx, unsigned int Wy,
                     unsigned int Wz, unsigned int c, double sigma, float* &outimg);

#endif // BRAINVESSELCPR_FILTER_H
