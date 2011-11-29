/* ITKCannySegmentation.cxx
 * 2010-06-04: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKCannySegmentation.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

#include "itkZeroCrossingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkCannySegmentationLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKCannySegmentation, ITKCannySegmentationPlugin)

//plugin funcs
//const QString title = "ITK CannySegmentationPlugin";
QStringList ITKCannySegmentationPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CannySegmentationPlugin")
						 << QObject::tr("about this plugin");
}

QStringList ITKCannySegmentationPlugin::funclist() const
{
    return QStringList();
}

void ITKCannySegmentationPlugin::dofunc(const QString & func_name,
const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKCannySegmentationSpecializaed  :  public V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
{
public:
	typedef V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >                    		Superclass;
	typedef itk::Image< TInputPixelType, 3 > 							InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 							OutputImageType;
        typedef itk::CastImageFilter< InputImageType, OutputImageType >                         	InputCastImageFilterType;
	typedef itk::CannySegmentationLevelSetImageFilter< OutputImageType, OutputImageType >   	FilterType;
	typedef itk::GradientAnisotropicDiffusionImageFilter<  OutputImageType, OutputImageType > 	DiffusionFilterType;

public:
	ITKCannySegmentationSpecializaed( V3DPluginCallback * callback ): Superclass(callback) 
        {
		this->cannySegmentation = FilterType::New();
		this->castImageFilter1  = InputCastImageFilterType::New();
		this->castImageFilter2  = InputCastImageFilterType::New();
		this->diffusion         = DiffusionFilterType::New();

	    	this->RegisterInternalFilter( this->cannySegmentation, 0.7 );
                this->RegisterInternalFilter( this->castImageFilter1, 0.1 );
 		this->RegisterInternalFilter( this->castImageFilter2, 0.1 );
 		this->RegisterInternalFilter( this->diffusion, 0.1 );
        }

	virtual ~ITKCannySegmentationSpecializaed() {};
	
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
                this->SetImageSelectionDialogTitle("Input Images");
		this->AddImageSelectionLabel("Image 1");
		this->AddImageSelectionLabel("Image 2");
		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

		diffusion->SetNumberOfIterations(5);
		diffusion->SetTimeStep(0.05);
		diffusion->SetConductanceParameter(1.0);
		
		cannySegmentation->SetAdvectionScaling( 10.0 );
		cannySegmentation->SetCurvatureScaling( 1.0 );
		cannySegmentation->SetPropagationScaling( 0.0 );
		
		cannySegmentation->SetMaximumRMSError( 0.01 );
		cannySegmentation->SetNumberOfIterations( 20 );
		
		cannySegmentation->SetThreshold( 7.0 );
		cannySegmentation->SetVariance( 0.1 );
		
		cannySegmentation->SetIsoSurfaceValue( 127.5 );

		this->Compute();		
        }	

	virtual void ComputeOneRegion()
	{
		this->castImageFilter1->SetInput( this->GetInput3DImage1() );
		this->castImageFilter2->SetInput( this->GetInput3DImage2() );
		this->diffusion->SetInput( castImageFilter1->GetOutput() );
		this->cannySegmentation->SetInput( castImageFilter2->GetOutput() );
		this->cannySegmentation->SetFeatureImage( diffusion->GetOutput() );

		this->cannySegmentation->Update();     
		this->SetOutputImage( this->cannySegmentation->GetOutput() );
   	 }

private:

    typename FilterType::Pointer              		cannySegmentation;	
    typename InputCastImageFilterType::Pointer    	castImageFilter1;
    typename InputCastImageFilterType::Pointer     	castImageFilter2;;	
    typename DiffusionFilterType::Pointer     		diffusion;
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKCannySegmentationSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
		EXECUTE( V3D_UINT8, unsigned char, float );  \
		EXECUTE( V3D_UINT16, unsigned short int, float );  \
		EXECUTE( V3D_FLOAT32, float, float );  \
		case V3D_UNKNOWN:  \
		{  \
		}  \
	} 
 
void ITKCannySegmentationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{  
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Canny SegmentationPlugin 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
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

  EXECUTE_ALL_PIXEL_TYPES;
}




