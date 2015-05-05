#ifndef CREATE_SLICMAP_HXX
#define CREATE_SLICMAP_HXX

#include "point3d.hxx"

struct  PixelInfo
{
    UIntPoint3D     coords; // (x,y,z) coordinates, zero-based
    unsigned int    index;  // raw index (linear array)

    inline PixelInfo() {
        index = 0;
    }

    inline PixelInfo( UIntPoint3D::ScalarType x, UIntPoint3D::ScalarType y, UIntPoint3D::ScalarType z, unsigned int idx ) {
        coords.x = x;
        coords.y = y;
        coords.z = z;
        index = idx;
    }
};

typedef std::vector<PixelInfo>      PixelInfoList;  // list of pixel info
typedef std::vector<PixelInfoList>  SlicMapType;    // one for each possible idx

// Iterator to access pixel values based on PixelInfoList
template<typename T>
class PixelInfoListValueIterator
{
private:
    const T *dataPtr;
    unsigned int mIdx;
    const PixelInfoList &mPixInfoList;

public:
    inline PixelInfoListValueIterator( const T *slicData, const PixelInfoList &pinfolist, unsigned int stIdx )
        : dataPtr(slicData), mPixInfoList( pinfolist )
    {
        mIdx = stIdx;
    }

    inline void operator++(int) { mIdx++; }
    inline unsigned int idx() const { return mIdx; }

    inline bool operator <(const PixelInfoListValueIterator<T>& b) const
    {
        return mIdx < b.idx();
    }

    inline static PixelInfoListValueIterator<T> begin( const T *slicData, const PixelInfoList &pinfolist )
    {
        return PixelInfoListValueIterator<T>( slicData, pinfolist, 0 );
    }

    inline static PixelInfoListValueIterator<T> end( const T *slicData, const PixelInfoList &pinfolist )
    {
        return PixelInfoListValueIterator<T>( slicData, pinfolist, pinfolist.size() );
    }

    inline T operator *() const
    {
        return dataPtr[mPixInfoList[mIdx].index];
    }
};

/**
 * Takes SLIC idx image (mImg) and (numLabels)
 * and generates a mapping from SLIC idxs -> pixel idxs
 */
template<bool ignoreZero>   // if ignorezero == true => zero is not considered to be a label and is skipped
#ifdef MEX_INFORMATION_VERSION  // mex file
static void	createSlicMapT( const mxArray *mImg, const unsigned int numLabels, SlicMapType  &slicMap, unsigned int preallocSize = 0 )
#else
static void	createSlicMapT( const Matrix3D<unsigned int> &mImg, const unsigned int numLabels, SlicMapType  &slicMap, unsigned int preallocSize = 0 )
#endif
{
#ifdef MEX_INFORMATION_VERSION  // mex file
    if (mxGetClassID(mImg) != mxUINT32_CLASS)
            mexErrMsgTxt("Image must be UINT32 class");

  const mwSize *mImgDims = mxGetDimensions(mImg);
  const unsigned int width = mImgDims[0];
  const unsigned int height = mImgDims[1];
  const unsigned int depth  = mImgDims[2];

  unsigned int *ubuff = (unsigned int *) mxGetData(mImg);
#else
    const unsigned int *ubuff = mImg.data();
    const unsigned int width = mImg.width();
    const unsigned int height = mImg.height();
    const unsigned int depth = mImg.depth();
#endif

  slicMap.clear();    //just in case
  // pre-allocate centroid idx list, each one will be empty
  slicMap.resize( numLabels );
  
  if (preallocSize != 0)
  {
	for (unsigned int i=0; i < numLabels; i++)
		slicMap.reserve( preallocSize );
  }

  const unsigned int totalSz = width*height*depth;

  // go through all pixels
  unsigned int x,y,z;
  x = y = z = 0;

  printf("Size: %d\n", (unsigned int) totalSz );

  for (unsigned int i=0; i < totalSz; i++)
  {
          // dummy check
          //unsigned int slicIdx = ubuff[i];
          //if (slicIdx >= numLabels)
                //fatalMsg("Value greater or equal than numLabels");
          unsigned int val = ubuff[i];
          if(ignoreZero) {
              if (val == 0)
                  goto slicmapSkip;
              val = val - 1;
          }

          slicMap[val].push_back( PixelInfo( x, y, z, i ) );

        slicmapSkip:
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

  // check if any is empty
  for (unsigned int i=0; i < numLabels; i++) {
      if ( slicMap[i].empty() )
          printf("Found empty map: %d", (int)i);
  }
}

#define createSlicMap   createSlicMapT<false>

#endif // CREATE_SLICMAP_HXX
