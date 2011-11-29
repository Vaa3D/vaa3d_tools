#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DoubleThreshold.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkDoubleThresholdImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DoubleThreshold, DoubleThresholdPlugin)


QStringList DoubleThresholdPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK DoubleThreshold")
            << QObject::tr("about this plugin");
}

QStringList DoubleThresholdPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::DoubleThresholdImageFilter< ImageType, ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("Double Threshold");

    //
    // This filter may require a smarter dialog
    //
    dialog.AddDialogElement("Threshold 1",32.0, 0.0, 255.0);
    dialog.AddDialogElement("Threshold 2",64.0, 0.0, 255.0);
    dialog.AddDialogElement("Threshold 3",128.0, 0.0, 255.0);
    dialog.AddDialogElement("Threshold 4",256.0, 0.0, 255.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetThreshold1( dialog.GetValue("Threshold 1" ) );
      this->m_Filter->SetThreshold2( dialog.GetValue("Threshold 2" ) );
      this->m_Filter->SetThreshold3( dialog.GetValue("Threshold 3" ) );
      this->m_Filter->SetThreshold4( dialog.GetValue("Threshold 4" ) );

      this->m_Filter->SetFullyConnected( 0 );

      this->m_Filter->SetInsideValue( 255 );
      this->m_Filter->SetOutsideValue( 0 );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

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


void DoubleThresholdPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void DoubleThresholdPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK DoubleThreshold 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

