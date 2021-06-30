#ifndef BILATERAL_FILTER_H
#define BILATERAL_FILTER_H

#include <vector>
#include "v3d_basicdatatype.h"
#include "helpFunc.h"
#include "v3d_message.h"
#include <QDebug>
#include <math.h>

template <class T>
void im_roll(T * src, T * &dst, V3DLONG* sz, int* d_roll)
{
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    dst = new T[tolSZ];

    int centerX = d_roll[0];
    int centerY = d_roll[1];
    int centerZ = d_roll[2];

    for(V3DLONG k=centerZ; k<sz[2]+centerZ; ++k){
        for(V3DLONG j=centerY; j<sz[1]+centerY; ++j){
            for(V3DLONG i=centerX; i<sz[0]+centerX; ++i){

                V3DLONG x = i, y=j, z=k;

                x = (x<0)?0:x;
                y = (y<0)?0:y;
                z = (z<0)?0:z;

                x = (x>=sz[0])?sz[0]-1:x;
                y = (y>=sz[1])?sz[1]-1:y;
                z = (z>=sz[2])?sz[2]-1:z;
                V3DLONG index = z*sz[0]*sz[1] + y*sz[0] + x;
                V3DLONG index1 = (k-centerZ)*sz[0]*sz[1] + (j-centerY)*sz[0] + i-centerX;
                dst[index1] = src[index];
            }
        }
    }
}

template <class T>
void bilateralfilter(T* src, T* &dst, V3DLONG* sz, int* kernelSZ, double spaceSigmaXY, double spaceSigmaZ, double colorSigma, int dt){
    qDebug()<<"-----bilateralfilter-------";

    if (!src || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || kernelSZ[0]<=0 || kernelSZ[1]<=0 || kernelSZ[2]<=0)
    {
        v3d_msg("Invalid parameters to bilateral_filter().", 0);
        return;
    }

    int centerX = (kernelSZ[0] - 1) / 2;
    int centerY = (kernelSZ[1] - 1) / 2;
    int centerZ = (kernelSZ[2] - 1) / 2;

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    vector<double> colorMask = vector<double>();
    getColorMask(colorMask, colorSigma, dt);
    float* spaceMask = 0;
    getGaussianMask(spaceMask, kernelSZ, spaceSigmaXY, spaceSigmaZ);
    dst = new T[tolSZ];
    double* dst1 = new double[tolSZ];
    T* srcNew;
    double* wgt_sum = new double[tolSZ];

    for (V3DLONG i=0; i<tolSZ; i++)
    {
        wgt_sum[i] = 0;
    }

    qDebug()<<"-------start--------";

    for(int kk=-centerZ; kk<=centerZ; ++kk){
        for(int jj=-centerY; jj<=centerY; ++jj){
            for(int ii=-centerX; ii<=centerX; ++ii){

                V3DLONG maskIndex = (kk+centerZ)*kernelSZ[0]*kernelSZ[1] + (jj+centerY)*kernelSZ[0] + (ii+centerX);

                int d_roll[3]={ii, jj, kk};
                im_roll(src, srcNew, sz, d_roll);
                for(int ip=0; ip<tolSZ; ++ip)
                {
                    double tw = colorMask[abs(srcNew[ip]-src[ip])]*spaceMask[maskIndex];
                    dst1[ip] += tw*srcNew[ip];
                    wgt_sum[ip] += tw;
                }
            }
        }
    }

    for (V3DLONG i=0; i<tolSZ; i++)
    {
        if(dst1[i]<0) dst1[i]=0;
        dst[i] = int(dst1[i]/wgt_sum[i]);

    }

    if(srcNew){
        delete[] srcNew;
        srcNew = 0;
    }
}
#endif // BILATERAL_FILTER_H

