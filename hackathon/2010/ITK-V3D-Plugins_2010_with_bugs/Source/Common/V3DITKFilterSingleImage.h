#ifndef __V3DITKFilterSingleImage_H__
#define __V3DITKFilterSingleImage_H__

#include "V3DITKFilterBaseImage.h"

template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterSingleImage : public V3DITKFilterBaseImage < TInputPixelType, TOutputPixelType >
{
public:

  typedef V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >  Superclass;

  typedef typename Superclass::Input2DImageType   Input2DImageType;
  typedef typename Superclass::Input3DImageType   Input3DImageType;

  typedef typename Superclass::Output2DImageType   Output2DImageType;
  typedef typename Superclass::Output3DImageType   Output3DImageType;

  V3DITKFilterSingleImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterSingleImage();

protected:

  virtual void Compute();

  virtual void TransferInput( const V3D_Image3DBasic & inputImage,
    V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 );

  const Input2DImageType * GetInput2DImage() const;
  const Input3DImageType * GetInput3DImage() const;

private:

  typedef typename Superclass::Import2DFilterType  Import2DFilterType;
  typedef typename Superclass::Import3DFilterType  Import3DFilterType;

  typename Import2DFilterType::Pointer      m_Impor2DFilter;
  typename Import3DFilterType::Pointer      m_Impor3DFilter;

  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

};


#include "V3DITKFilterSingleImage.txx"

#endif
