// image processing with GPUs
// -Yang
// 10/19/2017

#include "img.h"

// adaptive thresholding (for dt)
// distance transform (estimate radius)
// max filtering (find local maxima)


// histogram
template<class Tdata, class Tidx>
HistogramLUT<Tdata, Tidx> :: HistogramLUT()
{
    bins=0; index=0; lut=NULL;
}

template<class Tdata, class Tidx>
HistogramLUT<Tdata, Tidx> :: ~HistogramLUT()
{
}

template<class Tdata, class Tidx>
void HistogramLUT<Tdata, Tidx> :: initLUT(Tdata *p, Tidx sz, Tidx nbins)
{
    //
    if(!p || nbins<=0)
    {
        cout<<"Invalid inputs"<<endl;
        return;
    }

    //
    bins = nbins;
    y_new1dp<Tdata, Tidx>(lut, bins);

    // histogram bin #i [minv+i*stepv, minv+(i+1)*stepv)
    minv=1E10;
    maxv=-1E10;
    double stepv=0;
    for(Tidx i=0; i<sz; i++)
    {
        if(minv>p[i]) minv=p[i];
        if(maxv<p[i]) maxv=p[i];
    }
    stepv = (maxv - minv)/(double)bins;

    for(Tidx i=0; i<bins; i++)
    {
        lut[i] = minv + i*stepv; // only left values
    }

    return;
}

template<class Tdata, class Tidx>
Tidx HistogramLUT<Tdata, Tidx> :: getIndex(Tdata val)
{
    //
    Tidx min=0;
    Tidx max=bins-1;

    // binary search
    bool found = false;
    index=0;
    while(min<max && !found)
    {
        Tidx mid=(min+max)/2;

        if(val == lut[mid] || (val > lut[mid] && y_abs<Tdata>(val - lut[mid])<1E-10) ) // =
        {
            found = true;
            index = mid;
        }
        else if(val < lut[mid]) // <
        {
            max = mid - 1;
        }
        else // >
        {
            if(mid+1>=max)
            {
                found = true;
                index = mid;
            }
            else
            {
                min = mid + 1;
            }
        }
    }

    //
    return index;
}

int adaptiveThresholdMT(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r)
{
//    std::vector<boost::compute::platform> platforms = boost::compute::system::platforms();
//    int gpucnt=0; // more than one GPU
//    boost::compute::device gpu = boost::compute::system::default_device();
//    for(size_t itp = 0; itp < platforms.size(); itp++){
//        const boost::compute::platform &platform = platforms[itp];
//        std::cout << "Platform '" << platform.name() << "'" << std::endl;
//        std::vector<boost::compute::device> devices = platform.devices();
//        for(size_t itd = 0; itd < devices.size(); itd++){
//            const boost::compute::device &device = devices[itd];
//            if(device.type() & boost::compute::device::gpu)
//            {
//                gpucnt++;
//                if(gpucnt>=1)
//                {
//                    if(gpu.name().compare(device.name()) != 0)
//                    {
//                        gpu = device;
//                    }
//                }
//            }
//            else if(device.type() & boost::compute::device::cpu)
//            {
//                // cpu
//            }
//            else if(device.type() & boost::compute::device::accelerator)
//            {
//                // accelerator
//            }
//            else
//            {
//                // unknown
//            }
//        }
//    }

//    boost::compute::context context(gpu);
//    boost::compute::command_queue queue(context, gpu);

//    std::cout << "device: " << gpu.name() << " " << gpu.version() <<" w/ driver version: "<< gpu.driver_version()<< std::endl;
//    std::cout << "  global memory size: "
//              << gpu.get_info<cl_ulong>(CL_DEVICE_GLOBAL_MEM_SIZE) / 1024 / 1024
//              << " MB"
//              << std::endl;
//    std::cout << "  local memory size: "
//              << gpu.get_info<cl_ulong>(CL_DEVICE_LOCAL_MEM_SIZE) / 1024
//              << " KB"
//              << std::endl;
//    std::cout << "  constant memory size: "
//              << gpu.get_info<cl_ulong>(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE) / 1024
//              << " KB"
//              << std::endl;

//    //
//    int volsz = x*y*z;

//    cout<<" create memory buffers for the input and output \n";
//    boost::compute::buffer buffer_dst(context, volsz);
//    boost::compute::buffer buffer_src(context, volsz);

//    cout<<" create the program with the source \n";
//    boost::compute::program program = boost::compute::program::create_with_source(compute_source, context);

//    cout<<" compile the program \n";
//    program.build();

//    cout<<" create the kernel \n";
//    boost::compute::kernel kernel_adaptive_thresholding(program, "adaptive_thresholding");

//    cout<<" set the kernel arguments \n";
//    kernel_adaptive_thresholding.set_arg(0, buffer_dst);
//    kernel_adaptive_thresholding.set_arg(1, buffer_src);
//    kernel_adaptive_thresholding.set_arg(2, x);
//    kernel_adaptive_thresholding.set_arg(3, y);
//    kernel_adaptive_thresholding.set_arg(4, z);
//    kernel_adaptive_thresholding.set_arg(5, r);

//    // write the data from 'dst' and 'src' to the device
//    queue.enqueue_write_buffer(buffer_dst, 0, volsz, dst);
//    queue.enqueue_write_buffer(buffer_src, 0, volsz, src);

//    // run the kernel
//    queue.enqueue_1d_range_kernel(kernel_adaptive_thresholding, 0, volsz, 0);

//    // transfer result back to the host array 'dst'
//    queue.enqueue_read_buffer(buffer_dst, 0, volsz, dst);

    //
    return 0;
}

int adaptiveThreshold(unsigned char *&dst, unsigned char *src, int x, int y, int z, int r)
{
    //
    cout<<"adaptive searching radius ... "<<r<<endl;

    //
    float globalthresh, thresh;
    long i,j,k;
    long idx;

    long xs, xe, ys, ye, zs, ze;

    unsigned char *block = NULL;
    long xb, yb, zb, szblock;
    long ii, jj, kk;

    //
    estimateIntensityThreshold(src, x*y*z, globalthresh);

    //
    for(k=0; k<z; k+=r)
    {
        zs = k;
        ze = zs+r;

        if(ze>z-1)
            ze = z - 1;

        zb = ze - zs;

        for(j=0; j<y; j+=r)
        {
            ys = j;
            ye = ys+r;

            if(ye > y-1)
                ye = y - 1;

            yb = ye - ys;

            for(i=0; i<x; i+=r)
            {
                xs = i;
                xe = xs + r;

                if(xe > x-1)
                    xe = x-1;

                xb = xe - xs;

                //
                szblock = xb*yb*zb;
                if(szblock)
                {
                    try
                    {
                        block = new unsigned char [szblock];

                        for(kk=zs; kk<ze; kk++)
                        {
                            for(jj=ys; jj<ye; jj++)
                            {
                                for(ii=xs; ii<xe; ii++)
                                {
                                    block[(kk-zs)*yb*xb + (jj-ys)*xb + (ii-xs)] = src[kk*y*x + jj*x + ii];
                                }
                            }
                        }

                        estimateIntensityThreshold(block, szblock, thresh);
                        thresh = max(thresh, globalthresh);

                        for(kk=zs; kk<ze; kk++)
                        {
                            for(jj=ys; jj<ye; jj++)
                            {
                                for(ii=xs; ii<xe; ii++)
                                {
                                    idx = kk*y*x + jj*x + ii;
                                    if(src[idx]>thresh)
                                    {
                                        dst[idx] = src[idx];
                                    }
                                }
                            }
                        }
                    }
                    catch(...)
                    {
                        cout<<"fail to alloc memory\n";
                        return -1;
                    }

                    if(block)
                    {
                        delete []block;
                        block = NULL;
                    }
                }
                else
                {
                    continue;
                }

            }// i
        }// j
    }// k

    //
    return 0;
}

int estimateIntensityThreshold(unsigned char *p, long size, float &thresh, int method)
{
    // method 0: k-means 1: mean + stddev

    if(!p || size<2)
    {
        cout<<"NULL input \n";
        return -1;
    }

    //
    if(method==0)
    {
        long BINS = 256; // histogram bins

        long *h=NULL, *hc=NULL;

        try
        {
            h = new long [BINS];
            hc = new long [BINS];
        }
        catch(...)
        {
            cout<<"fail to alloc memory\n";
            return -1;
        }

        memset(h, 0, sizeof(long)*BINS);

        // histogram
        HistogramLUT<unsigned char, long> hlut;
        hlut.initLUT(p, size, BINS);

        for(long i=0; i<size; i++)
        {
            h[hlut.getIndex(p[i])] ++;
        }

        // heuristic init center
        float mub=0.05*(hlut.maxv - hlut.minv) + hlut.minv;
        float muf=0.30*(hlut.maxv - hlut.minv) + hlut.minv;

        //
        while (true)
        {
            float oldmub=mub, oldmuf=muf;

            for(long i=0; i<BINS; i++)
            {
                if(h[i]==0)
                    continue;

                float cb = y_abs<float>(float(hlut.lut[i])-mub);
                float cf = y_abs<float>(float(hlut.lut[i])-muf);

                hc[i] = (cb<=cf)?1:2; // class 1 and class 2
            }

            // update centers
            float sum_b=0, sum_bw=0, sum_f=0, sum_fw=0;

            for(long i=0; i<BINS; i++)
            {
                if(h[i]==0)
                    continue;

                if(hc[i]==1)
                {
                    sum_bw += (i+1)*h[i];
                    sum_b += h[i];
                }
                else if(hc[i]==2)
                {
                    sum_fw += (i+1)*h[i];
                    sum_f += h[i];
                }
            }

            if(sum_b)
                mub = hlut.lut[ long(sum_bw/sum_b) ];
            if(sum_f)
                muf = hlut.lut[ long(sum_fw/sum_f) ];

            if(y_abs<float>(mub - oldmub)<1 && y_abs<float>(muf - oldmuf)<1)  break;
        }

        //
        thresh = (mub+muf)/2;

        //cout<<"k-means estimates the threshold is ... "<<thresh<<endl;
    }
    else if(method==1)
    {
        //
        float mean, stddev;

        //
        float sum = 0;
        for(long i=0; i<size; i++)
        {
            sum += p[i];
        }

        //
        mean = sum / size;

        //
        sum = 0;
        for(int i=0; i<size; i++)
        {
            sum += (p[i] - mean)*(p[i] - mean);
        }

        stddev = sqrt(sum/(size-1));

        //
        thresh =  mean + stddev;

        cout<<"the threshold (mean + stddev) is ... "<<thresh<<endl;

    }
    else
    {

    }

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
