#include "helpFunc.h"
#include "v3d_interface.h"

double cal_percentile(int* src, V3DLONG sz, double per)
{
    sort(src, src + sz);

    V3DLONG index = (V3DLONG) (per/100.0*sz);
    int dst = src[index];

    return dst;
}

bool sigma_correction(unsigned char* src, V3DLONG * mysz, double cutoff, double gain, unsigned char* &dst)
{
    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];
    dst = new unsigned char[tolSZ];
    int * src1 = new int[tolSZ];
    for (V3DLONG i=0; i<tolSZ; i++) src1[i] = src[i];

    cutoff = cal_percentile(src1, tolSZ, cutoff);
    cout<<"cutoff = "<<cutoff<<endl;

    double minfl =255.0, maxfl=0.0;
    for (V3DLONG i=0; i<tolSZ; i++)
    {
        if(minfl>src[i]) minfl=src[i];
        if(maxfl<src[i]) maxfl=src[i];
    }
    double scale = maxfl-minfl;
    cutoff /= scale;

    int temp=0;
    for (V3DLONG i=0; i<tolSZ; i++)
    {
        temp = int(scale/(1+exp(gain*(cutoff-double(src[i])/scale))));
        if(temp>255) temp=255;
        dst[i] = temp;
    }

    return 1;
}

bool intensity_rescale(unsigned char* src, V3DLONG * mysz, unsigned char* &dst)
{
    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];
    dst = new unsigned char[tolSZ];

    double minfl =255.0, maxfl=0.0;
    for (V3DLONG i=0; i<tolSZ; i++)
    {
        if(src[i]<0) src[i]=0;
        if(minfl>src[i]) minfl=src[i];
        if(maxfl<src[i]) maxfl=src[i];
    }

    cout<<"minfl = "<<minfl<<endl;
    cout<<"maxfl = "<<maxfl<<endl;

    for(V3DLONG i=0; i<tolSZ; i++)
    {
        dst[i] = int((src[i]-minfl)/(maxfl-minfl)*255);
    }
    return 1;
}

bool intensity_projection(unsigned char* src, V3DLONG * mysz, unsigned char* & dst, int flag)
{
    V3DLONG stacksz =mysz[0]*mysz[1];

    dst = new unsigned char [stacksz];

    for(V3DLONG iy = 0; iy < mysz[1]; iy++)
    {
        V3DLONG offsetj = iy*mysz[0];
        for(V3DLONG ix = 0; ix < mysz[0]; ix++)
        {
            int max_mip = 0, min_mip=1E9;
            for(V3DLONG iz = 0; iz < mysz[2]; iz++)
            {
                V3DLONG offsetk = iz*mysz[1]*mysz[0];
                double datatl = src[offsetk + offsetj + ix];
                if(datatl>max_mip) max_mip = datatl;
                if(datatl<min_mip) min_mip = datatl;
            }

            if(flag==1)   ///maximum
            {
                dst[iy*mysz[0] + ix] = max_mip;
            }
            if(flag==2)   //minimum
            {
                dst[iy*mysz[0] + ix] = min_mip;
            }
        }
    }

    return 1;
}

bool subtract_min(unsigned char* src, V3DLONG * mysz, unsigned char* & dst)
{
    unsigned char* mip = 0;
    intensity_projection(src, mysz, mip, 2);

    dst = new unsigned char[mysz[0]*mysz[1]*mysz[1]];

    for(V3DLONG iz = 0; iz < mysz[2]; iz++){
        V3DLONG offsetz = iz*mysz[0]*mysz[1];
        for(V3DLONG iy = 0; iy < mysz[1]; iy++){
            V3DLONG offsety = iy*mysz[1];
            for(V3DLONG ix=0; ix<mysz[0]; ix++){
                dst[offsetz + offsety + ix] = src[offsetz + offsety + ix]-mip[offsety + ix];
            }
        }
    }
    return 1;
}

bool fft_filter(unsigned char* & src, V3DLONG * mysz)
{
    V3DLONG tolSize = mysz[0]*mysz[1]*mysz[2];

    complex_type* dst = new complex_type[tolSize];

    for (V3DLONG i=0; i<tolSize; ++i){
        dst[i] = complex<double>(1.0*src[i]);
    }

    FFT_inplace(dst, mysz, FFT_FORWARD);

    V3DLONG sz_x  = mysz[0];
    V3DLONG sz_y  = mysz[1];
    V3DLONG sz_z = mysz[2];
    for(V3DLONG iz = 0; iz<1; iz++)
        for(V3DLONG ix = 0; ix < sz_x; ix++)  //[:,:,sz_z/2]=0
            for(V3DLONG iy = 0; iy < sz_y; iy++){
                V3DLONG offset = (iz*sz_x+iy)*sz_y+ix;
                dst[offset] = 0;
            }

   for(V3DLONG iz = 0; iz < sz_z; iz++)  //[:,sz_y/2,:]=0
        for(V3DLONG iy = 0; iy < 1; iy++)
            for(V3DLONG ix = 0; ix < sz_x; ix++){
                V3DLONG offset = (iz*sz_x+iy)*sz_y+ix;
                dst[offset] = 0;
            }


    for(V3DLONG iz = 0; iz < sz_z; iz++)  //[sz_x/2,:,:]=0
        for(V3DLONG ix = 0; ix < 1; ix++)
            for(V3DLONG iy = 0; iy < sz_y; iy++){
                V3DLONG offset = (iz*sz_x+iy)*sz_y+ix;
                dst[offset] = 0;
            }

    FFT_inplace(dst, mysz, FFT_BACKWARD);

    int temp=0;
    for (V3DLONG i=0; i<tolSize; ++i){
        temp = double(dst[i].real());
        if(temp>255) temp=255;
        if(temp<0) temp=0;
        src[i]=temp;
    }

    delete[] dst;
    dst = 0;
    return 1;
}
