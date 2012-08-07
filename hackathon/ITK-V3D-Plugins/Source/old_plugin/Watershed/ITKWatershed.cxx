#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKWatershed.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkWatershedImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImageFileWriter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Watershed, WatershedPlugin)


QStringList WatershedPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Watershed")
            << QObject::tr("about this plugin");
}

QStringList WatershedPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, unsigned short int >
{
  typedef V3DITKFilterSingleImage< TPixelType, unsigned short int >   Superclass;
  typedef typename Superclass::Input3DImageType                       ImageType;

  typedef itk::Image< unsigned short int, 3 > OutputImageType;

  typedef itk::WatershedImageFilter< ImageType > WatershedFilterType;
  typedef typename WatershedFilterType::OutputImageType  LabeledImageType;
  typedef itk::RelabelComponentImageFilter< LabeledImageType, OutputImageType > RelabelComponentImageFilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = WatershedFilterType::New();
    this->m_RelabelFilter = RelabelComponentImageFilterType::New();
    this->m_RelabelFilter->SetInput( this->m_Filter->GetOutput() );
    this->RegisterInternalFilter(this->m_Filter,0.8);
    this->RegisterInternalFilter(this->m_RelabelFilter,0.2);
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("Watershed");

    dialog.AddDialogElement("Threshold",0.01, 0.0, 1.0);
    dialog.AddDialogElement("WaterLevel",0.2, 0.0, 1.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetThreshold( dialog.GetValue("Threshold") );
      this->m_Filter->SetLevel( dialog.GetValue("WaterLevel") );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );
    this->m_RelabelFilter->SetInput(this->m_Filter->GetOutput());

    this->m_RelabelFilter->Update();

    {
    typedef itk::ImageFileWriter< LabeledImageType > WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("WatershedOutput.mhd");
    writer->SetInput( this->m_Filter->GetOutput() );
    writer->Update();
    }

    {
    typedef itk::ImageFileWriter< OutputImageType > WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("WatershedOutputRelabeled.mhd");
    writer->SetInput( this->m_RelabelFilter->GetOutput() );
    writer->Update();
    }


    this->SetOutputImage( this->m_RelabelFilter->GetOutput() );
    }


private:

    typename WatershedFilterType::Pointer   m_Filter;
    typename RelabelComponentImageFilterType::Pointer   m_RelabelFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void WatershedPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void WatershedPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Watershed 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

