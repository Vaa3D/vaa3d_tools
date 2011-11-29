/* ITKGeodesicActiveContour.cxx
 * 2010-06-03: create this program by Yang Yu
 * 2011-20-04: Added dialog to pass parameters (Aurelien Lucchi)
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKGeodesicActiveContour.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
//#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkGeodesicActiveContourLevelSetImageFilter.h"

#include "itkImageFileWriter.h"
#include "V3DITKGenericDialog.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKGeodesicActiveContour, ITKGeodesicActiveContourPlugin)

void itkGeodesicActiveContourPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK GeodesicActiveContour";
QStringList ITKGeodesicActiveContourPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK GeodesicActiveContour")
                       << QObject::tr("about this plugin");
}

void ITKGeodesicActiveContourPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
  if (menu_name == QObject::tr("ITK GeodesicActiveContour"))
    {
      itkGeodesicActiveContourPlugin(callback, parent);
    }
  else if (menu_name == QObject::tr("about this plugin"))
    {
      QMessageBox::information(parent, "Version info", "ITK Shape Detection 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
    }
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKGeodesicActiveContourSpecializaed
{
public:
  void Execute(V3DPluginCallback &callback, QWidget *parent)
  {
    //
    V3DITKGenericDialog dialog("GeodesicActiveContou");
    dialog.AddDialogElement("propagationScaling",1.0, -100.0, 100.0);
    dialog.AddDialogElement("advectionScaling",1.0, -100.0, 100.0);
    dialog.AddDialogElement("curvatureScaling",1.0, -100.0, 100.0);		
    dialog.AddDialogElement("saveInputImages",0.0, 0.0, 1.0);
    dialog.AddDialogElement("initialDistance",15.0, 0.0, 1000.0);

    //
    if (dialog.exec()!=QDialog::Accepted)
      {
        return;
      }
    else
      {
        propagationScaling = dialog.GetValue("propagationScaling");
        advectionScaling = dialog.GetValue("advectionScaling");
        curvatureScaling = dialog.GetValue("curvatureScaling");
        initialDistance = dialog.GetValue("initialDistance");
        saveInputImages = dialog.GetValue("saveInputImages")!=0;

        V3D_GlobalSetting globalSetting = callback.getGlobalSetting();       		
	
        //ori image
        Image4DSimple *p4DImage = callback.getImage(callback.currentImageWindow());
		
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
			
        typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType    >    ThresholdingFilterType;
        typename ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
			
        typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType >  SmoothingFilterType;
        typename SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
			
        //typedef itk::GradientAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType >  SmoothingFilterType;
        //typename SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
			
        typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
        typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  SigmoidFilterType;
			
        typename GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();
        typename SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();
			
        sigmoid->SetOutputMinimum(  0.0  );
        sigmoid->SetOutputMaximum(  1.0  );
			
        //FastMarching
        typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingFilterType;
        typename FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();
			
        //ShapeDetection
        typedef  itk::GeodesicActiveContourLevelSetImageFilter< OutputImageType, OutputImageType > GeodesicActiveContourFilterType;
        typename GeodesicActiveContourFilterType::Pointer geodesicActiveContour = GeodesicActiveContourFilterType::New();
			
        typedef typename FastMarchingFilterType::NodeContainer	NodeContainer;
        typedef typename FastMarchingFilterType::NodeType		NodeType;
        typename NodeContainer::Pointer seeds = NodeContainer::New();
			
        typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2] 
			
        NodeType node;
			
        //set \pars
        const double seedValue = - initialDistance;
			
        //LandmarkList list_landmark_sub=callback.getLandmark(win_list[i1]);
        LandmarkList list_landmark_sub=callback.getLandmark(callback.currentImageWindow());
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
			
        const double stoppingTime = sqrt(nx*nx + ny*ny + nz*nz);
			
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

                // GAC algorithm
                smoothing->SetInput( castImageFilter->GetOutput() );
                gradientMagnitude->SetInput( smoothing->GetOutput() );
                sigmoid->SetInput( gradientMagnitude->GetOutput() );
					
                geodesicActiveContour->SetInput( fastMarching->GetOutput() );
                geodesicActiveContour->SetFeatureImage( sigmoid->GetOutput() );
					
                gradientMagnitude->SetSigma(  sigma  );
					
                smoothing->SetTimeStep( 0.05 ); // less than 0.625 for 3D less than 0.125 for 2D
                smoothing->SetNumberOfIterations(  5 );
                smoothing->SetConductanceParameter( 3.0 );
					
                sigmoid->SetOutputMinimum(  0.0  );
                sigmoid->SetOutputMaximum(  1.0  );
					
                sigmoid->SetAlpha( alpha );
                sigmoid->SetBeta(  beta  );
					
                fastMarching->SetTrialPoints(  seeds  );
                fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );
                //fastMarching->SetStoppingValue(  stoppingTime  );
                fastMarching->SetSpeedConstant( 1.0 );
					
                geodesicActiveContour->SetPropagationScaling( propagationScaling );
                geodesicActiveContour->SetCurvatureScaling( curvatureScaling );
                geodesicActiveContour->SetAdvectionScaling( advectionScaling );
					
                geodesicActiveContour->SetMaximumRMSError( 0.02 );
                geodesicActiveContour->SetNumberOfIterations( stoppingTime ); //
					
                //speedimag
                //					try
                //					{
                //						//smoothing->Update();
                //						//gradientMagnitude->Update();
                //						//sigmoid->Update();
                //					}
                //					catch( itk::ExceptionObject & excp)
                //					{
                //						std::cerr << "Error run this filter." << std::endl;
                //						std::cerr << excp << std::endl;
                //						return;
                //					}
					
                geodesicActiveContour->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
                try
                  {
                    geodesicActiveContour->Update();
                  }
                catch( itk::ExceptionObject & excp)
                  {
                    std::cerr << "Error run this filter." << std::endl;
                    std::cerr << excp << std::endl;
                    return;
                  }

                if(saveInputImages)
                  {
                    {
                      typedef itk::ImageFileWriter< OutputImageType > WriterType;
                      typename WriterType::Pointer writer = WriterType::New();
                      printf("Writing smoothing.mhd\n");
                      writer->SetFileName("smoothing.mhd");
                      writer->SetInput(smoothing->GetOutput());
                      writer->Update();
                      printf("Writing smoothing.mhd...DONE\n");
                    }
                                        
                    {
                      typedef itk::ImageFileWriter< OutputImageType > WriterType;
                      typename WriterType::Pointer writer = WriterType::New();
                      printf("Writing fastMarching.mhd\n");
                      writer->SetFileName("fastMarching.mhd");
                      writer->SetInput(fastMarching->GetOutput());
                      writer->Update();
                      printf("Writing fastMarching.mhd...DONE\n");
                    }
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
				

            if(saveInputImages)
              {
                {
                  typedef itk::ImageFileWriter< OutputImageType > WriterType;
                  typename WriterType::Pointer writer = WriterType::New();
                  printf("Writing smoothing.mhd\n");
                  writer->SetFileName("smoothing.mhd");
                  writer->SetInput(smoothing->GetOutput());
                  writer->Update();
                  printf("Writing smoothing.mhd...DONE\n");
                }
                                        
                {
                  typedef itk::ImageFileWriter< OutputImageType > WriterType;
                  typename WriterType::Pointer writer = WriterType::New();
                  printf("Writing fastMarching.mhd\n");
                  writer->SetFileName("fastMarching.mhd");
                  writer->SetInput(fastMarching->GetOutput());
                  writer->Update();
                  printf("Writing fastMarching.mhd...DONE\n");
                }
              }

            // GAC algorithm
            smoothing->SetInput( castImageFilter->GetOutput() );
            gradientMagnitude->SetInput( smoothing->GetOutput() );
            sigmoid->SetInput( gradientMagnitude->GetOutput() );
				
            geodesicActiveContour->SetInput( fastMarching->GetOutput() );
            geodesicActiveContour->SetFeatureImage( sigmoid->GetOutput() );
				
            gradientMagnitude->SetSigma(  sigma  );
				
            smoothing->SetTimeStep( 0.05 ); // less than 0.625 for 3D less than 0.125 for 2D
            smoothing->SetNumberOfIterations(  5 );
            smoothing->SetConductanceParameter( 3.0 );
				
            sigmoid->SetOutputMinimum(  0.0  );
            sigmoid->SetOutputMaximum(  1.0  );
				
            sigmoid->SetAlpha( alpha );
            sigmoid->SetBeta(  beta  );
				
            fastMarching->SetTrialPoints(  seeds  );
            fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );
            //fastMarching->SetStoppingValue(  stoppingTime  );
            fastMarching->SetSpeedConstant( 1.0 );
				
            geodesicActiveContour->SetPropagationScaling( propagationScaling );
            geodesicActiveContour->SetCurvatureScaling( curvatureScaling );
            geodesicActiveContour->SetAdvectionScaling( 1.0 );
				
            geodesicActiveContour->SetMaximumRMSError( 0.02 );
            geodesicActiveContour->SetNumberOfIterations( stoppingTime ); //
				
				
            //speedimag
            //				typename InvCastFilterType::Pointer caster = InvCastFilterType::New();
            //				caster->SetInput( sigmoid->GetOutput() );
				
            //				try
            //				{
            //					//smoothing->Update();
            //					//gradientMagnitude->Update();
            //					//sigmoid->Update();
            //					//caster->Update();
            //					
            //				}
            //				catch( itk::ExceptionObject & excp)
            //				{
            //					std::cerr << "Error run this filter." << std::endl;
            //					std::cerr << excp << std::endl;
            //					return;
            //				}
				
            //				typename InputImageType::PixelContainer * container1;
            //				
            //				container1 =caster->GetOutput()->GetPixelContainer();
            //				container1->SetContainerManageMemory( false );
            //				
            //				typedef TInputPixelType InputPixelType;
            //				InputPixelType * output1d_speed = container1->GetImportPointer();
            //				
            //				setPluginOutputAndDisplayUsingGlobalSetting(output1d_speed, nx, ny, nz, 1, callback);
				
            //fastmarching
            //				try
            //				{
            //					fastMarching->Update();
            //				}
            //				catch( itk::ExceptionObject & excp)
            //				{
            //					std::cerr << "Error run this filter." << std::endl;
            //					std::cerr << excp << std::endl;
            //					return;
            //				}
            //				typename OutputImageType::PixelContainer * container1;
            //				
            //				container1 =fastMarching->GetOutput()->GetPixelContainer();
            //				container1->SetContainerManageMemory( false );
            //				
            //				typedef TOutputPixelType OutputPixelType;
            //				OutputPixelType * output1d_fm = container1->GetImportPointer();
            //				
            //				setPluginOutputAndDisplayUsingGlobalSetting(output1d_fm, nx, ny, nz, 1, callback);
				
            thresholder->SetInput( geodesicActiveContour->GetOutput() );
				
            thresholder->SetLowerThreshold( -1000.0 );
            thresholder->SetUpperThreshold(     0.0 );
				
            thresholder->SetOutsideValue(  0  );
            thresholder->SetInsideValue(  255 );
				
            //shapedetection
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
				
            // output
            typename InputImageType::PixelContainer * container;
				
            container =thresholder->GetOutput()->GetPixelContainer();
            container->SetContainerManageMemory( false );
				
            typedef TInputPixelType InputPixelType;
            InputPixelType * output1d = container->GetImportPointer();
				
            setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, 1, callback);
				
          }
			
        std::cout << std::endl;
        std::cout << "Max. no. iterations: " << geodesicActiveContour->GetNumberOfIterations() << std::endl;
        std::cout << "Max. RMS error: " << geodesicActiveContour->GetMaximumRMSError() << std::endl;
        std::cout << std::endl;
        std::cout << "No. elpased iterations: " << geodesicActiveContour->GetElapsedIterations() << std::endl;
        std::cout << "RMS change: " << geodesicActiveContour->GetRMSChange() << std::endl;
      }
		
  }	

  double propagationScaling;
  double advectionScaling;
  double curvatureScaling;
  double initialDistance;
  bool saveInputImages;
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type )  \
  case v3d_pixel_type:                                                  \
  {                                                                     \
    ITKGeodesicActiveContourSpecializaed< input_pixel_type, output_pixel_type > runner; \
    runner.Execute( callback, parent );                                 \
    break;                                                              \
  } 

#define EXECUTE_ALL_PIXEL_TYPES                         \
  if (! p4DImage) return;                               \
  ImagePixelType pixelType = p4DImage->getDatatype();   \
  switch( pixelType )                                   \
    {                                                   \
      EXECUTE( V3D_UINT8, unsigned char, float );       \
      EXECUTE( V3D_UINT16, unsigned short int, float ); \
      EXECUTE( V3D_FLOAT32, float, float );             \
    case V3D_UNKNOWN:                                   \
      {                                                 \
      }                                                 \
    }  


void itkGeodesicActiveContourPlugin(V3DPluginCallback &callback, QWidget *parent)
{
  Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
  if (!p4DImage)
    {
      v3d_msg(QObject::tr("You don't have any image open in the main window."));
      return;
    }
	
  EXECUTE_ALL_PIXEL_TYPES;
}


