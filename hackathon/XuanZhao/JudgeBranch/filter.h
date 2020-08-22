#ifndef FILTER_H
#define FILTER_H

#include "v3d_interface.h"

#include "multi_convolution.hxx"
#include "multi_blockwise.hxx"


float* gaussianSmooth(unsigned char* pdata, V3DLONG* sz, double sigma);

float* laplacianOfGaussian(unsigned char* pdata, V3DLONG* sz, double sigma);

vector<vector<float> > hessianOfGaussian(unsigned char* pdata, V3DLONG* sz, double sigma);

float* gaussianGradientMagnitude(unsigned char* pdata, V3DLONG* sz, double sigma);


#endif // FILTER_H
