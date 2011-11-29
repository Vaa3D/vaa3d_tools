
#include "Subtract.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkSubtractImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Subtract, SubtractPlugin)


QStringList SubtractPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Subtract")
            << QObject::tr("about this plugin");
}

QStringList SubtractPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterDualImage< TPixelType, TPixelType >  	 Superclass;
  typedef typename Superclass::Input3DImageType               	 ImageType;
  typedef itk::SubtractImageFilter< ImageType, ImageType > 	 FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
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

    this->m_Filter->SetInput1( this->GetInput3DImage1() );
    this->m_Filter->SetInput2( this->GetInput3DImage2() );

    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
    }


private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void SubtractPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void SubtractPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Subtract 1.0 (2010-Jul-31): this plugin is developed by Luis Ibanez.");
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

