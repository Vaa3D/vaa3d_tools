#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Resample.h"
#include "V3DITKFilterSingleImage.h"
#include "itkResampleImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkVersorTransform.h"
#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"

Q_EXPORT_PLUGIN2(Resample, ResamplePlugin)

QStringList ResamplePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Resample")
            << QObject::tr("about this plugin");
}

QStringList ResamplePlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK Resample")
            << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   			Superclass;
	typedef typename Superclass::Input3DImageType               			ImageType;
	typedef float                                    				InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 					InternalImageType;
	typedef itk::TranslationTransform< double, 3 > 					TranTransformType;
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
		typename TranTransformType::Pointer tranTransform = TranTransformType::New();		
		typedef TranTransformType::ParametersType ParametersType;
  		ParametersType Parameters( tranTransform->GetNumberOfParameters() );

  		Parameters[0] = 0.0;  // Initial offset in mm along X
  		Parameters[1] = 0.0;  // Initial offset in mm along Y
  		Parameters[2] = 0.0;  // Initial offset in mm along Z

		V3DITKGenericDialog dialog("Set Translation");
		dialog.AddDialogElement("TranslationX", -10.0, -100.0, 100.0 );
		dialog.AddDialogElement("TranslationY", -8.0, -100.0, 100.0 );
		dialog.AddDialogElement("TranslationZ", 0.0, -100.0, 100.0 ); 

		if( dialog.exec() == QDialog::Accepted )
		{
			Parameters[0] = dialog.GetValue("TranslationX");
			Parameters[1] = dialog.GetValue("TranslationY");
			Parameters[2] = dialog.GetValue("TranslationZ");
			
			tranTransform->SetParameters( Parameters );		
			this->m_Filter->SetTransform( tranTransform );
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
		std::cout << "Welcome to Resample..." << std::endl;

		void * p1=NULL;
		void * p2=NULL;
		void * p3=NULL;

		p1=input.at(0).p;
		p2=input.at(1).p;
		p3=input.at(2).p;
                QString transform_str=input.at(0).type;
		QString interpolator_str=input.at(2).type;
		std::cout << "Resample-----transform_str: " <<transform_str.toStdString()<<std::endl;
		std::cout << "Resample-----interpolator_str: " <<interpolator_str.toStdString()<<std::endl;

		if((p1 == p2)||(p1==p3)||(p2==p3)||(!p1)||(!p2)||(!p3))
			std::cout << "Input Error!"<<std::endl;

		if(transform_str == "TranslationTransform")
		{
			typedef itk::TranslationTransform< double, 3 > 					TransformType;
			this->m_ReFilter->SetTransform( (TransformType*) p1 );
		}
		if(transform_str == "ScaleTransform")
		{
			typedef itk::ScaleTransform< double, 3 > 					TransformType;	
			this->m_ReFilter->SetTransform( (TransformType*) p1 );
		}
		if(transform_str == "VersorTransform")
		{	
			typedef itk::VersorTransform< double > 						TransformType;	
			this->m_ReFilter->SetTransform( (TransformType*) p1 );
		}		
		if(transform_str == "VersorRigid3DTransform")
		{
			typedef itk::VersorRigid3DTransform< double > 					TransformType;			
			this->m_ReFilter->SetTransform( (TransformType*) p1 );
		}
		if(transform_str == "AffineTransform")
                {
			typedef itk::AffineTransform< double, 3 > 			       		TransformType;
			this->m_ReFilter->SetTransform( (TransformType*) p1 );
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
		this->m_ReFilter->SetInput( (InternalImageType*) p3 );
		InternalImageType* fixedImage = (InternalImageType*) p2;
		this->m_ReFilter->SetSize(    fixedImage->GetLargestPossibleRegion().GetSize() );
		this->m_ReFilter->SetOutputOrigin(  fixedImage->GetOrigin() );
		this->m_ReFilter->SetOutputSpacing( fixedImage->GetSpacing() );
		this->m_ReFilter->SetOutputDirection( fixedImage->GetDirection() );
		this->m_ReFilter->SetDefaultPixelValue( 0 );
		this->m_ReFilter->Update();

		V3DPluginArgItem arg;
		arg.p=(void*)this->m_ReFilter->GetOutput();
		output.replace(0,arg);
		std::cout << "Resample Finished!" << std::endl;
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


bool ResamplePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
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


void ResamplePlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
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

