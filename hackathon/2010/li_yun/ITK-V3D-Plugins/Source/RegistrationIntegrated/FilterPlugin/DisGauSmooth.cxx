#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DisGauSmooth.h"
#include "V3DITKFilterSingleImage.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImage.h"

Q_EXPORT_PLUGIN2(DisGauSmooth, DisGauSmoothPlugin)

QStringList DisGauSmoothPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK DisGauSmooth")
            << QObject::tr("about this plugin");
}

QStringList DisGauSmoothPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK DisGauSmooth")
            << QObject::tr("about this plugin");
}

template<typename TPixelType>
class PluginSpecialized: public V3DITKFilterSingleImage< TPixelType,TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >  					Superclass;
	typedef typename Superclass::Input3DImageType			                		ImageType;
	typedef float                                    						InternalPixelType;
  	typedef itk::Image< InternalPixelType, 3 > 							InternalImageType;
	typedef itk::DiscreteGaussianImageFilter<InternalImageType,InternalImageType> 			DisFilterType;
public:
	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->m_disFilter = DisFilterType::New();	
		this->RegisterInternalFilter( this->m_disFilter, 1.0 );  		
	}

	virtual ~PluginSpecialized() {};

	void Execute(const QString &menu_name, QWidget *parent)
	{
		float gaussianVariance=1.0;
		unsigned int maxKernelWidth=4;
		this->m_disFilter->SetVariance( gaussianVariance );
		this->m_disFilter->SetMaximumKernelWidth( maxKernelWidth );
		this->Compute();
	}

	virtual void ComputeOneRegion()
	{    
		//cast unsigned char to float
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;

		arg.p=(void*)this->GetInput3DImage();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* out=(InternalImageType*)(output.at(0).p);

		//DisGauSmooth
		this->m_disFilter->SetInput(out);
		this->m_disFilter->Update();
		
		//cast float to unsigned char 
		arg.p = (void*)this->m_disFilter->GetOutput();
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so"; 
		function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);

		this->SetOutputImage(unOut);	
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		std::cout << "Welcome to DisGauSmooth!" << std::endl;
		InternalImageType* p=(InternalImageType*)input.at(0).p;
		if(!p)
		{
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));
			return;
		}	

		this->m_disFilter->SetInput( p );
		this->m_disFilter->Update();
		V3DPluginArgItem arg;
		arg.p=(void*)this->m_disFilter->GetOutput();
		output.replace(0,arg);
		std::cout << "DisGauSmooth Finished!" << std::endl;
	}

private:
	QString	plugin_name;
	QString function_name;
	typename DisFilterType::Pointer   	m_disFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool DisGauSmoothPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK DisGauSmooth with float data type (2011-May): this plugin is developed by Li Yun");
	return false ;
	}
	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void DisGauSmoothPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK DisGauSmooth with float data type (2011-May): this plugin is developed by Li Yun");
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

