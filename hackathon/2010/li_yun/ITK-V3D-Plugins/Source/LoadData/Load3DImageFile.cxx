#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Load3DImageFile.h"
#include "V3DITKFilterNullImage.h"

// ITK Header Files
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Load3DImageFile, Load3DImageFilePlugin)


QStringList Load3DImageFilePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Load3DImageFile")
            << QObject::tr("ITK Load DICOM Series")
            << QObject::tr("about this plugin");
}

QStringList Load3DImageFilePlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterNullImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterNullImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ImageFileReader< ImageType >   FileReaderType;
  typedef itk::ImageSeriesReader< ImageType > SeriesReaderType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_FileReader = FileReaderType::New();
    this->m_SeriesReader = SeriesReaderType::New();

    //
    //  They run independently, so each one is scaled to 100%.
    //
    this->RegisterInternalFilter( this->m_FileReader, 1.0 );
    this->RegisterInternalFilter( this->m_SeriesReader, 1.0 );

    this->m_InputIsDICOM = false;
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent, const std::string & inputFileName, bool isDicom )
    {
    this->m_InputIsDICOM = isDicom;
    this->m_FileName = inputFileName;

    this->Compute();
    }


  virtual void ComputeOneRegion()
    {
    if ( this->m_InputIsDICOM )
      {
      this->ReadDICOMSeries3DImage();
      }
    else
      {
      this->ReadSingle3DImage();
      }    
    }


  virtual void ReadSingle3DImage()
    {
    this->m_FileReader->SetFileName( this->m_FileName );
    this->m_FileReader->Update();
    this->SetOutputImage( this->m_FileReader->GetOutput() );
    }

  virtual void ReadDICOMSeries3DImage()
    {
    itk::GDCMImageIO::Pointer imageIO = itk::GDCMImageIO::New();

    imageIO->SetFileName( this->m_FileName );

    try
      {
      imageIO->ReadImageInformation();
      }
    catch( itk::ExceptionObject & excp )
      {
      std::cerr << "Could not read: " << this->m_FileName << std::endl;
      std::cerr << excp << std::endl;
      return;
      }
    catch( std::exception & excp )
      {
      std::cerr << "Could not read: " << this->m_FileName << std::endl;
      std::cerr << excp.what() << std::endl;
      return;
      }

    itk::MetaDataDictionary & dict = imageIO->GetMetaDataDictionary();

    std::string  tagkey = "0020|000e";
    std::string seriesIdentifier;

    if( ! itk::ExposeMetaData<std::string>(dict,tagkey, seriesIdentifier ) )
      {
      std::cerr << "Series UID not found: " << std::endl;
      return;
      }

    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

    std::string directory = itksys::SystemTools::GetFilenamePath( this->m_FileName );

    std::cout << "DICOM Directory = " << directory << std::endl;

    nameGenerator->SetInputDirectory( directory );

    nameGenerator->SetUseSeriesDetails( true );
    nameGenerator->AddSeriesRestriction("0008|0021" );

    typedef std::vector< std::string >   FileNamesContainerType;

    std::cout << "Series Identifier = " << seriesIdentifier << std::endl;

    FileNamesContainerType fileNames =
      nameGenerator->GetFileNames( seriesIdentifier );

    this->m_SeriesReader->SetFileNames( fileNames );

    this->m_SeriesReader->Update();

    this->SetOutputImage( this->m_SeriesReader->GetOutput() );
    }

private:

    typename FileReaderType::Pointer      m_FileReader;
    typename SeriesReaderType::Pointer    m_SeriesReader;

    std::string                           m_FileName;
    bool                                  m_InputIsDICOM;
};


#define EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk_io_pixel_type, c_pixel_type ) \
  case itk_io_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent, inputFileName, isDicom ); \
    break; \
    }


void Load3DImageFilePlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void Load3DImageFilePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Load3DImageFile 1.0 (2010-Aug-1): this plugin is developed by Luis Ibanez.");
    return;
    }

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
  fileDialog.setLabelText( QFileDialog::LookIn,"Select Input");

  fileDialog.exec();

  QStringList listOfFiles = fileDialog.selectedFiles();

  if( listOfFiles.isEmpty() )
    {
    return;
    }

  std::string inputFileName = listOfFiles[0].toStdString();

  // Find out the pixel type of the image in file
  typedef itk::ImageIOBase::IOComponentType  ScalarPixelType;

  itk::ImageIOBase::Pointer imageIO =
    itk::ImageIOFactory::CreateImageIO( inputFileName.c_str(),
                                   itk::ImageIOFactory::ReadMode );

  if( !imageIO )
    {
    return;
    }

  bool isDicom = false;

  if (menu_name == QObject::tr("ITK Load DICOM Series"))
    {
    isDicom = true;
    }

  // Now that we found the appropriate ImageIO class,
  // ask it to read the meta data from the image file.
  imageIO->SetFileName( inputFileName.c_str() );
  imageIO->ReadImageInformation();

  ScalarPixelType pixelType = imageIO->GetComponentType();

  switch( pixelType )
    {
    // These are the supported pixel types (with acceptable conversions)
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::CHAR, unsigned char ); // Conversion
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::UCHAR, unsigned char );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::SHORT, unsigned short int ); // Conversion
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::USHORT, unsigned short int );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::FLOAT, float );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::DOUBLE, float ); // Conversion

    // There are the unsupported pixel types (they simply get ignored)
    case itk::ImageIOBase::UINT:
    case itk::ImageIOBase::INT:
    case itk::ImageIOBase::ULONG:
    case itk::ImageIOBase::LONG:
    default:
      {
      }
    }

}

