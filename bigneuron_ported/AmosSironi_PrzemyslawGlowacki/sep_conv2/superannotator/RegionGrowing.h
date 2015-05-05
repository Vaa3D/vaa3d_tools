#ifndef REGIONGROWING_H
#define REGIONGROWING_H

/**
 * Simple region growing algorithm
 */
#include "Matrix3D.h"
#include "SuperVoxeler.h"
#include <map>
#include <queue>

#include <QTime>

template<typename T>
void RegionGrow( Matrix3D<T> &img, const PixelInfoList &startPixels, T minVal, T maxVal, unsigned int maxRegionSize, PixelInfoList *pixListResult )
{
    QTime Tm; Tm.start();

    typedef std::map< unsigned int, PixelInfo >   MapType;
    MapType  pixList;

    for (unsigned int i=0; i < startPixels.size(); i++)
        pixList[ startPixels[i].index ] = startPixels[i];

    const int iWidth  = img.width();
    const int iHeight = img.height();
    const int iDepth  = img.depth();

#define ADD_NEIGHBOR( K, dx, dy, dz ) \
    do {    \
            int nx = (int)((K).x) + (int)(dx); \
            int ny = (int)((K).y) + (int)(dy); \
            int nz = (int)((K).z) + (int)(dz); \
            if ( nx >= iWidth ) break; \
            if ( ny >= iHeight ) break; \
            if ( nz >= iDepth ) break; \
            if ( nx < 0 ) break; \
            if ( ny < 0 ) break; \
            if ( nz < 0 ) break; \
            unsigned int _idx = img.coordToIdx(nx,ny,nz); \
            T _pixVal = img.data()[_idx];   \
            if ( (_pixVal < minVal) || (_pixVal > maxVal) ) \
                break; \
            if ( pixList.count( _idx ) > 0 ) break; \
            pixStack.push( PixelInfo( nx, ny, nz, _idx ) ); \
    } while(0)

#define ADD_6_NEIGHBORS( K )   \
    ADD_NEIGHBOR(K, 1, 0, 0); \
    ADD_NEIGHBOR(K,-1, 0, 0); \
    ADD_NEIGHBOR(K, 0, 1, 0); \
    ADD_NEIGHBOR(K, 0,-1, 0); \
    ADD_NEIGHBOR(K, 0, 0, 1); \
    ADD_NEIGHBOR(K, 0, 0,-1);

#define ADD_12_NEIGHBORS( K )   \
    ADD_6_NEIGHBORS(K); \
    ADD_NEIGHBOR(K, 1, 1, 0); \
    ADD_NEIGHBOR(K,-1, 1, 0); \
    ADD_NEIGHBOR(K, 1,-1, 0); \
    ADD_NEIGHBOR(K,-1,-1, 0); \
    ADD_NEIGHBOR(K, 1, 0, 1); \
    ADD_NEIGHBOR(K,-1, 0, 1); \
    ADD_NEIGHBOR(K, 1, 0,-1); \
    ADD_NEIGHBOR(K,-1, 0,-1); \
    ADD_NEIGHBOR(K, 0, 1, 1); \
    ADD_NEIGHBOR(K, 0,-1, 1); \
    ADD_NEIGHBOR(K, 0, 1,-1); \
    ADD_NEIGHBOR(K, 0,-1,-1); \
    ADD_NEIGHBOR(K, 1, 1, 1); \
    ADD_NEIGHBOR(K, 1,-1, 1); \
    ADD_NEIGHBOR(K, 1, 1,-1); \
    ADD_NEIGHBOR(K, 1,-1,-1); \
    ADD_NEIGHBOR(K,-1, 1, 1); \
    ADD_NEIGHBOR(K,-1,-1, 1); \
    ADD_NEIGHBOR(K,-1, 1,-1); \
    ADD_NEIGHBOR(K,-1,-1,-1);

    // begin pixel stack with neighbors
    std::queue< PixelInfo >  pixStack;
    for (unsigned int i=0; i < startPixels.size(); i++)
    {
        ADD_6_NEIGHBORS( startPixels[i].coords );
    }


    qDebug() << "Elapsed: " << Tm.elapsed();

    // now begin, until the stack is empty
    unsigned int Nn = 0;
    while( !pixStack.empty() )
    {
        Nn++;

        if ( (Nn % 5000) == 0 )
            qDebug() << "Nn: " << Nn << " / " << pixList.size();

        // copy and delete from stack
        PixelInfo pix = pixStack.front();
        pixStack.pop();

        // check if it meets the constraints
        //T pixVal = img.data()[pix.index];

        //if ( (pixVal < minVal) || (pixVal > maxVal))
            //continue;

        // then add to list
        pixList[ pix.index ] = pix;

        if ( pixList.size() >= maxRegionSize )
            break;

        ADD_6_NEIGHBORS( pix.coords );
    }

    qDebug() << "Elapsed2: " << Tm.elapsed();
    qDebug() << "N: " << Nn;

    // put  back in pixListResult
    MapType::const_iterator iter;
    pixListResult->clear();
    for (iter = pixList.begin(); iter != pixList.end(); ++iter)
        pixListResult->push_back( iter->second );

#undef ADD_NEIGHBOR
#undef ADD_6_NEIGHBORS
#undef ADD_12_NEIGHBORS
}

#endif // REGIONGROWING_H
