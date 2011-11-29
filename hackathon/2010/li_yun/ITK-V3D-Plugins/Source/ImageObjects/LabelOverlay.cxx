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
  typedef V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >                    Superclass;
  typedef typename Superclass::Input3DImageType                                         ImageType;
  typedef typename Superclass::Output3DImageType                                        OutputImageType;
  typedef itk::RGBPixel<unsigned char>                                                  ColorPixelType;
  typedef itk::Image< ColorPixelType, 3 >                                               ColorImageType;	
  typedef unsigned short                                                                LabelPixelType;
  typedef itk::Image< LabelPixelType, 3 >                                               LabelImageType;
 
  typedef itk::CastImageFilter<ImageType,LabelImageType>                                CastFilterType;

  typedef itk::LabelOverlayImageFilter< ImageType, LabelImageType, ColorImageType >     FilterType;

  typedef itk::VectorIndexSelectionCastImageFilter<ColorImageType, OutputImageType>     VectorFilterType;


public:
	
  MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
  {
    this->m_Filter = FilterType::New();
    this->m_InputCastFilter = CastFilterType::New();
    this->m_OutputCastFilter = VectorFilterType::New();

    this->RegisterInternalFilter( this->m_Filter, 0.8 );
    this->RegisterInternalFilter( this->m_InputCastFilter, 0.1 );
    this->RegisterInternalFilter( this->m_OutputCastFilter, 0.1 );
  }
	
  virtual ~MySpecialized() {};
	
	
  void Execute(const QString &menu_name, QWidget *parent, bool useSingleColor)
  {
    this->SetImageSelectionDialogTitle("Input Images");
    this->AddImageSelectionLabel("Image 1");
    this->AddImageSelectionLabel("Image 2");
    this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

    SetupParameters(useSingleColor);
    this->Compute();
    
  }

  virtual void ComputeOneRegion()
  {

    this->m_Filter->SetInput( this->GetInput3DImage1() );
    this->m_InputCastFilter->SetInput( this->GetInput3DImage2() );		
    this->m_Filter->SetLabelImage( this->m_InputCastFilter->GetOutput() );
   
    this->m_Filter->Update();

    this->m_OutputCastFilter->SetInput( this->m_Filter->GetOutput() );

    this->m_OutputCastFilter->Update();

    this->SetOutputImage( this->m_OutputCastFilter->GetOutput() );

  } 

  virtual void SetupParameters(bool useSingleColor)
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
        }
     }

	
private:
	
  typename FilterType::Pointer  	        m_Filter;
  typename CastFilterType::Pointer 	 	m_InputCastFilter;
  typename VectorFilterType::Pointer 		m_OutputCastFilter;
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

