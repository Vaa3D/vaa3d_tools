#ifndef COLORMAP_H
#define COLORMAP_H

#include <itkRGBPixel.h>
#include <vector>

class Colormap
{
public:
    typedef unsigned char            Scalar;
    typedef itk::RGBPixel<Scalar>    itkRGBPixel;
    typedef std::vector<itkRGBPixel> itkRGBPixelVector;

    enum ColormapType
    {
        Lines
    };

    // creates a colormap with the requested type
    Colormap( ColormapType type );

    // get the colormap as a vector of RGB Pixels
    void get( std::vector<itkRGBPixel> &list );

private:
    ColormapType    mType;
};

#endif // COLORMAP_H
