#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Relabel.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkRelabelComponentImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BinaryStatisticsOpening, BinaryStatisticsOpeningPlugin)


QStringList BinaryStatisticsOpeningPlugin::menulist() const
{
    return QStringList() << QObject::tr("Relabel Indexed Image Objects")
	<< QObject::tr("about this plugin");
}

QStringList BinaryStatisticsOpeningPlugin::funclist() const
{
    return QStringList();
}


template <typename TInPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >   Superclass;
	typedef typename Superclass::Input3DImageType               ImageType;
	typedef typename Superclass::Output3DImageType              OutputImageType;
	
	typedef itk::RelabelComponentImageFilter< ImageType, OutputImageType > FilterType;
	
public:
	
	MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
		this->m_Filter = FilterType::New();
    }
	
	virtual ~MySpecialized() {};
	
	
	void Execute(const QString &menu_name, QWidget *parent)
    {
		this->Compute(); 
    }
	
	virtual void ComputeOneRegion()
    {
		
		this->m_Filter->SetInput( this->GetInput3DImage() );
		
		if( !this->ShouldGenerateNewWindow() )
		{
			this->m_Filter->InPlaceOn();
		}
		
		this->m_Filter->Update();
		
		this->SetOutputImage( this->m_Filter->GetOutput() );
    }
	
	virtual void SetupParameters()
  	{
		//
		// These values should actually be provided by the Qt Dialog...
		//
		
		this->m_Filter->SetFullyConnected( true );
    this->m_Filter->SetBackgroundValue( 0 );
    this->m_Filter->SetForegroundValue( 100 );
    this->m_Filter->SetLambda( 5.0 );
    this->m_Filter->SetReverseOrdering( false );
    this->m_Filter->SetBS(0);    
    this->m_Filter->SetAttribute( 0 );
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


void BinaryStatisticsOpeningPlugin::dofunc(const QString & func_name,
									 const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


void BinaryStatisticsOpeningPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
    {
		QMessageBox::information(parent, "Version info", "ITK Relabel Image Objects (Connnected Components) 1.0 (2010-June-4): this plugin is developed by Hanchuan Peng.");
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

