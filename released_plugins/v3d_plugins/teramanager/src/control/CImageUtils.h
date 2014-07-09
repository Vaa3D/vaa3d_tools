#ifndef CIMAGEUTILS_H
#define CIMAGEUTILS_H

#include "CPlugin.h"

class teramanager::CImageUtils
{
    private:

        CImageUtils();  // no constructors available. This class contains static methods only.

    public:

        /**********************************************************************************
        * Copies the given VOI from "src" to "dst". Offsets and scaling are supported.
        ***********************************************************************************/
        static void
            copyVOI(itm::uint8 const * src, //pointer to const data source
                uint src_dims[5],           //dimensions of "src" along X, Y, Z, channels and T
                uint src_offset[5],         //VOI's offset along X, Y, Z, <empty> and T
                uint src_count[5],          //VOI's dimensions along X, Y, Z, <empty> and T
                itm::uint8* dst,            //pointer to data destination
                uint dst_dims[5],           //dimensions of "dst" along X, Y, Z, channels and T
                uint dst_offset[5],         //offset of "dst" along X, Y, Z, <empty> and T
                uint scaling = 1)           //scaling factor (integer only)
        throw (itm::RuntimeException);

        /**********************************************************************************
        * Returns the Maximum Intensity Projection of the given VOI in a newly allocated array.
        ***********************************************************************************/
        static itm::uint8*
            mip(itm::uint8 const * src,     //pointer to const data source
                   uint src_dims[5],        //dimensions of "src" along X, Y, Z, channels and T
                   uint src_offset[5],      //VOI's offset along X, Y, Z, <empty> and T
                   uint src_count[5],       //VOI's dimensions along X, Y, Z, <empty> and T
                   itm::direction dir,      //direction of projection
                   bool to_BGRA = false,    //true if mip data must be stored into BGRA format
                   itm::uint8 alpha = 255)  //alpha transparency (used if to_BGRA = true)
           throw (itm::RuntimeException);
};

#endif // CIMAGEUTILS_H
