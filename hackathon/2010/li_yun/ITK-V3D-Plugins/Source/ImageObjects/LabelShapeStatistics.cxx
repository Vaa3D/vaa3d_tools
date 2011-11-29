#include <QtGui>

#include <math.h>
#include <fstream>
#include <stdlib.h>

#include "LabelShapeStatistics.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkShapeLabelObject.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(LabelShapeStatistics, LabelShapeStatisticsPlugin)


QStringList LabelShapeStatisticsPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK LabelShapeStatistics")
            << QObject::tr("about this plugin");
}

QStringList LabelShapeStatisticsPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ShapeLabelObject< TPixelType, 3 >       LabelObjectType;
  typedef itk::LabelMap< LabelObjectType >             LabelMapType;

  typedef itk::LabelImageToShapeLabelMapFilter< ImageType, LabelMapType > FilterType;

  
public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("LabelShapeStatistics");

    dialog.AddDialogElement("BackgroundValue",0.0, 0.0, 255.0);

    // FIXME : add support for selecting other Attributes...
    // this->m_Filter->SetComputePhysicalSize( true )

    dialog.AddDialogElement("ComputeFeretDiameter",0.0, 0.0, 1.0);
    dialog.AddDialogElement("ComputePerimeter",0.0, 0.0, 1.0);

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetBackgroundValue( dialog.GetValue("BackgroundValue") );
      this->m_Filter->SetComputeFeretDiameter( dialog.GetValue("ComputeFeretDiameter") );
      this->m_Filter->SetComputePerimeter( dialog.GetValue("ComputePerimeter") );

      this->Compute();

      }
    }

  void Compute()
  {
    this->Initialize();

    const V3DLONG x1 = 0;
    const V3DLONG y1 = 0;
    const V3DLONG z1 = 0;

    const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
    const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
    const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

    QList< V3D_Image3DBasic > inputImageList =
      getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

    V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

    this->AddObserver( progressDialog.GetCommand() );
    progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );

    progressDialog.show();

    const unsigned int numberOfChannelsToProcess = inputImageList.size();
    if (numberOfChannelsToProcess<=0)
      {
        return;
      }
    
    std::ofstream ofsAttributes("attributes.txt");
    
    for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
      {
        const V3D_Image3DBasic inputImage = inputImageList.at(channel);

        this->TransferInput( inputImage, x1, x2, y1, y2, z1, z2 );
        //ComputeOneRegion();
  
        this->m_Filter->SetInput( this->GetInput3DImage() );

        this->m_Filter->Update();

        LabelMapType * outputLabelMap = this->m_Filter->GetOutput();
        unsigned long numberOfLabelMapObjects = outputLabelMap->GetNumberOfLabelObjects();

        // FIXME : attributes.txt should be a parameter passed to the plugin through the generic dialog.
        // We first need to add the string type to the generic dialog
        //std::ofstream ofsAttributes("attributes.txt");

        const char separator = ';';

        // output names
        ofsAttributes << "Label" << separator << "Size" << separator << "PhysicalSize" << separator << "RegionElongation" << separator << "SizeRegionRatio"
                      << separator << "Centroid" << separator << "RegionIndex" << separator << "RegionSize" << separator << "NumberOfPixelsOnBorder" << separator
                      << "PerimeterOnBorder" << separator << "FeretDiameter" << separator << "PrincipalMoments" << separator << "PrincipalAxes"
                      << separator << "Elongation" << separator << "Perimeter" << separator << "Roundness"
                      << separator << "EquivalentSphericalRadius" << separator << "EquivalentSphericalPerimeter" << separator << "EquivalentEllipsoidDiameter"        <<separator << "Flatness\n";

        for( unsigned long labelCounter = 0; labelCounter < numberOfLabelMapObjects; labelCounter++ )
        {
		LabelObjectType * labelObject = outputLabelMap->GetNthLabelObject( labelCounter );

		ofsAttributes << (unsigned long int)labelObject->GetLabel();
	//      ofsAttributes << separator << labelObject->GetNumberOfPixels();
		ofsAttributes << separator << labelObject->GetSize();
		ofsAttributes << separator << labelObject->GetPhysicalSize();
		ofsAttributes << separator << labelObject->GetRegionElongation();
		ofsAttributes << separator << labelObject->GetSizeRegionRatio();


		typename LabelObjectType::CentroidType centroid = labelObject->GetCentroid();
		ofsAttributes << separator;
		for(int i = 0; i < 3; i++)
		  {
		  ofsAttributes << centroid[i] << " ";
		  }

	//      typename LabelObjectType::RegionType region = labelObject->GetBoundingBox();
		typename LabelObjectType::RegionType region = labelObject->GetRegion();
		typename ImageType::IndexType indexType = region.GetIndex();
		typename ImageType::SizeType sizeType = region.GetSize();
		ofsAttributes << separator;
		for(int i = 0; i < 3; i++)
		  ofsAttributes << indexType[i] << " ";
		ofsAttributes << separator;
		for(int i = 0; i < 3; i++)
		  ofsAttributes << sizeType[i] << " ";

	 //       ofsAttributes << separator << labelObject->GetNumberOfPixelsOnBorder();
		ofsAttributes << separator << labelObject->GetSizeOnBorder();
	 //       ofsAttributes << separator << labelObject->GetPerimeterOnBorder();
		ofsAttributes << separator << labelObject->GetPhysicalSizeOnBorder();
		ofsAttributes << separator << labelObject->GetFeretDiameter();

	 //       typename LabelObjectType::VectorType vector = labelObject->GetPrincipalMoments();
		typename LabelObjectType::VectorType vector = labelObject->GetBinaryPrincipalMoments();
		ofsAttributes << separator;
		for(int i = 0; i < 3; i++)
		  ofsAttributes << vector[i] << " ";
	 //       typename LabelObjectType::MatrixType matrix = labelObject->GetPrincipalAxes();
		typename LabelObjectType::MatrixType matrix = labelObject->GetBinaryPrincipalAxes();
		ofsAttributes << separator;
		for(int i = 0; i < 3; i++)
		  for(int j = 0; j < 3; j++)
		    ofsAttributes << matrix[i][j] << " ";
	 //       ofsAttributes << separator << labelObject->GetElongation();
		ofsAttributes << separator << labelObject->GetBinaryElongation();
		ofsAttributes << separator << labelObject->GetPerimeter();
		ofsAttributes << separator << labelObject->GetRoundness();
	  //      ofsAttributes << separator << labelObject->GetEquivalentSphericalRadius();
		ofsAttributes << separator << labelObject->GetEquivalentRadius();
	 //       ofsAttributes << separator << labelObject->GetEquivalentSphericalPerimeter();
		ofsAttributes << separator << labelObject->GetEquivalentPerimeter();
	//        ofsAttributes << separator << labelObject->GetEquivalentEllipsoidDiameter();
		ofsAttributes << separator << labelObject->GetEquivalentEllipsoidSize();
	//      ofsAttributes << separator << labelObject->GetFlatness() << std::endl;
		ofsAttributes << separator << labelObject->GetBinaryFlatness() << std::endl;
        }
      }
    
      ofsAttributes.close();

      v3d_msg(QObject::tr("A file named attributes.txt has been created\n"));

      // There is no output image for that plugin.
      // Therefore, we are not calling AddOutputImageChannel and ComposeOutputImage
  }

 virtual void ComputeOneRegion()
    {

/*    this->m_Filter->SetInput( this->GetInput3DImage() );

    this->m_Filter->Update();

    LabelMapType * outputLabelMap = this->m_Filter->GetOutput();
    unsigned long numberOfLabelMapObjects = outputLabelMap->GetNumberOfLabelObjects();

    // FIXME : attributes.txt should be a parameter passed to the plugin through the generic dialog.
    // We first need to add the string type to the generic dialog
    //std::ofstream ofsAttributes("attributes.txt");

    const char separator = ';';

    // output names
    ofsAttributes << "Label" << separator << "Size" << separator << "PhysicalSize" << separator << "RegionElongation" << separator << "SizeRegionRatio"
                  << separator << "Centroid" << separator << "RegionIndex" << separator << "RegionSize" << separator << "NumberOfPixelsOnBorder" << separator
                  << "PerimeterOnBorder" << separator << "FeretDiameter" << separator << "PrincipalMoments" << separator << "PrincipalAxes"
                  << separator << "Elongation" << separator << "Perimeter" << separator << "Roundness"
                  << separator << "EquivalentSphericalRadius" << separator << "EquivalentSphericalPerimeter" << separator << "EquivalentEllipsoidDiameter" <<          separator << "Flatness\n";

    for( unsigned long labelCounter = 0; labelCounter < numberOfLabelMapObjects; labelCounter++ )
      {
        LabelObjectType * labelObject = outputLabelMap->GetNthLabelObject( labelCounter );

        ofsAttributes << (unsigned long int)labelObject->GetLabel();
//        ofsAttributes << separator << labelObject->GetNumberOfPixels();
	ofsAttributes << separator << labelObject->GetSize();
        ofsAttributes << separator << labelObject->GetPhysicalSize();
        ofsAttributes << separator << labelObject->GetRegionElongation();
        ofsAttributes << separator << labelObject->GetSizeRegionRatio();


        typename LabelObjectType::CentroidType centroid = labelObject->GetCentroid();
        ofsAttributes << separator;
        for(int i = 0; i < 3; i++)
          {
          ofsAttributes << centroid[i] << " ";
          }

//        typename LabelObjectType::RegionType region = labelObject->GetBoundingBox();
	typename LabelObjectType::RegionType region = labelObject->GetRegion();
        typename ImageType::IndexType indexType = region.GetIndex();
        typename ImageType::SizeType sizeType = region.GetSize();
        ofsAttributes << separator;
        for(int i = 0; i < 3; i++)
          ofsAttributes << indexType[i] << " ";
        ofsAttributes << separator;
        for(int i = 0; i < 3; i++)
          ofsAttributes << sizeType[i] << " ";

 //       ofsAttributes << separator << labelObject->GetNumberOfPixelsOnBorder();
	ofsAttributes << separator << labelObject->GetSizeOnBorder();
 //       ofsAttributes << separator << labelObject->GetPerimeterOnBorder();
	ofsAttributes << separator << labelObject->GetPhysicalSizeOnBorder();
        ofsAttributes << separator << labelObject->GetFeretDiameter();

 //       typename LabelObjectType::VectorType vector = labelObject->GetPrincipalMoments();
	typename LabelObjectType::VectorType vector = labelObject->GetBinaryPrincipalMoments();
        ofsAttributes << separator;
        for(int i = 0; i < 3; i++)
          ofsAttributes << vector[i] << " ";
 //       typename LabelObjectType::MatrixType matrix = labelObject->GetPrincipalAxes();
	typename LabelObjectType::MatrixType matrix = labelObject->GetBinaryPrincipalAxes();
        ofsAttributes << separator;
        for(int i = 0; i < 3; i++)
          for(int j = 0; j < 3; j++)
            ofsAttributes << matrix[i][j] << " ";
 //       ofsAttributes << separator << labelObject->GetElongation();
	ofsAttributes << separator << labelObject->GetBinaryElongation();
        ofsAttributes << separator << labelObject->GetPerimeter();
        ofsAttributes << separator << labelObject->GetRoundness();
  //      ofsAttributes << separator << labelObject->GetEquivalentSphericalRadius();
	ofsAttributes << separator << labelObject->GetEquivalentRadius();
 //       ofsAttributes << separator << labelObject->GetEquivalentSphericalPerimeter();
	ofsAttributes << separator << labelObject->GetEquivalentPerimeter();
//        ofsAttributes << separator << labelObject->GetEquivalentEllipsoidDiameter();
	ofsAttributes << separator << labelObject->GetEquivalentEllipsoidSize();
  //      ofsAttributes << separator << labelObject->GetFlatness() << std::endl;
	ofsAttributes << separator << labelObject->GetBinaryFlatness() << std::endl;
      }*/

    //ofsAttributes.close();

    //v3d_msg(QObject::tr("A file named attributes.txt has been created\n"));
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


void LabelShapeStatisticsPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void LabelShapeStatisticsPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK LabelShapeStatistics 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

