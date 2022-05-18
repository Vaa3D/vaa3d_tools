#include "HistogramEqualization_plugin.h"

#include <math.h>
#include "v3d_message.h"
#include <iostream>

//#include <thread>
#include <vector>

#include <algorithm>
using namespace std;

float abstest(float data){
    return data>0?data:-data;
}

unsigned char quantile(unsigned char* data1d, float percentage, V3DLONG len)
{

    vector<int>temp(len);
    V3DLONG i;
    for (i=0;i<len;i++)
    {
        temp[i] = data1d[i];
    }
    sort(temp.begin(),temp.end());

    unsigned char quantile_per;
    if (abstest((percentage * temp.size() + 0.5) - round((percentage * temp.size() + 0.5))) < 0.000001)
    {
        quantile_per = temp[round((0.95 * temp.size() + 0.5) - 1)];
    }
    else
    {
        quantile_per = 0.5 * (temp[0.95 * temp.size()] + temp[0.95 * temp.size() - 1]);
    }

    if (percentage>=0.999999999995)
    {
       quantile_per =  temp.back();
    }
    return quantile_per;
}

bool equalimage(unsigned char* data1d, V3DLONG* sz)
{
    // this method is for uint8 image

    V3DLONG len = sz[0] * sz[1] * sz[2];

    //unsigned char lowerbound = 0;
    unsigned char lowerbound = quantile(data1d,0.8,len);

    unsigned char upperbound = quantile(data1d,0.999999999995,len);

    printf("lowerbound%d ",lowerbound);
    printf("upperbound%d ",upperbound);

    if (!data1d || len<=0)
    {
        printf("The input parameters are invalid in hist_eq_uint8().\n");
        return false;
    }

    if (lowerbound>upperbound)
    {
        unsigned char tmp=lowerbound; lowerbound=upperbound; upperbound=tmp;
    }

    V3DLONG NBIN=256;
    V3DLONG *h = new V3DLONG [NBIN];
    double *c = new double [NBIN];
    if (!h)
    {
        printf("Fail to allocate memory in hist_eq_uint8().\n");
        return false;
    }

    V3DLONG i;

    for (i=0;i<NBIN;i++) h[i]=0;

    for (i=0;i<len; i++)
    {
        h[data1d[i]]++;
    }

    c[lowerbound]=h[lowerbound];
    for (i=lowerbound+1;i<=upperbound;i++) c[i] = c[i-1]+h[i];
    double range = upperbound-lowerbound;
    for (i=lowerbound;i<=upperbound;i++) {c[i] /= c[upperbound]; c[i] *= range; c[i] += lowerbound;}

    for (i=0;i<len;i++)
    {
        if (data1d[i]>=lowerbound && data1d[i]<=upperbound)
            data1d[i] = c[data1d[i]];
    }

    if (c) {delete []c; c=0;}
    if (h) {delete []h; h=0;}
    return true;

//    V3DLONG totalSz = sz[0] * sz[1] * sz[2];
//    double gray_prob[256];
//    double gray_accum[256];
//    int gray_equal[256];
//    for(int i=0; i<256; ++i){
//        gray_accum[i] = 0;
//        gray_prob[i] = 0;
//        gray_equal[i] = 0;
//    }
//    for (V3DLONG i = 0; i < totalSz; i++)
//    {
//        gray_prob[pdata[i]]++;
//    }
//    for(int i=0; i<256; ++i){
//        gray_prob[i] /= (double) totalSz;
//    }
//    gray_accum[0] = gray_prob[0];
//    for (int i = 1; i < 256; i++)
//    {
//        gray_accum[i] = gray_accum[i - 1] + gray_prob[i];
//    }
//    for (int i = 0; i < 256; i++)
//    {
//        gray_equal[i]= (255 * gray_accum[i] + 0.5);
//    }
//    for (V3DLONG i = 0; i < totalSz; i++)
//    {
//        pdata[i] = gray_equal[pdata[i]];
//    }
}


