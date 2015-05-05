#ifndef _find_neighbors_hxx_
#define _find_neighbors_hxx_

#include <vector>

#ifdef _OPENMP
	#include <omp.h>
#endif

typedef std::vector< std::vector<unsigned int> >	NeighborListType;

/**
 * Takes SLIC index image (mImg) with (numLabels) amount of labels
 * and returns neighbor list for each region idex (neighborList)
 * if 26connectivity == false
 */
template<bool tUse26Connectivity>
#ifdef MEX_INFORMATION_VERSION  // mex file
static void	findNeighbors( const mxArray *mImg, const unsigned int numLabels, NeighborListType  &neighborList )
#else
static void	findNeighbors( const Matrix3D<unsigned int> &mImg, const unsigned int numLabels, NeighborListType  &neighborList )
#endif
{
#ifdef MEX_INFORMATION_VERSION  // mex file
    if (mxGetClassID(mImg) != mxUINT32_CLASS)
            fatalMsg("Image must be UINT32 class");
		
  const mwSize *mImgDims = mxGetDimensions(mImg);
  const unsigned int width = mImgDims[0];
  const unsigned int height = mImgDims[1];
  const unsigned int depth  = mImgDims[2];
  unsigned int *ubuff = (unsigned int *) mxGetData(mImg);
#else
    const unsigned int width = mImg.width();
    const unsigned int height = mImg.height();
    const unsigned int depth = mImg.depth();
    const unsigned int *ubuff = mImg.data();
#endif

  const unsigned int sz = width*height;



#define valAt( arr, x, y, z ) (arr[ (x) + (y)*width + (z)*sz ])
#define idxAt(x,y,z) valAt( ubuff, x, y, z )

  // pre-allocate neighbor idx list, each one will be empty
  neighborList.resize( numLabels );
  
// this openmp business makes things 'crowded'
#ifdef _OPENMP
	// create list of locks, one for each list to avoid conflicts
	std::vector<omp_lock_t> ompLockList(numLabels);
	for (unsigned int i=0; i < numLabels; i++)
		omp_init_lock( &ompLockList[i] );
	#define NN_SET_LOCK(idx)	omp_set_lock(&ompLockList[idx])
	#define NN_UNSET_LOCK(idx)	omp_unset_lock(&ompLockList[idx])
#else
	#define NN_SET_LOCK(idx)
	#define NN_UNSET_LOCK(idx)
#endif

  // go through the image
  for (unsigned int z=1; z < depth-1; z++)
  {
	#pragma omp parallel for schedule(dynamic)
    for (unsigned int y=1; y < height-1; y++)
    {
      for (unsigned int x=1; x < width-1; x++)
      {
          unsigned int curIdx = idxAt(x,y,z);
          if (curIdx >= numLabels)
              fatalMsg("Label error");

        #define DO_NEIGHBOR_SINGLE(dx,dy,dz)    \
        do { \
			std::vector<unsigned int> *snList = &neighborList[curIdx]; \
            unsigned int thisIdx = idxAt(x+dx, y+dy, z+dz);\
            if ( thisIdx != curIdx ) \
            {\
				NN_SET_LOCK(curIdx);\
                bool found = false;\
                unsigned int vsize = snList->size(); \
                \
                for (unsigned int _qq=0; _qq < vsize; _qq++) \
                    if ( (*snList)[_qq] == thisIdx ) \
                        found = true; \
               \
                if (!found)\
                    snList->push_back(thisIdx);\
                \
				NN_UNSET_LOCK(curIdx);\
				NN_SET_LOCK(thisIdx);\
                \
                found = false; \
                std::vector<unsigned int> *invList = &neighborList[thisIdx]; \
                vsize = invList->size(); \
                for (unsigned int _qq=0; _qq < vsize; _qq++) \
                    if ( (*invList)[_qq] == curIdx ) \
                        found = true; \
               \
                if (!found)\
                    invList->push_back(curIdx);\
				\
				NN_UNSET_LOCK(thisIdx);\
            }\
        } while(0)

        #define DO_NEIGHBOR(dx,dy,dz) \
            DO_NEIGHBOR_SINGLE(dx,dy,dz); \
            DO_NEIGHBOR_SINGLE(-dx,-dy,-dz)

    if(tUse26Connectivity)
    {
          // do neighbor search, '8-connectivity'
          DO_NEIGHBOR(  1, 0, 0 );
          DO_NEIGHBOR(  0, 1, 0 );
          DO_NEIGHBOR(  0, 0, 1 );

          DO_NEIGHBOR(  1, 1, 0 );
          DO_NEIGHBOR(  1, -1, 0 );
          DO_NEIGHBOR(  0, 1, 1 );
          DO_NEIGHBOR(  0, 1, -1 );
          DO_NEIGHBOR(  1, 0, 1 );
          DO_NEIGHBOR(  1, 0, -1 );

          DO_NEIGHBOR(  1, 1, 1 );
          DO_NEIGHBOR(  1, 1, -1 );
          DO_NEIGHBOR(  1, -1, -1 );
          DO_NEIGHBOR(  -1, 1, -1 );
    }
    else
    {
          // do neighbor search, '4-connectivity'
          DO_NEIGHBOR(  1, 0, 0 );
          DO_NEIGHBOR(  0, 1, 0 );
          DO_NEIGHBOR(  0, 0, 1 );
    }
      }
    }
  }
  
#ifdef _OPENMP
	// destroy locks
	for (unsigned int i=0; i < numLabels; i++)
		omp_destroy_lock( &ompLockList[i] );
#endif
  
#undef valAt
#undef idxAt
}

#endif
