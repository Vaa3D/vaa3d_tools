#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "SubResample.h"
#include "V3DITKFilterSingleImage.h"
#include "itkResampleImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkVersorTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"

Q_EXPORT_PLUGIN2(SubResample, SubResamplePlugin)

QStringList SubResamplePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK SubResample")
            << QObject::tr("about this plugin");
}

QStringList SubResamplePlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK SubResample")
            << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   			Superclass;
	typedef typename Superclass::Input3DImageType               			ImageType;
	typedef float                                    				InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 					InternalImageType;
	typedef itk::ScaleTransform< double, 3 > 					ScaleTransformType;
	typedef itk::ResampleImageFilter< ImageType, ImageType > 			FilterType;
	typedef itk::ResampleImageFilter< InternalImageType, InternalImageType > 	ReFilterType;

public:
	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->m_Filter = FilterType::New();
		this->RegisterInternalFilter( this->m_Filter, 1.0 );

		this->m_ReFilter = ReFilterType::New();
	}

	virtual ~PluginSpecialized() {};

	void Execute(const QString &menu_name, QWidget *parent)
	{		
		typename ScaleTransformType::Pointer scaleTransform = ScaleTransformType::New();		
		typedef ScaleTransformType::ParametersType ParametersType;
  		ParametersType Parameters( scaleTransform->GetNumberOfParameters() );

  		Parameters[0] = 1.0;  // Scale along X
  		Parameters[1] = 1.0;  // Scale along Y
  		Parameters[2] = 1.0;  // Scale along Z

		V3DITKGenericDialog dialog("Set ScaleTransform");
		dialog.AddDialogElement("ScaleX", 0.8, 0.1, 10.0 );
		dialog.AddDialogElement("ScaleY", 0.8, 0.1, 10.0 );
		dialog.AddDialogElement("ScaleZ", 1.0, 0.1, 10.0 );

		if( dialog.exec() == QDialog::Accepted )
		{
			Parameters[0] = dialog.GetValue("ScaleX");
			Parameters[1] = dialog.GetValue("ScaleY");
			Parameters[2] = dialog.GetValue("ScaleZ");
			
			scaleTransform->SetParameters( Parameters );		
			this->m_Filter->SetTransform( scaleTransform );
			this->Compute();
		}  		
	}

	virtual void ComputeOneRegion()
	{
		
		this->m_Filter->SetInput( this->GetInput3DImage() );
		this->m_Filter->SetSize( this->GetInput3DImage()->GetLargestPossibleRegion().GetSize() );
		this->m_Filter->SetOutputOrigin( this->GetInput3DImage()->GetOrigin() );
		this->m_Filter->SetOutputSpacing( this->GetInput3DImage()->GetSpacing() );
		this->m_Filter->SetOutputDirection( this->GetInput3DImage()->GetDirection() );
		this->m_Filter->SetDefaultPixelValue( 1 );
		
		this->m_Filter->Update();
		
		this->SetOutputImage( this->m_Filter->GetOutput() );

	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{  
		std::cout << "Welcome to Resample for ImageSubtract..." << std::endl;

		void * p1=NULL;
		void * p2=NULL;
		QString transform_str;
		QString interpolator_str;

		p1=input.at(0).p;
		p2=input.at(1).p;
		transform_str=input.at(0).type;
		interpolator_str=input.at(1).type;
               	std::cout << "SubResample-----transform_str: " <<transform_str.toStdString()<<std::endl;
		std::cout << "SubResample-----interpolator_str: " <<interpolator_str.toStdString()<<std::endl;

		if(transform_str == "TranslationTransform")
		{
			typedef itk::TranslationTransform< double, 3 > 		TransformType;	
			TransformType::Pointer identityTransform = TransformType::New();
			identityTransform->SetIdentity();
			this->m_ReFilter->SetTransform( identityTransform );
		}
		if(transform_str == "ScaleTransform")
		{
			typedef itk::ScaleTransform< double, 3 > 		TransformType;
			TransformType::Pointer identityTransform = TransformType::New();
			identityTransform->SetIdentity();
			this->m_ReFilter->SetTransform( identityTransform );
		}
		if(transform_str == "VersorTransform")
		{	
			typedef itk::VersorTransform< double >                  TransformType;
			TransformType::Pointer identityTransform = TransformType::New();
			identityTransform->SetIdentity();
			this->m_ReFilter->SetTransform( identityTransform );
		}	

		if(transform_str == "VersorRigid3DTransform")
		{
			typedef itk::VersorRigid3DTransform< double > 		TransformType;
			TransformType::Pointer identityTransform = TransformType::New();
			identityTransform->SetIdentity();
			this->m_ReFilter->SetTransform( identityTransform );
		}
		if(transform_str == "AffineTransform")
		{
			typedef itk::AffineTransform< double, 3 >		TransformType;
			TransformType::Pointer identityTransform = TransformType::New();
			identityTransform->SetIdentity();
			this->m_ReFilter->SetTransform( identityTransform );
		}

		//Set Interpolator
		if(interpolator_str == "LinearInterpolateImageFunction")
		{
			typedef itk::LinearInterpolateImageFunction< InternalImageType, double > 		InterpolatorType;
			typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
			this->m_ReFilter->SetInterpolator( interpolator );
		}
		if(interpolator_str == "NearestNeighborInterpolateImageFunction")
		{
			typedef itk::NearestNeighborInterpolateImageFunction< InternalImageType, double > 	InterpolatorType;
			typename InterpolatorType::Pointer interpolator =InterpolatorType::New();
			this->m_ReFilter->SetInterpolator( interpolator );
		}
		if(interpolator_str == "BSplineInterpolateImageFunction")
		{
			typedef itk::BSplineInterpolateImageFunction< InternalImageType, double > 		InterpolatorType;
			typename InterpolatorType::Pointer interpolator =InterpolatorType::New();
			this->m_ReFilter->SetInterpolator( interpolator );
		}
		
		this->m_ReFilter->SetInput( (InternalImageType*) p2 );
		InternalImageType* fixedImage = (InternalImageType*) p1;
		this->m_ReFilter->SetSize(    fixedImage->GetLargestPossibleRegion().GetSize() );
		this->m_ReFilter->SetOutputOrigin(  fixedImage->GetOrigin() );
		this->m_ReFilter->SetOutputSpacing( fixedImage->GetSpacing() );
		this->m_ReFilter->SetOutputDirection( fixedImage->GetDirection() );
		this->m_ReFilter->SetDefaultPixelValue( 0 );
		this->m_ReFilter->Update();

		V3DPluginArgItem arg;
		arg.p=(void*)this->m_ReFilter->GetOutput();
		output.replace(0,arg);
		std::cout << "SubResample Finished!" << std::endl;
	}    


private:
	typename FilterType::Pointer 	m_Filter;
	typename ReFilterType::Pointer 	m_ReFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool SubResamplePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Resample with float data type (2011-May): this plugin is developed by Li Yun.");
	return false ;
	}

	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void SubResamplePlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Resample with float data type (2011-May): this plugin is developed by Li Yun.");
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

