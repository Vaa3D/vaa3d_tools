#include "Colormap.h"
#include <QDebug>

#define COLORMAP_SIZE(x) (sizeof(x) / (sizeof(x[0]) * 3) )

static Colormap::Scalar colormapLines[] =
{
    0,     0,   255,
    0,   128,     0,
  255,     0,     0,
    0,   191,   191,
  191,     0,   191,
  191,   191,     0,
   64,    64,    64
};


Colormap::Colormap( ColormapType type )
{
    mType = type;
}

void Colormap::get( std::vector<itkRGBPixel> &list )
{
    list.clear();

    unsigned int colormapSize = 0;

    unsigned char *colormapPtr = 0;

    switch(mType)
    {
        case Lines:
            colormapPtr = colormapLines;
            colormapSize = COLORMAP_SIZE(colormapLines);
            break;
    }

    qDebug("Map size: %d", (int) colormapSize);

    list.resize( colormapSize );

    for (unsigned i=0; i < colormapSize; i++)
    {
        unsigned char R = colormapPtr[ 3*i + 0 ];
        unsigned char G = colormapPtr[ 3*i + 1 ];
        unsigned char B = colormapPtr[ 3*i + 2 ];
        list[i].Set( R, G, B );
    }
}
