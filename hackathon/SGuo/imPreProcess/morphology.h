#ifndef MINMAXFILTER_H
#define MINMAXFILTER_H

#include <vector>
#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include "helpFunc.h"

using namespace std;

template<class T>
void grey_tophat(T* data1d,
                 V3DLONG *in_sz,
                 unsigned int Wx,
                 unsigned int Wy,
                 unsigned int c,
                 T* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0)
    {
        v3d_msg("Invalid parameters to grey_tophat.", 0);
        return;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;
    //declare temporary pointer
    outimg = new T [pagesz];
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                int minfl = data1d[offsetc+iz*M*N + iy*N + ix];
                for(V3DLONG j=yb; j<=ye; j++)
                {
                    for(V3DLONG i=xb; i<=xe; i++)
                    {
                        int temp = data1d[offsetc + iz*M*N + j*N + i];
                        if(minfl>temp)
                            minfl = temp;
                    }
                }
                //set value
                outimg[iz*M*N + iy*N + ix] = data1d[offsetc + iz*M*N + iy*N + ix]-minfl;
            }
        }
    }
}

template<class T>
void grey_erosion(T* data1d, V3DLONG *in_sz,
                  unsigned int Wx,
                  unsigned int Wy,
                  unsigned int c,
                  T* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0)
    {
        v3d_msg("Invalid parameters to grey_tophat.", 0);
        return;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;
    //declare temporary pointer
    outimg = new T [pagesz];
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                int minfl = data1d[offsetc+iz*M*N + iy*N + ix];
                for(V3DLONG j=yb; j<=ye; j++)
                {
                    for(V3DLONG i=xb; i<=xe; i++)
                    {
                        int temp = data1d[offsetc + iz*M*N + j*N + i];
                        if(minfl>temp)
                            minfl = temp;
                    }
                }
                //set value
                outimg[iz*M*N + iy*N + ix] = minfl;
            }
        }
    }
}

template<class T>
void grey_dilation(T* data1d, V3DLONG *in_sz,
                  unsigned int Wx,
                  unsigned int Wy,
                  unsigned int c,
                  T* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0)
    {
        v3d_msg("Invalid parameters to grey_tophat.", 0);
        return;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;
    //declare temporary pointer
    outimg = new T [pagesz];
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                int maxfl = data1d[offsetc+iz*M*N + iy*N + ix];
                for(V3DLONG j=yb; j<=ye; j++)
                {
                    for(V3DLONG i=xb; i<=xe; i++)
                    {
                        int temp = data1d[offsetc + iz*M*N + j*N + i];
                        if(maxfl<temp)
                            maxfl = temp;
                    }
                }
                //set value
                outimg[iz*M*N + iy*N + ix] = maxfl;
            }
        }
    }
}

#endif // MINMAXFILTER_H
