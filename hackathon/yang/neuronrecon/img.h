// image processing with GPUs
// -Yang
// 10/19/2017


#ifndef _IMG_H_
#define _IMG_H_

//
#include <vector>
#include <iostream>
#include <string.h>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <string>
#include <map>

using namespace std;

//#include <boost/compute/core.hpp>
//#include <boost/compute/algorithm/copy.hpp>
//#include <boost/compute/container/vector.hpp>
//#include <boost/compute/type_traits/type_name.hpp>
//#include <boost/compute/utility/source.hpp>

#include "distance_transform.hpp"

using namespace dt;
using namespace dope;

////
//const char compute_source[] = BOOST_COMPUTE_STRINGIZE_SOURCE
//        (
//            //#pragma OPENCL EXTENSION cl_intel_printf : enable
//            //#pragma OPENCL EXTENSION cl_amd_printf : enable

//            __kernel void adaptive_thresholding(__global uchar* dst, __global uchar* src, int dimx, int dimy, int dimz, int r) {

//                int i = get_global_id(0);
//                int z = (i/(dimx*dimy))%dimz;
//                int y = (i/dimx)%dimy;
//                int x = i%dimx;
//                int slicesz = dimx*dimy;

//                if(z<r || z >=dimz-r || y<r || y>=dimy-r || x<r || x>=dimx-r)
//                {
//                    return;
//                }

//                for(int k=z-r; k<=z+r; k++)
//                {
//                    for(int j=y-r; j<=y+r; j++)
//                    {
//                        for(int i=x-r; i<=x+r; i++)
//                        {
//                            thresh += src[k*slicesz + j*dimx + i];
//                        }
//                    }
//                }

//                int volsz = (2*r+1)*(2*r+1)*(2*r+1);

//                if(src[z*slicesz + y*dimx + x] > thresh/volsz)
//                {
//                    dst[z*slicesz + y*dimx + x] = 255;
//                }
//                else
//                {
//                    dst[z*slicesz + y*dimx + x] = 0;
//                }
//            }

//            );


// abs
template <class T>
T y_abs(T x)
{
    return (x<(T)0)?-x:x;
}

// delete 1d pointer
template <class T>
void y_del1dp(T *&p)
{
    if(p) {delete []p; p=NULL;}
    return;
}

// new 1d pointer
template<class T, class Tidx>
void y_new1dp(T *&p, Tidx n)
{
    //
    y_del1dp<T>(p);

    //
    try
    {
        p = new T [n];
        memset(p, 0, n);
    }
    catch(...)
    {
        cout<<"Attempt to allocate memory failed!"<<endl;
        y_del1dp<T>(p);
        return;
    }
    return;
}

// histogram
template<class Tdata, class Tidx>
class HistogramLUT
{
public:
    HistogramLUT();
    ~HistogramLUT();

public:
    void initLUT(Tdata *p, Tidx sz, Tidx nbins);
    Tidx getIndex(Tdata val);

public:
    Tidx bins, index;
    Tdata *lut; // look up table
    double minv, maxv;
};

//
template<class T>
int distanceTransformL2(T *&dst, unsigned char *src, int x, int y, int z)
{
    // init
    Index3 size3D;

    size3D[0] = x;
    size3D[1] = y;
    size3D[2] = z;

    Grid<SizeType, 3> indices(size3D);
    DistanceTransform::initializeIndices(indices);

    Grid<T, 3> u3d(size3D);

    long i,j,k,offk,offj;

    // copy data
    for (k = 0; k < z; ++k)
    {
        offk = k*x*y;
        for (j = 0; j < y; ++j)
        {
            offj = offk + j*x;
            for (i = 0; i < x; ++i)
            {
                u3d[i][j][k] = src[offj+i];
            }
        }
    }

    u3d[0][0][0] = 0.0f;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    DistanceTransform::distanceTransformL2(u3d, u3d, false, std::thread::hardware_concurrency());
    std::cout << std::endl << size3D[0] << 'x' << size3D[1] << 'x' << size3D[2]
              << " distance function (concurrently) computed in: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
              << " ms. (with " << std::thread::hardware_concurrency() << " threads)." << std::endl;

    // copy result
    for (k = 0; k < z; ++k)
    {
        offk = k*x*y;
        for (j = 0; j < y; ++j)
        {
            offj = offk + j*x;
            for (i = 0; i < x; ++i)
            {
                dst[offj+i] = u3d[i][j][k];
            }
        }
    }

    //
    return 0;
}

//
int adaptiveThresholdMT(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r);
int adaptiveThreshold(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r);

//
int estimateIntensityThreshold(unsigned char *p, long size, float &thresh, int method=0);


#endif // _IMG_H_
