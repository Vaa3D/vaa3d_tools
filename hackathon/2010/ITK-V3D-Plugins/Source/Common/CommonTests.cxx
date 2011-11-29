#include "V3DITKFilterSingleImage.h"

template< typename TInputPixel, typename TOutputPixel >
class V3DITKFilterSingleImageTest : public V3DITKFilterSingleImage< TInputPixel, TOutputPixel >
{
public:

  typedef V3DITKFilterSingleImage< TInputPixel, TOutputPixel > Superclass;

  V3DITKFilterSingleImageTest( V3DPluginCallback * callback ):Superclass(callback) {}
  virtual void ComputeOneRegion() {}
};

int main()
{
  typedef V3DITKFilterSingleImageTest< unsigned char, float >  FilterSingleType;

  V3DPluginCallback * callback = NULL;

  FilterSingleType filter1( callback );

  delete callback;

  return 0;
}
