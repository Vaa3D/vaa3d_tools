#ifndef __V3DITKFilterNullImage_TXX__
#define __V3DITKFilterNullImage_TXX__

#include "V3DITKFilterNullImage.h"


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterNullImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterNullImage( V3DPluginCallback * callback ):Superclass(callback)
{
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterNullImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterNullImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterNullImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

  this->AddObserver( progressDialog.GetCommand() );
  progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );

  // FIXME: We are still missing to connect the logic for "cancel" button that
  // will trigger Abort in the ITK filters.
  progressDialog.show();

  this->ComputeOneRegion();

  this->AddOutputImageChannel( 0 );

  this->ComposeOutputImage();
}

#endif
