#ifndef _Header_H_
#define _Header_H_

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>
#include "V3DITKFilterDualImage.h"
#include "itkImageRegistrationMethod.h"
#include "V3DITKFilterBaseImage.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkResampleImageFilter.h"
//ITK Metric
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMatchCardinalityImageToImageMetric.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
//ITK Interpolator
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
//ITK Transform
#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkVersorTransform.h"
#include "itkAffineTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransform.h"
//ITK Optimizer
#include "itkGradientDescentOptimizer.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorTransformOptimizer.h"//Only for VersorTransform
#include "itkVersorRigid3DTransformOptimizer.h"//Only for VersorRigid3DTransform
#include "itkAmoebaOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkNormalVariateGenerator.h" 
#include "itkSubtractImageFilter.h"
#include "itkCommand.h"
class ReCommandIterationUpdate : public itk::Command 
{
public:
	typedef  ReCommandIterationUpdate	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self> 	Pointer;
 	itkNewMacro( Self );

protected:
  	ReCommandIterationUpdate() {};

public:

	typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
	typedef const OptimizerType                          *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
		         dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		std::cout << optimizer->GetCurrentIteration() << " : ";
		std::cout << optimizer->GetValue() << " : ";
		std::cout << optimizer->GetCurrentPosition() << std::endl;
	}
};
class GrCommandIterationUpdate : public itk::Command 
{
public:
	typedef  GrCommandIterationUpdate  	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self>  	Pointer;
  	itkNewMacro( Self );

protected:
  	GrCommandIterationUpdate() {};

public:
	typedef itk::GradientDescentOptimizer     OptimizerType;
	typedef const OptimizerType               *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
		         dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		std::cout << optimizer->GetCurrentIteration() << " = ";
		std::cout << optimizer->GetValue() << " : ";
		std::cout << optimizer->GetCurrentPosition() << std::endl;
  	}
   
};
class VeCommandIterationUpdate : public itk::Command 
{
public:
	typedef  VeCommandIterationUpdate   	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self>  	Pointer;
	itkNewMacro( Self );

protected:
	VeCommandIterationUpdate() {};

public:

	typedef itk::VersorTransformOptimizer       OptimizerType;
	typedef const OptimizerType                 *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
				 dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		std::cout << optimizer->GetCurrentIteration() << " = ";
		std::cout << optimizer->GetValue() << " : ";
		std::cout << optimizer->GetCurrentPosition() << std::endl;
	}
   
};
class Ve3DCommandIterationUpdate : public itk::Command 
{
public:
	typedef  Ve3DCommandIterationUpdate   	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self>  	Pointer;
	itkNewMacro( Self );

protected:
	Ve3DCommandIterationUpdate() {};

public:

	typedef itk::VersorRigid3DTransformOptimizer       OptimizerType;
	typedef const OptimizerType                        *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
				 dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		std::cout << optimizer->GetCurrentIteration() << " = ";
		std::cout << optimizer->GetValue() << " : ";
		std::cout << optimizer->GetCurrentPosition() << std::endl;
	}
   
};
class AmoeCommandIterationUpdate : public itk::Command 
{
public:
	typedef  AmoeCommandIterationUpdate	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self> 	Pointer;
 	itkNewMacro( Self );

protected:
  	AmoeCommandIterationUpdate() 
	{
	    m_IterationNumber=0;
	}
public:

	typedef itk::AmoebaOptimizer		    	     OptimizerType;
	typedef const OptimizerType                          *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
		         dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		std::cout << m_IterationNumber++ << "   ";
		std::cout << optimizer->GetCachedValue() << "   ";
		std::cout << optimizer->GetCachedCurrentPosition() << std::endl;
	}
private:
  	unsigned long m_IterationNumber;
};
class PlusCommandIterationUpdate : public itk::Command 
{
public:
	typedef  PlusCommandIterationUpdate	Self;
	typedef  itk::Command             	Superclass;
	typedef itk::SmartPointer<Self> 	Pointer;
 	itkNewMacro( Self );

protected:
  	PlusCommandIterationUpdate() {};

public:

	typedef itk::OnePlusOneEvolutionaryOptimizer	     OptimizerType;
	typedef const OptimizerType                          *OptimizerPointer;

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute( (const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		OptimizerPointer optimizer = 
		         dynamic_cast< OptimizerPointer >( object );

		if( ! itk::IterationEvent().CheckEvent( &event ) )
		{
		return;
		}
		double currentValue = optimizer->GetValue();
		// Only print out when the Metric value changes
		if( vcl_fabs( m_LastMetricValue - currentValue ) > 1e-7 )
		{ 
			std::cout << currentValue << "   ";
			std::cout << optimizer->GetFrobeniusNorm() << "   ";
			std::cout << optimizer->GetCurrentPosition() << std::endl;
			m_LastMetricValue = currentValue;
		}
	}
private:
  	double m_LastMetricValue;
};

template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command 
{
public:
	typedef  RegistrationInterfaceCommand   Self;
	typedef  itk::Command                   Superclass;
	typedef  itk::SmartPointer<Self>        Pointer;
	itkNewMacro( Self );
protected:
	RegistrationInterfaceCommand() {};
public:
	typedef   TRegistration                              RegistrationType;
	typedef   RegistrationType *                         RegistrationPointer;
	typedef   itk::RegularStepGradientDescentOptimizer   OptimizerType;
	typedef   OptimizerType *                            OptimizerPointer;
  
	void Execute(itk::Object * object, const itk::EventObject & event)
	{

		if( !(itk::IterationEvent().CheckEvent( &event )) )
		{
		return;
		}

		RegistrationPointer registration =
			    dynamic_cast<RegistrationPointer>( object );

		OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( 
		       registration->GetOptimizer() );

		std::cout << "-------------------------------------" << std::endl;
		std::cout << "MultiResolution Level : "
		<< registration->GetCurrentLevel()  << std::endl;
		std::cout << std::endl;

		if ( registration->GetCurrentLevel() == 0 )
		{
		optimizer->SetMaximumStepLength( 16.00 );  
		optimizer->SetMinimumStepLength( 1.0 );
		}
		else
		{
		optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() / 4.0 );
		optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() / 10.0 );
		}
		std::cout << "optimizer->GetMaximumStepLength(): "
		<< optimizer->GetMaximumStepLength()  << std::endl;

		std::cout << "optimizer->GetMinimumStepLength(): "
		<< optimizer->GetMinimumStepLength()  << std::endl;
	}

	void Execute(const itk::Object * , const itk::EventObject & )
	{ return; }
};
#endif
