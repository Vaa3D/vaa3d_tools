/* RegistrationAffine.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "RegistrationAffine.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSubtractImageFilter.h"

#include "itkCenteredTransformInitializer.h"

#include "itkImageRegistrationMethod.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImage.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(RegistrationAffine, ITKRegistrationAffinePlugin)

QStringList ITKRegistrationAffinePlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Affine Registration...");
}


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
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
  typedef   const OptimizerType *                  OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
	{
	Execute( (const itk::Object *)caller, event);
	}

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( object );
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetValue() << "   ";
      std::cout << optimizer->GetCurrentPosition() << std::endl;
    }
};


template<typename TPixelType>
class ITKRegistrationAffineSpecializaed
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
//		long nz=p4DImage_fix->getZDim();
//		if(nz==1)
//		{
//			const unsigned int Dimension = 2;
//		}
//		else if(nz>4)
//		{
//			const unsigned int Dimension = 3;
//		}
//		else
//		{
//			v3d_msg(QObject::tr("For 3D image, need pixles along z dir big than 4!"));
//			return;
//		}

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
		typedef itk::AffineTransform<double,Dimension  >		TransformType;
		typedef itk::RegularStepGradientDescentOptimizer		OptimizerType;
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

		//align the mass center of two images (use the offset here to initialize the transform of registration)
		typedef itk::CenteredTransformInitializer<
										TransformType,
										ImageType_mid,
										ImageType_mid >  TransformInitializerType;
		typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
		initializer->SetTransform(   transform );
		initializer->SetFixedImage(  rescaler_to_32f_fix->GetOutput() );
		initializer->SetMovingImage( rescaler_to_32f_mov->GetOutput() );
		initializer->MomentsOn();
		initializer->InitializeTransform();

		//use the offset of the mass center of two image to initialize the transform of registration
		registration->SetInitialTransformParameters(transform->GetParameters() );

		//set optimizer paras
		typedef OptimizerType::ScalesType OptimizerScalesType;
		OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
		double translationScale = 1.0 / 1000.0;
		if(Dimension==2)
		{
			optimizerScales[0] =  1.0;
			optimizerScales[1] =  1.0;
			optimizerScales[2] =  1.0;
			optimizerScales[3] =  1.0;
			optimizerScales[4] =  translationScale;
			optimizerScales[5] =  translationScale;
		}
		else if(Dimension==3)
		{
			optimizerScales[0] =  1.0;
			optimizerScales[1] =  1.0;
			optimizerScales[2] =  1.0;
			optimizerScales[3] =  1.0;
			optimizerScales[4] =  1.0;
			optimizerScales[5] =  1.0;
			optimizerScales[6] =  1.0;
			optimizerScales[7] =  1.0;
			optimizerScales[8] =  1.0;
			optimizerScales[9]  =  translationScale;
			optimizerScales[10] =  translationScale;
			optimizerScales[11] =  translationScale;
		}
		optimizer->SetScales( optimizerScales );

		double steplength = 0.1;
		unsigned int maxNumberOfIterations = 300;
		optimizer->SetMaximumStepLength( steplength );
		optimizer->SetMinimumStepLength( 0.001 );
		optimizer->SetNumberOfIterations( maxNumberOfIterations );
		optimizer->MinimizeOn();

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

		//------------------------------------------------------------------
		//  Once the optimization converges, we recover the parameters from the registration method
		OptimizerType::ParametersType finalParameters = registration->GetLastTransformParameters();

		const double finalRotationCenterX = transform->GetCenter()[0];
		const double finalRotationCenterY = transform->GetCenter()[1];
		const double finalTranslationX    = finalParameters[4];
		const double finalTranslationY    = finalParameters[5];

		const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
		const double bestValue = optimizer->GetValue();

		// Print out results
		std::cout << "Result = " << std::endl;
		std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
		std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
		std::cout << " Translation X = " << finalTranslationX  << std::endl;
		std::cout << " Translation Y = " << finalTranslationY  << std::endl;
		std::cout << " Iterations    = " << numberOfIterations << std::endl;
		std::cout << " Metric value  = " << bestValue          << std::endl;

		//------------------------------------------------------------------
		//  resample the moving image and write out the difference image
		//  before and after registration. We will also rescale the intensities of the
		//  difference images, so that they look better!
		typedef itk::ResampleImageFilter<ImageType_mid,ImageType_mid> ResampleFilterType;

		typename TransformType::Pointer finalTransform = TransformType::New();
		finalTransform->SetParameters( finalParameters );
		finalTransform->SetFixedParameters( transform->GetFixedParameters() );

		typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
		resampler->SetTransform( finalTransform );
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
		typedef itk::ImageRegionConstIterator<ImageType_input> IteratorType;
		IteratorType it(caster->GetOutput(), caster->GetOutput()->GetRequestedRegion());
		it.GoToBegin();

//		if(!globalSetting.b_plugin_dispResInNewWindow)
//		{
			printf("display results in a new window\n");
			//copy data back to V3D
			while(!it.IsAtEnd())
			{
				*data1d_mov=it.Get();
				++it;
				++data1d_mov;
			}

			callback.setImageName(oldwin, callback.getImageName(oldwin)+"_new");
			callback.updateImageWindow(oldwin);
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
	ITKRegistrationAffineSpecializaed< c_pixel_type > runner; \
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

void ITKRegistrationAffinePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
