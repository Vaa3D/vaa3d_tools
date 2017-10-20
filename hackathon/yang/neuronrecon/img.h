// image processing with GPUs
// -Yang
// 10/19/2017


#ifndef _IMG_H_
#define _IMG_H_

#include <boost/compute/core.hpp>
#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/type_traits/type_name.hpp>
#include <boost/compute/utility/source.hpp>

const char compute_source[] = BOOST_COMPUTE_STRINGIZE_SOURCE
        (
            //#pragma OPENCL EXTENSION cl_intel_printf : enable
            //#pragma OPENCL EXTENSION cl_amd_printf : enable

            __kernel void adaptive_thresholding(__global uchar* dst, __global uchar* src, int dimx, int dimy, int dimz, int r) {

                int i = get_global_id(0);
                int z = (i/(dimx*dimy))%dimz;
                int y = (i/dimx)%dimy;
                int x = i%dimx;
                int slicesz = dimx*dimy;

                if(z<r || z >=get_image_depth(src)-r || y<r || y>=get_image_height(src)-r || x<r || x>=get_image_width(src)-r)
                {
                    return;
                }

                for(int k=z-r; k<=z+r; k++)
                {
                    for(int j=y-r; j<=y+r; j++)
                    {
                        for(int i=x-r; i<=x+r; i++)
                        {
                            thresh += src[k*slicesz + j*dimx + i];
                        }
                    }
                }

                int volsz = (2*r+1)*(2*r+1)*(2*r+1);

                if(src[z*slicesz + y*dimx + x] > thresh/volsz)
                {
                    dst[z*slicesz + y*dimx + x] = 255;
                }
                else
                {
                    dst[z*slicesz + y*dimx + x] = 0;
                }
            }

            );

//
int adaptiveThresholding(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r);

//


#endif // _IMG_H_
