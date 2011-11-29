#ifndef __V3DITKFilterBaseImage_H__
#define __V3DITKFilterBaseImage_H__

#include <v3d_interface.h>
#include "V3DITKGenericDialog.h"
#include "V3DITKImageSelectionDialog.h"
#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkProgressAccumulator.h"
#include <QList>


#define EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT8, unsigned char );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT16, unsigned short int );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }

#define EXECUTE_PLUGIN_FOR_INTEGER_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT8, unsigned char );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT16, unsigned short int );  \
      case V3D_FLOAT32:  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }


#define EXECUTE_PLUGIN_FOR_ALL_INPUT_AND_OUTPUT_PIXEL_TYPES \
    ImagePixelType inputPixelType = p4DImage->getDatatype(); \
    ImagePixelType outputPixelType = p4DImage->getDatatype(); \
    switch( inputPixelType )  \
      {  \
      case V3D_UINT8: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_UINT8, unsigned char, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_UINT16, unsigned char, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_FLOAT32, unsigned char, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_UINT16: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_UINT8, unsigned short int, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_UINT16, unsigned short int, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_FLOAT32, unsigned short int, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_FLOAT32: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_UINT8, float, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_UINT16, float, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_FLOAT32, float, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }


template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterBaseImage
{
public:

  typedef TInputPixelType                   InputPixelType;
  typedef itk::Image< InputPixelType, 2 >   Input2DImageType;
  typedef itk::Image< InputPixelType, 3 >   Input3DImageType;

  typedef TOutputPixelType                  OutputPixelType;
  typedef itk::Image< OutputPixelType, 2 >  Output2DImageType;
  typedef itk::Image< OutputPixelType, 3 >  Output3DImageType;

  typedef itk::ProgressAccumulator          ProgressAccumulatorType;
  typedef ProgressAccumulatorType::GenericFilterType   GenericFilterType;

public:

  V3DITKFilterBaseImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterBaseImage();

  void SetOutputImage( Output2DImageType * image );
  void SetOutputImage( Output3DImageType * image );

  // Schedule images to be passed as return to v3d
  virtual void AddOutputImageChannel( V3DLONG channelId );
  virtual void ComposeOutputImage();

  // Add an image label to the list of images to prompt the user for.
  void AddImageSelectionLabel( const char * imageLabel );

  // Set the title of the dialog that prompts the user for input image names.
  void SetImageSelectionDialogTitle( const char * imageLabel );

  Image4DSimple * GetInputImageFromIndex( unsigned int imageIndex );

  void Execute(const QString &menu_name, QWidget *parent);

  void SetPluginName( const char * name );
  QString GetPluginName() const;

  void AddObserver( itk::Command * observer );

  void RegisterInternalFilter( GenericFilterType *filter, float weight );

  ProgressAccumulatorType * GetProgressAccumulator();

protected:

  virtual void Initialize();

  virtual void ComputeOneRegion() = 0;  //this needs to be implemented for new plugin code
  virtual void TransferOutput( V3D_Image3DBasic & outputImage ) const;

  bool ShouldGenerateNewWindow() const;


protected:

  typedef itk::ImportImageFilter< InputPixelType, 2 > Import2DFilterType;
  typedef itk::ImportImageFilter< InputPixelType, 3 > Import3DFilterType;

  V3DPluginCallback  *                      m_V3DPluginCallback;

  v3dhandle                                 m_CurrentWindow;

  V3D_GlobalSetting                         m_GlobalSetting;

  Image4DSimple *                           m_4DImage;

  InputPixelType *                          m_Data1D;

  V3DLONG                                   m_NumberOfPixelsAlongX;
  V3DLONG                                   m_NumberOfPixelsAlongY;
  V3DLONG                                   m_NumberOfPixelsAlongZ;
  V3DLONG                                   m_NumberOfChannels;

  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

  std::string                               m_PluginName;


  class ProcessHelper : public itk::ProcessObject
    {
    public:
      typedef ProcessHelper                   Self;
      typedef ProcessObject                   Superclass;
      typedef itk::SmartPointer<Self>         Pointer;
      typedef itk::SmartPointer<const Self>   ConstPointer;

      itkTypeMacro(ProcessHelper,ProcessObject);
      itkNewMacro(ProcessHelper);
    protected:
      ProcessHelper() {}
      ~ProcessHelper() {}
    };

  ProgressAccumulatorType::Pointer          m_ProgressAccumulator;
  typename ProcessHelper::Pointer           m_ProcessObjectSurrogate;

  QList< V3D_Image3DBasic >                 m_OutputImageList;

  V3DITKImageSelectionDialog                m_ImageSelectionDialog;
};


#include "V3DITKFilterBaseImage.txx"

#endif
