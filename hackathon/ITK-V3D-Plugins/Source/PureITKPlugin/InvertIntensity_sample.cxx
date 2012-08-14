#include "InvertIntensity_sample.h"

#include "itkInvertIntensityImageFilter.h"

#include <QDebug>
#include <QMessageBox>
// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(InvertIntensity, InvertIntensityPlugin)

QStringList InvertIntensityPlugin::menulist() const
{
  return QStringList() << QObject::tr("Method to Use ITK InvertIntensity")
                        << QObject::tr("about this plugin");
}
QStringList InvertIntensityPlugin::funclist() const
{
  return QStringList() << QObject::tr("Invert Intensity");
}

void InvertIntensityPlugin::domenu(const QString& menu_name, V3DPluginCallback2& callback, QWidget* parent)
{
  if (menu_name == QObject::tr("Method to Use ITK InvertIntensity")) {
      QMessageBox::information(parent, "Intronduction to Use", "This plugin is pure ITK plugin, can be called by Superplugin in Vaa3d only, please place it under the folder : plugins/Vaa3D_ITK/");
      return;
  }
  
  if (menu_name == QObject::tr("about this plugin")) {
      QMessageBox::information(parent, "Version info", "ITK InvertIntensity 1.0 (2012-8-8): this plugin is developed by ping yu");
      return;
  }
}
///itk function plugin
template <typename TPixelType>
class ItkFuncPlugin
{
  typedef TPixelType  PixelType;
  typedef itk::Image<PixelType, 3>  ImageType;
  typedef itk::InvertIntensityImageFilter<ImageType, ImageType> FilterType;
  public:
    ItkFuncPlugin()
    {
      m_filter = FilterType::New();
    }
    ~ItkFuncPlugin(){}
    bool execute(const V3DPluginArgList& input, V3DPluginArgList& output)
    {
      typename ImageType::Pointer inputImage = (ImageType*) input.at(0).p;
      m_filter->SetInput(inputImage);
      try {
        m_filter->Update();
      }catch(...)
      {
        qDebug() << "error running";
        return false;
      }
      typename ImageType::Pointer outputImage = m_filter->GetOutput();
      outputImage->Register();
      V3DPluginArgItem arg;
      arg.p = (void*)outputImage;
      arg.type = input.at(0).type;
      output.replace(0, arg);
      return true;
    }
  private:
    typename FilterType::Pointer m_filter;
};

bool InvertIntensityPlugin::dofunc(const QString& func_name, const V3DPluginArgList& input, V3DPluginArgList& output, V3DPluginCallback2& callback, QWidget* parent)
{
  //if more function to do determine by the func_name;
  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin) {
    qDebug() << "No image open in the main window";
    v3d_msg(tr("No image open in the main window"));
    return false;
  }
  //do the func
  if ( input.at(0).type == QString("UINT8Image")) {
    ItkFuncPlugin<unsigned char> runner;
    return runner.execute(input, output);
    }
  else if (input.at(0).type == QString("UINT16Image")) {
    ItkFuncPlugin<unsigned short int> runner;
    return runner.execute(input, output);
    }
  else if (input.at(0).type == QString("FLOATImage")) {
    ItkFuncPlugin<float> runner;
    return runner.execute(input, output);
    }
  else {
    qDebug() << "No data type support, run error";
    return false;
    }
}
  
