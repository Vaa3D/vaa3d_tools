#ifndef __V3DITKFilterDualImage_H__
#define __V3DITKFilterDualImage_H__

#include "V3DITKFilterBaseImage.h"

template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterDualImage : public V3DITKFilterBaseImage < TInputPixelType, TOutputPixelType >
{
public:

  typedef V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >  Superclass;

  typedef typename Superclass::Input2DImageType   Input2DImageType;
  typedef typename Superclass::Input3DImageType   Input3DImageType;

  typedef typename Superclass::Output2DImageType   Output2DImageType;
  typedef typename Superclass::Output3DImageType   Output3DImageType;

  V3DITKFilterDualImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterDualImage();

protected:
  
  virtual void TransferInputImages( V3DPluginCallback * callback );

  virtual void TransferInputDualImages( const V3D_Image3DBasic & inputImage1, const V3D_Image3DBasic & inputImage2,
    V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 );

  virtual void Compute();
  virtual void ComputeSelectedChannel( V3DLONG channelToProcess );

  const Input2DImageType * GetInput2DImage1() const;
  const Input2DImageType * GetInput2DImage2() const;

  const Input3DImageType * GetInput3DImage1() const;
  const Input3DImageType * GetInput3DImage2() const;

private:

  typedef typename Superclass::Import2DFilterType  Import2DFilterType;
  typedef typename Superclass::Import3DFilterType  Import3DFilterType;

  typename Import2DFilterType::Pointer      m_Impor2DFilter1;
  typename Import2DFilterType::Pointer      m_Impor2DFilter2;

  typename Import3DFilterType::Pointer      m_Impor3DFilter1;
  typename Import3DFilterType::Pointer      m_Impor3DFilter2;

  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

};


#include "V3DITKFilterDualImage.txx"

#endif
