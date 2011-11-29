/* RegistrationBSpline_3D.cpp
 * 2010-06-04: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "V3DITKFilterDualImage.h"
#include "RegistrationBSpline_3D.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"

#include "itkCenteredTransformInitializer.h"

#include "itkImageRegistrationMethod.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineDeformableTransform.h"
#include "itkLBFGSBOptimizer.h"//for 3D
#include "itkImage.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(RegistrationBSpline, ITKRegistrationBSplinePlugin)

QStringList ITKRegistrationBSplinePlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK BSpline Registration")
                             << QObject::tr("about this plugin");
}

//for 3D
#include "itkCommand.h"
class CommandIterationUpdate : public itk::Command
{
public:
	typedef  CommandIterationUpdate   Self;
	typedef  itk::Command             Superclass;
	typedef itk::SmartPointer<Self>   Pointer;
	itkNewMacro( Self );
protected:
	CommandIterationUpdate() {};
public:
	typedef itk::LBFGSBOptimizer       OptimizerType;
	typedef   const OptimizerType *    OptimizerPointer;

void Execute(itk::Object *caller, const itk::EventObject & event)
{
	Execute( (const itk::Object *)caller, event);
}

void Execute(const itk::Object * object, const itk::EventObject & event)
{
	OptimizerPointer optimizer =
	dynamic_cast< OptimizerPointer >( object );
	if( !(itk::IterationEvent().CheckEvent( &event )) )
	{
	return;
	}
	std::cout << optimizer->GetCurrentIteration() << "   ";
	std::cout << optimizer->GetValue() << "   ";
	std::cout << optimizer->GetInfinityNormOfProjectedGradient() << std::endl;
}
};



template<typename TPixelType>
class ITKRegistrationBSplineSpecializaed : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
public:
	typedef V3DITKFilterDualImage< TPixelType, TPixelType >                                       Superclass;
	typedef TPixelType                                                                            PixelType;

	typedef typename Superclass::Input3DImageType                                         	Input3DImageType;
	typedef itk::ImportImageFilter<PixelType, 3>                                                  ImportFilterType;
	typedef itk::Image< PixelType, 3 >   	                 				        ImageType_mid;
	typedef itk::Image< PixelType,  3 >                                                             ImageType_input;
	typedef itk::RescaleIntensityImageFilter< ImageType_input, ImageType_mid >            	        RescaleFilterType_input;    // cast datatype to float
	typedef itk::ImageRegistrationMethod<	ImageType_mid, ImageType_mid > 	                	RegistrationType;
	typedef itk::CastImageFilter<ImageType_mid,ImageType_input > 					CastFilterType;
	typedef itk::ImageFileWriter< ImageType_input >  						WriterType;

public:
	ITKRegistrationBSplineSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->rescaler_to_32f_fix = RescaleFilterType_input::New();
		this->rescaler_to_32f_mov = RescaleFilterType_input::New();
		this->registration  	  = RegistrationType::New();
		this->caster		  = CastFilterType::New();
		this->writer		  = WriterType::New();

		this->RegisterInternalFilter( this->rescaler_to_32f_fix, 0.1 );
		this->RegisterInternalFilter( this->rescaler_to_32f_mov, 0.1 );
		this->RegisterInternalFilter( this->registration, 0.7 );
		this->RegisterInternalFilter( this->caster, 0.1 );
	}
  	virtual ~ITKRegistrationBSplineSpecializaed() {};
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList wndlist = callback.getImageWindowList();
		Image4DSimple* p4DImage=callback.getImage(wndlist[0]);

		//set image Origin	
		origin.Fill(0.0);
		//set spacing
		spacing.Fill(1.0);
		//set ROI region
		typename ImportFilterType::IndexType start;
		start.Fill(0);
		typename ImportFilterType::SizeType size;
		size[0] = p4DImage->getXDim();
		size[1] = p4DImage->getYDim();
		size[2] = p4DImage->getZDim();
		region.SetIndex(start);
		region.SetSize(size);

		this->SetImageSelectionDialogTitle("Input Images");
		this->AddImageSelectionLabel("Image 1");
		this->AddImageSelectionLabel("Image 2");

		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
		this->Compute();	
	} 
	virtual void ComputeOneRegion()
	{
		const unsigned int Dimension = 3;        
		//------------------------------------------------------------------
		//setup filter: cast datatype to float

		rescaler_to_32f_fix->SetOutputMinimum(   0 );	rescaler_to_32f_fix->SetOutputMaximum( 255 );
		rescaler_to_32f_mov->SetOutputMinimum(   0 );	rescaler_to_32f_mov->SetOutputMaximum( 255 );

		rescaler_to_32f_fix->SetInput( this->GetInput3DImage1() );
		rescaler_to_32f_mov->SetInput( this->GetInput3DImage2() );
		rescaler_to_32f_fix->Update();
		rescaler_to_32f_mov->Update();

		//------------------------------------------------------------------
		//set method type used in each module
		const unsigned int SpaceDimension = Dimension;
		const unsigned int SplineOrder = 3;
		typedef double CoordinateRepType;
		typedef itk::BSplineDeformableTransform< CoordinateRepType, SpaceDimension, SplineOrder >       TransformType;
		typedef itk::LBFGSBOptimizer									OptimizerType;
		typedef itk::MeanSquaresImageToImageMetric< ImageType_mid, ImageType_mid > 			MetricType;
		typedef itk::LinearInterpolateImageFunction< ImageType_mid, double > 				InterpolatorType;

		typename TransformType::Pointer  	transform 	= TransformType::New();
		typename OptimizerType::Pointer      	optimizer     	= OptimizerType::New();
		typename MetricType::Pointer         	metric        	= MetricType::New();
		typename InterpolatorType::Pointer   	interpolator  	= InterpolatorType::New();


		registration->SetTransform(     transform     );
		registration->SetMetric(        metric        );
		registration->SetOptimizer(     optimizer     );
		registration->SetInterpolator(  interpolator  );

		registration->SetFixedImage(    rescaler_to_32f_fix->GetOutput()   );
		registration->SetMovingImage(   rescaler_to_32f_mov->GetOutput()   );

		registration->SetFixedImageRegion(rescaler_to_32f_fix->GetOutput()->GetBufferedRegion() );

		//------------------------------------------------------------------
		//Here we define the parameters of the BSplineDeformableTransform grid
		typedef TransformType::RegionType RegionType;
		RegionType bsplineRegion;
		RegionType::SizeType   gridSizeOnImage;
		RegionType::SizeType   gridBorderSize;
		RegionType::SizeType   totalGridSize;

		gridSizeOnImage.Fill( 10 );
		gridBorderSize.Fill( 3 );    // Border for spline order = 3 ( 1 lower, 2 upper )
		totalGridSize = gridSizeOnImage + gridBorderSize;

		bsplineRegion.SetSize( totalGridSize );

		typedef TransformType::SpacingType SpacingType;
		typedef TransformType::OriginType OriginType;

		typename ImageType_mid::SizeType fixedImageSize = region.GetSize();

		for(unsigned int r=0; r<Dimension; r++)
		{
		spacing[r] *= static_cast<double>(fixedImageSize[r] - 1)  /
					  static_cast<double>(gridSizeOnImage[r] - 1);
		}

		typename ImageType_mid::DirectionType gridDirection = rescaler_to_32f_fix->GetOutput()->GetDirection();
		SpacingType gridOriginOffset = gridDirection * spacing;

		OriginType gridOrigin = origin - gridOriginOffset;

		transform->SetGridSpacing( spacing );
		transform->SetGridOrigin( gridOrigin );
		transform->SetGridRegion( bsplineRegion );
		transform->SetGridDirection( gridDirection );


		typedef TransformType::ParametersType     ParametersType;

		const unsigned int numberOfParameters = transform->GetNumberOfParameters();

		ParametersType parameters( numberOfParameters );

		parameters.Fill( 0.0 );

		transform->SetParameters( parameters );

		//------------------------------------------------------------------
		//use the offset of the mass center of two image to initialize the transform of registration
		registration->SetInitialTransformParameters(transform->GetParameters() );

		//for 3D
		OptimizerType::BoundSelectionType boundSelect( transform->GetNumberOfParameters() );
		OptimizerType::BoundValueType upperBound( transform->GetNumberOfParameters() );
		OptimizerType::BoundValueType lowerBound( transform->GetNumberOfParameters() );

		boundSelect.Fill( 0 );
		upperBound.Fill( 0.0 );
		lowerBound.Fill( 0.0 );

		optimizer->SetBoundSelection( boundSelect );
		optimizer->SetUpperBound( upperBound );
		optimizer->SetLowerBound( lowerBound );

		optimizer->SetCostFunctionConvergenceFactor( 1e+12 );
		optimizer->SetProjectedGradientTolerance( 1.0 );
		optimizer->SetMaximumNumberOfIterations( 500 );
		optimizer->SetMaximumNumberOfEvaluations( 500 );
		optimizer->SetMaximumNumberOfCorrections( 5 );

		// Create the Command observer and register it with the optimizer.
		CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
		optimizer->AddObserver( itk::IterationEvent(), observer );

		try
		{
			registration->StartRegistration();
			std::cout << "Optimizer stop condition: "
					  << registration->GetOptimizer()->GetStopConditionDescription()
					  << std::endl;
		}
		catch( itk::ExceptionObject & err )
		{
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return;
		}

		OptimizerType::ParametersType finalParameters = registration->GetLastTransformParameters();

		std::cout << "Last Transform Parameters" << std::endl;
		std::cout << finalParameters << std::endl;

		//------------------------------------------------------------------
		//  resample the moving image and write out the difference image
		//  before and after registration. We will also rescale the intensities of the
		//  difference images, so that they look better!
		typedef itk::ResampleImageFilter<ImageType_mid,ImageType_mid> ResampleFilterType;
		typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
		resampler->SetTransform( transform );
		resampler->SetInput( rescaler_to_32f_mov->GetOutput() );

		typename ImageType_mid::Pointer fixedImage = rescaler_to_32f_fix->GetOutput();
		resampler->SetSize(    fixedImage->GetLargestPossibleRegion().GetSize() );
		resampler->SetOutputOrigin(  fixedImage->GetOrigin() );
		resampler->SetOutputSpacing( fixedImage->GetSpacing() );
		resampler->SetOutputDirection( fixedImage->GetDirection() );
		resampler->SetDefaultPixelValue( 0 );

		// write the warped moving image to disk
		writer->SetFileName("output.tif");
		printf("save output.tif complete\n");

		// cast datatype to original one for write/display
		caster->SetInput( resampler->GetOutput() );
		writer->SetInput( caster->GetOutput()   );

		caster->Update();
		//writer->Update();
		this->SetOutputImage( caster->GetOutput() );   
	}
private:
	typename RescaleFilterType_input::Pointer 	rescaler_to_32f_fix ;
	typename RescaleFilterType_input::Pointer 	rescaler_to_32f_mov ;
        typename RegistrationType::Pointer   	  	registration ;
        typename CastFilterType::Pointer     		caster ;// cast datatype to original one for write/display
   	typename WriterType::Pointer     	  	writer ;

	typename ImageType_input::PointType 		origin;
        typename ImportFilterType::SpacingType 		spacing;
	typename ImportFilterType::RegionType 		region;



};

#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    ITKRegistrationBSplineSpecializaed< c_pixel_type > runner( &callback ); \
    runner.Execute(  menu_name, callback, parent ); \
    break; \
    }

void ITKRegistrationBSplinePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "This is about ITKRegistrationPlugin");
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
