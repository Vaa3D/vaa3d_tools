/* ITKFastMarchingPlus.cxx
 * 2010-06-02: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKFastMarchingPlus.h"

// ITK Header Files
#include "itkImage.h"
#include "itkFastMarchingImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
//#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKFastMarchingPlus, ITKFastMarchingPlusPlugin)

void itkFastMarchingPlusPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK FastMarchingPlus";
QStringList ITKFastMarchingPlusPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK FastMarchingPlus")
  << QObject::tr("about this plugin");
}

void ITKFastMarchingPlusPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK FastMarchingPlus"))
    {
      itkFastMarchingPlusPlugin(callback, parent);
    }
  else if (menu_name == QObject::tr("about this plugin"))
  {
    QMessageBox::information(parent, "Version info", "ITK Fast Marching 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
  }
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKFastMarchingPlusSpecializaed
{
public:
  void Execute(V3DPluginCallback &callback, QWidget *parent)
  {
    //
    ITKFastMarchingPlusDialog d(callback, parent);

    //
    if (d.exec()!=QDialog::Accepted)
    {
      return;
    }
    else
    {
      //passing \pars
      d.update();

      int i1 = d.i1;

      //ori image
      v3dhandleList win_list = callback.getImageWindowList();

      V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
      Image4DSimple *p4DImage = callback.getImage(win_list[i1]);

      //init
      typedef TInputPixelType  PixelType;

      PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
      unsigned long int numberOfPixels = p4DImage->getTotalBytes();

      long pagesz = p4DImage->getTotalUnitNumberPerChannel();

      long nx = p4DImage->getXDim();
      long ny = p4DImage->getYDim();
      long nz = p4DImage->getZDim();
      long nc = p4DImage->getCDim();  // Number of channels

      int channelToFilter = globalSetting.iChannel_for_plugin;

      if( channelToFilter >= nc )
      {
        v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
        return;
      }

      long offsets=0;
      if(channelToFilter>=0) offsets = channelToFilter*pagesz;

      const unsigned int Dimension = 3; // \par

      typedef itk::Image< TInputPixelType, Dimension > InputImageType;
      typedef itk::Image< TOutputPixelType, Dimension > OutputImageType;
      typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;

      typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

      typename ImportFilterType::SizeType size;
      size[0] = nx;
      size[1] = ny;
      size[2] = nz;

      typename ImportFilterType::IndexType start;
      start.Fill( 0 );

      typename ImportFilterType::RegionType region;
      region.SetIndex( start );
      region.SetSize(  size  );

      importFilter->SetRegion( region );

      region.SetSize( size );

      typename InputImageType::PointType origin;
      origin.Fill( 0.0 );

      importFilter->SetOrigin( origin );

      typename ImportFilterType::SpacingType spacing;
      spacing.Fill( 1.0 );

      importFilter->SetSpacing( spacing );

      const bool importImageFilterWillOwnTheBuffer = false;

      typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
      typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();

      //Generate Speed Image
      typedef itk::RescaleIntensityImageFilter< OutputImageType, InputImageType >   InvCastFilterType;

      typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType >  SmoothingFilterType;
      typename SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();

      typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
      typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  SigmoidFilterType;

      typename GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();
      typename SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();

      sigmoid->SetOutputMinimum(  0.0  );
      sigmoid->SetOutputMaximum(  1.0  );

      //FastMarchingPlus
      typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingPlusFilterType;
      typename FastMarchingPlusFilterType::Pointer fastMarching = FastMarchingPlusFilterType::New();

      typedef itk::ThresholdImageFilter< OutputImageType > ThresholdFilterType;
      typename ThresholdFilterType::Pointer thresholder = ThresholdFilterType::New();

      const double stoppingTime = sqrt(nx*nx + ny*ny + nz*nz);

      thresholder->ThresholdAbove( stoppingTime );
      thresholder->SetOutsideValue( stoppingTime );
      fastMarching->SetStoppingValue(  stoppingTime  );

      typedef typename FastMarchingPlusFilterType::NodeContainer  NodeContainer;
      typedef typename FastMarchingPlusFilterType::NodeType    NodeType;
      typename NodeContainer::Pointer seeds = NodeContainer::New();

      typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

      NodeType node;

      //set \pars
      const double seedValue = 0.0;

      LandmarkList list_landmark_sub=callback.getLandmark(win_list[i1]);
      if(list_landmark_sub.size()<1)
      {
        v3d_msg(QObject::tr("You should select one seed from your image."));
        return;
      }
      else
      {
          //seeds
        seeds->Initialize();

        for(int i=0;  i<list_landmark_sub.size(); i++)
        {
          //
          seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
          seedPosition[1] = list_landmark_sub[i].y -1;
          seedPosition[2] = list_landmark_sub[i].z -1;

          node.SetValue( seedValue );
          node.SetIndex( seedPosition );

          seeds->InsertElement( i, node );
        }
      }

      const double sigma = 0.5; // GradientMagnitudeRecursiveGaussianImageFilter

      const double alpha =  -1; // SigmoidImageFilter
      const double beta  =  20;

      //consider multiple channels
      if(channelToFilter==-1)
      {
        TOutputPixelType *output1d;
        try
        {
          output1d = new TOutputPixelType [numberOfPixels];
        }
        catch(...)
        {
          std::cerr << "Error memroy allocating." << std::endl;
          return;
        }

        const bool filterWillDeleteTheInputBuffer = false;

        for(long ch=0; ch<nc; ch++)
        {
          offsets = ch*pagesz;

          TOutputPixelType *p = output1d+offsets;

          importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );

          castImageFilter->SetInput( importFilter->GetOutput() );

          try
          {
            castImageFilter->Update();
          }
          catch( itk::ExceptionObject & excp)
          {
            std::cerr << "Error run this filter." << std::endl;
            std::cerr << excp << std::endl;
            return;
          }

          // fast marching
          // fast marching algorithm
          smoothing->SetInput( castImageFilter->GetOutput() );
          gradientMagnitude->SetInput( smoothing->GetOutput() );
          sigmoid->SetInput( gradientMagnitude->GetOutput() );
          fastMarching->SetInput( sigmoid->GetOutput() );
          thresholder->SetInput( fastMarching->GetOutput() );

          gradientMagnitude->SetSigma(  sigma  );

          smoothing->SetTimeStep( 0.125 );
          smoothing->SetNumberOfIterations(  5 );
          smoothing->SetConductanceParameter( 9.0 );

          sigmoid->SetAlpha( alpha );
          sigmoid->SetBeta(  beta  );

          fastMarching->SetTrialPoints(  seeds  );
          fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );

          //speedimag
          try
          {
            smoothing->Update();
            gradientMagnitude->Update();
            sigmoid->Update();
          }
          catch( itk::ExceptionObject & excp)
          {
            std::cerr << "Error run this filter." << std::endl;
            std::cerr << excp << std::endl;
            return;
          }

          thresholder->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);

          try
          {
            thresholder->Update();
          }
          catch( itk::ExceptionObject & excp)
          {
            std::cerr << "Error run this filter." << std::endl;
            std::cerr << excp << std::endl;
            return;
          }

          {
          typedef itk::ImageFileWriter< OutputImageType > WriterType;
          typename WriterType::Pointer writer = WriterType::New();
          std::cout << "WRITING FAST MARCHING OUTPUT" << std::endl;
          writer->SetFileName("FastMarchingPlus.mhd");
          writer->SetInput( thresholder->GetOutput() );
          writer->Update();
          }
        }

        setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, nc, callback);
      }
      else if(channelToFilter<nc)
      {
        importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );

        castImageFilter->SetInput( importFilter->GetOutput() );

        try
        {
          castImageFilter->Update();
        }
        catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error run this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }

        // fast marching algorithm
        smoothing->SetInput( castImageFilter->GetOutput() );
        gradientMagnitude->SetInput( smoothing->GetOutput() );
        sigmoid->SetInput( gradientMagnitude->GetOutput() );
        fastMarching->SetInput( sigmoid->GetOutput() );
        thresholder->SetInput( fastMarching->GetOutput() );

        gradientMagnitude->SetSigma(  sigma  );

        smoothing->SetTimeStep( 0.05 );
        smoothing->SetNumberOfIterations(  5 );
        smoothing->SetConductanceParameter( 3.0 );

        sigmoid->SetOutputMinimum(  0.0  );
        sigmoid->SetOutputMaximum(  1.0  );

        sigmoid->SetAlpha( alpha );
        sigmoid->SetBeta(  beta  );

        fastMarching->SetTrialPoints(  seeds  );
        fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );

        //speedimag
        typename InvCastFilterType::Pointer caster = InvCastFilterType::New();
        caster->SetInput( sigmoid->GetOutput() );

        try
        {
          //smoothing->Update();
          //gradientMagnitude->Update();
          //sigmoid->Update();
          caster->Update();

        }
        catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error run this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }

        typename InputImageType::PixelContainer * container1;

        container1 =caster->GetOutput()->GetPixelContainer();
        container1->SetContainerManageMemory( false );

        typedef TInputPixelType InputPixelType;
        InputPixelType * output1d_speed = container1->GetImportPointer();

        setPluginOutputAndDisplayUsingGlobalSetting(output1d_speed, nx, ny, nz, 1, callback);

        //fastmarching
        try
        {
          thresholder->Update();
        }
        catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error run this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }

        {
        typedef itk::ImageFileWriter< OutputImageType > WriterType;
        typename WriterType::Pointer writer = WriterType::New();
        std::cout << "WRITING FAST MARCHING OUTPUT" << std::endl;
        writer->SetFileName("FastMarchingPlus.mhd");
        writer->SetInput( thresholder->GetOutput() );
        writer->Update();
        }


        // output
        typename OutputImageType::PixelContainer * container;

        container = thresholder->GetOutput()->GetPixelContainer();
        container->SetContainerManageMemory( false );

        typedef TOutputPixelType OutputPixelType;
        OutputPixelType * output1d = container->GetImportPointer();

        setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, 1, callback);
      }

    }

  }

};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
  case v3d_pixel_type: \
  { \
    ITKFastMarchingPlusSpecializaed< input_pixel_type, output_pixel_type > runner; \
    runner.Execute( callback, parent ); \
    break; \
  }

#define EXECUTE_ALL_PIXEL_TYPES \
  if (! p4DImage) return; \
  ImagePixelType pixelType = p4DImage->getDatatype(); \
  switch( pixelType )  \
  {  \
    EXECUTE( V3D_UINT8, unsigned char, float );  \
    EXECUTE( V3D_UINT16, unsigned short int, float );  \
    EXECUTE( V3D_FLOAT32, float, float );  \
    case V3D_UNKNOWN:  \
    {  \
    }  \
  }


void itkFastMarchingPlusPlugin(V3DPluginCallback &callback, QWidget *parent)
{
  Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
  if (!p4DImage)
    {
    v3d_msg(QObject::tr("You don't have any image open in the main window."));
    return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}


