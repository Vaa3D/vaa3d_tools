//========================================================================
// This is a sample main function for using the implementation and header
// files LKM.cpp and LKM.h respectively
//========================================================================

#include <vector>
#include <string>
#include "LKM.h"
#include "mex.h"
#include <string.h>
//#include "utils.h"

using namespace std;


string getNameFromPathWithoutExtension(string path){
  string nameWith =  path.substr(path.find_last_of("/\\")+1);
  string nameWithout = nameWith.substr(0,nameWith.find_last_of("."));
  return nameWithout;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs < 1 || nrhs > 3)
    mexErrMsgTxt("Incorrect input format: [labelImg, numLabels, labelIdxs] = slic_supervoxel(imgData, step_size[optional],CUBENESS[optional], verbosity[optional])\n");

  if (nlhs > 3) {
      mexErrMsgTxt("Maximum three output arguments required.");
  }
  if (nlhs < 1) {
      mexErrMsgTxt("At least one output argument required.");
  }

#define mImg (prhs[0])
#define mStepSize (prhs[1])
#define mVerbosity (prhs[2])

  if ( mxGetNumberOfDimensions(mImg) != 3 )
      mexErrMsgTxt("imgData must be 3-dimensional");

  if (mxGetClassID(mImg) != mxUINT8_CLASS)
      mexErrMsgTxt("Image must be uint8 class");

  const mwSize *mImgDims = mxGetDimensions(mImg);
  int width = mImgDims[0];
  int height = mImgDims[1];
  int depth  = mImgDims[2];

  unsigned char *ubuff = (unsigned char *) mxGetData(mImg);
  sidType** klabels;
  
  //const int STEP = 10;//STEP decides superpixel size (which will roughly be STEP^2 pixels)
  int STEP = 15;
  if(nrhs > 1)
    STEP = (int)(mxGetScalar(prhs[1]));

  double M = 10;
  if(nrhs > 2)
    M = (double)(mxGetScalar(prhs[2]));

  bool verbosity = false;
  if(nrhs > 3)
    if ((int)(mxGetScalar(prhs[3]))>0)
      verbosity=true;

  int numlabels = 0;

  LKM* lkm = new LKM;

  mexPrintf("Volume size: %dx%dx%d\n", width, height, depth);
  mexPrintf("Generating superpixels. STEP=%d, CUBENESS=%f\n", STEP, M);
  mexEvalString("drawnow");

  lkm->DoSupervoxelSegmentationForGrayVolume(ubuff, width, height, depth, klabels, numlabels, STEP, M);

  mexPrintf("Done %d regions, generating output!\n", numlabels);
  mexEvalString("drawnow");

  plhs[0] = mxCreateNumericArray( 3, mImgDims, mxUINT32_CLASS, mxREAL );

  unsigned int *outData = (unsigned int *) mxGetData(plhs[0]);

  unsigned int sz = width*height;
  for (int z=0; z < depth; z++)
  {
      unsigned int zOff = z * sz;

      memcpy( outData + zOff, klabels[z], sz*sizeof(unsigned int) );
  }
  
  if (nlhs > 1) // number of labels requested
  {
      plhs[1] = mxCreateNumericMatrix( 1, 1, mxUINT32_CLASS, mxREAL );
      *((unsigned int *) mxGetData(plhs[1])) = numlabels;
  }

  if (nlhs > 2) // then the 'inverse' list was requested
  {
      mexPrintf("Computing inverse list...\n");
      mexEvalString("drawnow");

      plhs[2] = mxCreateCellMatrix( numlabels, 1 );

      for (int i=0; i < numlabels; i++) // go for each label and find the corresponding pixel idxs
      {
          std::vector<unsigned int>  pixIdxs;

          for (int z=0; z < depth; z++)
          {
              unsigned int zOff = z*sz;

              const sidType *ptr = klabels[z];
              for (int idx=0; idx < sz; idx++)
              {
                  if (ptr[idx] == i)
                      pixIdxs.push_back(idx + zOff + 1);    // +1 is for matlab compatibility
              }
          }

          // now add to cell array
          mxArray *mxList = mxCreateNumericMatrix( pixIdxs.size(), 1, mxUINT32_CLASS, mxREAL );
          memcpy( mxGetData(mxList), pixIdxs.data(), pixIdxs.size()*sizeof(pixIdxs[0]) );

          // add
          mxSetCell( plhs[2], i, mxList );
      }
  }

  mexPrintf("Done\n");
  mexEvalString("drawnow");
}
