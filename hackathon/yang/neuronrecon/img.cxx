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

// adaptive thresholding (for dt)
// distance transform (estimate radius)
// max filtering (find local maxima)

int adaptiveThresholding(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r)
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

    // create memory buffers for the input and output
    boost::compute::buffer buffer_dst(context, volsz);
    boost::compute::buffer buffer_src(context, volsz);

    // create the program with the source
    boost::compute::program program = boost::compute::program::create_with_source(compute_source, context);

    // compile the program
    program.build();

    /// rotate
    // create the kernel
    boost::compute::kernel kernel_adaptive_thresholding(program, "adaptive_thresholding");

    // set the kernel arguments
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
