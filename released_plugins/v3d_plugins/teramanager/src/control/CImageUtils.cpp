#include "CImageUtils.h"

using namespace teramanager;

/**********************************************************************************
* Copies the given VOI from "src" to "dst". Offsets and scaling are supported.
***********************************************************************************/
void
    CImageUtils::copyVOI(
        itm::uint8 const * src,     //pointer to const data source
        uint src_dims[5],           //dimensions of "src" along X, Y, Z, channels and T
        uint src_offset[5],         //VOI's offset along X, Y, Z, <empty> and T
        uint src_count[5],          //VOI's dimensions along X, Y, Z, <empty> and T
        itm::uint8* dst,            //pointer to data destination
        uint dst_dims[5],           //dimensions of "dst" along X, Y, Z, channels and T
        uint dst_offset[5],         //offset of "dst" along X, Y, Z, <empty> and T
        uint scaling /*= 1 */)      //scaling factor (integer only)
throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("src_dims = (%d x %d x %d x %d x %d), src_offset = (%d, %d, %d, %d, %d), src_count = (%d, %d, %d, %d, %d), dst_dims = (%d x %d x %d x %d x %d), dst_offset = (%d, %d, %d, %d, %d), scaling = %d",
                                        src_dims[0], src_dims[1],src_dims[2],src_dims[3],src_dims[4], src_offset[0],src_offset[1],src_offset[2],src_offset[3], src_offset[4],src_count[0],src_count[1],src_count[2],src_count[3], src_count[4],
                                        dst_dims[0], dst_dims[1],dst_dims[2],dst_dims[3],dst_dims[4], dst_offset[0],dst_offset[1],dst_offset[2],dst_offset[3], dst_offset[4],scaling).c_str(), __itm__current__function__);

    //if source and destination are the same thing, returning without doing anything
    if(src == dst)
        return;

    //cheking preconditions
    if(src_dims[3] != dst_dims[3])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: source has %d channels, destination has %d", src_dims[3], dst_dims[3]).c_str());
    for(int d=0; d<3; d++)
    {
        if(src_offset[d] + src_count[d] > src_dims[d])
            throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds image size (%u) along axis %d", src_offset[d], src_offset[d] + src_count[d], src_dims[d], d).c_str());
        if(dst_offset[d] + src_count[d]*scaling > dst_dims[d])
        {
            //cutting copiable VOI to the largest one that can be stored into the destination image
            int old = src_count[d];
            src_count[d] = (dst_dims[d] - dst_offset[d]) / scaling; //it's ok to approximate this calculation to the floor.


            itm::warning(strprintf("--------------------- teramanager plugin [thread *] !! WARNING in copyVOI !! VOI exceeded destination dimension along axis %d, then cutting VOI from %d to %d\n",
                   d, old, src_count[d]).c_str());
        }
    }
    if(src_offset[4] + src_count[4] > src_dims[4])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds source image size (%u) along T axis", src_offset[4], src_offset[4] + src_count[4], src_dims[4]).c_str());
    if(dst_offset[4] + src_count[4] > dst_dims[4])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds destination image size (%u) along T axis", dst_offset[4], dst_offset[4] + src_count[4], dst_dims[4]).c_str());


    //quick version (with precomputed offsets, strides and counts: "1" for "dst", "2" for "src")
    const uint64 stride_t1 =                dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0]  * (uint64)1;
    const uint64 offset_t1 =  dst_offset[4]*dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0]  * (uint64)1;
    const uint64 count_t1  =  src_count[4] *dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0]  * (uint64)1;
    const uint64 stride_t2 =                src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 offset_t2 =  src_offset[4]*src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 stride_c1 =                               dst_dims [2] * dst_dims[1]   * dst_dims[0]  * (uint64)1;
    const uint64 stride_c2 =                               src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 count_c1  =                dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0]  * (uint64)1;
    const uint64 stride_k1 =                                              dst_dims[1]   * dst_dims[0]  * (uint64) scaling;
    const uint64 count_k1  =                               src_count[2] * dst_dims[1]   * dst_dims[0]  * (uint64) scaling;
    const uint64 offset_k2 =                               src_offset[2]* src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 stride_k2 =                                              src_dims[1]   * src_dims[0]  * (uint64)1;
    //const uint64 count_k2=                               src_count[2] * src_dims[1]   * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint64 stride_i1 =                                                              dst_dims[0]  * (uint64) scaling;
    const uint64 count_i1  =                                              src_count[1]  * dst_dims[0]  * (uint64) scaling;
    const uint64 offset_i2 =                                              src_offset[1] * src_dims[0]  * (uint64)1;
    const uint64 stride_i2 =                                                              src_dims[0]  * (uint64)1;
    //const uint64 count_i2  =                                            src_count[1]  * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint64 stride_j1 =                                                                             (uint64) scaling;
    const uint64 count_j1  =                                                              src_count[0] * (uint64) scaling;
    const uint64 offset_j2 =                                                              src_offset[0]* (uint64)1;
    //const uint64 count_j2  =                                                            src_count[0];           //not used in the OR so as to speed up the inner loops
//    const uint64 dst_dim =    dst_dims [4]* dst_dims [3] * dst_dims [2] * dst_dims [1] *  dst_dims [0] * (uint64)1;;

    for(int sk = 0; sk < scaling; sk++)
        for(int si = 0; si < scaling; si++)
            for(int sj = 0; sj < scaling; sj++)
            {
                const uint64 offset_k1 = dst_offset[2] * dst_dims[1]    * dst_dims[0]   * scaling + sk * dst_dims[1] * dst_dims[0] ;
                const uint64 offset_i1 =                 dst_offset[1]  * dst_dims[0]   * scaling + si * dst_dims[0];
                const uint64 offset_j1 =                                  dst_offset[0] * scaling + sj;

                uint8* const start_t1 = dst + offset_t1;
                uint8* const start_t2 = const_cast<uint8*>(src) + offset_t2;
                for(uint8 *img_t1 = start_t1, *img_t2 = start_t2; img_t1 - start_t1 < count_t1; img_t1 += stride_t1, img_t2 += stride_t2)
                {
                    for(uint8 *img_c1 = img_t1, *img_c2 = img_t2; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, img_c2 += stride_c2)
                    {
                        uint8* const start_k1 = img_c1 + offset_k1;
                        uint8* const start_k2 = img_c2 + offset_k2;
                        for(uint8 *img_k1 = start_k1, *img_k2 = start_k2; img_k1 - start_k1  < count_k1; img_k1 += stride_k1, img_k2 += stride_k2)
                        {
                            uint8* const start_i1 = img_k1 + offset_i1;
                            uint8* const start_i2 = img_k2 + offset_i2;
                            for(uint8 *img_i1 = start_i1, *img_i2 = start_i2; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, img_i2 += stride_i2)
                            {
                                uint8* const start_j1 = img_i1 + offset_j1;
                                uint8* const start_j2 = img_i2 + offset_j2;
                                for(uint8 *img_j1 = start_j1, *img_j2 = start_j2; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, img_j2++)
                                    *img_j1 = *img_j2;
                            }
                        }
                    }
                }
             }

    /**/itm::debug(itm::LEV3, "copy VOI finished",  __itm__current__function__);
}

/**********************************************************************************
* Returns the Maximum Intensity Projection of the given ROI in a newly allocated array.
***********************************************************************************/
itm::uint8*
    CImageUtils::mip(itm::uint8 const * src,    //pointer to const data source
                        uint src_dims[5],       //dimensions of "src" along X, Y, Z, channels and T
                        uint src_offset[5],     //VOI's offset along X, Y, Z, <empty> and T
                        uint src_count[5],      //VOI's dimensions along X, Y, Z, <empty> and T
                        itm::direction dir,     //direction of projection
                        bool align32 /*=false*/)//true if mip data must be 32-bit aligned
   throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("src_dims = (%u x %u x %u x %u x %u), src_offset = (%u, %u, %u, %u, %u), src_count = (%u, %u, %u, %u, %u), direction = %d",
                                        src_dims[0], src_dims[1],src_dims[2],src_dims[3],src_dims[4], src_offset[0],src_offset[1],src_offset[2],src_offset[3],
                                        src_offset[4],src_count[0],src_count[1],src_count[2],src_count[3], src_count[4], dir).c_str(), __itm__current__function__);


    // precondition checks
    if(src_count[4] != 1)
        throw itm::RuntimeException("Maximum Intensity Projection is not supported on 5D data yet.");
    if(src_dims[3] > 3)
        throw itm::RuntimeException("Maximum Intensity Projection is not supported on real 4D data yet.");
    if(dir != itm::z)
        throw itm::RuntimeException("Maximum Intensity Projection is supported along Z only.");
    for(int d=0; d<5; d++)
    {
        if(d != 3 && (src_offset[d] + src_count[d] > src_dims[d]))
            throw itm::RuntimeException(itm::strprintf("Can't compute MIP from the selected VOI: VOI [%u, %u) exceeds image size (%u) along axis %d.", src_offset[d], src_offset[d] + src_count[d], src_dims[d], d).c_str());
        else if(d != 3 && src_offset[d] >= src_dims[d])
            throw itm::RuntimeException(itm::strprintf("Can't compute MIP from the selected VOI: invalid offset (%u) compared to image size (%u) along axis %d.", src_offset[d], src_dims[d], d).c_str());
    }

    // source strides
    const uint64 stride_t1 =                src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 count_t1  =  src_count[4] *src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 offset_t1 =  src_offset[4]*src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 stride_c1 =                               src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 count_c1  =                src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 stride_k1 =                                              src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 count_k1  =                               src_count[2] * src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 offset_k1 =                               src_offset[2]* src_dims[1]   * src_dims[0]  * (uint64)1;
    const uint64 stride_i1 =                                                              src_dims[0]  * (uint64)1;
    const uint64 count_i1  =                                              src_count[1]  * src_dims[0]  * (uint64)1;
    const uint64 offset_i1 =                                              src_offset[1] * src_dims[0]  * (uint64)1;
    const uint64 stride_j1 =                                                                             (uint64)1;
    const uint64 count_j1  =                                                              src_count[0] * (uint64)1;
    const uint64 offset_j1 =                                                              src_offset[0]* (uint64)1;

    // z-mip strides
    const uint64 stride_c2 =                                              src_count[1]  * src_count[0] * (uint64)1;
    const uint64 count_c2  =                src_dims [3]                * src_count[1]  * src_count[0] * (uint64)1;
    const uint64 stride_i2 =                                                              src_count[0] * (uint64)1;

    // mip size
    uint64 mip_size = align32 ? src_count[1]*src_count[0]*(uint64)4 : count_c2;

    // allocation and initialization
    uint8* mip = new uint8[mip_size];
    for(int i = 0; i < mip_size; i++)
        mip[i] = 0;

    // define utility pointers
    uint8 const *start_t1, *img_t1, *img_c1, *start_k1, *img_k1, *start_i1, *img_i1, *start_j1, *img_j1 = 0;
    uint8 *img_c2, *img_i2, *img_j2 = 0;

    // mip
    if(align32)
    {
        for(start_t1 = src + offset_t1, img_t1 = start_t1; img_t1 - start_t1 < count_t1; img_t1 += stride_t1)
        {
            int c = 0;
            for(img_c1 = img_t1; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, c++)
            {
                for(start_k1 = img_c1 + offset_k1, img_k1 = start_k1; img_k1 - start_k1  < count_k1; img_k1 += stride_k1)
                {
                    int y = 0;
                    for(start_i1 = img_k1 + offset_i1, img_i1 = start_i1; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, y++)
                    {
                        int x=0;
                        uint64 y_stride = y*src_count[0]*4;
                        for(start_j1 = img_i1 + offset_j1, img_j1 = start_j1; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, x++)
                        {
                            uint64 offset = y_stride + x*4;
                            mip[offset + c] = std::max(*img_j1, mip[offset + c]);
                        }
                    }
                }
            }
        }

        // if grayscale, convert to RGBA
        if(src_dims[3] == 1)
        {
            for(int y=0; y<src_count[1]; y++)
                for(int x=0; x<src_count[0]; x++)
                {
                    mip[y*src_count[0]*4 + x*4 + 1] = mip[y*src_count[0]*4 + x*4 + 0];
                    mip[y*src_count[0]*4 + x*4 + 2] = mip[y*src_count[0]*4 + x*4 + 0];
                }
        }
    }
    else
    {
        for(start_t1 = src + offset_t1, img_t1 = start_t1; img_t1 - start_t1 < count_t1; img_t1 += stride_t1)
        {
            for(img_c1 = img_t1, img_c2 = mip; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, img_c2 += stride_c2)
            {
                for(start_k1 = img_c1 + offset_k1, img_k1 = start_k1; img_k1 - start_k1  < count_k1; img_k1 += stride_k1)
                {
                    for(start_i1 = img_k1 + offset_i1, img_i1 = start_i1, img_i2 = img_c2; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, img_i2 += stride_i2)
                    {
                        for(start_j1 = img_i1 + offset_j1, img_j1 = start_j1, img_j2 = img_i2; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, img_j2++)
                            *img_j2 = std::max(*img_j1, *img_j2);
                    }
                }
            }
        }
    }

//    else
//    {
//        // allocation and initialization
//        uint64 mip_size = src_count[1]   * src_count[0]  * (uint64)4;
//        mip = new uint8[mip_size];
//        for(int i = 0; i < mip_size; i++)
//            mip[i] = 0;

//        for(int y=0; y<src_count[1]; y++)
//            for(int x=0; x<src_count[0]; x++)
//            {
//                int max = 0;
//                for(int z=src_offset[2]; z<src_offset[2]+src_count[2]; z++)
//                    max = std::max(max, src[]);

//                mip[y*src_count[0] + x*4 + 0] = 0;
//                mip[y*src_count[0] + x*4 + 1] = 0;
//                mip[y*src_count[0] + x*4 + 2] = 0;
//                mip[y*src_count[0] + x*4 + 3] = 0;
//            }
//    }

    return mip;
}
