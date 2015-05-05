#include <vector>
#include <string>
#include "mex.h"
#include <string.h>

#define fatalMsg(x) mexErrMsgTxt(x)

#include "include/create_slicmap.hxx"
#include "include/compute_histogram.hxx"
#include "include/find_neighbors.hxx"
#include "include/matlab_utils.hxx"

//#include "utils.h"

using namespace std;

/**
 * Creates two histograms for each supervoxel. The first one with the supervoxel itself
 * and the second one with the neighbor supervoxels.
 *
 * Histograms are NOT normalized.
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 6)
      mexErrMsgTxt("Incorrect input format: [ownHist, neighborHist] = slic_genhistograms( image, labelImage, numLabels, hMinVal, hMaxVal, nBins )\n");

  if (nlhs != 2) {
      mexErrMsgTxt("Two output arguments required.");
  }

#define mImg (prhs[0])
#define mLabelImg (prhs[1])
#define mNLabels (prhs[2])
#define mMinVal (prhs[3])
#define mMaxVal (prhs[4])
#define mNBins (prhs[5])

  typedef unsigned char ImageDataType;

  if (mxGetClassID(mImg) != matlabClassID<ImageDataType>())
      mexErrMsgTxt("Image must be UINT8 class");

  if ( mxGetNumberOfDimensions(mImg) != 3 )
      mexErrMsgTxt("Image must be 3-dimensional");

  MatlabInputMatrix<unsigned int> pNumLabels( mNLabels, 1, 1, "numLabels" );
  MatlabInputMatrix<ImageDataType>    pMinVal( mMinVal, 1, 1, "minVal" );
  MatlabInputMatrix<ImageDataType>    pMaxVal( mMaxVal, 1, 1, "maxVal" );
  MatlabInputMatrix<unsigned int>    pNBins( mNBins, 1, 1, "nBins" );

  if ( mxGetNumberOfDimensions(mLabelImg) != 3 )
      mexErrMsgTxt("labelImg must be 3-dimensional");

  if (mxGetClassID(mLabelImg) != mxUINT32_CLASS)
      mexErrMsgTxt("Label image must be UINT32 class");


  // check that label image and image have same dimensions
  {
      const mwSize *mImgDims = mxGetDimensions(mImg);
      const mwSize *mLabelImgDims = mxGetDimensions(mLabelImg);

      for (int i=0; i < mxGetNumberOfDimensions(mImg); i++)
          if ( mImgDims[i] != mLabelImgDims[i] )
              mexErrMsgTxt("Image dimensions different from label image dimensions.");
  }

  const unsigned int numLabels = pNumLabels.data(0);


  // ---------------------------------------
  mexPrintf("Computing SLIC Map\n");
  mexEvalString("drawnow");
  SlicMapType slicMap;
  createSlicMap( mLabelImg, numLabels, slicMap );

  HistogramOpts<ImageDataType> hOpts;
  hOpts.begin = pMinVal.data(0);
  hOpts.end =   pMaxVal.data(0);
  hOpts.nbins = pNBins.data(0);

  mexPrintf("Histogram options: <%d %d %d>\n", hOpts.begin, hOpts.end, hOpts.nbins);
  mexEvalString("drawnow");

  ImageDataType *imgData = (ImageDataType *) mxGetData(mImg);

  MatlabOutputMatrix< HistogramType::value_type >  pOwnHist( &plhs[0], hOpts.nbins, numLabels );
  MatlabOutputMatrix< HistogramType::value_type >  pNbHist( &plhs[1], hOpts.nbins, numLabels );



  // --------------------------------------- compute supervoxel neighbours
  mexPrintf("Computing neighbors\n");
  mexEvalString("drawnow");
  NeighborListType	neighborList;
  findNeighbors<false>( mLabelImg, numLabels, neighborList );

  // prepare pointers
  HistogramType::value_type *ptrOwnHist = pOwnHist.data();
  HistogramType::value_type *ptrNbHist = pNbHist.data();

  // size in bytes of a given histogram
  const unsigned int histogramByteCount = sizeof( HistogramType::value_type ) * hOpts.nbins;


  // ---------------------------------------
  mexPrintf("Computing histograms\n");
  mexEvalString("drawnow");

  // now go through supervoxels and compute histograms
  #pragma omp parallel for schedule(dynamic)
  for (unsigned int sIdx=0; sIdx < numLabels; sIdx++)
  {
      HistogramType hist;

      // -------------- First do this supervoxel
      computeHistogram(
                  PixelInfoListValueIterator<ImageDataType>::begin( imgData, slicMap[sIdx] ),
                  PixelInfoListValueIterator<ImageDataType>::end( imgData, slicMap[sIdx] ),
                  hist, hOpts );

      // copy to output array
      memcpy( ptrOwnHist + hOpts.nbins * sIdx , &hist[0], histogramByteCount );



      // -------------- Now do the neighbors
      NeighborListType::value_type &nbList = neighborList[sIdx];
      for (unsigned int i=0; i < nbList.size(); i++)
      {
          unsigned int nIdx = nbList[i];
          computeHistogram(
                      PixelInfoListValueIterator<ImageDataType>::begin( imgData, slicMap[nIdx] ),
                      PixelInfoListValueIterator<ImageDataType>::end( imgData, slicMap[nIdx] ),
                      hist, hOpts, i > 0 );
      }

      // copy to output array
      memcpy( ptrNbHist + hOpts.nbins * sIdx, &hist[0], histogramByteCount );
  }


  mexPrintf("Done\n");
  mexEvalString("drawnow");
}

