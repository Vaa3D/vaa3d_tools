#include "image_process_lib.h"

bool enhanceImage(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz,bool biilateral_filter){

    double gain=5, cutoff=25;
    double spaceSigmaXY, spaceSigmaZ, colorSigma=35;

    int k_sz[3] = {3, 3, 1};

    spaceSigmaXY = k_sz[0]/3.0;
    spaceSigmaZ = k_sz[2]/3.0;

    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];

    cout<<"adaptive thresholding "<<endl;
    adaptiveThresholding(data1d,dst,mysz);
    for(V3DLONG i=0; i<tolSZ; i++)
        data1d[i]=dst[i];

    if(sigma_correction(data1d, mysz, cutoff, gain, dst, 1)){
        cout<<"finish sigma correction "<<endl;
        for(V3DLONG i=0; i<tolSZ; i++)
            data1d[i]=dst[i];
    }
    else
        return false;
    if(subtract_min(data1d, mysz, dst)){
        cout<<"finish subtract min"<<endl;
        for(V3DLONG i=0; i<tolSZ; i++)
            data1d[i]=dst[i];
    }
    else
        return false;
    if(biilateral_filter){
        cout<<"do bilateral filter "<<endl;
        bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);
        cout<<"finish bilateral filter "<<endl;
    }

    if(intensity_rescale(data1d, mysz, dst, 1)){
        cout<<"finish intensity rescale"<<endl;
        return true;
    }
    else
        return false;
}
void adaptiveThresholding(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz)
{
    V3DLONG h = 5, d = 3;
    V3DLONG iImageWidth, iImageHeight,iImageLayer;
    iImageWidth=mysz[0];
    iImageHeight=mysz[1];
    iImageLayer=mysz[2];

    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += data1d[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += data1d[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += data1d[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += data1d[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += data1d[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += data1d[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  data1d[curpos]-temp/(count);
                dst[curpos]= (t > 0)? t : 0;
            }
        }
    }
}
