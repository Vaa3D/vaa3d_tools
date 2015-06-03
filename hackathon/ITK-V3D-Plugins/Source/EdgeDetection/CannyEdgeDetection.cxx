#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "CannyEdgeDetection.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkCastImageFilter.h"
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(CannyEdgeDetection, CannyEdgeDetectionPlugin)


QStringList CannyEdgeDetectionPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CannyEdgeDetection")
            << QObject::tr("about this plugin");
}

QStringList CannyEdgeDetectionPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::Image< float, 3 >                              FloatImageType;

  typedef itk::CastImageFilter< ImageType, FloatImageType >   InputCastFilterType;
  typedef itk::CannyEdgeDetectionImageFilter< FloatImageType, FloatImageType > FilterType;
 // typedef itk::CastImageFilter< FloatImageType, ImageType >   OutputCastFilterType;
  typedef itk::RescaleIntensityImageFilter<FloatImageType,ImageType> OutputCastFilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->m_InputCast = InputCastFilterType::New();
    this->m_OutputCast = OutputCastFilterType::New();

    this->RegisterInternalFilter( this->m_InputCast, 0.1 );
    this->RegisterInternalFilter( this->m_Filter, 0.8 );
    this->RegisterInternalFilter( this->m_OutputCast, 0.1 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("CannyEdgeDetection");

    dialog.AddDialogElement("Sigma",1.0, 0.0, 50.0);
    dialog.AddDialogElement("UpperThreshold",1.0, 0.0, 50.0);
    dialog.AddDialogElement("LowerThreshold",1.0, 0.0, 50.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      const double sigma = vcl_sqrt( dialog.GetValue("Sigma") );
      this->m_Filter->SetVariance( sigma );
      this->m_Filter->SetUpperThreshold( dialog.GetValue("UpperThreshold") );
      this->m_Filter->SetLowerThreshold( dialog.GetValue("LowerThreshold") );
	m_OutputCast->SetOutputMinimum(   0 );
	m_OutputCast->SetOutputMaximum( 255 );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {

    this->m_InputCast->SetInput( this->GetInput3DImage() );
    this->m_Filter->SetInput( this->m_InputCast->GetOutput() );
    this->m_OutputCast->SetInput( this->m_Filter->GetOutput() );

    this->m_OutputCast->Update();

    if (!this->m_OutputCast->GetOutput())
      {
      qDebug()<<"Error Canny: outputCast is null";
      }
    this->SetOutputImage( this->m_OutputCast->GetOutput() );
    }


private:

    typename FilterType::Pointer              m_Filter;
    typename InputCastFilterType::Pointer     m_InputCast;
    typename OutputCastFilterType::Pointer    m_OutputCast;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void CannyEdgeDetectionPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void CannyEdgeDetectionPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK CannyEdgeDetection 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
    return;
    }

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

