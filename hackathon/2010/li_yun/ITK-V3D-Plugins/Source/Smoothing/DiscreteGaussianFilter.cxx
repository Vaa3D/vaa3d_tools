/* DiscreteGaussianFilter.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DiscreteGaussianFilter.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DiscreteGaussianFilter, ITKDiscreteGaussianFilterPlugin)

QStringList ITKDiscreteGaussianFilterPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK DiscreteGaussian Filter ...")
			     << QObject::tr("about this plugin");
}

template<typename TPixelType>
class ITKDiscreteGaussianFilterSpecializaed: public V3DITKFilterSingleImage< TPixelType,TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >  				Superclass;
	typedef TPixelType  									PixelType;
	typedef itk::Image< PixelType,  3 > 							ImageType_input;
	typedef itk::Image< float, 3 >   							ImageType_mid;
	typedef itk::RescaleIntensityImageFilter<ImageType_input, ImageType_mid > 		RescaleFilterType_input;
	typedef itk::RescaleIntensityImageFilter<ImageType_mid,ImageType_input> 		RescaleFilterType_output; 
	typedef itk::DiscreteGaussianImageFilter<ImageType_mid,ImageType_mid> 			FilterType;
public:
ITKDiscreteGaussianFilterSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
{
	this->rescaler_8u_32f = RescaleFilterType_input::New();
	this->rescaler_32f_8u = RescaleFilterType_output::New();
	this->filter 	      = FilterType::New();

	this->RegisterInternalFilter( this->rescaler_8u_32f, 0.2 ); 
	//this->RegisterInternalFilter( this->filter, 0.1 ); 
	this->RegisterInternalFilter( this->rescaler_32f_8u, 0.8 );   		
}

virtual ~ITKDiscreteGaussianFilterSpecializaed() {};

void Execute(V3DPluginCallback &callback, QWidget *parent)
{
	this->Compute();
}
virtual void ComputeOneRegion()
{    
	float gaussianVariance=1.0;
	unsigned int maxKernelWidth=3;
	filter->SetVariance( gaussianVariance );
	filter->SetMaximumKernelWidth( maxKernelWidth );

	rescaler_32f_8u->SetOutputMinimum(   0 );
	rescaler_32f_8u->SetOutputMaximum( 255 );

	//------------------------------------------------------------------
	//setup filter: write processed image to disk
	//typedef itk::ImageFileWriter< ImageType_input >  WriterType;
	//typename WriterType::Pointer writer = WriterType::New();
	//writer->SetFileName("output.tif");

	//------------------------------------------------------------------
	//build pipeline
	rescaler_8u_32f->SetInput( this->GetInput3DImage() );
	filter->SetInput(rescaler_8u_32f->GetOutput());
	rescaler_32f_8u->SetInput(filter->GetOutput());
	//writer->SetInput(rescaler_32f_8u->GetOutput());

	//------------------------------------------------------------------
	//update the pixel value
	rescaler_32f_8u->Update();
	this->SetOutputImage( rescaler_32f_8u->GetOutput() );
}
private:
	typename RescaleFilterType_input::Pointer 		rescaler_8u_32f;
	typename RescaleFilterType_output::Pointer 		rescaler_32f_8u;
	typename FilterType::Pointer				filter;

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKDiscreteGaussianFilterSpecializaed< c_pixel_type > runner(&callback); \
    	runner.Execute( callback, parent ); \
   	break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
	if (! p4DImage) return; \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
	EXECUTE( V3D_UINT8, unsigned char );  \
	EXECUTE( V3D_UINT16, unsigned short int );  \
	EXECUTE( V3D_FLOAT32, float );  \
	case V3D_UNKNOWN:  \
	{  \
	}  \
	}  

void ITKDiscreteGaussianFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK DiscreteGaussian Filter ...");
	return;
	}

	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
	v3d_msg(tr("You don't have any image open in the main window."));
	return;
	}

	Image4DSimple *p4DImage = callback.getImage(curwin);
	if (!p4DImage)
	{
	v3d_msg(tr("The input image is null."));
	return;
	}
	EXECUTE_ALL_PIXEL_TYPES; 
}
