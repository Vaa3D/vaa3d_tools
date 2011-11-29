#ifndef __V3D_IOV3DUTIL_H__
#define __V3D_IOV3DUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <v3d_basicdatatype.h>
#include <v3d_interface.h>

void rescaleForDisplay(double* input, double* output, int numVoxels, ImagePixelType dataType);
unsigned char* doubleArrayToCharArray(double* data1dD, int numVoxels, ImagePixelType dataType);
double* channelToDoubleArray(Image4DSimple* inputImage, int channel);

#endif
