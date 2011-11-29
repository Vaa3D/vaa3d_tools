#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "GradientAnisotropicDiffusion.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkCastImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(GradientAnisotropicDiffusion, GradientAnisotropicDiffusionPlugin)


QStringList GradientAnisotropicDiffusionPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK GradientAnisotropicDiffusion")
            << QObject::tr("about this plugin");
}

QStringList GradientAnisotropicDiffusionPlugin::funclist() const
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
  typedef itk::GradientAnisotropicDiffusionImageFilter< FloatImageType, FloatImageType > FilterType;
  typedef itk::CastImageFilter< FloatImageType, ImageType >   OutputCastFilterType;

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
    V3DITKGenericDialog dialog("GradientAnisotropicDiffusion");

    dialog.AddDialogElement("Iterations",2.0, 1.0, 10.0);
    dialog.AddDialogElement("TimeStep",0.06, 0.001, 0.625);
    dialog.AddDialogElement("Conductance",3.0, 1.0, 5.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetNumberOfIterations( dialog.GetValue("Iterations") );
      this->m_Filter->SetTimeStep( dialog.GetValue("TimeStep") );
      this->m_Filter->SetConductanceParameter( dialog.GetValue("Conductance") );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {

    this->m_InputCast->SetInput( this->GetInput3DImage() );
    this->m_Filter->SetInput( this->m_InputCast->GetOutput() );
    this->m_OutputCast->SetInput( this->m_Filter->GetOutput() );

    if( !this->ShouldGenerateNewWindow() )
      {
      this->m_Filter->InPlaceOn();
      }

    this->m_OutputCast->Update();

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


void GradientAnisotropicDiffusionPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void GradientAnisotropicDiffusionPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK GradientAnisotropicDiffusion 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

