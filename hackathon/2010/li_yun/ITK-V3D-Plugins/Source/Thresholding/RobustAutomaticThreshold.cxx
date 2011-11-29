#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "RobustAutomaticThreshold.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkRobustAutomaticThresholdImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(RobustAutomaticThreshold, RobustAutomaticThresholdPlugin)


QStringList RobustAutomaticThresholdPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK RobustAutomaticThreshold")
            << QObject::tr("about this plugin");
}

QStringList RobustAutomaticThresholdPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   				Superclass;
	typedef typename Superclass::Input3DImageType               				ImageType;
	typedef itk::RobustAutomaticThresholdImageFilter< ImageType, ImageType > 		FilterType;
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< ImageType, ImageType > 	GradientMagnitudeFilterType;

public:

PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
{
	this->m_Filter = FilterType::New();
	this->m_GradientFilter = GradientMagnitudeFilterType::New();
	this->RegisterInternalFilter( this->m_Filter, 0.8 );
	this->RegisterInternalFilter( this->m_GradientFilter, 0.2 );
}

virtual ~PluginSpecialized() {};

void Execute(const QString &menu_name, QWidget *parent)
{
	SetupParameters();
	this->Compute();
}

virtual void ComputeOneRegion()
{
	this->m_GradientFilter->SetInput( this->GetInput3DImage() );
	this->m_Filter->SetInput( this->GetInput3DImage() );
	this->m_Filter->SetGradientImage( this->m_GradientFilter->GetOutput() );

	this->m_Filter->Update();

	this->SetOutputImage( this->m_Filter->GetOutput() );
}

virtual void SetupParameters()
{
	V3DITKGenericDialog dialog("RobustAutomaticThreshold");

	dialog.AddDialogElement("Sigma", 1.5, 0.1, 10.0);
	dialog.AddDialogElement("InsideValue",255.0, 0.0, 255.0);
	dialog.AddDialogElement("OutsideValue",0.0, 0.0, 255.0);

	if( dialog.exec() == QDialog::Accepted )
	{
		this->m_GradientFilter->SetSigma( dialog.GetValue("Sigma") );
		this->m_Filter->SetInsideValue( dialog.GetValue("InsideValue") );
		this->m_Filter->SetOutsideValue( dialog.GetValue("OutsideValue") );
	}
}
private:

    typename FilterType::Pointer   m_Filter;
    typename GradientMagnitudeFilterType::Pointer m_GradientFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void RobustAutomaticThresholdPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void RobustAutomaticThresholdPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK RobustAutomaticThreshold 1.0 (2010-Jun-21): this plugin is developed by Sophie Chen.");
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

