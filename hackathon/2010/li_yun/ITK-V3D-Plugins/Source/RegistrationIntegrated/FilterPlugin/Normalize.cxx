#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Normalize.h"
#include "V3DITKFilterSingleImage.h"
#include "itkNormalizeImageFilter.h"
#include "itkImage.h"

Q_EXPORT_PLUGIN2(Normalize, NormalizePlugin)

QStringList NormalizePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Normalize")
            << QObject::tr("about this plugin");
}

QStringList NormalizePlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK Normalize")
            << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   			Superclass;
	typedef typename Superclass::Input3DImageType               			InputImageType;
	typedef typename Superclass::Output3DImageType               			OutputImageType;
	typedef float                                    				InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 					InternalImageType;
	typedef itk::NormalizeImageFilter< InputImageType, OutputImageType >      	FilterType;
	typedef itk::NormalizeImageFilter< InternalImageType, InternalImageType >       NormalizeFilterType;

public:

	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->m_Filter = FilterType::New();
		this->RegisterInternalFilter( this->m_Filter, 1.0 );

		this->m_norFilter = NormalizeFilterType::New();

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
		this->SetOutputImage( this->m_Filter->GetOutput() );
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		std::cout << "Welcome to Normalize!" << std::endl;
		InternalImageType* p=(InternalImageType*)input.at(0).p;
		if(!p)
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));

		this->m_norFilter->SetInput( p );
		this->m_norFilter->Update();
		V3DPluginArgItem arg;
		arg.p=(void*)this->m_norFilter->GetOutput();
		output.replace(0,arg);
		std::cout << "Normalize Finished!" << std::endl;
	}


private:

	typename FilterType::Pointer   		m_Filter;
	typename NormalizeFilterType::Pointer   	m_norFilter;
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool NormalizePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Normalize with float data type (2011-May): this plugin is developed by Li Yun");
	return false ;
	}
	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void NormalizePlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Normalize with float data type (2011-May): this plugin is developed by Li Yun");
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

