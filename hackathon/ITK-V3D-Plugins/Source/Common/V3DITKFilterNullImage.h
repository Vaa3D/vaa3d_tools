#ifndef __V3DITKFilterNullImage_H__
#define __V3DITKFilterNullImage_H__

#include "V3DITKFilterBaseImage.h"

template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterNullImage : public V3DITKFilterBaseImage < TInputPixelType, TOutputPixelType >
{
public:

  typedef V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >  Superclass;

  typedef typename Superclass::Output2DImageType   Output2DImageType;
  typedef typename Superclass::Output3DImageType   Output3DImageType;

  V3DITKFilterNullImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterNullImage();

protected:

  virtual void Compute();

private:

  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

};


#include "V3DITKFilterNullImage.txx"

#endif
