#ifndef _find_centroids_hxx_
#define _find_centroids_hxx_

#include "point3d.hxx"

/**
 * Finds centroids from slic idx image (mImg) with (numLabels) labels.
 * Creates a temporary to store the number of pixels per supervoxel
 *   BEWARE: returns zero-based arrays, no directly compatible with matlab
 */
void	findCentroids( const mxArray *mImg, const unsigned int numLabels, UIntPoint3DList  &centroidList, bool forceInsideSupervoxel = true )
{
	if (mxGetClassID(mImg) != mxUINT32_CLASS)
		mexErrMsgTxt("Image must be UINT32 class");
		
  const mwSize *mImgDims = mxGetDimensions(mImg);
  const unsigned int width = mImgDims[0];
  const unsigned int height = mImgDims[1];
  const unsigned int depth  = mImgDims[2];

  const unsigned int sz = width*height;

  unsigned int *ubuff = (unsigned int *) mxGetData(mImg);

  // pre-allocate centroid idx list, each one will be empty
  centroidList.resize( numLabels );
  
  // number of pixels per supervoxel
  std::vector<unsigned int>  numPixs(numLabels);
  
  const unsigned int totalSz = width*height*depth;
  
  // go through all pixels
  unsigned int x,y,z;
  x = y = z = 0;
  
  mexPrintf("Size: %d\n", (unsigned int) totalSz );
  
  for (unsigned int i=0; i < totalSz; i++)
  {
	  // dummy check
	  unsigned int slicIdx = ubuff[i];
	  if (slicIdx >= numLabels)
		mexErrMsgTxt("Value greater or equal than numLabels");
		
	  centroidList[slicIdx].add(x,y,z);
	  numPixs[slicIdx]++;
      
	  /** Increment x,y,z **/
	  x++;
	  if (x >= width) {
		  x = 0;
		  y++;
		  
		  if (y >= height) {
			  y = 0;
			  z++;
		  }
	  }
  }
  
  // compute mean point
  for (unsigned int i=0; i < numLabels; i++)
	  centroidList[i].divideBy( numPixs[i] );
	  
  if (!forceInsideSupervoxel)
	return;
	  
  mexPrintf("Fixing idxs...\n");
  mexEvalString("drawnow");

  // check if mean point is within corresponding supervoxel, otherwise
  //  find nearest point according to euclidean distance
  // this should be the least likely case, that is why we don't pre-compute
  // an idx -> pixel map, but this would depend on the case
  int numWrong = 0;
  for (unsigned int i=0; i < numLabels; i++)
  {
    #define valAt( arr, x, y, z ) (arr[ (x) + (y)*width + (z)*sz ])
    #define idxAt(x,y,z) valAt( ubuff, x, y, z )

      unsigned int realIdx = idxAt(centroidList[i].x, centroidList[i].y, centroidList[i].z);
      if ( realIdx != i )
      {
          //mexPrintf("Centroid pixel idx incorrect: %d != %d\n", (int)realIdx, (int)i);

          double minSqDist = 1e12;
          UIntPoint3D nearestPt;
          for (unsigned int q=0; q < totalSz; q++ )
          {
              if ( ubuff[q] != i )
                  continue;

              unsigned int modA = q % sz;

              unsigned int x = modA % width;
              unsigned int y = (( q - x ) % sz) / width;
              unsigned int z = (q - x - y*width) / sz;

              UIntPoint3D curPos(x,y,z);
              double sqDist = centroidList[i].sqDist( curPos );
              if (sqDist < minSqDist) {
                  nearestPt = curPos;
                  minSqDist = sqDist;
              }
          }

          centroidList[i] = nearestPt;

          // check again, just in case
          realIdx = idxAt(centroidList[i].x, centroidList[i].y, centroidList[i].z);
          if ( realIdx != i )
              mexErrMsgTxt("Still wrong idx after distance search");

          numWrong++;
      }

    #undef valAt
    #undef idxAt
  }
  mexPrintf("Incorrect idxs fixed: %d/%d\n", numWrong, (int)numLabels);
}

#endif
