#ifndef _Registration_H_
#define _Registration_H_

#include "header.h"
class Registration:public QObject,public V3DPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface)
public:
	Registration(){}
	QStringList menulist()const;
	QStringList funclist()const;
	void domenu(const QString &menu_name,V3DPluginCallback &callback,QWidget* parent);
	void dofunc(const QString & func_name,const V3DPluginArgList &input,V3DPluginArgList & output,QWidget *parent);

};

template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType>
{
	typedef V3DITKFilterDualImage< TPixelType,TPixelType >	                        				Superclass;
	typedef typename Superclass::Input3DImageType			                				ImageType;
	typedef float                                    								InternalPixelType;
  	typedef itk::Image< InternalPixelType, 3 > 									InternalImageType;
	//----------------------Registration--------------------------
	typedef itk::ImageRegistrationMethod< InternalImageType,InternalImageType > 	         			RegistrationType;
	typedef itk::MultiResolutionImageRegistrationMethod< InternalImageType, InternalImageType >   			MultiRegistrationType;
	typedef typename RegistrationType::ParametersType 								ParametersType;
	typedef typename MultiRegistrationType::ParametersType								MultiParametersType;
	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   			FixedImagePyramidType;
  	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   			MovingImagePyramidType;
	typedef itk::ResampleImageFilter< InternalImageType, InternalImageType >    					ResampleFilterType;
	//----------------------Transform for 3D----------------------
	typedef itk::TranslationTransform< double, 3 > 									TranTransformType;
	typedef itk::ScaleTransform< double, 3 > 									ScaleTransformType;
	typedef itk::VersorTransform< double >                                                                          VersorTransformType;
	typedef itk::VersorRigid3DTransform< double > 									Versor3DTransformType;
	typedef itk::AffineTransform< double, 3 > 			       						AffineTransformType;        
	//----------------------Interpolator--------------------------
	typedef itk::LinearInterpolateImageFunction< InternalImageType, double > 					LinearInterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction< InternalImageType, double > 				NearestInterpolatorType;
	typedef itk::BSplineInterpolateImageFunction< InternalImageType, double > 					BSplineInterpolatorType;
	//-----------------------Metric-------------------------------
	typedef itk::MeanSquaresImageToImageMetric< InternalImageType, InternalImageType > 	        		MeanSqMetricType;
	typedef itk::NormalizedCorrelationImageToImageMetric< InternalImageType, InternalImageType > 			NorCorMetricType;
	typedef itk::MatchCardinalityImageToImageMetric< InternalImageType, InternalImageType >    			MatchMetricType;
	typedef itk::MutualInformationImageToImageMetric< InternalImageType, InternalImageType >        		MutualMetricType;
  	typedef itk::MattesMutualInformationImageToImageMetric< InternalImageType, InternalImageType >  		MattesMetricType;
	//----------------------Optimizer-----------------------------
	typedef itk::RegularStepGradientDescentOptimizer								ReStepOptimizerType;
	typedef itk::GradientDescentOptimizer										GrDeOptimizerType;
	typedef itk::VersorTransformOptimizer           								VersorOptimizerType;
	typedef itk::VersorRigid3DTransformOptimizer           								Versor3DOptimizerType;
	typedef itk::AmoebaOptimizer       										AmoeOptimizerType;
	typedef itk::OnePlusOneEvolutionaryOptimizer           								PlusOptimizerType;

public:
	PluginSpecialized(V3DPluginCallback* callback,QString transform,QString interpolator,QString metric,QString optimizer,QString reg):Superclass(callback)
	{
		registration = RegistrationType::New();
		multiRegistration = MultiRegistrationType::New();
		fixedImagePyramid = FixedImagePyramidType::New();
  		movingImagePyramid = MovingImagePyramidType::New();
		resample = ResampleFilterType::New();					
		registration_str = reg;
		transform_str = transform;
		metric_str = metric;
		interpolator_str = interpolator;
		optimizer_str = optimizer;

		//this->RegisterInternalFilter( registration, 1.0 );
	}

	PluginSpecialized(V3DPluginCallback* callback):Superclass(callback)
	{	
		resample = ResampleFilterType::New();
	}

	void SetRegTransform(QString transform_str)
	{
		if(transform_str == "TranslationTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			tranTransform = TranTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     tranTransform    );
			else
				registration->SetTransform(     tranTransform    );
		
  			ParametersType initialParameters( tranTransform->GetNumberOfParameters() );

  			initialParameters[0] = 0;  	// Initial offset in mm along X
  			initialParameters[1] = 0;   	// Initial offset in mm along Y
  			initialParameters[2] = 0;  	// Initial offset in mm along Z

  			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( initialParameters );
			else
				registration->SetInitialTransformParameters( initialParameters );
			std::cout << "End the Initialize" << std::endl;			
			numberOfParameters = tranTransform->GetNumberOfParameters();
			std::cout << "numberOfParameters: " << numberOfParameters << std::endl;
		}
		if(transform_str == "ScaleTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			scaleTransform = ScaleTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     scaleTransform    );
			else
				registration->SetTransform(     scaleTransform    );
		
  			ParametersType initialParameters( scaleTransform->GetNumberOfParameters() );

  			initialParameters[0] = 1.0;  	
  			initialParameters[1] = 1.0;   	
  			initialParameters[2] = 1.0;  	

  			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( initialParameters );
			else
				registration->SetInitialTransformParameters( initialParameters );
			std::cout << "End the Initialize" << std::endl;			
			numberOfParameters = scaleTransform->GetNumberOfParameters();
			std::cout << "numberOfParameters: " << numberOfParameters << std::endl;
		}
		if(transform_str == "VersorTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			versorTransform = VersorTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     versorTransform    );
			else
				registration->SetTransform(     versorTransform    );

  			typedef itk::CenteredTransformInitializer< VersorTransformType, InternalImageType, InternalImageType >  TransformInitializerType;
			typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform( versorTransform );
			initializer->SetFixedImage( fixedOut );
			initializer->SetMovingImage( movedOut );
			initializer->MomentsOn();
			initializer->InitializeTransform();

			typedef VersorTransformType::VersorType    VersorType;
			typedef VersorType::VectorType     	   VectorType;
			VersorType     rotation;
			VectorType     axis;
			axis[0] = 0.0;
			axis[1] = 0.0;
			axis[2] = 1.0;
			const double angle = 0;
			rotation.Set(  axis, angle  );
			versorTransform->SetRotation( rotation );

  			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( versorTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( versorTransform->GetParameters() );
			std::cout << "End the Initialize" << std::endl;			
			numberOfParameters = versorTransform->GetNumberOfParameters();
			std::cout << "numberOfParameters: " << numberOfParameters << std::endl;
		}
		
		if(transform_str == "VersorRigid3DTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			versor3DTransform = Versor3DTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     versor3DTransform    );
			else
				registration->SetTransform(     versor3DTransform    );

			typedef itk::CenteredTransformInitializer< Versor3DTransformType, InternalImageType, InternalImageType >  TransformInitializerType;
			typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform( versor3DTransform );
			initializer->SetFixedImage(  fixedOut );
			initializer->SetMovingImage( movedOut );
			initializer->MomentsOn();
			initializer->InitializeTransform();

			typedef Versor3DTransformType::VersorType  VersorType;
			typedef VersorType::VectorType     	   VectorType;
			VersorType     rotation;
			VectorType     axis;
			axis[0] = 1.0;
			axis[1] = 0.0;
			axis[2] = 0.0;
			const double angle = 0;
			rotation.Set(  axis, angle  );
			versor3DTransform->SetRotation( rotation );			
			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( versor3DTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( versor3DTransform->GetParameters() );
			
			std::cout << "End the Initialize" << std::endl;
			numberOfParameters = versor3DTransform->GetNumberOfParameters();
			std::cout << "numberOfParameters: " << numberOfParameters << std::endl;
	
		}
		if(transform_str == "AffineTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			affTransform = AffineTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     affTransform    );
			else
				registration->SetTransform(     affTransform    );

			typedef itk::CenteredTransformInitializer< AffineTransformType, InternalImageType, InternalImageType >  TransformInitializerType;
			typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform( affTransform );
			initializer->SetFixedImage( fixedOut );
			initializer->SetMovingImage( movedOut );
			initializer->MomentsOn();
			initializer->InitializeTransform();
			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( affTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( affTransform->GetParameters() );
  		
			std::cout << "End the Initialize" << std::endl;
			numberOfParameters = affTransform->GetNumberOfParameters();
			std::cout << "numberOfParameters: " << numberOfParameters << std::endl;
		}
	}
	void SetRegInterpolator(QString interpolator_str)
	{
		if(interpolator_str == "LinearInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			linInterpolator = LinearInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( linInterpolator );
			else
   				registration->SetInterpolator( linInterpolator );
			
			resample->SetInterpolator( linInterpolator );
		}
		if(interpolator_str == "NearestNeighborInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			neaInterpolator = NearestInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( neaInterpolator );
			else
   				registration->SetInterpolator( neaInterpolator );

			resample->SetInterpolator( neaInterpolator );
		}
		if(interpolator_str == "BSplineInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			BspInterpolator = BSplineInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( BspInterpolator );
			else
   				registration->SetInterpolator( BspInterpolator );
			
			resample->SetInterpolator( BspInterpolator );
		}		
	}
	void SetRegMetric(QString metric_str)
	{
		if(metric_str == "MeanSquaresImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			meanSqmetric = MeanSqMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( meanSqmetric );
			else
   				registration->SetMetric( meanSqmetric );
		}
		if(metric_str == "NormalizedCorrelationImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			norCormetric = NorCorMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( norCormetric );
			else
   				registration->SetMetric( norCormetric );
		}	
		if(metric_str == "MatchCardinalityImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			matchMetric = MatchMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( matchMetric );
			else
   				registration->SetMetric( matchMetric );
			
			matchMetric->MeasureMatchesOff();
		}
		if(metric_str == "MutualInformationImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			mutualMetric = MutualMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( mutualMetric );
			else
   				registration->SetMetric( mutualMetric );

			mutualMetric->SetFixedImageStandardDeviation(  0.4 );
  			mutualMetric->SetMovingImageStandardDeviation( 0.4 );
			std::cout<<"numberOfSamples: "<<numberOfSamples<<std::endl;
			mutualMetric->SetNumberOfSpatialSamples( numberOfSamples );

		}  
		if(metric_str == "MattesMutualInformationImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			mattesMetric = MattesMetricType::New();			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{		
				multiRegistration->SetMetric( mattesMetric );
				mattesMetric->ReinitializeSeed( 76926294 );
			}
			else
   			{
				registration->SetMetric( mattesMetric );
			}
			mattesMetric->SetNumberOfHistogramBins( numberOfBins );
  			mattesMetric->SetNumberOfSpatialSamples( numberOfSamples );
			
		} 		
					
	}
	void SetRegOptimizer(QString optimizer_str)
	{
		if(optimizer_str == "RegularStepGradientDescentOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			reStepOptimizer = ReStepOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( reStepOptimizer );
			else
   				registration->SetOptimizer( reStepOptimizer );

			if(transform_str == "VersorRigid3DTransformOptimizer")
			{
				typedef Versor3DOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
	    			optimizerScales[0] = 1.0;
				optimizerScales[1] = 1.0;
				optimizerScales[2] = 1.0;
				optimizerScales[3] = translationScale;
				optimizerScales[4] = translationScale;
				optimizerScales[5] = translationScale;

				versor3DOptimizer->SetScales( optimizerScales );
			}
			else if(transform_str == "AffineTransform")
			{
				typedef ReStepOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
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
		
				reStepOptimizer->SetScales( optimizerScales );
				
				reStepOptimizer->SetNumberOfIterations( 300 );
				reStepOptimizer->SetRelaxationFactor( 0.8 );
			}
			else
			{
				reStepOptimizer->SetNumberOfIterations( 300 );
				reStepOptimizer->SetRelaxationFactor( 0.8 );
			}

			if(registration_str != "MultiResolutionImageRegistrationMethod")
			{
				reStepOptimizer->SetMaximumStepLength( 4.0 );
				reStepOptimizer->SetMinimumStepLength( 0.01 );
			}
			reStepOptimizer->MinimizeOn();

			ReCommandIterationUpdate::Pointer observer = ReCommandIterationUpdate::New();
			reStepOptimizer->AddObserver( itk::IterationEvent(), observer );
			
		}
		if(optimizer_str == "GradientDescentOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			grDeOptimizer = GrDeOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( grDeOptimizer );
			else
   				registration->SetOptimizer( grDeOptimizer );
			if(transform_str == "AffineTransform")
			{
				typedef GrDeOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
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
				grDeOptimizer->SetScales( optimizerScales );

			}
			if(transform_str == "VersorRigid3DTransformOptimizer")
			{
				typedef Versor3DOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
	    			optimizerScales[0] = 1.0;
				optimizerScales[1] = 1.0;
				optimizerScales[2] = 1.0;
				optimizerScales[3] = translationScale;
				optimizerScales[4] = translationScale;
				optimizerScales[5] = translationScale;

				versor3DOptimizer->SetScales( optimizerScales );
			}
			grDeOptimizer->SetLearningRate( 15.0 );
 			grDeOptimizer->SetNumberOfIterations( 200 );
  			grDeOptimizer->MaximizeOn();
			
			GrCommandIterationUpdate::Pointer observer = GrCommandIterationUpdate::New();
			grDeOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "VersorTransformOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			versorOptimizer = VersorOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( versorOptimizer );
			else
   				registration->SetOptimizer( versorOptimizer );

			typedef VersorOptimizerType::ScalesType       OptimizerScalesType;
			OptimizerScalesType optimizerScales( numberOfParameters );	
    			optimizerScales[0] = 1.0;
			optimizerScales[1] = 1.0;
			optimizerScales[2] = 1.0;
			versorOptimizer->SetScales( optimizerScales );

			double maximumStepLength = 0.2;
			double minimumStepLength = 0.0001;
			int numberOfIterations = 200;

			versorOptimizer->SetMaximumStepLength( maximumStepLength );
			versorOptimizer->SetMinimumStepLength( minimumStepLength );
			versorOptimizer->SetNumberOfIterations( numberOfIterations );
			
			//Create the Command observer and register it with the optimizer.
			VeCommandIterationUpdate::Pointer observer = VeCommandIterationUpdate::New();
			versorOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "VersorRigid3DTransformOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			versor3DOptimizer = Versor3DOptimizerType::New();
			std::cout<<"Test dialog14!"<<std::endl;
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( versor3DOptimizer );
			else
   				registration->SetOptimizer( versor3DOptimizer );
			std::cout<<"Test dialog15!"<<std::endl;
			typedef Versor3DOptimizerType::ScalesType       OptimizerScalesType;
			OptimizerScalesType optimizerScales( numberOfParameters );
			double translationScale = 1.0 / 1000.0;
    			optimizerScales[0] = 1.0;
			optimizerScales[1] = 1.0;
			optimizerScales[2] = 1.0;
			optimizerScales[3] = translationScale;
			optimizerScales[4] = translationScale;
			optimizerScales[5] = translationScale;
			versor3DOptimizer->SetScales( optimizerScales );
	
			double maximumStepLength = 2;
			double minimumStepLength = 0.0001;
			int numberOfIterations = 200;
			
			versor3DOptimizer->SetMaximumStepLength( maximumStepLength );
			versor3DOptimizer->SetMinimumStepLength( minimumStepLength );
			versor3DOptimizer->SetNumberOfIterations( numberOfIterations );
			
			//Create the Command observer and register it with the optimizer.
			Ve3DCommandIterationUpdate::Pointer observer = Ve3DCommandIterationUpdate::New();
			versor3DOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "AmoebaOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			amoeOptimizer = AmoeOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( amoeOptimizer );
			else
   				registration->SetOptimizer( amoeOptimizer );
			if(transform_str == "VersorRigid3DTransformOptimizer")
			{
				typedef Versor3DOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
	    			optimizerScales[0] = 1.0;
				optimizerScales[1] = 1.0;
				optimizerScales[2] = 1.0;
				optimizerScales[3] = translationScale;
				optimizerScales[4] = translationScale;
				optimizerScales[5] = translationScale;

				versor3DOptimizer->SetScales( optimizerScales );
			}
			if(transform_str == "AffineTransform")
			{
				typedef AmoeOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
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
		
				amoeOptimizer->SetScales( optimizerScales );

			}
			AmoeOptimizerType::ParametersType simplexDelta( numberOfParameters );
			simplexDelta.Fill( 5.0 );

			amoeOptimizer->AutomaticInitialSimplexOff();
			amoeOptimizer->SetInitialSimplexDelta( simplexDelta );			 
			amoeOptimizer->SetParametersConvergenceTolerance( 0.25 ); // quarter pixel
			amoeOptimizer->SetFunctionConvergenceTolerance(0.001); // 0.1%
			amoeOptimizer->SetMaximumNumberOfIterations( 200 );
		
			AmoeCommandIterationUpdate::Pointer observer = AmoeCommandIterationUpdate::New();
			amoeOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "OnePlusOneEvolutionaryOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			plusOptimizer = PlusOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( plusOptimizer );
			else
   				registration->SetOptimizer( plusOptimizer );

			if(transform_str == "VersorRigid3DTransformOptimizer")
			{
				typedef Versor3DOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
	    			optimizerScales[0] = 1.0;
				optimizerScales[1] = 1.0;
				optimizerScales[2] = 1.0;
				optimizerScales[3] = translationScale;
				optimizerScales[4] = translationScale;
				optimizerScales[5] = translationScale;

				versor3DOptimizer->SetScales( optimizerScales );
			}
			if(transform_str == "AffineTransform")
			{
				typedef PlusOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
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
		
				plusOptimizer->SetScales( optimizerScales );

			}
			typedef itk::Statistics::NormalVariateGenerator  GeneratorType;
			GeneratorType::Pointer generator = GeneratorType::New();
			generator->Initialize(12345);
			plusOptimizer->MaximizeOff();

			plusOptimizer->SetNormalVariateGenerator( generator );
			plusOptimizer->Initialize( 10 );
			plusOptimizer->SetEpsilon( 1.0 );
			plusOptimizer->SetMaximumIteration( 400);

			PlusCommandIterationUpdate::Pointer observer = PlusCommandIterationUpdate::New();
			plusOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
	}

	void GetFinalParameters(QString transform_str,QString optimizer_str)
	{
		std::cout << "Get Parameters:" << std::endl;
		unsigned int numberOfIterations;
		double bestValue;	
		if(optimizer_str == "RegularStepGradientDescentOptimizer")
		{
			numberOfIterations = reStepOptimizer->GetCurrentIteration();
			bestValue = reStepOptimizer->GetValue();
		}		
		if(optimizer_str == "GradientDescentOptimizer")
		{
			numberOfIterations = grDeOptimizer->GetCurrentIteration();
			bestValue = grDeOptimizer->GetValue();
		}
		if(optimizer_str == "VersorTransformOptimizer")
		{
			numberOfIterations = versorOptimizer->GetCurrentIteration();
			bestValue = versorOptimizer->GetValue();
		}	
		if(optimizer_str == "VersorRigid3DTransformOptimizer")
		{
			numberOfIterations = versor3DOptimizer->GetCurrentIteration();
			bestValue = versor3DOptimizer->GetValue();
		}
		
		if(optimizer_str == "AmoebaOptimizerOptimizer")
		{
			numberOfIterations = 200;
			bestValue = amoeOptimizer->GetCachedValue();
		}
		if(optimizer_str == "OnePlusOneEvolutionaryOptimizer")
		{
			numberOfIterations = plusOptimizer->GetCurrentIteration();
			bestValue = plusOptimizer->GetValue();
		}
		std::cout << "Result = " << std::endl;
		std::cout << " Iterations    = " << numberOfIterations << std::endl;
		std::cout << " Metric value  = " << bestValue          << std::endl;
		paraList<<numberOfIterations<<bestValue;
		
		if(transform_str == "TranslationTransform")
		{
			TranTransformType::Pointer finalTransform = TranTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );
				
				std::cout << " TranslationX = " << finalParameters[0]  << std::endl;
				std::cout << " TranslationX = " << finalParameters[1]  << std::endl;
				std::cout << " TranslationX = " << finalParameters[2]  << std::endl;
				
				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}
			else
			{
		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				std::cout << " TranslationX = " << finalParameters[0]  << std::endl;
				std::cout << " TranslationX = " << finalParameters[1]  << std::endl;
				std::cout << " TranslationX = " << finalParameters[2]  << std::endl;
				
				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}
  			
  			finalTransform->SetFixedParameters( tranTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "TranslationTransform";
			resample->SetTransform( finalTransform );
		}

		if(transform_str == "ScaleTransform")
		{
			ScaleTransformType::Pointer finalTransform = ScaleTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );
	
				std::cout << " scaleX = " << finalParameters[0]  << std::endl;
				std::cout << " scaleY = " << finalParameters[1]  << std::endl;
				std::cout << " scaleZ = " << finalParameters[2]  << std::endl;
				
				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}
			else
			{		
				ParametersType finalParameters = registration->GetLastTransformParameters();			
				finalTransform->SetParameters( finalParameters );

				std::cout << " scaleX = " << finalParameters[0]  << std::endl;
				std::cout << " scaleY = " << finalParameters[1]  << std::endl;
				std::cout << " scaleZ = " << finalParameters[2]  << std::endl;
				
				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}			
  			//finalTransform->SetFixedParameters( scaleTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "ScaleTransform";
			resample->SetTransform( finalTransform );
		}
		if(transform_str == "VersorTransform")
		{
			VersorTransformType::Pointer finalTransform = VersorTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				std::cout << " versorX = " << finalParameters[0]  << std::endl;
				std::cout << " versorY = " << finalParameters[1]  << std::endl;
				std::cout << " versorZ = " << finalParameters[2]  << std::endl;

				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];				
			}
			else
			{		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				std::cout << " versorX = " << finalParameters[0]  << std::endl;
				std::cout << " versorY = " << finalParameters[1]  << std::endl;
				std::cout << " versorZ = " << finalParameters[2]  << std::endl;

				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}				
			
			// Print out results
			VersorTransformType::MatrixType matrix = versorTransform->GetRotationMatrix();		
			std::cout << "Matrix = " << std::endl << matrix << std::endl;	
  			
  			finalTransform->SetFixedParameters( versorTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "VersorTransform";
			resample->SetTransform( finalTransform );
		}	
		if(transform_str == "VersorRigid3DTransform")
		{

			Versor3DTransformType::Pointer finalTransform = Versor3DTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				std::cout << " versorX = " << finalParameters[0]  << std::endl;
				std::cout << " versorY = " << finalParameters[1]  << std::endl;
				std::cout << " versorZ = " << finalParameters[2]  << std::endl;
				std::cout << " finalTranslationX = " << finalParameters[3]  << std::endl;
				std::cout << " finalTranslationY = " << finalParameters[4]  << std::endl;
				std::cout << " finalTranslationZ = " << finalParameters[5]  << std::endl;

				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
				
			}
			else
			{		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				std::cout << " versorX = " << finalParameters[0]  << std::endl;
				std::cout << " versorY = " << finalParameters[1]  << std::endl;
				std::cout << " versorZ = " << finalParameters[2]  << std::endl;
				std::cout << " finalTranslationX = " << finalParameters[3]  << std::endl;
				std::cout << " finalTranslationY = " << finalParameters[4]  << std::endl;
				std::cout << " finalTranslationZ = " << finalParameters[5]  << std::endl;

				//Put the last parameters to paraList
				for(unsigned int i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];	
			}
			// Print out results
			Versor3DTransformType::MatrixType matrix = versor3DTransform->GetRotationMatrix();
			Versor3DTransformType::OffsetType offset = versor3DTransform->GetOffset();			
			std::cout << "Matrix = " << std::endl << matrix << std::endl;
			std::cout << "Offset = " << std::endl << offset << std::endl;		
  			
  			finalTransform->SetFixedParameters( versor3DTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "VersorRigid3DTransform";
			resample->SetTransform( finalTransform );
		}	
		if(transform_str == "AffineTransform")
		{

			AffineTransformType::Pointer finalTransform = AffineTransformType::New();	
			// Print out results			
			std::cout << " M = " << std::endl;
			unsigned int i;
			unsigned int j;		
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				for(i = 0; i < 3; i++)
				{
					for(j = 0; j < 3; j++)
						std::cout << finalParameters[3*i+j] <<"	";
					std::cout << std::endl;
				}
				std::cout << " T = "<< std::endl;
				for(i = 9; i < 12; i++)
					std::cout << finalParameters[i] <<std::endl;		

				//Put the last parameters to paraList
				for(i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];						
			}
			else
			{		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				finalTransform->SetParameters( finalParameters );

				for(i = 0; i < 3; i++)
				{
					for(j = 0; j < 3; j++)
						std::cout << finalParameters[3*i+j] <<"	";
					std::cout << std::endl;
				}
				std::cout << " T = "<< std::endl;
				for(i = 9; i < 12; i++)
					std::cout << finalParameters[i] <<std::endl;		

				//Put the last parameters to paraList
				for(i = 0;i < numberOfParameters; i ++)
					paraList<<finalParameters[i];		
			}		
  			
  			finalTransform->SetFixedParameters( affTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "AffineTransform";
			resample->SetTransform( finalTransform );
		}
		
	}
	QList<double> GetParaList()
	{
		return paraList;
	}
	void Execute(QWidget *parent)
	{       
		this->SetImageSelectionDialogTitle("ITK-V3D Registration");
    		this->AddImageSelectionLabel("FixedImage: ");
    		this->AddImageSelectionLabel("MovedImage: ");
    		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback); 
		if(registration_str == "MultiResolutionImageRegistrationMethod")
		{
			typedef RegistrationInterfaceCommand<MultiRegistrationType> CommandType;
			CommandType::Pointer command = CommandType::New();
			multiRegistration->AddObserver( itk::IterationEvent(), command );
		}
		
		this->Compute();		    
		
	}	
	
	virtual void ComputeOneRegion()
	{		
		std::cout << "Set Parameters!" << std::endl;
		
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;
		
		arg.p=(void*)this->GetInput3DImage1();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		fixedOut=(InternalImageType*)(output.at(0).p);

		arg.p=(void*)this->GetInput3DImage2();
		input.replace(0,arg);
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		movedOut=(InternalImageType*)(output.at(0).p);
		if(metric_str == "MattesMutualInformationImageToImageMetric")
		{ 
			V3DITKGenericDialog dialog("Set Up Parameters");
			dialog.AddDialogElement("numberOfBins:", 24.0, 8.0, 256.0);
			dialog.AddDialogElement("numberOfSamples:", 50000.0, 1000.0, 100000.0);
			if(dialog.exec() == QDialog::Accepted )
			{
				numberOfBins = dialog.GetValue("numberOfBins:");
				numberOfSamples = dialog.GetValue("numberOfSamples:");
			}	
			//const unsigned int numberOfPixels = fixedOut->GetBufferedRegion().GetNumberOfPixels();  
    			//numberOfSamples = static_cast< unsigned int >( numberOfPixels * 0.01 );
			std::cout << "numberOfSamples: "<<numberOfSamples<< std::endl;  			
		}
		if(metric_str == "MutualInformationImageToImageMetric")
		{
			V3DITKGenericDialog dialog("Set Up Parameters");
			dialog.AddDialogElement("numberOfSamples:", 5000.0, 500.0, 100000.0);
			if(dialog.exec() == QDialog::Accepted )
			{
				numberOfSamples = dialog.GetValue("numberOfSamples:");
			}	
			//Normalize
			arg.p =(void*)fixedOut;		
			input.replace(0,arg);	
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Normalize/Normalize.so";
		        function_name="Normalize";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* fixedTempOut=(InternalImageType*)(output.at(0).p);
			//DisGauSmooth
			arg.p=(void*)fixedTempOut;
			input.replace(0,arg);			
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/DisGauSmooth/DisGauSmooth.so";
		        function_name="DisGauSmooth";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* fixedSmoothOut=(InternalImageType*)(output.at(0).p);
			
			arg.p=(void*)movedOut;
			input.replace(0,arg);
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Normalize/Normalize.so";
		        function_name="Normalize";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* movedTempOut=(InternalImageType*)(output.at(0).p);
			//DisGauSmooth
			arg.p=(void*)movedTempOut;
			input.replace(0,arg);			
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/DisGauSmooth/DisGauSmooth.so";
		        function_name="DisGauSmooth";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* movedSmoothOut=(InternalImageType*)(output.at(0).p);

			const unsigned int numberOfPixels = fixedSmoothOut->GetBufferedRegion().GetNumberOfPixels();  
    			numberOfSamples = static_cast< unsigned int >( numberOfPixels * 0.01 );  
			std::cout << "numberOfSamples: "<<numberOfSamples<< std::endl;
			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				multiRegistration->SetFixedImage(    fixedSmoothOut   );
				multiRegistration->SetMovingImage(   movedSmoothOut  );		
			}
			else
			{
				registration->SetFixedImage(    fixedSmoothOut   );
				registration->SetMovingImage(   movedSmoothOut  );
			}	
						
		}		

		//Cast unsigned char to float by calling CastIn
		else
		{
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				multiRegistration->SetFixedImage(    fixedOut   );
				multiRegistration->SetMovingImage(   movedOut  );		
			}
			else
			{
				registration->SetFixedImage(    fixedOut   );
				registration->SetMovingImage(   movedOut  );
			}	
		}

		SetRegTransform(transform_str);		
		SetRegInterpolator(interpolator_str);		
		SetRegMetric(metric_str);		
		SetRegOptimizer(optimizer_str);
		
		if(registration_str == "MultiResolutionImageRegistrationMethod")
		{
			FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
  			MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();
			multiRegistration->SetFixedImagePyramid( fixedImagePyramid );
  			multiRegistration->SetMovingImagePyramid( movingImagePyramid );
			multiRegistration->SetFixedImageRegion(fixedOut->GetBufferedRegion() );
			multiRegistration->SetNumberOfLevels( 3 );
			std::cout << "Parameters Set Finished" << std::endl;
			//参数在构造函数中设置完毕，开始单分辨率配准
			std::cout << "Start Registration..." << std::endl;
			try 
			{ 
			multiRegistration->StartRegistration(); 
			std::cout << "Optimizer stop condition: "
			<< multiRegistration->GetOptimizer()->GetStopConditionDescription()
			<< std::endl;
			} 
			catch( itk::ExceptionObject & err ) 
			{ 
			std::cerr << "ExceptionObject caught !" << std::endl; 
			std::cerr << err << std::endl; 
			} 
			std::cout << "\nMultiRegistration Finished! " << std::endl;
		}
		else
		{
			registration->SetFixedImageRegion(fixedOut->GetBufferedRegion() );
			std::cout << "Parameters Set Finished" << std::endl;
			//参数在构造函数中设置完毕，开始单分辨率配准
			std::cout << "Start Registration..." << std::endl;
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
			} 
			std::cout << "\nRegistration Finished! " << std::endl;
		}
		GetFinalParameters(transform_str,optimizer_str);
                //开始重采样
		std::cout << "Start Resample..." << std::endl;

		/*resample->SetInput( movedOut );
		resample->SetSize( fixedOut->GetLargestPossibleRegion().GetSize() );
		resample->SetOutputOrigin(  fixedOut->GetOrigin() );
		resample->SetOutputSpacing( fixedOut->GetSpacing() );
		resample->SetOutputDirection( fixedOut->GetDirection() );
		resample->SetDefaultPixelValue( 0 );
		resample->Update();*/

		//call resample
		V3DPluginArgItem arg1;
		V3DPluginArgItem arg2;
		V3DPluginArgList resInput;
		arg1.p=(void*)fixedOut;
		arg2.p=(void*)movedOut;
		arg2.type=interpolator_str;
		resInput<<tran_arg<<arg1<<arg2;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Resample/Resample.so";
                function_name="Resample";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,resInput,output);
		InternalImageType* resOut=(InternalImageType*)(output.at(0).p);
		

		//cast float to unsigned char 
		//arg.p = (void*)resample->GetOutput();
		arg.p = (void*)resOut;		
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so";
                function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);
		this->SetOutputImage(unOut);		
	}	
	
	void ImageSubtract()
	{
		this->SetImageSelectionDialogTitle("ITK-V3D Subtract");
    		this->AddImageSelectionLabel("FixedImage: ");
    		this->AddImageSelectionLabel("MovedImage: ");
    		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

		this->Initialize();
		
		const V3DLONG x1 = 0;
		const V3DLONG y1 = 0;
		const V3DLONG z1 = 0;

		const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
		const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
		const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

		//get image pointers
		v3dhandleList wndlist = this->m_V3DPluginCallback->getImageWindowList();
		if(wndlist.size()<2)
		{
			v3d_msg(QObject::tr("Subtract needs at least two images!"));
			return;
		}

		if( this->m_ImageSelectionDialog.exec() != QDialog::Accepted )
		{
			return;
		}   

		Image4DSimple* p4DImage_1 = this->GetInputImageFromIndex( 0 );
		Image4DSimple* p4DImage_2 = this->GetInputImageFromIndex( 1 );

		QList< V3D_Image3DBasic > inputImageList1 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_1, *(this->m_V3DPluginCallback) );
		QList< V3D_Image3DBasic > inputImageList2 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_2, *(this->m_V3DPluginCallback) );

		//Add the progress dialog 
		V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );
		this->AddObserver( progressDialog.GetCommand() );
		progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
		progressDialog.show();

		const unsigned int numberOfChannelsToProcess = inputImageList1.size();
		if (numberOfChannelsToProcess<=0)
			return;
		
		for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
		{
			const V3D_Image3DBasic inputImage1 = inputImageList1.at(channel);
			const V3D_Image3DBasic inputImage2 = inputImageList2.at(channel);

			this->TransferInputDualImages( inputImage1, inputImage2, x1, x2, y1, y2, z1, z2 );

			this->SubtractOneRegion();

			this->AddOutputImageChannel( channel );
		}
	
		this->ComposeOutputImage();	
		
	}
	void SubtractOneRegion()
	{
		//Cast unsigned char to float
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;
		
		arg.p=(void*)this->GetInput3DImage1();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* fixedImage=(InternalImageType*)(output.at(0).p);

		arg.p=(void*)this->GetInput3DImage2();
		input.replace(0,arg);
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* movedImage=(InternalImageType*)(output.at(0).p);

		//call SubResampleV3DPluginArgItem arg1;
		V3DPluginArgItem arg1,arg2;
		V3DPluginArgList resInput;
		arg1.p=(void*)fixedImage;
		arg2.p=(void*)movedImage;
		arg1.type=transform_str;
		arg2.type=interpolator_str;
		resInput<<arg1<<arg2;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/SubResample/SubResample.so";
                function_name="SubResample";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,resInput,output);
		InternalImageType* resaOut=(InternalImageType*)(output.at(0).p);
				
		//调用subtract插件
		arg.p = (void*)fixedImage;
		input.replace(0,arg);
		V3DPluginArgItem temp;
		temp.p = (void*)resaOut;
		input<<temp;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/ImageSubtract/ImageSubtract.so";
		function_name="ImageSubtract";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* diffOut=(InternalImageType*)(output.at(0).p);

		//call Rescale
		arg.p = (void*)diffOut;
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/RescaleIntensity/RescaleIntensity.so";
		function_name="RescaleIntensity";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* resOut=(InternalImageType*)(output.at(0).p);

		//cast float to unsigned char 
		arg.p = (void*)resOut;
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so"; 
                function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);
		
		this->SetOutputImage(unOut);		
	}


private:
	QString plugin_name;
        QString function_name;
	QString registration_str;
	QString transform_str;
	QString metric_str;
	QString interpolator_str;
	QString optimizer_str;

	typename TranTransformType::Pointer  		tranTransform;
	typename ScaleTransformType::Pointer  		scaleTransform;
	typename VersorTransformType::Pointer  		versorTransform;
	typename Versor3DTransformType::Pointer  	versor3DTransform;
	typename AffineTransformType::Pointer  		affTransform;
	typename LinearInterpolatorType::Pointer      	linInterpolator;
	typename NearestInterpolatorType::Pointer      	neaInterpolator;
	typename BSplineInterpolatorType::Pointer      	BspInterpolator;
	typename MeanSqMetricType::Pointer  		meanSqmetric;
	typename NorCorMetricType::Pointer  		norCormetric;
	typename MatchMetricType::Pointer  		matchMetric;
	typename MutualMetricType::Pointer  		mutualMetric;
	typename MattesMetricType::Pointer  		mattesMetric;	
	typename ReStepOptimizerType::Pointer      	reStepOptimizer;
	typename GrDeOptimizerType::Pointer      	grDeOptimizer;
	typename VersorOptimizerType::Pointer      	versorOptimizer;
	typename Versor3DOptimizerType::Pointer      	versor3DOptimizer;
	typename AmoeOptimizerType::Pointer      	amoeOptimizer;
	typename PlusOptimizerType::Pointer      	plusOptimizer;
	
	unsigned int numberOfSamples;
	unsigned int numberOfParameters;
	unsigned int numberOfBins;

	typename RegistrationType::Pointer              registration;
	typename MultiRegistrationType::Pointer   	multiRegistration;
	typename FixedImagePyramidType::Pointer 	fixedImagePyramid;
	typename MovingImagePyramidType::Pointer 	movingImagePyramid;
	typename ResampleFilterType::Pointer 		resample;
	
	V3DPluginArgItem 				tran_arg;
	InternalImageType* 				fixedOut;
	InternalImageType* 				movedOut;
	QList<double>                                   paraList;

};
#endif
