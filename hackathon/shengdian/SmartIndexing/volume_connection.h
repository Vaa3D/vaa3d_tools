#ifndef VOLUME_CONNECTION_H
#define VOLUME_CONNECTION_H
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include "volimg_proc.h"
#include "v3d_interface.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <iterator>
using namespace std;
#define INF 3.4e+38
bool terafly_vol_index(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool volume_filter(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool volume_signals(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
// func converting
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt);

bool img_2_8bit(unsigned char * & src, unsigned char * & data1d, V3DLONG * in_sz, int src_dt=1);

template <class T>
bool intensity_rescale(T* src, V3DLONG * mysz, T* &dst, int dt)
{
    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];

    double minfl = 9999999, maxfl=0.0;
    for (V3DLONG i=0; i<tolSZ; i++)
    {
        if(src[i]<0) src[i]=0;
        if(minfl>src[i]) minfl=src[i];
        if(maxfl<src[i]) maxfl=src[i];
    }

    cout<<"minfl = "<<minfl<<endl;
    cout<<"maxfl = "<<maxfl<<endl;

    int max_val = 0;
    switch(dt)
    {
    case 1: max_val = 255;
        break;
    case 2: max_val = 65535;
        break;
    default:;
    }
    cout<<"start to rescale"<<endl;
    for(V3DLONG i=0; i<tolSZ; i++)
    {
        dst[i] = int((src[i]-minfl)/(maxfl-minfl)*max_val);
    }
    return true;
}
#endif // VOLUME_CONNECTION_H
