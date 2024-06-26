#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "BinaryShapeOpening.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkBinaryShapeOpeningImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BinaryShapeOpening, BinaryShapeOpeningPlugin)


QStringList BinaryShapeOpeningPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK BinaryShapeOpeningImageFilter")
	<< QObject::tr("about this plugin");
}

QStringList BinaryShapeOpeningPlugin::funclist() const
{
    return QStringList();
}


template <typename TInputPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >   Superclass;
	typedef typename Superclass::Input3DImageType               ImageType;
	
	typedef itk::BinaryShapeOpeningImageFilter< ImageType > FilterType;
	
public:
	
    MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
	this->m_Filter = FilterType::New();
        this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }
	
    virtual ~MySpecialized() {};
	
	
    void Execute(const QString &menu_name, QWidget *parent)
    {
        SetupParameters();
	this->Compute(); 
    }
	
    virtual void ComputeOneRegion()
    {
		
	this->m_Filter->SetInput( this->GetInput3DImage() );
		
	if( !this->ShouldGenerateNewWindow() )
	{
        //this->m_Filter->InPlaceOn();
	}
		
	this->m_Filter->Update();

        this->SetOutputImage( this->m_Filter->GetOutput() );
    }
	
    virtual void SetupParameters()
    {
	//
	// These values should actually be provided by the Qt Dialog...
	// just search the respective .h file for the itkSetMacro for parameters
        V3DITKGenericDialog dialog("BinaryShapeOpening");
        
        dialog.AddDialogElement("FullyConnected",1.0, 0.0, 1.0);
        dialog.AddDialogElement("BackgroundValue",0.0, 0.0, 255.0);
        dialog.AddDialogElement("ForegroundValue",100.0, 0.0, 255.0);
        dialog.AddDialogElement("Lambda",5.0, 0.0, 50.0);
        dialog.AddDialogElement("ReverseOrdering",0.0, 0.0, 1.0);
        dialog.AddDialogElement("Attribute",0.0, 0.0, 10.0);

        if( dialog.exec() == QDialog::Accepted )
        {
	this->m_Filter->SetFullyConnected( dialog.GetValue("FullyConnected") );
	this->m_Filter->SetBackgroundValue( dialog.GetValue("BackgroundValue") );
	this->m_Filter->SetForegroundValue( dialog.GetValue("ForegroundValue") );
	this->m_Filter->SetLambda( dialog.GetValue("Lambda") );
	this->m_Filter->SetReverseOrdering( dialog.GetValue("ReverseOrdering") );
	this->m_Filter->SetAttribute( dialog.GetValue("Attribute") );
        }
	
    }
	
private:
	
    typename FilterType::Pointer   m_Filter;
	
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
case v3d_pixel_type: \
{ \
MySpecialized< c_pixel_type, c_pixel_type > runner( &callback ); \
runner.Execute( menu_name, parent ); \
break; \
} 


void BinaryShapeOpeningPlugin::dofunc(const QString & func_name,
				const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


void BinaryShapeOpeningPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
    {
		QMessageBox::information(parent, "Version info", "ITK BinaryShapeOpening 1.0 (2010-July-15): this plugin is developed by Sophie Chen.");
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

