#include <vector>
#include <string>
#include "mex.h"
#include <string.h>

#define fatalMsg(x) mexErrMsgTxt(x)
#include "include/find_neighbors.hxx"

//#include "utils.h"

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs != 2)
    mexErrMsgTxt("Incorrect input format: labelNeighbors = slic_findneighbors( labelImage, numLabels )\n");

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
  NeighborListType	neighborList;
  
  findNeighbors<false>( mImg, numLabels, neighborList );

  // copy ret structure to matlab cell array
  plhs[0] = mxCreateCellMatrix( numLabels, 1 );
  for (int i=0; i < numLabels; i++)
  {
      const std::vector<unsigned int> &snList = neighborList[i];

      if (snList.size() == 0) {
          mexPrintf("Found no neighbors for idx %d\n", (int) i);
      }

      mxArray *mxList = mxCreateNumericMatrix( snList.size(), 1, mxUINT32_CLASS, mxREAL );
      memcpy( mxGetData(mxList), snList.data(), snList.size()*sizeof(snList[0]) );

      // add
      mxSetCell( plhs[0], i, mxList );
  }


  mexPrintf("Done\n");
  mexEvalString("drawnow");
}
