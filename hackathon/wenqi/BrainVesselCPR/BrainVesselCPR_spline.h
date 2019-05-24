#ifndef __BRAINVESSELCPR_SPLINE__
#define __BRAINVESSELCPR_SPLINE__

#include <v3d_interface.h>
#include "BrainVesselCPR_centerline.h"

Coor3D catmull_rom_spline(double t, Coor3D p1, Coor3D p2, Coor3D p3, Coor3D p4);

#endif
