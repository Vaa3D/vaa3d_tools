/* LabelOverlayPlugin.cxx
 * 2010-07-30: Plugin created by Aurelien Lucchi
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "LabelOverlay.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkLabelOverlayImageFilter.h"
#include "itkRGBPixel.h"
#include "itkCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(LabelOverlay, LabelOverlayPlugin)


QStringList LabelOverlayPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK LabelOverlayImageFilter palette")
                       << QObject::tr("ITK LabelOverlayImageFilter single color")
                       << QObject::tr("about this plugin");
}

QStringList LabelOverlayPlugin::funclist() const
{
  return QStringList();
}


template <typename TInputPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
{
  typedef V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;
  typedef typename Superclass::Output3DImageType              OutputImageType;
  typedef itk::RGBPixel<unsigned char>                ColorPixelType;
  typedef itk::Image< ColorPixelType, 3 >     ColorImageType;	
  typedef unsigned short                               LabelPixelType;
  typedef itk::Image< LabelPixelType, 3 >          LabelImageType;
 
  typedef itk::CastImageFilter<ImageType,LabelImageType> CastFilterType;

  typedef itk::LabelOverlayImageFilter< ImageType, LabelImageType, ColorImageType > FilterType;

  typedef itk::VectorIndexSelectionCastImageFilter< 
                                     ColorImageType, 
                                     OutputImageType> VectorFilterType;


public:
	
  MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
  {
    this->m_Filter = FilterType::New();
    this->m_CastFilter = CastFilterType::New();
  }
	
  virtual ~MySpecialized() {};
	
	
  void Execute(const QString &menu_name, QWidget *parent, bool useSingleColor)
  {
    V3DITKGenericDialog dialog("Label overlay");

    dialog.AddDialogElement("Background value",0.0, 0.0, 255.0);
    dialog.AddDialogElement("Opacity",0.5, 0.0, 1.0);

    if(useSingleColor)
      {
        dialog.AddDialogElement("Red",255.0, 0.0, 255.0);
        dialog.AddDialogElement("Green",0.0, 0.0, 255.0);
        dialog.AddDialogElement("Blue",0.0, 0.0, 255.0);
      }

    if( dialog.exec() == QDialog::Accepted )
      {
        this->m_Filter->SetBackgroundValue(dialog.GetValue("Background value"));
        this->m_Filter->SetOpacity(dialog.GetValue("Opacity"));

        if(useSingleColor)
          {
            int red = dialog.GetValue("Red");
            int green = dialog.GetValue("Green");
            int blue = dialog.GetValue("Blue");
            for(int i =0; i < 255; i++)
              {
                this->m_Filter->AddColor(red, green, blue);
              }
          }

        this->Compute();
        this->ComposeOutputImage();
      }
  }
  
  void Compute()
  {
    this->Initialize();

    QList< V3D_Image3DBasic > inputImageList =
      getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

    const unsigned int numberOfChannelsToProcess = inputImageList.size();
    if (numberOfChannelsToProcess<=0)
      {
        return;
      }

    for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
      {
        const V3D_Image3DBasic inputImage = inputImageList.at(channel);

        this->TransferInputImages( this->m_V3DPluginCallback );

        this->ComputeOneRegion();
      }
  }

  virtual void ComputeOneRegion()
  {
    this->GetInput3DImage1()->Print(std::cout);
    this->GetInput3DImage2()->Print(std::cout);

    this->m_Filter->SetInput( this->GetInput3DImage1() );
    this->m_CastFilter->SetInput( this->GetInput3DImage2() );		
    this->m_Filter->SetLabelImage( this->m_CastFilter->GetOutput() );

    if( !this->ShouldGenerateNewWindow() )
      {
      }
	
    try
      {
        this->m_CastFilter->Update();
      }
    catch(itk::ExceptionObject& e)
      {
        std::cout << e;
      }

    try
      {
        this->m_Filter->Update();
      }
    catch(itk::ExceptionObject& e)
      {
        std::cout << e;
      }

    typename VectorFilterType::Pointer vectorFilterRed = VectorFilterType::New();
    typename VectorFilterType::Pointer vectorFilterGreen = VectorFilterType::New();
    typename VectorFilterType::Pointer vectorFilterBlue = VectorFilterType::New();


    // red channel
    vectorFilterRed->SetInput( this->m_Filter->GetOutput() );
    vectorFilterRed->SetIndex( 0 );
 
    try
      {
        vectorFilterRed->Update();
      }
    catch ( itk::ExceptionObject& e )
      {
        std::cerr << "Exception caught as expected: "  << e;
      }

    this->SetOutputImage(vectorFilterRed->GetOutput());
    this->AddOutputImageChannel( 0 );

    //green channel
    vectorFilterGreen->SetInput( this->m_Filter->GetOutput() );
    vectorFilterGreen->SetIndex( 1 );
 
    try
      {
        vectorFilterGreen->Update();
      }
    catch ( itk::ExceptionObject& e )
      {
        std::cerr << "Exception caught as expected: "  << e;
      }

    this->SetOutputImage(vectorFilterGreen->GetOutput());
    this->AddOutputImageChannel( 1 );

    // blue channel
    vectorFilterBlue->SetInput( this->m_Filter->GetOutput() );
    vectorFilterBlue->SetIndex( 2 );
 
    try
      {
        vectorFilterBlue->Update();
      }
    catch ( itk::ExceptionObject& e )
      {
        std::cerr << "Exception caught as expected: "  << e;
      }

    this->SetOutputImage(vectorFilterBlue->GetOutput());
    this->AddOutputImageChannel( 2 );
  }       
	
private:
	
  typename FilterType::Pointer   m_Filter;
  typename CastFilterType::Pointer m_CastFilter;
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type:                                                  \
  {                                                                     \
    MySpecialized< c_pixel_type, c_pixel_type > runner( &callback );    \
    runner.Execute( menu_name, parent, useSingleColor );                               \
    break;                                                              \
  } 


void LabelOverlayPlugin::dofunc(const QString & func_name,
				const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void LabelOverlayPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
      QMessageBox::information(parent, "Version info", "ITK LabelOverlay 1.0 (2010-July-15): this plugin is developed by Sophie Chen.");
      return;
    }
  
  if (menu_name == QObject::tr("ITK LabelOverlayImageFilter palette"))
    useSingleColor = false;
  else
    useSingleColor = true;

  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin)
    {
      v3d_msg(tr("You don't have any image open in the main window."));
      return;
    }
	
  Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage)
    {
      v3d_msg(tr("The input image is null."));
      return;
    }
	
  EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES; 
}

