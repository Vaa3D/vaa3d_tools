#include <vector>
#include <string>
#include "mex.h"
#include <string.h>

#include <cmath>

#define fatalMsg(x) mexErrMsgTxt(x)

#include "include/create_slicmap.hxx"
#include "include/compute_histogram.hxx"
#include "include/find_neighbors.hxx"
#include "include/find_centroids.hxx"
#include "include/matlab_utils.hxx"

#include <set>
#include <omp.h>

//#include "utils.h"

using namespace std;


/**
 * Creates three histograms for each supervoxel. 
 * The first one with the supervoxel itself.
 * The other two are from the neighbor supervoxels behind a plane with supervoxelOrient orientation
 *   and the supervoxels in front of that plane
 *
 * Histograms are NOT normalized.
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 9)
      mexErrMsgTxt("Incorrect input format: [ownHist, frontHist, backHist, sideHist, frontBackVals] = slic_gen_twosided_histograms( image, smoothedImg, labelImage, numLabels, supervoxelOrient, hMinVal, hMaxVal, nBins, neighborDepth )\n");

  if (nlhs != 5) {
      mexErrMsgTxt("Five output arguments required.");
  }

#define mImg (prhs[0])
#define mSmoothImg (prhs[1])
#define mLabelImg (prhs[2])
#define mNLabels (prhs[3])
#define mSOrient (prhs[4])
#define mMinVal (prhs[5])
#define mMaxVal (prhs[6])
#define mNBins (prhs[7])
#define mNeighbDepth (prhs[8])

  typedef unsigned char ImageDataType;

  if (mxGetClassID(mImg) != matlabClassID<ImageDataType>())
      mexErrMsgTxt("Image must be UINT8 class");

  if ( mxGetNumberOfDimensions(mImg) != 3 )
      mexErrMsgTxt("Image must be 3-dimensional");
      
  if (mxGetClassID(mSmoothImg) != matlabClassID<ImageDataType>())
      mexErrMsgTxt("Image must be UINT8 class");

  if ( mxGetNumberOfDimensions(mSmoothImg) != 3 )
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
  
  MatlabInputMatrix<float>	pSOrient( mSOrient, 3, numLabels, "supervoxelOrient" );
  MatlabInputMatrix<double>	pNeighbDepth( mNeighbDepth, 1, 1, "neighborDepth" );
  
  const unsigned int neighborSearchDepth = round(pNeighbDepth.data()[0]);
  if (neighborSearchDepth < 1)
	mexErrMsgTxt("Neighbor search depth must be at least 1");


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
  MatlabOutputMatrix< HistogramType::value_type >  pFrontHist( &plhs[1], hOpts.nbins, numLabels );
  MatlabOutputMatrix< HistogramType::value_type >  pBackHist( &plhs[2], hOpts.nbins, numLabels );
  MatlabOutputMatrix< HistogramType::value_type >  pSideHist( &plhs[3], hOpts.nbins, numLabels );
  MatlabOutputMatrix< HistogramType::value_type >  pSideVals( &plhs[4], 2, numLabels );


  // --------------------------------------- compute supervoxel neighbours
  mexPrintf("Computing neighbors\n");
  mexEvalString("drawnow");
  NeighborListType	neighborList;
  findNeighbors<false>( mLabelImg, numLabels, neighborList );
  
  // -------------------------------------- compute centroids
  mexPrintf("Computing centroids\n");
  mexEvalString("drawnow");
  UIntPoint3DList centroidList;
  findCentroids( mLabelImg, numLabels, centroidList, false );


  // prepare pointers
  HistogramType::value_type *ptrOwnHist = pOwnHist.data();
  HistogramType::value_type *ptrFrontHist = pFrontHist.data();
  HistogramType::value_type *ptrBackHist = pBackHist.data();
  HistogramType::value_type *ptrSideHist = pSideHist.data();

  // size in bytes of a given histogram
  const unsigned int histogramByteCount = sizeof( HistogramType::value_type ) * hOpts.nbins;

  // ---------------------------------------
  mexPrintf("Computing cached histograms\n");
  mexEvalString("drawnow");
  
  std::vector<HistogramType> cachedHistograms( numLabels );
  std::vector<ImageDataType> cachedHistogramsMean( numLabels );
  
  #pragma omp parallel for schedule(dynamic)
  for (unsigned int sIdx=0; sIdx < numLabels; sIdx++)
  {
	  cachedHistogramsMean[sIdx] = computeHistogram(
                  PixelInfoListValueIterator<ImageDataType>::begin( imgData, slicMap[sIdx] ),
                  PixelInfoListValueIterator<ImageDataType>::end( imgData, slicMap[sIdx] ),
                  cachedHistograms[sIdx], hOpts, false, true );
  }

  // ---------------------------------------
  mexPrintf("Computing histograms\n");
  mexEvalString("drawnow");

  // now go through supervoxels and compute histograms
  #pragma omp parallel for schedule(dynamic)
  for (unsigned int sIdx=0; sIdx < numLabels; sIdx++)
  {
	  FloatPoint3D curPt;	// pre-assign, convert to float
	  curPt.x = centroidList[sIdx].x;
	  curPt.y = centroidList[sIdx].y;
	  curPt.z = centroidList[sIdx].z;
	  
	  FloatPoint3D curOr;	// orientation
	  curOr.x = pSOrient.data( 3*sIdx + 0 );
	  curOr.y = pSOrient.data( 3*sIdx + 1 );
	  curOr.z = pSOrient.data( 3*sIdx + 2 );

      // copy to output array the supervoxel hist
      memcpy( ptrOwnHist + hOpts.nbins * sIdx, cachedHistograms[sIdx].data(), histogramByteCount );


	  HistogramType histFront( hOpts.nbins, 0 );
	  HistogramType	histBack ( hOpts.nbins, 0 );	// a histogram for each side, already set to 0
	  HistogramType	histSide ( hOpts.nbins, 0 );
	  
	  unsigned int nFront = 0;
	  unsigned int nBack = 0;
	  
	  double frontMean = 0.0;
	  double backMean = 0.0;
	  
	  const float mPI = 3.14159265;
	  const float projThreshold = cos(mPI/2 -25.0*mPI/180.0);
	  
	  if (projThreshold < 0)
		mexErrMsgTxt("projThreshold < 0!");
		
		// this specifies how long to search through neighbors of neighbors
		// 1 means that only the neighbors of the corresp. supervoxel will be used
	  const unsigned int searchDepth = neighborSearchDepth;
	  
      // -------------- Now do the neighbors
      std::vector< const NeighborListType::value_type * >	nbSearchLists;
      nbSearchLists.reserve( neighborList[sIdx].size() * 10 );
      nbSearchLists.push_back( &neighborList[sIdx] );	// push the first one
      
      std::vector<unsigned int> depthList;
      depthList.reserve( neighborList[sIdx].size() * 10 );
      depthList.push_back(0);
      
      std::set< unsigned int > alreadyVisited;
      //alreadyVisited.reserve( neighborList[sIdx].size() * 10 );
      
      alreadyVisited.insert( sIdx );	// the main one
      
      //
      while( !nbSearchLists.empty() )
      {
		  // get and pop a neighbor list
		  const NeighborListType::value_type &nbList = *nbSearchLists.back();
		  nbSearchLists.pop_back();
		  
		  const unsigned int curDepth = depthList.back();
		  depthList.pop_back();
		  
		  for (unsigned int i=0; i < nbList.size(); i++)
		  {
			  unsigned int nIdx = nbList[i];
			  if (alreadyVisited.count(nIdx) > 0)
				continue;
			  
			  const UIntPoint3D &nbPt = centroidList[nIdx];
			  
			  float dx = nbPt.x - curPt.x;
			  float dy = nbPt.y - curPt.y;
			  float dz = nbPt.z - curPt.z;
			  
			  float dNorm = sqrt(dx*dx + dy*dy + dz*dz);
			  
			  // project centroid on orientation vector
			  float proj = 	dx * curOr.x +
							dy * curOr.y + 
							dz * curOr.z;
			  proj /= dNorm;
							
			  if (proj > projThreshold)	// front
			  {
				  for (unsigned int q=0; q < hOpts.nbins; q++)
				    histFront[q] += cachedHistograms[nIdx][q];

				 frontMean += cachedHistogramsMean[nIdx];
				 nFront++;
			  } 
			  else if (proj < -projThreshold)
			  {
				  for (unsigned int q=0; q < hOpts.nbins; q++)
				    histBack[q] += cachedHistograms[nIdx][q];
				    
				  backMean += cachedHistogramsMean[nIdx];
				    
				  nBack++;
			  } else 
			  {
				  // side histogram
				  for (unsigned int q=0; q < hOpts.nbins; q++)
				    histSide[q] += cachedHistograms[nIdx][q];
			  }
			  
			  // push back a list for each neighbor, if it is not in the list
			  if (curDepth < searchDepth)
			  {
				  nbSearchLists.push_back( &neighborList[nIdx] );
				  depthList.push_back( curDepth + 1 );
			  }
			  alreadyVisited.insert(nIdx);
		  }
	  }
      
      // compute mean values to see if we should invert it
      frontMean /= nFront;
      backMean /= nBack;
      if ( frontMean < backMean )
      {
		  HistogramType hist = histBack;
		  histBack = histFront;
		  histFront = hist;
		  
		  double temp = frontMean;
		  frontMean = backMean;
		  backMean = temp;
	  }
	  
	  pSideVals.data()[ sIdx * 2 + 0] = frontMean;
	  pSideVals.data()[ sIdx * 2 + 1] = backMean;

      // copy to output array
      memcpy( ptrFrontHist + hOpts.nbins*sIdx, &histFront[0], histogramByteCount );
      
      memcpy( ptrBackHist + hOpts.nbins*sIdx, &histBack[0], histogramByteCount );
      
      memcpy( ptrSideHist + hOpts.nbins*sIdx, &histSide[0], histogramByteCount );
  }


  mexPrintf("Done\n");
  mexEvalString("drawnow");
}

