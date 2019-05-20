#ifndef BRAINVESSELCPR_SAMPLEPLANE_H
#define BRAINVESSELCPR_SAMPLEPLANE_H

#include <v3d_interface.h>
#include "BrainVesselCPR_centerline.h"


Coor3D operator - (const Coor3D &n1, const Coor3D &n2);

Coor3D operator + (const Coor3D &n1, const Coor3D &n2);

Coor3D operator * (const Coor3D &n1, const double &n2);

Coor3D operator / (const Coor3D &n1, const double &n2);

Coor3D normCoor3D(const Coor3D &v);

unsigned short int interpolation(Coor3D coor, unsigned short int * data1d, V3DLONG x_len, V3DLONG y_len, V3DLONG z_len);

unsigned short int * samplePlane(unsigned short int * data1d, vector<Coor3D> centerline, V3DLONG x_len, V3DLONG y_len, V3DLONG z_len, int radius);




#endif
