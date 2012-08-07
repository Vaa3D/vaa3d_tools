/* ITKCannySegmentation.cxx
 * 2010-06-04: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKCannySegmentation.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

#include "itkZeroCrossingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkCannySegmentationLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKCannySegmentation, ITKCannySegmentationPlugin)

//plugin funcs
const QString title = "ITK CannySegmentationPlugin";
QStringList ITKCannySegmentationPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK CannySegmentationPlugin")
						 << QObject::tr("about this plugin");
}

QStringList ITKCannySegmentationPlugin::funclist() const
{
    return QStringList();
}

void ITKCannySegmentationPlugin::dofunc(const QString & func_name,
											const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKCannySegmentationSpecializaed // : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
public:
	//typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >    Superclass;
	
	//ITKCannySegmentationSpecializaed( V3DPluginCallback * callback ): Superclass(callback) {}
	virtual ~ITKCannySegmentationSpecializaed() {};
	
	//
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		
		//
		ITKCannySegmentationDialog d(callback, parent);
		
		if (d.exec()!=QDialog::Accepted)
		{
			return;
		}
		else
		{
			//passing \pars
			d.update();
			
			int i1 = d.i1;
			int i2 = d.i2;
			
			//
			v3dhandleList win_list = callback.getImageWindowList();
			
			V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
			Image4DSimple *p4DImage = callback.getImage(win_list[i1]);
			Image4DSimple *p4DModel = callback.getImage(win_list[i2]);
			
			//init
			typedef TInputPixelType  PixelType;
			
			PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
			unsigned long int numberOfPixels_data = p4DImage->getTotalBytes();
			
			PixelType * model1d = reinterpret_cast< PixelType * >( p4DModel->getRawData() );
			unsigned long int numberOfPixels_model = p4DModel->getTotalBytes();
			
			// suppose the image and model same size
			if(numberOfPixels_data != numberOfPixels_model)
				return;
			
			unsigned long int numberOfPixels = numberOfPixels_data;
			
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
			
			typename ImportFilterType::Pointer importFilter1 = ImportFilterType::New();
			typename ImportFilterType::Pointer importFilter2 = ImportFilterType::New();
			
			typename ImportFilterType::SizeType size;
			size[0] = nx;
			size[1] = ny;
			size[2] = nz;
			
			typename ImportFilterType::IndexType start;
			start.Fill( 0 );
			
			typename ImportFilterType::RegionType region;
			region.SetIndex( start );
			region.SetSize(  size  );
			
			importFilter1->SetRegion( region );
			importFilter2->SetRegion( region );
			
			region.SetSize( size );
			
			typename InputImageType::PointType origin;
			origin.Fill( 0.0 );
			
			importFilter1->SetOrigin( origin );
			importFilter2->SetOrigin( origin );
			
			typename ImportFilterType::SpacingType spacing;
			spacing.Fill( 1.0 );
			
			importFilter1->SetSpacing( spacing );
			importFilter2->SetSpacing( spacing );
			
			const bool importImageFilterWillOwnTheBuffer = false;
			
			typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
			typename CastImageFilterType::Pointer castImageFilter1 = CastImageFilterType::New();
			typename CastImageFilterType::Pointer castImageFilter2 = CastImageFilterType::New();
			
			//Declaration of fileters
			typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType >  ThresholdingFilterType;
			typename ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
			
			typedef itk::CannySegmentationLevelSetImageFilter< OutputImageType, OutputImageType >  CannySegmentationLevelSetImageFilterType;
			typename CannySegmentationLevelSetImageFilterType::Pointer cannySegmentation = CannySegmentationLevelSetImageFilterType::New();
			
			typedef itk::GradientAnisotropicDiffusionImageFilter< OutputImageType,OutputImageType> DiffusionFilterType;
			typename DiffusionFilterType::Pointer diffusion = DiffusionFilterType::New();
			
			//set \pars

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
					
					importFilter1->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
					importFilter2->SetImportPointer( model1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
					
					castImageFilter1->SetInput( importFilter1->GetOutput() );
					castImageFilter2->SetInput( importFilter2->GetOutput() );
					
					try
					{
						castImageFilter1->Update();
						castImageFilter2->Update();
					}
					catch( itk::ExceptionObject & excp)
					{
						std::cerr << "Error run this filter." << std::endl;
						std::cerr << excp << std::endl;
						return;
					}
					
					//canny segmentation algorithm
					diffusion->SetInput( castImageFilter1->GetOutput() );
					cannySegmentation->SetInput( castImageFilter2->GetOutput() );
					cannySegmentation->SetFeatureImage( diffusion->GetOutput() );
					thresholder->SetInput( cannySegmentation->GetOutput() );
					
					//
					diffusion->SetNumberOfIterations(5);
					diffusion->SetTimeStep(0.05);
					diffusion->SetConductanceParameter(1.0);
					
					cannySegmentation->SetAdvectionScaling( 10.0 );
					cannySegmentation->SetCurvatureScaling( 1.0 );
					cannySegmentation->SetPropagationScaling( 0.0 );
					
					cannySegmentation->SetMaximumRMSError( 0.01 );
					cannySegmentation->SetNumberOfIterations( 20 );
					
					cannySegmentation->SetThreshold( 7.0 );
					cannySegmentation->SetVariance( 0.1 );
					
					cannySegmentation->SetIsoSurfaceValue( 127.5 );
					
					//thresholdSegmentation
					cannySegmentation->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
					try
					{
						cannySegmentation->Update();
					}
					catch( itk::ExceptionObject & excp)
					{
						std::cerr << "Error run this filter." << std::endl;
						std::cerr << excp << std::endl;
						return;
					}
					
				}
				
				setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, nc, callback);
			}
			else if(channelToFilter<nc)
			{
				importFilter1->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
				importFilter2->SetImportPointer( model1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
				
				castImageFilter1->SetInput( importFilter1->GetOutput() );
				castImageFilter2->SetInput( importFilter2->GetOutput() );
				
				try
				{
					castImageFilter1->Update();
					castImageFilter2->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
				//canny segmentation algorithm
				diffusion->SetInput( castImageFilter1->GetOutput() );
				cannySegmentation->SetInput( castImageFilter2->GetOutput() );
				cannySegmentation->SetFeatureImage( diffusion->GetOutput() );
				thresholder->SetInput( cannySegmentation->GetOutput() );
				
				//
				diffusion->SetNumberOfIterations(5);
				diffusion->SetTimeStep(0.05);
				diffusion->SetConductanceParameter(1.0);
				
				cannySegmentation->SetAdvectionScaling( 0.5 );
				cannySegmentation->SetCurvatureScaling( 1.0 );
				cannySegmentation->SetPropagationScaling( 0.0 );
				
				cannySegmentation->SetMaximumRMSError( 0.01 );
				cannySegmentation->SetNumberOfIterations( 20 );
				
				cannySegmentation->SetThreshold( 100 );
				cannySegmentation->SetVariance( 100 );
				
				cannySegmentation->SetIsoSurfaceValue( 0 );
				
				thresholder->SetUpperThreshold( 10.0 );
				thresholder->SetLowerThreshold( 0.0 );
				
				thresholder->SetOutsideValue(  0  );
				thresholder->SetInsideValue(  255 );
				
				//
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
			std::cout << "Max. no. iterations: " << cannySegmentation->GetNumberOfIterations() << std::endl;
			std::cout << "Max. RMS error: " << cannySegmentation->GetMaximumRMSError() << std::endl;
			std::cout << std::endl;
			std::cout << "No. elpased iterations: " << cannySegmentation->GetElapsedIterations() << std::endl;
			std::cout << "RMS change: " << cannySegmentation->GetRMSChange() << std::endl;
		}
		
		
	}	// Execute
		
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKCannySegmentationSpecializaed< input_pixel_type, output_pixel_type > runner; \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
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



void ITKCannySegmentationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK CannySegmentationPlugin"))
    {
    	v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg(QObject::tr("You don't have any image open in the main window."));
			return;
		}
		
		Image4DSimple *p4DImage = callback.getImage(curwin);
		if (! p4DImage)
		{
			v3d_msg(QObject::tr("The input image is null."));
			return;
		}	
		
		EXECUTE_ALL_PIXEL_TYPES;
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Canny SegmentationPlugin 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
	}
}



