#ifndef IMAGEDATATRANSMIT_CXX
#define IMAGEDATATRANSMIT_CXX

#include "ImageDataTransmit.h"
#include <QDebug>
#include <iostream>

template <typename TInputPixelType>
ImageDataTransmit < TInputPixelType > ::
ImageDataTransmit (V3DPluginCallback2* V3dCallback): m_V3DPluginCallback(V3dCallback) 
{
  this->m_Import2DFilter = Import2DFilterType::New();
  this->m_Import3DFilter = Import3DFilterType::New();
  this->initial();
}

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType >
::initial() 
{
  this->m_GlobalSetting = this->m_V3DPluginCallback->getGlobalSetting();
  this->m_4DImage = this->m_V3DPluginCallback->getImage(m_V3DPluginCallback->currentImageWindow());
  this->m_Data1D = reinterpret_cast<InputPixelType *>(this->m_4DImage->getRawData());

  this->m_NumberOfPixelsAlongX = this->m_4DImage->getXDim();
  this->m_NumberOfPixelsAlongY = this->m_4DImage->getYDim();
  this->m_NumberOfPixelsAlongZ = this->m_4DImage->getZDim();
  this->m_channelNumbers = this->m_4DImage->getCDim();
}
template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
transferInputImage()
{
  const V3DLONG x1 = 0;
  const V3DLONG y1 = 0;
  const V3DLONG z1 = 0;

  const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
  const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
  const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

  QList<V3D_Image3DBasic> inputImageList = getChannelDataForProcessingFromGlobalSetting(this->m_4DImage, *(this->m_V3DPluginCallback));
  for (int i = 0; i < m_channelNumbers; i++)
  {

    const V3D_Image3DBasic inputImage = inputImageList.at(i);

    this->transferInput( inputImage, x1, x2, y1, y2, z1, z2 );
  }
}

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
transferInput( const V3D_Image3DBasic & inputImage, V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2)
{
  const TInputPixelType * constInputBuffer = reinterpret_cast<TInputPixelType *>( inputImage.data1d );

  typename Import3DFilterType::SizeType size;
  size[0] = x2 - x1;
  size[1] = y2 - y1;
  size[2] = z2 - z1;

  typename Import3DFilterType::IndexType start;
  start[0] = x1;
  start[1] = y1;
  start[2] = z1;

  typename Import3DFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );

  this->m_Import3DFilter = Import3DFilterType::New();
  this->m_Import3DFilter->SetRegion( region );

  region.SetSize( size );

  typename Input3DImageType::PointType origin;
  origin.Fill( 0.0 );

  this->m_Import3DFilter->SetOrigin( origin );


  typename Import3DFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );

  this->m_Import3DFilter->SetSpacing( spacing );

  const unsigned int numberOfPixels = region.GetNumberOfPixels();

  const bool importImageFilterWillOwnTheBuffer = false;

  TInputPixelType * inputBuffer = const_cast<TInputPixelType *>( constInputBuffer );

  this->m_Import3DFilter->SetImportPointer( inputBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer );

  this->m_Import3DFilter->Update();
    
  this->m_InputItkImageList << this->m_Import3DFilter->GetOutput();
}

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
transferOutputImage()
{

  QList<V3D_Image3DBasic> m_outputImageList;
  for (int i = 0; i < m_channelNumbers; i++)
  {
    V3D_Image3DBasic image3DBasic;
    image3DBasic.cid = i;
    transferOutput(image3DBasic, i);
    m_outputImageList << image3DBasic;
  } 
  bool transferResult = assembleProcessedChannels2Image4DClass(m_outputImageList, *(this->m_V3DPluginCallback));

  if (!transferResult)
  {
    v3d_msg(QObject::tr("Error while transfering output image."));
  }
} 

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
transferOutput(V3D_Image3DBasic & outputImage, int channel) const
{

  OutputImagePointer m_outputItkImage = m_OutputItkImageList.at(channel);

  typedef typename Output3DImageType::PixelContainer  PixelContainer3DType;

  PixelContainer3DType * container = m_outputItkImage->GetPixelContainer();

  container->SetContainerManageMemory( false );

  OutputPixelType * output1d = container->GetImportPointer();

  outputImage.data1d = reinterpret_cast< unsigned char * >( output1d );

  typename Output3DImageType::RegionType region = m_outputItkImage->GetBufferedRegion();

  typename Output3DImageType::SizeType size = region.GetSize();

  outputImage.sz0 = size[0];
  outputImage.sz1 = size[1];
  outputImage.sz2 = size[2];


  //
  //  Set the pixel type id.
  //
  if( typeid(OutputPixelType) == typeid( unsigned char ) )
    {
    outputImage.datatype = V3D_UINT8;
    }
  else if ( typeid(OutputPixelType) == typeid( unsigned short int ) )
    {
    outputImage.datatype = V3D_UINT16;
    }
  else if ( typeid(OutputPixelType) == typeid( float ) )
    {
    outputImage.datatype = V3D_FLOAT32;
    }
}

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
copyOneChannel(V3D_Image3DBasic &outputImage_t, V3D_Image3DBasic &outputImage_s)
{
  V3DLONG mysz0=outputImage_s.sz0,mysz1=outputImage_s.sz1,mysz2=outputImage_s.sz2;
        V3DLONG nunitbytes=1;if(outputImage_s.datatype==V3D_UINT16)nunitbytes=2;else if(outputImage_s.datatype==V3D_FLOAT32)nunitbytes=4;
	V3DLONG channelbytes=mysz0*mysz1*mysz2*nunitbytes;
      	unsigned char * pout=0;
	try{
		pout=new unsigned char[channelbytes];
	    }catch(...)
	{
	v3d_msg(QString("Fail to allocate a buffer memory for output of the plugin in assembleProcessedChannels2Image4DClass()."));
		return false;
	}
	unsigned char *p1=0,*p2=0;
	p1=pout;p2=outputImage_s.data1d;
        for(int i=0;i<channelbytes;i++)
	{
	 *p1++=*p2++;
	}
	outputImage_t.sz0=mysz0;outputImage_t.sz1=mysz1;outputImage_t.sz2=mysz2;
	outputImage_t.data1d=pout;
	outputImage_t.datatype=outputImage_s.datatype;
}

template <typename TInputPixelType>
typename ImageDataTransmit<TInputPixelType>::InputImagePointer
ImageDataTransmit < TInputPixelType > ::
getInput3DImage(int channel)
{
  if (channel >= m_channelNumbers) {
    qDebug() << "error channel id is larger than channel number";
    return NULL;
  }
  return m_InputItkImageList.at(channel);
}

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
setOutput3DImage(OutputImagePointer outputImage, int channel)
{
  if (channel >= m_channelNumbers) {
    qDebug() << "error channel id is larger than channel number";
    return;
  }
  this->m_OutputItkImageList << outputImage;
} 

template <typename TInputPixelType>
void
ImageDataTransmit < TInputPixelType > ::
setPluginName(const QString& pluginName)
{
  this->m_pluginName = pluginName;
}

template <typename TInputPixelType>
QString
ImageDataTransmit < TInputPixelType > ::
getPluginName() const
{
  return m_pluginName;
}
template <typename TInputPixelType>
ImageDataTransmit < TInputPixelType > ::
~ImageDataTransmit()
{
}
#endif
