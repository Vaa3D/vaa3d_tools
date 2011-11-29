#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "BinaryThreshold.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkBinaryThresholdImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BinaryThreshold, BinaryThresholdPlugin)


QStringList BinaryThresholdPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK BinaryThreshold")
            << QObject::tr("about this plugin");
}

QStringList BinaryThresholdPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("Binary Threshold");

    dialog.AddDialogElement("Lower",128.0, 0.0, 255.0);
    dialog.AddDialogElement("Upper",255.0, 0.0, 255.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetUpperThreshold( dialog.GetValue("Upper") );
      this->m_Filter->SetLowerThreshold( dialog.GetValue("Lower") );
      this->m_Filter->SetInsideValue(255);
      this->m_Filter->SetOutsideValue(0);

      this->Compute();
      }

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


void BinaryThresholdPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void BinaryThresholdPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK BinaryThreshold 1.0 (2010-Jun-21): this plugin is developed by Sophie Chen.");
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

