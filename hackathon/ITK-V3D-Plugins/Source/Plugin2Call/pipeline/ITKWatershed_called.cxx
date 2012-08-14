#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <exception>
using namespace std;

#include "ITKWatershed_called.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkWatershedImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkShapeLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkCastImageFilter.h"

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
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, unsigned char >
{
  typedef V3DITKFilterSingleImage< TPixelType, unsigned char >   Superclass;
  typedef typename Superclass::Input3DImageType                       ImageType;

  typedef itk::Image< unsigned char, 3 > OutputImageType;

  typedef itk::WatershedImageFilter< ImageType > WatershedFilterType;
  typedef typename WatershedFilterType::OutputImageType  LabeledImageType;
  typedef typename WatershedFilterType::OutputImagePixelType LabeledImagePixelType;
  typedef itk::ShapeLabelObject< LabeledImagePixelType, 3 >       LabelObjectType;
  typedef itk::LabelMap< LabelObjectType >             LabelMapType;

  typedef itk::LabelImageToShapeLabelMapFilter< LabeledImageType, LabelMapType > LabelMapFilterType;
  typedef itk::RelabelComponentImageFilter< LabeledImageType, OutputImageType > RelabelComponentImageFilterType;
  typedef itk::CastImageFilter<ImageType, ImageType>  CastImageFilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = WatershedFilterType::New();
    this->m_RelabelFilter = RelabelComponentImageFilterType::New();
    this->m_LabelMapFilter = LabelMapFilterType::New();
    this->m_castImageFilter = CastImageFilterType::New();
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
 bool ComputeOneRegion(const V3DPluginArgList& input, V3DPluginArgList output)
 {

    V3DITKGenericDialog dialog("Watershed");

    dialog.AddDialogElement("Threshold",0.01, 0.0, 1.0);
    dialog.AddDialogElement("WaterLevel",0.2, 0.0, 1.0);

    if( dialog.exec() != QDialog::Accepted )
      {
        qDebug() << "refuse to do the plugin func";
        return false;
      }
    V3DITKGenericDialog dialog1("LabelShapeStatistics");
    dialog1.AddDialogElement("BackgroundValue",0.0, -5.0, 5.0);
    if (dialog1.exec() != QDialog::Accepted)
    {
      qDebug() << "refuse to do the plugin func";
      return false;
    }
    //Now do the filter

    this->m_Filter->SetThreshold( dialog.GetValue("Threshold") );
    this->m_Filter->SetLevel( dialog.GetValue("WaterLevel") );

    this->m_LabelMapFilter->SetBackgroundValue(dialog1.GetValue("BackgroundValue"));
    //setup the progressdialog

        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter( this->m_Filter, 1.0 );
    //now start use the filter

    void* p = input.at(0).p;
    this->m_castImageFilter->SetInput((ImageType*)p);
    this->m_Filter->SetInput(m_castImageFilter->GetOutput());
    this->m_RelabelFilter->SetInput(this->m_Filter->GetOutput());

    this->m_LabelMapFilter->SetInput(this->m_Filter->GetOutput());


    try {
    this->m_RelabelFilter->Update();
    this->m_LabelMapFilter->Update();
    }catch (exception &e)
    {
      qDebug() << "exception is : " << e.what();
    }

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
    //get the label inf
    LabelMapType* outputLabelMap = this->m_LabelMapFilter->GetOutput();
    unsigned long numberOfLabelMapObjects = outputLabelMap->GetNumberOfLabelObjects();
    QString fileName = QString("WatershedLabelCount_%1").arg((QTime::currentTime()).toString());
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
         return false;
     QTextStream out(&file);
     out << "number of the labeled count is :  " << numberOfLabelMapObjects << "\n"; 
    file.close();

    qDebug() << "Label number is : " << numberOfLabelMapObjects;
    //get outputImage
    V3DPluginArgItem arg;
    typename OutputImageType::Pointer outputImage = m_RelabelFilter->GetOutput();
    outputImage->Register();
    arg.p = (void*)outputImage;
    arg.type = "UINT8Image";
    output.replace(0, arg);
    return true;
    }


private:

    typename WatershedFilterType::Pointer   m_Filter;
    typename RelabelComponentImageFilterType::Pointer   m_RelabelFilter;
    typename LabelMapFilterType::Pointer  m_LabelMapFilter; 
    typename CastImageFilterType::Pointer m_castImageFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool WatershedPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
    V3DPluginCallback2 & v3d,  QWidget * parent)
{
  //do the func
  if ( input.at(0).type == QString("UINT8Image")) {
    PluginSpecialized<unsigned char> runner(&v3d);
    return runner.ComputeOneRegion(input, output);
    }
  else if (input.at(0).type == QString("UINT16Image")) {
    PluginSpecialized<unsigned short int> runner(&v3d);
    return runner.ComputeOneRegion(input, output);
    }
  else if (input.at(0).type == QString("FLOATImage")) {
    PluginSpecialized<float> runner(&v3d);
    return runner.ComputeOneRegion(input, output);
    }
  else {
    qDebug() << "No data type support, run error";
    return false;
    }
}


void WatershedPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Watershed 2.0 (2012-8-13): this plugin is developed by Ping Yu.");
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

