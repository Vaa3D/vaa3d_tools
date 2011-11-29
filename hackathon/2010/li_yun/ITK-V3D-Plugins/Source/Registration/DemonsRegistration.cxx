/* DemonsRegistration.cpp
 * 2010-06-03: create this program by Luis Ibanez
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "V3DITKFilterDualImage.h"
#include "DemonsRegistration.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkCommand.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DemonsRegistration, ITKDemonsRegistrationPlugin)

QStringList ITKDemonsRegistrationPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK Demons Registration")
                       << QObject::tr("about this plugin");
}


template<typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterDualImage< TPixelType, TPixelType >     		Superclass;
	typedef typename Superclass::Input3DImageType              		Input3DImageType;
	typedef TPixelType 							PixelType;

  	itkStaticConstMacro(Image3Dimension, unsigned int, 3);

	typedef itk::Vector< float, Image3Dimension >                                                      VectorPixelType;
	typedef itk::Image<  VectorPixelType, Image3Dimension >                                            DeformationFieldType;
	typedef itk::DemonsRegistrationFilter< Input3DImageType, Input3DImageType, DeformationFieldType>   RegistrationFilterType;
	typedef itk::WarpImageFilter< Input3DImageType, Input3DImageType, DeformationFieldType  >          WarpFilterType;
	typedef itk::LinearInterpolateImageFunction< Input3DImageType, double >                            InterpolatorType;


class CommandIterationUpdate : public itk::Command
{
public:
	typedef  CommandIterationUpdate   Self;
	typedef  itk::Command             Superclass;
	typedef  itk::SmartPointer<CommandIterationUpdate>  Pointer;
	itkNewMacro( Self );
protected:
	CommandIterationUpdate() {};

public:

void Execute(itk::Object *caller, const itk::EventObject & event)
{
	Execute( (const itk::Object *)caller, event);
}

void Execute(const itk::Object * object, const itk::EventObject & event)
{
	//static int iteration = 0;
	const RegistrationFilterType * filter =
	dynamic_cast< const RegistrationFilterType * >( object );
	if( !(itk::IterationEvent().CheckEvent( &event )) )
	{
	return;
	}
	//std::cout << "Iteration: " << iteration;
	std::cout << "  Metric : " << filter->GetMetric();
	std::cout << "  RMS Change: " << filter->GetRMSChange() << std::endl;
}
};

public:
PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
{
	this->m_Filter = RegistrationFilterType::New();
	this->m_Warper = WarpFilterType::New();

	this->RegisterInternalFilter( this->m_Warper, 1.0);  
}

virtual ~PluginSpecialized() {};


void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	this->SetImageSelectionDialogTitle("Input Images");
	this->AddImageSelectionLabel("Image 1");
	this->AddImageSelectionLabel("Image 2");
	this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

	this->SetupParameters();
	this->Compute();
}

virtual void ComputeOneRegion()
{
	const Input3DImageType * fixedImage  = this->GetInput3DImage1();
	const Input3DImageType * movingImage = this->GetInput3DImage2();

	this->m_Filter->SetFixedImage( fixedImage );
	this->m_Filter->SetMovingImage( movingImage );

	typename CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();

	this->m_Filter->AddObserver( itk::IterationEvent(), observer );

	std::cout << "Demons filter started" << std::endl;

	try
	{
	this->m_Filter->Update();
	}
	catch( itk::ExceptionObject & excp)
	{
	std::cerr << "Error run this filter." << std::endl;
	std::cerr << excp << std::endl;
	return;
	}

	std::cout << "Demons filter finished" << std::endl;


	typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

	this->m_Warper->SetInput( movingImage );
	this->m_Warper->SetInterpolator( interpolator );
	this->m_Warper->SetOutputSpacing( fixedImage->GetSpacing() );
	this->m_Warper->SetOutputOrigin( fixedImage->GetOrigin() );
	this->m_Warper->SetOutputDirection( fixedImage->GetDirection() );

	this->m_Warper->SetDeformationField( this->m_Filter->GetOutput() );

	std::cout << "Warping filter started" << std::endl;

	try
	{
	this->m_Warper->Update();
	}
	catch( itk::ExceptionObject & excp)
	{
	std::cerr << "Error run this filter." << std::endl;
	std::cerr << excp << std::endl;
	return;
	}

	std::cout << "Warping filter finished" << std::endl;

	this->SetOutputImage( this->m_Warper->GetOutput() );

}

virtual void SetupParameters()
{
	V3DITKGenericDialog dialog("ITK Demons Registration");

	dialog.AddDialogElement("NumberOfIterations",50.0, 1.0, 10000.0); 
	dialog.AddDialogElement("StandardDeviations",1.0, 0.1, 5.0);
	dialog.AddDialogElement("MaximumRMSError",0.01, 0.001, 0.2);

	if( dialog.exec() == QDialog::Accepted )
	{
	    this->m_Filter->SetNumberOfIterations( dialog.GetValue("NumberOfIterations") );
	    this->m_Filter->SetStandardDeviations( dialog.GetValue("StandardDeviations") );
	    this->m_Filter->SetMaximumRMSError( dialog.GetValue("MaximumRMSError") );
	}
}

private:

typename RegistrationFilterType::Pointer   	m_Filter;
typename WarpFilterType::Pointer           	m_Warper;

};

#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
	case v3d_pixel_type: \
	{ \
	PluginSpecialized< c_pixel_type > runner( &callback ); \
	runner.Execute(  menu_name, callback, parent ); \
	break; \
}

void ITKDemonsRegistrationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "Demons Registration 1.0 (2010-Jun-3): this plugin is developed by Luis Ibanez.");
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

	EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES; // Defined in V3DITKFilterDualImage.h
}
