#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ConnectedComponents.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkConnectedComponentFunctorImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ConnectedComponents, ConnectedComponentsPlugin)


QStringList ConnectedComponentsPlugin::menulist() const
{
    return QStringList() << QObject::tr("Detect Connected Components")
  << QObject::tr("about this plugin");
}

QStringList ConnectedComponentsPlugin::funclist() const
{
    return QStringList();
}


template <typename TInPixelType, typename TOutputPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >
{
  typedef V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;
  typedef typename Superclass::Output3DImageType              OutputImageType;

  typedef itk::ConnectedComponentImageFilter< ImageType, OutputImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
      SetupParameters();
      this->Compute();
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
    }
  virtual void SetupParameters()
    {
    V3DITKGenericDialog dialog("Connected Components");

    dialog.AddDialogElement("FullyConnected",0.0, 0.0, 1.0); // This should be a boolean
    dialog.AddDialogElement("BackgroundValue",0.0, 0.0, 255.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetFullyConnected( (int)dialog.GetValue("FullyConnected") ); // This should be a boolean
      this->m_Filter->SetBackgroundValue( dialog.GetValue("BackgroundValue") );
      }
    }

private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type, unsigned short int > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void ConnectedComponentsPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void ConnectedComponentsPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Connnected Components 1.0 (2010-June-4): this plugin is developed by Hanchuan Peng.");
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

