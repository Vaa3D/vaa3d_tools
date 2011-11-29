#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "CastOut.h"
#include "V3DITKFilterSingleImage.h"
#include "itkCastImageFilter.h"

Q_EXPORT_PLUGIN2(CastOut, CastOutPlugin)

QStringList CastOutPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CastOut")
            << QObject::tr("about this plugin");
}

QStringList CastOutPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK CastOut")
            << QObject::tr("about this plugin");
}


template <typename TInputPixelType,typename TOutputPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >   		Superclass;
	typedef typename Superclass::Output3DImageType              			OutputImageType;
	typedef float                                    				InternalPixelType;
  	typedef itk::Image< InternalPixelType, 3 > 					InternalImageType;	
	typedef itk::CastImageFilter< InternalImageType, OutputImageType > 		FilterType;

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
		//cast input type to float
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

		this->m_Filter->SetInput( out );
		this->m_Filter->Update();
		this->SetOutputImage( this->m_Filter->GetOutput() );
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		std::cout << "Welcome to CastOut!" << std::endl;	
		InternalImageType* p=(InternalImageType*)input.at(0).p;
		if(!p)
		{
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));
			return;
		}
		this->m_Filter->SetInput( p );
		this->m_Filter->Update();

		V3DPluginArgItem arg;
		arg.p=(void*)m_Filter->GetOutput();
		output.replace(0,arg);
		std::cout << "CastOut Finished!" << std::endl;
	}

private:
	QString	plugin_name;
	QString function_name;
	typename FilterType::Pointer   m_Filter;

};


#define EXECUTE( v3d_pixel_type, c_input_pixel_type, c_output_pixel_type ) \
  case v3d_pixel_type: \
    { \
	PluginSpecialized< c_input_pixel_type, c_output_pixel_type > runner(&callback); \
    	runner.Execute( menu_name, parent ); \
   	break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
	if (! p4DImage) return; \
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


bool CastOutPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
	{
	if (func_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Cast Out with float data type (2011-May): this plugin is developed by Li Yun");
		return false ;
	}
	PluginSpecialized<float, unsigned char> *runner=new PluginSpecialized<float, unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void CastOutPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Cast Out with float data type (2011-May): this plugin is developed by Li Yun");
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

