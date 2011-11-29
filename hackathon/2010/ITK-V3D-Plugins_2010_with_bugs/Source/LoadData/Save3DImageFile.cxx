#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Save3DImageFile.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImageFileWriter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Save3DImageFile, Save3DImageFilePlugin)


QStringList Save3DImageFilePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Save3DImageFile")
            << QObject::tr("about this plugin");
}

QStringList Save3DImageFilePlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ImageFileWriter< ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {

    QFileDialog fileDialog(parent);

    fileDialog.setViewMode( QFileDialog::Detail );

    QStringList filters;
    filters << "Analyze (*.hdr)"
            << "MetaImage (*.mhd)"
            << "MetaImage (*.mha)"
            << "VTK Image (*.vtk)"
            << "Nifti Image (*.nii)"
            << "Nifti Compressed (*.nii.gz)"
            << "Nrrd (*.nrrd)"
            << "TIFF (*.tif*)"
            << "ALL (*)";

    fileDialog.setFilters( filters );
    fileDialog.setLabelText( QFileDialog::LookIn,"Select OutpuOutput");

    fileDialog.exec();

    QStringList listOfFiles = fileDialog.selectedFiles();

    if( listOfFiles.isEmpty() )
      {
      return;
      }

    std::string inputFileName = listOfFiles[0].toStdString();

    this->m_Filter->SetFileName( inputFileName );

    this->Compute();
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

    this->m_Filter->Update();
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


void Save3DImageFilePlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void Save3DImageFilePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Save3DImageFile 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

