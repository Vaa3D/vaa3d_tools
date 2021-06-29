#ifndef BILATERAL_FILTER_H
#define BILATERAL_FILTER_H

#include <vector>
#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include <QDebug>
#include <math.h>

using namespace std;

void getColorMask(vector<double> &colorMask, double colorSigma);
void getGaussianMask(float* &mask, int* kernelSZ, double spaceSigmaXY, double spaceSigmaZ);
void im_roll(unsigned char * src, unsigned char * &dst, V3DLONG* sz, int* d_roll);
void bilateralfilter(unsigned char* src, unsigned char* &dst, V3DLONG* sz, int* kernelSZ, double spaceSigmaXY, double spaceSigmaZ, double colorSigma);

#endif // BILATERAL_FILTER_H

