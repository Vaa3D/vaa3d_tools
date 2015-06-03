#ifndef __V3DITKFilterSingleImage_TXX__
#define __V3DITKFilterSingleImage_TXX__

#include "V3DITKFilterSingleImage.h"


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterSingleImage( V3DPluginCallback * callback ):Superclass(callback)
{
  this->m_Impor2DFilter = Import2DFilterType::New();
  this->m_Impor3DFilter = Import3DFilterType::New();
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterSingleImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >::Input2DImageType *
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::GetInput2DImage() const
{
  return this->m_Impor2DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >::Input3DImageType *
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::GetInput3DImage() const
{
  return this->m_Impor3DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::TransferInput( const V3D_Image3DBasic & inputImage, V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 )
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

  this->m_Impor3DFilter->SetRegion( region );

  region.SetSize( size );

  typename Input3DImageType::PointType origin;
  origin.Fill( 0.0 );

  this->m_Impor3DFilter->SetOrigin( origin );


  typename Import3DFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );

  this->m_Impor3DFilter->SetSpacing( spacing );

  const unsigned int numberOfPixels = region.GetNumberOfPixels();

  const bool importImageFilterWillOwnTheBuffer = false;

  TInputPixelType * inputBuffer = const_cast<TInputPixelType *>( constInputBuffer );

  this->m_Impor3DFilter->SetImportPointer( inputBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer );

  this->m_Impor3DFilter->Update();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  this->Initialize();

  const V3DLONG x1 = 0;
  const V3DLONG y1 = 0;
  const V3DLONG z1 = 0;

  const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
  const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
  const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

  QList< V3D_Image3DBasic > inputImageList =
    getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

  QList< V3D_Image3DBasic > outputImageList;

  V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

  this->AddObserver( progressDialog.GetCommand() );
  progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );

//Now add connect to cancel button
//  if ( this->m_used_filer ) {
//	qDebug() << "add cancel button";
//	progressDialog.setFilter(this->m_used_filer); 
//	QObject::connect(&progressDialog, SIGNAL(cancelButtonClicked()), &progressDialog, SLOT(stopFilter()));
 // }
  // FIXME: We are still missing to connect the logic for "cancel" button that
  // will trigger Abort in the ITK filters.
  progressDialog.show();

  const unsigned int numberOfChannelsToProcess = inputImageList.size();
  if (numberOfChannelsToProcess<=0)
    {
    return;
    }

  for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
    {
    const V3D_Image3DBasic inputImage = inputImageList.at(channel);

    this->TransferInput( inputImage, x1, x2, y1, y2, z1, z2 );

    this->ComputeOneRegion();
	if (this->m_Output3DImage.IsNull()) {
		qDebug() << "Error in V3DITKFilterSingleImage.txx:  Empty Output!";
	}
    this->AddOutputImageChannel( channel );
    }

  this->ComposeOutputImage();
}

#endif
