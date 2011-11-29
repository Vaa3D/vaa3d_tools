/* RegistrationBSpline_3D.cpp
 * 2010-06-04: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

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
	return QStringList() << QObject::tr("ITK BSpline Registration...");
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
class ITKRegistrationBSplineSpecializaed
{
public:
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		//get image pointers
		v3dhandleList wndlist = callback.getImageWindowList();
		if(wndlist.size()<2)
		{
			v3d_msg(QObject::tr("Registration need at least two images!"));
			return;
		}
		v3dhandle oldwin = wndlist[1];
		Image4DSimple* p4DImage_fix=callback.getImage(wndlist[0]);
		Image4DSimple* p4DImage_mov=callback.getImage(wndlist[1]);
		if(p4DImage_fix->getXDim()!=p4DImage_mov->getXDim() ||
		   p4DImage_fix->getYDim()!=p4DImage_mov->getYDim() ||
		   p4DImage_fix->getZDim()!=p4DImage_mov->getZDim() ||
		   p4DImage_fix->getCDim()!=p4DImage_mov->getCDim())
		{
			v3d_msg(QObject::tr("Two input images have different size!"));
			return;
		}

		//set dimention info
		const unsigned int Dimension = 3;

		//get global setting
		V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
	    int channelToFilter = globalSetting.iChannel_for_plugin;
	    if( channelToFilter >= p4DImage_fix->getCDim())
		{
			v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
			return;
		}

		//------------------------------------------------------------------
		//import images from V3D
		typedef TPixelType PixelType;
		typedef itk::Image< PixelType,  Dimension > ImageType_input;
		typedef itk::ImportImageFilter<PixelType, Dimension> ImportFilterType;

		typename ImportFilterType::Pointer importFilter_fix = ImportFilterType::New();
		typename ImportFilterType::Pointer importFilter_mov = ImportFilterType::New();

		//set ROI region
		typename ImportFilterType::RegionType region;
		typename ImportFilterType::IndexType start;
		start.Fill(0);
		typename ImportFilterType::SizeType size;
		size[0] = p4DImage_fix->getXDim();
		size[1] = p4DImage_fix->getYDim();
		size[2] = p4DImage_fix->getZDim();
		region.SetIndex(start);
		region.SetSize(size);
		importFilter_fix->SetRegion(region);
		importFilter_mov->SetRegion(region);

		//set image Origin
		typename ImageType_input::PointType origin;
		origin.Fill(0.0);
		importFilter_fix->SetOrigin(origin);
		importFilter_mov->SetOrigin(origin);
		//set spacing
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill(1.0);
		importFilter_fix->SetSpacing(spacing);
		importFilter_mov->SetSpacing(spacing);

		//set import image pointer
		PixelType * data1d_fix = reinterpret_cast<PixelType *> (p4DImage_fix->getRawData());
		PixelType * data1d_mov = reinterpret_cast<PixelType *> (p4DImage_mov->getRawData());
		unsigned long int numberOfPixels = p4DImage_fix->getTotalBytes();
		const bool importImageFilterWillOwnTheBuffer = false;
		importFilter_fix->SetImportPointer(data1d_fix, numberOfPixels,importImageFilterWillOwnTheBuffer);
		importFilter_mov->SetImportPointer(data1d_mov, numberOfPixels,importImageFilterWillOwnTheBuffer);

		//------------------------------------------------------------------
		//setup filter: cast datatype to float
		typedef itk::Image< float, Dimension >   	ImageType_mid;
		typedef itk::RescaleIntensityImageFilter<ImageType_input, ImageType_mid > RescaleFilterType_input;

		typename RescaleFilterType_input::Pointer rescaler_to_32f_fix = RescaleFilterType_input::New();
		typename RescaleFilterType_input::Pointer rescaler_to_32f_mov = RescaleFilterType_input::New();
		rescaler_to_32f_fix->SetOutputMinimum(   0 );	rescaler_to_32f_fix->SetOutputMaximum( 255 );
		rescaler_to_32f_mov->SetOutputMinimum(   0 );	rescaler_to_32f_mov->SetOutputMaximum( 255 );

		rescaler_to_32f_fix->SetInput(importFilter_fix->GetOutput());
		rescaler_to_32f_mov->SetInput(importFilter_mov->GetOutput());
		rescaler_to_32f_fix->Update();
		rescaler_to_32f_mov->Update();

		//------------------------------------------------------------------
		//set method type used in each module
		const unsigned int SpaceDimension = Dimension;
		const unsigned int SplineOrder = 3;
		typedef double CoordinateRepType;
		typedef itk::BSplineDeformableTransform<
											CoordinateRepType,
											SpaceDimension,
											SplineOrder >       TransformType;

//		typedef itk::LBFGSOptimizer							OptimizerType;//for 2D
		typedef itk::LBFGSBOptimizer						OptimizerType;//for 3D

		typedef itk::MeanSquaresImageToImageMetric<
											ImageType_mid,
											ImageType_mid > 	MetricType;
		typedef itk::LinearInterpolateImageFunction<
											ImageType_mid,
											double          > 	InterpolatorType;
		typedef itk::ImageRegistrationMethod<
											ImageType_mid,
											ImageType_mid > 	RegistrationType;

		typename TransformType::Pointer  		transform 		= TransformType::New();
		typename OptimizerType::Pointer      	optimizer     	= OptimizerType::New();
		typename MetricType::Pointer         	metric        	= MetricType::New();
		typename InterpolatorType::Pointer   	interpolator  	= InterpolatorType::New();
		typename RegistrationType::Pointer   	registration  	= RegistrationType::New();

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

		ImageType_mid::SizeType fixedImageSize = region.GetSize();

		for(unsigned int r=0; r<Dimension; r++)
		{
		spacing[r] *= static_cast<double>(fixedImageSize[r] - 1)  /
					  static_cast<double>(gridSizeOnImage[r] - 1);
		}

		ImageType_mid::DirectionType gridDirection = rescaler_to_32f_fix->GetOutput()->GetDirection();
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

		//set optimizer paras
		//for 2D
//		optimizer->SetGradientConvergenceTolerance( 0.01 );
//		optimizer->SetLineSearchAccuracy( 0.9 );
//		optimizer->SetDefaultStepLength( 0.5 );
//		optimizer->TraceOn();
//		optimizer->SetMaximumNumberOfFunctionEvaluations( 1000 );
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

		// cast datatype to original one for write/display
		typedef itk::CastImageFilter<ImageType_mid,ImageType_input > CastFilterType;
		typename CastFilterType::Pointer  caster =  CastFilterType::New();

		// write the warped moving image to disk
		typedef itk::ImageFileWriter< ImageType_input >  WriterType;
		typename WriterType::Pointer      writer =  WriterType::New();
		writer->SetFileName("output.tif");
		printf("save output.tif complete\n");

		caster->SetInput( resampler->GetOutput() );
		writer->SetInput( caster->GetOutput()   );

		caster->Update();
//		writer->Update();

		//------------------------------------------------------------------
//		typedef itk::ImageRegionConstIterator<ImageType_input> IteratorType;
//		IteratorType it(caster->GetOutput(), caster->GetOutput()->GetRequestedRegion());
//		it.GoToBegin();
//
////		if(!globalSetting.b_plugin_dispResInNewWindow)
////		{
//			printf("display results in a new window\n");
//			//copy data back to V3D
//			while(!it.IsAtEnd())
//			{
//				*data1d_mov=it.Get();
//				++it;
//				++data1d_mov;
//			}
//
//			callback.setImageName(oldwin, callback.getImageName(oldwin)+"_new");
//			callback.updateImageWindow(oldwin);
//		}
//		else
//		{
//			printf("display results in current window\n");
//			long N = p4DImage_mov->getTotalBytes();
//			unsigned char* newdata1d = new unsigned char[N];
//			Image4DSimple tmp;
//			tmp.setData(newdata1d, p4DImage_mov );
//
//			//copy data back to the new image
//			while(!it.IsAtEnd())
//			{
//				*newdata1d=it.Get();
//				++it;
//				++newdata1d;
//			}
//
//			v3dhandle newwin = callback.newImageWindow();
//			callback.setImage(newwin, &tmp);
//			callback.setImageName(newwin, callback.getImageName(oldwin)+"_new");
//		    callback.updateImageWindow(newwin);
//		}
	}

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKRegistrationBSplineSpecializaed< c_pixel_type > runner; \
    runner.Execute(  menu_name, callback, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
	Image4DSimple *p4DImage = callback.getImage(curwin); \
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

void ITKRegistrationBSplinePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
