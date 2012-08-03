#ifndef _IMAGEDATATRANSMIT_H_
#define _IMAGEDATATRANSMIT_H_

#include <v3d_interface.h>
#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkProgressAccumulator.h"
#include <QList>
#include <QString>


//assume input and output has the same pixeltype midPixelType is float
template < typename TInputPixelType>
class ImageDataTransmit {
  public:
    typedef TInputPixelType  InputPixelType;
    typedef TInputPixelType  OutputPixelType;
    typedef float       MidPixelType;

    typedef itk::Image<InputPixelType, 2> Input2DImageType;
    typedef itk::Image<InputPixelType, 3> Input3DImageType;
    typedef itk::Image<OutputPixelType, 2>  Output2DImageType;
    typedef itk::Image<OutputPixelType, 3>  Output3DImageType;
//import filter to tranform image in v3d to itk::image
    typedef itk::ImportImageFilter<InputPixelType, 2> Import2DFilterType;
    typedef itk::ImportImageFilter<InputPixelType, 3> Import3DFilterType;

    typedef typename Input3DImageType::Pointer InputImagePointer;
    typedef typename Output3DImageType::Pointer OutputImagePointer;
 public:
    explicit ImageDataTransmit(V3DPluginCallback2* V3dCallback);
    void transferInputImage();
    void transferOutputImage(); 
    InputImagePointer getInput3DImage(int channel);
    void setOutput3DImage(OutputImagePointer outputImage, int channel);
    void setPluginName(const QString& pluginName);
    QString getPluginName() const;
    virtual ~ImageDataTransmit();
  private:
    void initial();
    void transferInput( const V3D_Image3DBasic & inputImage, V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2);
    void transferOutput(V3D_Image3DBasic & outputImage, int channel) const;
    void copyOneChannel(V3D_Image3DBasic &outputImage_t, V3D_Image3DBasic &outputImage_s);
 private:
    typename Import2DFilterType::Pointer  m_Import2DFilter;
    typename Import3DFilterType::Pointer  m_Import3DFilter;
//    typename Input2DImageType::Pointer m_Input2DImage;
//    typename Input3DImageType::Pointer m_Input3DImage;
//    typename Output2DImageType::Pointer m_Output2DImage;
//    typename Output3DImageType::Pointer m_Output3DImage;

    V3DPluginCallback2*    m_V3DPluginCallback;
    int m_channelNumbers;

    QList<InputImagePointer> m_InputItkImageList;
    QList<OutputImagePointer> m_OutputItkImageList;

    V3D_GlobalSetting m_GlobalSetting;
    Image4DSimple*  m_4DImage;
    InputPixelType* m_Data1D;
    V3DLONG m_NumberOfPixelsAlongX;
    V3DLONG m_NumberOfPixelsAlongY;
    V3DLONG m_NumberOfPixelsAlongZ;
    QString m_pluginName;
};
#include "ImageDataTransmit.cxx"
#endif /* _IMAGEDATATRANSMIT_H_ */
