#include <vector>
#include <string>
#include "mex.h"
#include <string.h>

#define fatalMsg(x) mexErrMsgTxt(x)

#include "include/find_centroids.hxx"
#include "include/create_slicmap.hxx"
#include "include/compute_histogram.hxx"
#include "include/matlab_utils.hxx"

//#include "utils.h"

using namespace std;

/**
 * BEWARE: point coordinates are zero-based, so add + 1 to make them MATLAB-compatible
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 3)
    mexErrMsgTxt("Incorrect input format: centroids = slic_findcentroids( labelImage, numLabels, forceInsideSupervoxel )\n");

  if (nlhs != 1) {
      mexErrMsgTxt("One output argument required.");
  }

#define mImg (prhs[0])
#define mNLabels (prhs[1])
#define mForceInside (prhs[2])

  MatlabInputMatrix<unsigned int> pInsideSupervoxel( mForceInside, 1, 1, "forceInsideSupervoxel" );

  if (mxGetClassID(mNLabels) != mxUINT32_CLASS)
      mexErrMsgTxt("numLabels must be UINT32 type.");

  if ((mxGetN(mNLabels) != 1) || (mxGetM(mNLabels) != 1))
      mexErrMsgTxt("numLabels must be a SCALAR of UINT32 type.");

  if ( mxGetNumberOfDimensions(mImg) != 3 )
      mexErrMsgTxt("labelImg must be 3-dimensional");

  if (mxGetClassID(mImg) != mxUINT32_CLASS)
      mexErrMsgTxt("Image must be UINT32 class");

  const unsigned int numLabels = *((unsigned int *) mxGetData(mNLabels));
  UIntPoint3DList	centroidList;
  
  findCentroids( mImg, numLabels, centroidList, pInsideSupervoxel.data(0) != 0 );

//--------TEST HISTOGRAM -------------------------------------------------
#if 0
  std::vector<int> test;
  test.push_back(23);
  test.push_back(24);
  test.push_back(40);
  test.push_back(42);
  test.push_back(-12);
  test.push_back(80);

  HistogramType hist;

  HistogramOpts<int> hOpts;
  hOpts.begin = 0;
  hOpts.end = 100;
  hOpts.nbins = 10;

  computeHistogram( test.begin(), test.end(), hist, hOpts );
  for (int i=0; i < hist.size(); i++)
  {
      mexPrintf("Bin %d: %d\n", i, (int)hist[i]);
  }
#endif

// ------------- TEST HISTOGRAM + SLIC PIXEL MAP --------
#if 0
  SlicMapType slicMap;
  createSlicMap( mImg, numLabels, slicMap );

  HistogramType hist;
  HistogramOpts<int> hOpts;
  hOpts.begin = 0;
  hOpts.end = 100;
  hOpts.nbins = 10;

  unsigned int *ubuff = (unsigned int *) mxGetData(mImg);

  computeHistogram(
              PixelInfoListValueIterator<unsigned int>::begin( ubuff, slicMap[0] ),
              PixelInfoListValueIterator<unsigned int>::end( ubuff, slicMap[0] ),
              hist, hOpts );

  for (int i=0; i < hist.size(); i++)
  {
      mexPrintf("Bin %d: %d\n", i, (int)hist[i]);
  }

#endif

  // copy ret structure to matlab cell array

  plhs[0] = mxCreateNumericMatrix( 3, centroidList.size(), mxUINT32_CLASS, mxREAL );
  unsigned int *plhsData = (unsigned int *) mxGetData(plhs[0]);
  memcpy( plhsData, centroidList.data(), centroidList.size()*sizeof(centroidList[0]) );
  
  mexPrintf("Done\n");
  mexEvalString("drawnow");
}
