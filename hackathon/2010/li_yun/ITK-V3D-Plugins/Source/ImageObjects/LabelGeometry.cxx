#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "LabelGeometry.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkLabelGeometryImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(LabelGeometry, LabelGeometryPlugin)


QStringList LabelGeometryPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK LabelGeometryImageFilter")
	<< QObject::tr("about this plugin");
}

QStringList LabelGeometryPlugin::funclist() const
{
    return QStringList();
}


template <typename TInputPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
  typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType                          ImageType;
  typedef typename Superclass::Output3DImageType                         OutputImageType;
	
  typedef itk::LabelGeometryImageFilter< ImageType, OutputImageType > FilterType;
	
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
	}
	
	this->m_Filter->Update();
    }
	
    virtual void SetupParameters()
    {
	//
	// These values should actually be provided by the Qt Dialog...
	// just search the respective .h file for the itkSetMacro for parameters
	V3DITKGenericDialog dialog("ITK LabelGeometryImageFilter");

	dialog.AddDialogElement("CalculatePixelIndices",1.0, 0.0, 1.0);
	dialog.AddDialogElement("CalculateOrientedBoundingBox",1.0, 0.0, 1.0);
	dialog.AddDialogElement("OrientedLabelRegions",1.0, 0.0, 1.0);
	dialog.AddDialogElement("OrientedIntensityRegions",1.0, 0.0, 1.0);

	this->m_Filter->SetCalculatePixelIndices( dialog.GetValue("CalculatePixelIndices") );		
	this->m_Filter->SetCalculateOrientedBoundingBox( dialog.GetValue("CalculateOrientedBoundingBox") );		
	this->m_Filter->SetOrientedLabelRegions( dialog.GetValue("OrientedLabelRegions") );		
	this->m_Filter->SetOrientedIntensityRegions( dialog.GetValue("OrientedIntensityRegions") );		


	//respectively, MapType, LabelGeometry, LabelsType	
	this->m_Filter->SetLabelGeometryMapper();
	this->m_Filter->SetLabelGeometry();
	this->m_Filter->SetAllLabels();
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


void LabelGeometryPlugin::dofunc(const QString & func_name,
				const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


void LabelGeometryPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
    {
		QMessageBox::information(parent, "Version info", "ITK LabelGeometry 1.0 (2010-July-15): this plugin is developed by Sophie Chen.");
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

