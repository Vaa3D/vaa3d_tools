
#include "helpFunc.h"
#include "v3d_interface.h"
double cal_percentile(int* src, V3DLONG sz, double per)
{
    sort(src, src + sz);

    V3DLONG index = (V3DLONG) (per/100.0*sz);
    int dst = src[index];

    return dst;
}

void getColorMask(vector<double> &colorMask, double colorSigma, int dt){
    int max_val = 0;

    switch(dt)
    {
    case 1: max_val=256;
        break;
    case 2: max_val=65536;
        break;
    }

    for(int i=0; i<max_val; ++i){
        double colorDiff = exp(-(i*i)/(2*colorSigma*colorSigma));
        colorMask.push_back(colorDiff);
    }
}

void getGaussianMask(float* &mask, int* kernelSZ, double spaceSigmaXY, double spaceSigmaZ)
{
    V3DLONG tolSZ = kernelSZ[0]*kernelSZ[1]*kernelSZ[2];

    V3DLONG kernelSZ01 = kernelSZ[0]*kernelSZ[1];
    int centerX = (kernelSZ[0] - 1) / 2;
    int centerY = (kernelSZ[1] - 1) / 2;
    int centerZ = (kernelSZ[2] - 1) / 2;
    double x,y,z;
    double sum = 0;

    for(int k=0; k<kernelSZ[2]; ++k){
        z = pow(k-centerZ,2);
        for(int j=0; j<kernelSZ[1]; ++j){
            y = pow(j-centerY,2);
            for(int i=0; i<kernelSZ[0]; ++i){
                x = pow(i-centerX,2);
                double g =0;
                if(spaceSigmaZ>0) g = exp(-(x+y)/(2*spaceSigmaXY*spaceSigmaXY)-z/(2*spaceSigmaZ*spaceSigmaZ));
                else g = exp(-(x+y)/(2*spaceSigmaXY*spaceSigmaXY));
                qDebug()<<g;
                V3DLONG index = k*kernelSZ01 + j*kernelSZ[0] + i;
                mask[index] = g;
                sum += g;
            }
        }
    }

    for(int i=0; i<tolSZ; i++){
        mask[i] /= sum;
    }
}
