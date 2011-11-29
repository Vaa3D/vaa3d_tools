#ifndef __V3DITKFilterBaseImage_TXX__
#define __V3DITKFilterBaseImage_TXX__

#include "V3DITKFilterBaseImage.h"
#include "V3DITKProgressDialog.h"


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterBaseImage( V3DPluginCallback * callback )
{
  this->m_V3DPluginCallback = callback;

  this->m_ProcessObjectSurrogate = ProcessHelper::New();
  this->m_ProgressAccumulator = ProgressAccumulatorType::New();
  this->m_ProgressAccumulator->SetMiniPipelineFilter( this->m_ProcessObjectSurrogate );

  this->m_ImageSelectionDialog.SetCallback( this->m_V3DPluginCallback );
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterBaseImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output3DImageType * image )
{
  this->m_Output3DImage = image;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output2DImageType * image )
{
  this->m_Output2DImage = image;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::AddOutputImageChannel( V3DLONG channelId )
{
  V3D_Image3DBasic outputImage;

  outputImage.cid = channelId;

  this->TransferOutput( outputImage );

  this->m_OutputImageList.append( outputImage );
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::ComposeOutputImage()
{
  bool transferResult =  assembleProcessedChannels2Image4DClass( this->m_OutputImageList, *(this->m_V3DPluginCallback) );

  if( !transferResult )
    {
    v3d_msg(QObject::tr("Error while transfering output image."));
    }
}




template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::SetPluginName( const char * name )
{
  this->m_PluginName = name;
}


template <typename TInputPixelType, typename TOutputPixelType>
QString
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::GetPluginName() const
{
  return QObject::tr( this->m_PluginName.c_str() );
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::AddObserver( itk::Command * observer )
{
  this->m_ProgressAccumulator->ResetProgress();
  this->m_ProcessObjectSurrogate->AddObserver( itk::ProgressEvent(), observer );
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::RegisterInternalFilter( GenericFilterType *filter, float weight )
{
  this->m_ProgressAccumulator->RegisterInternalFilter( filter, weight );
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::Initialize()
{
  this->m_CurrentWindow = this->m_V3DPluginCallback->currentImageWindow();

  this->m_GlobalSetting = this->m_V3DPluginCallback->getGlobalSetting();

  this->m_4DImage = this->m_V3DPluginCallback->getImage( this->m_CurrentWindow );

  this->m_Data1D = reinterpret_cast< InputPixelType * >( this->m_4DImage->getRawData() );

  this->m_NumberOfPixelsAlongX = this->m_4DImage->getXDim();
  this->m_NumberOfPixelsAlongY = this->m_4DImage->getYDim();
  this->m_NumberOfPixelsAlongZ = this->m_4DImage->getZDim();
  this->m_NumberOfChannels =     this->m_4DImage->getCDim();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::Execute(const QString &menu_name, QWidget *parent)
{
  this->Compute();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::AddImageSelectionLabel( const char * imageLabel )
{
  this->m_ImageSelectionDialog.AddImageSelectionLabel( imageLabel );
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::SetImageSelectionDialogTitle( const char * imageLabel )
{
  this->m_ImageSelectionDialog.SetWindowTitle( imageLabel );
}


template <typename TInputPixelType, typename TOutputPixelType>
Image4DSimple *
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::GetInputImageFromIndex( unsigned int imageIndex )
{
  return this->m_ImageSelectionDialog.GetImageFromIndex( imageIndex );
}


template <typename TInputPixelType, typename TOutputPixelType>
bool
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::ShouldGenerateNewWindow() const
{
  return this->m_GlobalSetting.b_plugin_dispResInNewWindow;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterBaseImage< TInputPixelType, TOutputPixelType >
::TransferOutput( V3D_Image3DBasic & outputImage ) const
{
  typedef typename Output3DImageType::PixelContainer  PixelContainer3DType;

  PixelContainer3DType * container = this->m_Output3DImage->GetPixelContainer();

  container->SetContainerManageMemory( false );

  OutputPixelType * output1d = container->GetImportPointer();

  outputImage.data1d = reinterpret_cast< unsigned char * >( output1d );

  typename Output3DImageType::RegionType region = this->m_Output3DImage->GetBufferedRegion();

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

#endif
