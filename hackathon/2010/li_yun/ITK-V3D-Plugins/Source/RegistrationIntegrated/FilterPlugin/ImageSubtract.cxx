#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ImageSubtract.h"
#include "V3DITKFilterDualImage.h"
#include "itkSubtractImageFilter.h"

Q_EXPORT_PLUGIN2(ImageSubtract, SubtractPlugin)

QStringList SubtractPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Subtract")
            << QObject::tr("about this plugin");
}

QStringList SubtractPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK Subtract")
            << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterDualImage< TPixelType, TPixelType >   					Superclass;
	typedef typename Superclass::Input3DImageType               					ImageType;
	typedef float                                    						InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 							InternalImageType;
	typedef itk::SubtractImageFilter< InternalImageType, InternalImageType, InternalImageType > 	DifferenceFilterType;
public:

	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		difference = DifferenceFilterType::New();	
		this->RegisterInternalFilter( difference, 1.0 );	
	}

	virtual ~PluginSpecialized() {};

	void Execute(const QString &menu_name, QWidget *parent)
	{
		this->SetImageSelectionDialogTitle("Subtract Input Images");
		this->AddImageSelectionLabel("Image 1");
		this->AddImageSelectionLabel("Image 2");
		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
		this->Compute();
	}

	virtual void ComputeOneRegion()
	{
		/*this->m_Filter->SetInput( this->GetInput3DImage1() );
		this->m_Filter->SetInput( this->GetInput3DImage2() );
		this->m_Filter->Update();
		this->SetOutputImage( this->m_Filter->GetOutput() );*/

		//cast unsigned char to float
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;

		arg.p=(void*)this->GetInput3DImage1();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* firstOut=(InternalImageType*)(output.at(0).p);

		arg.p=(void*)this->GetInput3DImage2();
		input.replace(0,arg);
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* secondOut=(InternalImageType*)(output.at(0).p);

		//Subtract
		difference->SetInput1( firstOut );
		difference->SetInput2( secondOut );
		difference->Update();

		//cast float to unsigned char 
		arg.p = (void*)difference->GetOutput();
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so"; 
		function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);

		this->SetOutputImage(unOut);	
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		InternalImageType* p1=(InternalImageType*)input.at(0).p;
		if(!p1)
		{
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));
			return;
		}
	
		InternalImageType* p2=(InternalImageType*)input.at(1).p;
		if(!p2)
		{
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));
			return;
		}	

		difference->SetInput1( p1 );
		difference->SetInput2( p2 );
		difference->Update();

		V3DPluginArgItem arg;
		arg.p=difference->GetOutput();
		output.replace(0,arg);
	}


private:

	QString	plugin_name;
	QString function_name;
	typename DifferenceFilterType::Pointer	difference;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool SubtractPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Subtract with float data type (2011-May): this plugin is developed by Li Yun");
	return false ;
	}
	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void SubtractPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Subtract with float data type (2011-May): this plugin is developed by Li Yun");
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

