// image processing with GPUs
// -Yang
// 10/19/2017

#include "img.h"

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

#include "distance_transform.hpp"

using namespace dt;
using namespace dope;


// adaptive thresholding (for dt)
// distance transform (estimate radius)
// max filtering (find local maxima)

int adaptiveThreshold(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r)
{
    std::vector<boost::compute::platform> platforms = boost::compute::system::platforms();
    int gpucnt=0; // more than one GPU
    boost::compute::device gpu = boost::compute::system::default_device();
    for(size_t itp = 0; itp < platforms.size(); itp++){
        const boost::compute::platform &platform = platforms[itp];
        std::cout << "Platform '" << platform.name() << "'" << std::endl;
        std::vector<boost::compute::device> devices = platform.devices();
        for(size_t itd = 0; itd < devices.size(); itd++){
            const boost::compute::device &device = devices[itd];
            if(device.type() & boost::compute::device::gpu)
            {
                gpucnt++;
                if(gpucnt>=1)
                {
                    if(gpu.name().compare(device.name()) != 0)
                    {
                        gpu = device;
                    }
                }
            }
            else if(device.type() & boost::compute::device::cpu)
            {
                // cpu
            }
            else if(device.type() & boost::compute::device::accelerator)
            {
                // accelerator
            }
            else
            {
                // unknown
            }
        }
    }

    boost::compute::context context(gpu);
    boost::compute::command_queue queue(context, gpu);

    std::cout << "device: " << gpu.name() << " " << gpu.version() <<" w/ driver version: "<< gpu.driver_version()<< std::endl;
    std::cout << "  global memory size: "
              << gpu.get_info<cl_ulong>(CL_DEVICE_GLOBAL_MEM_SIZE) / 1024 / 1024
              << " MB"
              << std::endl;
    std::cout << "  local memory size: "
              << gpu.get_info<cl_ulong>(CL_DEVICE_LOCAL_MEM_SIZE) / 1024
              << " KB"
              << std::endl;
    std::cout << "  constant memory size: "
              << gpu.get_info<cl_ulong>(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE) / 1024
              << " KB"
              << std::endl;

    //
    int volsz = x*y*z;

    cout<<" create memory buffers for the input and output \n";
    boost::compute::buffer buffer_dst(context, volsz);
    boost::compute::buffer buffer_src(context, volsz);

    cout<<" create the program with the source \n";
    boost::compute::program program = boost::compute::program::create_with_source(compute_source, context);

    cout<<" compile the program \n";
    program.build();

    cout<<" create the kernel \n";
    boost::compute::kernel kernel_adaptive_thresholding(program, "adaptive_thresholding");

    cout<<" set the kernel arguments \n";
    kernel_adaptive_thresholding.set_arg(0, buffer_dst);
    kernel_adaptive_thresholding.set_arg(1, buffer_src);
    kernel_adaptive_thresholding.set_arg(2, x);
    kernel_adaptive_thresholding.set_arg(3, y);
    kernel_adaptive_thresholding.set_arg(4, z);
    kernel_adaptive_thresholding.set_arg(5, r);

    // write the data from 'dst' and 'src' to the device
    queue.enqueue_write_buffer(buffer_dst, 0, volsz, dst);
    queue.enqueue_write_buffer(buffer_src, 0, volsz, src);

    // run the kernel
    queue.enqueue_1d_range_kernel(kernel_adaptive_thresholding, 0, volsz, 0);

    // transfer result back to the host array 'dst'
    queue.enqueue_read_buffer(buffer_dst, 0, volsz, dst);

    //
    return 0;
}

int distanceTransformL2(unsigned char *&dst, unsigned char *src, int x, int y, int z)
{
    // init
    Index3 size3D;

    size3D[0] = x;
    size3D[1] = y;
    size3D[2] = z;

    Grid<SizeType, 3> indices(size3D);
    DistanceTransform::initializeIndices(indices);

    Grid<unsigned char, 3> u3d(size3D);

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
