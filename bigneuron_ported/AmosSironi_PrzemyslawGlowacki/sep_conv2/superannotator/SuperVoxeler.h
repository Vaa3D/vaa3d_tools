#ifndef SUPERVOXELER_H
#define SUPERVOXELER_H

#include "Matrix3D.h"

#define fatalMsg(x) qFatal(x)
#include <slic/LKM.h>
#include <slic/mex/include/create_slicmap.hxx>
#include <slic/mex/include/find_neighbors.hxx>
#include <slic/mex/include/compute_histogram.hxx>
#undef fatalMsg


#ifdef _OPENMP
    #include <omp.h>
#endif

/**
 ** Class to ease the task of computing and using supervoxels
 *  So far only works with T == unsigned char, for compatibility with supervoxel library
 */
template<typename T>    // T is the input data type
class SuperVoxeler
{
public:
    typedef unsigned int    IDType; // supervoxel ID type

private:
    bool mIsEmpty;

    Matrix3D<IDType>    mPixelToVoxel;  // pixel to voxel ID 'table'
    SlicMapType         mVoxelToPixel;  // voxel ID to pixel 'table'

    std::vector< HistogramType >    mHistograms; // one histogram per supervoxel
    std::vector<float>              mMean;       // mean of a given svox

    unsigned int mNumLabels; //number of supervoxels


public:
    SuperVoxeler() { mIsEmpty = true; mNumLabels = 0; }

    inline bool empty() { return mIsEmpty; }

    unsigned int numLabels() const { return mNumLabels; }

    // generic, needs no instantiation
    static void rawGenSupervoxels( const Matrix3D<T> &img, int step, unsigned int cubeness, Matrix3D<IDType> *destination, unsigned int &_numLabels )
    {
        LKM* lkm = new LKM;

        sidType**   kLabels;
        int numLabels;

        lkm->DoSupervoxelSegmentationForGrayVolume(img.data(), img.width(), img.height(), img.depth(), kLabels, numLabels, step, cubeness);

        //qDebug("Num elem: %u", img.numElem());

        // now another waste.. copy labels back to a normal array
        destination->realloc( img.width(), img.height(), img.depth() );

        //qDebug("Size: %d %d %d", mPixelToVoxel.width(), mPixelToVoxel.height(), mPixelToVoxel.depth());


        unsigned int sz = img.width() * img.height();
        qDebug("Sz: %d", (int)sz);
        for (unsigned int z=0; z < img.depth(); z++)
        {
            unsigned int zOff = z * sz;
            memcpy( destination->data() + zOff, kLabels[z], sz*sizeof(unsigned int) );
        }

        // free kLabels
        for (unsigned int z=0; z < img.depth(); z++)
            delete[] kLabels[z];
        delete[] kLabels;

        delete lkm; // free lkm itself

        _numLabels = numLabels;
    }

    void apply( const Matrix3D<T> &img, int step, unsigned int cubeness )
    {
        rawGenSupervoxels( img, step, cubeness, &mPixelToVoxel, mNumLabels );

        qDebug("Num labels: %d", (int)mNumLabels);

        /** Compute the inverse map **/
        qDebug("Computing slic map");
        createSlicMap( mPixelToVoxel, mNumLabels, mVoxelToPixel );

        mIsEmpty = false;
    }

    // just saves volume, no other info
    bool save( const std::string &fName ) const {
        if (!mPixelToVoxel.save( fName ))
            return false;

        return true;
    }

    bool load( const std::string &fName ) {
        if (!mPixelToVoxel.load( fName ))
            return false;

        mHistograms.clear();
        mMean.clear();

        // compute number of labels
        mNumLabels = 0;
        for (unsigned int i=0; i < mPixelToVoxel.numElem(); i++) {
            if ( mPixelToVoxel.data()[i] > mNumLabels )
                mNumLabels = mPixelToVoxel.data()[i];
        }

        mNumLabels++;   // add 1 (zero-based index)

        qDebug("Computing slic map");
        createSlicMap( mPixelToVoxel, mNumLabels, mVoxelToPixel );

        return true;
    }

    // computes the histogram and mean of every supervoxel
    void computeSingleHistogramAndMean( const Matrix3D<T> &rawImg, HistogramOpts<T> hOpts )
    {
        mHistograms.resize( mNumLabels );
        mMean.resize( mNumLabels );

        #pragma omp parallel for schedule(dynamic)
        for (unsigned int sIdx=0; sIdx < mNumLabels; sIdx++)
        {
            mMean[sIdx] = computeHistogram(
                        PixelInfoListValueIterator<T>::begin( rawImg.data(), mVoxelToPixel[sIdx] ),
                        PixelInfoListValueIterator<T>::end( rawImg.data(), mVoxelToPixel[sIdx] ),
                        mHistograms[sIdx], hOpts, false, true );
        }
    }

    // warning, no range check!
    inline const std::vector< HistogramType > &histograms() {
        return mHistograms;
    }

    // warning: no range check
    inline const std::vector<float> & means() {
        return mMean;
    }

    #ifdef _OPENMP
    static void rawGenSupervoxelsMultithread( const Matrix3D<T> &img, int step, unsigned int cubeness, Matrix3D<IDType> *destination, unsigned int &_numLabels )
    {
        const int numThreads = omp_get_num_threads();

        qDebug("Using %d threads.", numThreads);

        // splitting per dimension
        const int dimSplit = 4;
        const int numSubVol = dimSplit*dimSplit*dimSplit;
        const int dimOverlap = step * 3;

        qDebug("Dividing in %d subvolumes.", numSubVol);

        unsigned int stepX = img.width() / dimSplit;
        unsigned int stepY = img.height() / dimSplit;
        unsigned int stepZ = img.depth() / dimSplit;


    }
    #endif


    const Matrix3D<IDType> & pixelToVoxel() const { return mPixelToVoxel; }
    const SlicMapType & voxelToPixel() const { return mVoxelToPixel; }
};

#endif // SUPERVOXELER_H
