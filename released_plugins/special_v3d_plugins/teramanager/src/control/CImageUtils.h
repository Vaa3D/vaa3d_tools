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


        /**********************************************************************************
        * QImage manipulation functions (brightness, contrast, gamma correction, colormap)
        ***********************************************************************************/
        static void applyVaa3DColorMap(QImage& image, RGBA8 cmap[4][256]);
        static void changeBrightness( QImage& image, int brightness );
        static void changeContrast(QImage &image, int contrast );
        static void changeGamma(QImage& image, int gamma );
        inline
        static int changeBrightness( int value, int brightness ){
            return kClamp( value + brightness * 255 / 100, 0, 255 );
        }
        inline
        static int changeContrast( int value, int contrast ){
            return kClamp((( value - 127 ) * contrast / 100 ) + 127, 0, 255 );
        }
        inline
        static int changeGamma( int value, int gamma ){
            return kClamp( int( pow( value / 255.0, 100.0 / gamma ) * 255 ), 0, 255 );
        }
        inline
        static int changeUsingTable( int value, const int table[] ){
            return table[ value ];
        }
        template< int operation( int, int ) >
        static
        void changeImage(QImage& image, int value );
        /*********************************************************************************/

        static inline RGBA8 vaa3D_color(itm::uint8 r, itm::uint8 g, itm::uint8 b){
            RGBA8 color;
            color.r = r;
            color.g = g;
            color.b = b;
            return color;
        }
};

#endif // CIMAGEUTILS_H
