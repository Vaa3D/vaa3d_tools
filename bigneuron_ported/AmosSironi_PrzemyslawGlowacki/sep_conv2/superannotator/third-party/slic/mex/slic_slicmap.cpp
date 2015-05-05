#include <vector>
#include <string>
#include "mex.h"
#include <string.h>

#define fatalMsg(x) mexErrMsgTxt(x)

#include "include/find_neighbors.hxx"
#include "include/create_slicmap.hxx"

//#include "utils.h"

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 2)
    mexErrMsgTxt("Incorrect input format: pixelIdxs = slic_slicmap( labelImage, numLabels )\n");

  if (nlhs != 1) {
      mexErrMsgTxt("One output argument required.");
  }

#define mImg (prhs[0])
#define mNLabels (prhs[1])

  if (mxGetClassID(mNLabels) != mxUINT32_CLASS)
      mexErrMsgTxt("numLabels must be UINT32 type.");

  if ((mxGetN(mNLabels) != 1) || (mxGetM(mNLabels) != 1))
      mexErrMsgTxt("numLabels must be a SCALAR of UINT32 type.");

  if ( mxGetNumberOfDimensions(mImg) != 3 )
      mexErrMsgTxt("labelImg must be 3-dimensional");

  if (mxGetClassID(mImg) != mxUINT32_CLASS)
      mexErrMsgTxt("Image must be UINT32 class");

  const unsigned int numLabels = *((unsigned int *) mxGetData(mNLabels));

  mexPrintf("Computing SLIC Map\n");
  mexEvalString("drawnow");
  SlicMapType slicMap;
  createSlicMap( mImg, numLabels, slicMap );


  // copy ret structure to matlab cell array
  plhs[0] = mxCreateCellMatrix( numLabels, 1 );
  for (unsigned int i=0; i < numLabels; i++)
  {
      const SlicMapType::value_type &snList = slicMap[i];

      mxArray *mxList = mxCreateNumericMatrix( snList.size(), 1, mxUINT32_CLASS, mxREAL );

      unsigned int *data = (unsigned int *) mxGetData(mxList);
      for (unsigned int j=0; j < snList.size(); j++)
          data[j] = snList[j].index;

      // add
      mxSetCell( plhs[0], i, mxList );
  }


  mexPrintf("Done\n");
  mexEvalString("drawnow");
}

