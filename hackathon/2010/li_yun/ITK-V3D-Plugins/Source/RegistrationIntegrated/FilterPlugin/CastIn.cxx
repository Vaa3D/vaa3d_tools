#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "CastIn.h"
#include "V3DITKFilterSingleImage.h"
#include "itkCastImageFilter.h"
#include "itkImage.h"

Q_EXPORT_PLUGIN2(CastIn, CastInPlugin)

QStringList CastInPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CastIn")
            << QObject::tr("about this plugin");
}

QStringList CastInPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK CastIn")
            << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage<TPixelType, TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   		Superclass;
	typedef typename Superclass::Input3DImageType               		ImageType;
	typedef float                                    			InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 				InternalImageType;
	typedef itk::CastImageFilter< ImageType,InternalImageType > 		FilterType;

public:

	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->m_Filter = FilterType::New();
		this->RegisterInternalFilter( this->m_Filter, 1.0 );
	}

	virtual ~PluginSpecialized() {};

	void Execute(const QString &menu_name, QWidget *parent)
	{
		this->Compute();
	}

	virtual void ComputeOneRegion()
	{
		this->m_Filter->SetInput( this->GetInput3DImage() );
		this->m_Filter->Update();

		//cast float to unsigned char
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;
		arg.p = (void*)this->m_Filter->GetOutput();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so"; 
		function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);

		this->SetOutputImage(unOut);		
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		std::cout << "Welcome to CastIn!" << std::endl;
		ImageType* p=(ImageType*)input.at(0).p;
		if(!p)
		{
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));
			return;
		}
		this->m_Filter->SetInput( p );
		this->m_Filter->Update();

		V3DPluginArgItem arg;
		arg.p=(void*)this->m_Filter->GetOutput();
		output.replace(0,arg);
		std::cout << "CastIn Finished!" << std::endl;
	}


private:
	QString	plugin_name;
	QString function_name;
	typename FilterType::Pointer   	m_Filter;
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }

bool CastInPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Cast In with float data type (2011-May): this plugin is developed by Li Yun");
		return false ;
	}
	PluginSpecialized< unsigned char > *runner=new PluginSpecialized< unsigned char >(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void CastInPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Cast In with float data type (2011-May): this plugin is developed by Li Yun");
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

