#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "BinaryStatisticsKeepNObjects.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkBinaryStatisticsKeepNObjectsImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BinaryStatisticsKeepNObjects, BinaryStatisticsKeepNObjectsPlugin)


QStringList BinaryStatisticsKeepNObjectsPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK BinaryStatisticsKeepNObjectsImageFilter")
                       << QObject::tr("about this plugin");
}

QStringList BinaryStatisticsKeepNObjectsPlugin::funclist() const
{
  return QStringList();
}


template <typename TInputPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
{
  typedef V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType                          ImageType;
  typedef typename Superclass::Output3DImageType                         OutputImageType;
	
  typedef itk::BinaryStatisticsKeepNObjectsImageFilter< ImageType,  OutputImageType> FilterType;
	
public:
	
  MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
  {
    this->m_Filter = FilterType::New();
	this->RegisterInternalFilter(this->m_Filter,1.0);
  }
	
  virtual ~MySpecialized() {};
	
	
  void Execute(const QString &menu_name, QWidget *parent)
  {

	this->SetImageSelectionDialogTitle("BinaryStaticKeepNObjects");
	this->AddImageSelectionLabel("Image 1");
	this->AddImageSelectionLabel("Image 2");
	this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
    V3DITKGenericDialog dialog("BinaryStatisticsKeepNObjects");

    dialog.AddDialogElement("BackgroundValue",0.0, 0.0, 255.0);
    dialog.AddDialogElement("ForegroundValue",0.0, 0.0, 255.0);
    dialog.AddDialogElement("NumberOfObjects",100.0, 0.0, 50000.0);
    dialog.AddDialogElement("Attribute",0.0, 0.0, 10.0);

    if( dialog.exec() == QDialog::Accepted )
      {
        this->m_Filter->SetFullyConnected( true );
        this->m_Filter->SetBackgroundValue( dialog.GetValue("BackgroundValue") );
        this->m_Filter->SetForegroundValue( dialog.GetValue("ForegroundValue") );
        this->m_Filter->SetNumberOfObjects( dialog.GetValue("NumberOfObjects") );
        this->m_Filter->SetReverseOrdering( false );
        this->m_Filter->SetAttribute( 0 );
        this->Compute(); 
      }
  }
	
  virtual void ComputeOneRegion()
  {
    this->m_Filter->SetInput1( this->GetInput3DImage1() );
    this->m_Filter->SetInput2( this->GetInput3DImage2() );

    if( !this->ShouldGenerateNewWindow() )
      {
      }
		
    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
  }
	
private:
	
  typename FilterType::Pointer   m_Filter;
	
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type:                                                  \
  {                                                                     \
    MySpecialized< c_pixel_type, c_pixel_type > runner( &callback );    \
    runner.Execute( menu_name, parent );                                \
    break;                                                              \
  } 


void BinaryStatisticsKeepNObjectsPlugin::dofunc(const QString & func_name,
                                                const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void BinaryStatisticsKeepNObjectsPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
      QMessageBox::information(parent, "Version info", "ITK BinaryStatisticsKeepNObjects 1.0 (2010-July-15): this plugin is developed by Sophie Chen.");
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

