#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "RescaleIntensity.h"
#include "V3DITKFilterSingleImage.h"
#include "itkRescaleIntensityImageFilter.h"

Q_EXPORT_PLUGIN2(RescaleIntensity, RescaleIntensityPlugin)

QStringList RescaleIntensityPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK RescaleIntensity")
            << QObject::tr("about this plugin");
}

QStringList RescaleIntensityPlugin::funclist() const
{
    return QStringList() << QObject::tr("ITK RescaleIntensity")
            << QObject::tr("about this plugin");
}


template <typename TInputPixelType, typename TOutputPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >   			Superclass;
	typedef typename Superclass::Input3DImageType               				InputImageType;
	typedef typename Superclass::Output3DImageType              				OutputImageType;
	typedef itk::RescaleIntensityImageFilter< InputImageType, OutputImageType > 		FilterType;

	typedef float                                    					InternalPixelType;
	typedef itk::Image< InternalPixelType, 3 > 						InternalImageType;
	typedef itk::RescaleIntensityImageFilter< InternalImageType, InternalImageType >	RescalerType;

public:

	PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->m_Filter = FilterType::New();
		this->RegisterInternalFilter( this->m_Filter, 1.0 );

		intensityRescaler = RescalerType::New();
	}

	virtual ~PluginSpecialized() {};

	void Execute(const QString &menu_name, QWidget *parent)
	{
		V3DITKGenericDialog dialog("Rescale");

		dialog.AddDialogElement("Output Minimum",1.0, 0.0, 255.0);
		dialog.AddDialogElement("Output Maximum",255.0, 0.0, 255.0);

		if( dialog.exec() == QDialog::Accepted )
		{
			this->m_Filter->SetOutputMinimum( dialog.GetValue("Output Minimum") );
			this->m_Filter->SetOutputMaximum( dialog.GetValue("Output Maximum") );	
			this->Compute();	
		}
	
	}

	virtual void ComputeOneRegion()
	{
		this->m_Filter->SetInput( this->GetInput3DImage() );
		this->m_Filter->Update();
		this->SetOutputImage( this->m_Filter->GetOutput() );
	}

	void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
	{
		InternalImageType* p=(InternalImageType*)input.at(0).p;
		if(!p)
			v3d_msg(QObject::tr("Null Pointer! Please check your code!"));

		intensityRescaler->SetInput( p );
		intensityRescaler->SetOutputMinimum(   0 );
		intensityRescaler->SetOutputMaximum( 255 );

		intensityRescaler->Update();
		V3DPluginArgItem arg;
		arg.p=(void*)intensityRescaler->GetOutput();
		output.replace(0,arg);

	}

private:

	typename FilterType::Pointer   	m_Filter;
	typename RescalerType::Pointer 	intensityRescaler;
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


bool RescaleIntensityPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
	if (func_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK RescaleIntensity with float data type (2011-May): this plugin is developed by Li Yun.");
	return false ;
	}
	PluginSpecialized <unsigned char,unsigned char> *runner=new PluginSpecialized <unsigned char,unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 

	return true;
}


void RescaleIntensityPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK RescaleIntensity with float data type (2011-May): this plugin is developed by Li Yun.");
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

