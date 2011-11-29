/* ITKConnectedThreshold.cxx
 * 2010-06-03: create this program by Yang Yu
 */


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKConnectedThreshold.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKConnectedThreshold, ITKConnectedThresholdPlugin)

void itkConnectedThresholdPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK ConnectedThreshold";
QStringList ITKConnectedThresholdPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ConnectedThreshold")
	<< QObject::tr("about this plugin");
}

void ITKConnectedThresholdPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK ConnectedThreshold"))
    {
    	itkConnectedThresholdPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Connected Threshold 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKConnectedThresholdSpecializaed
{
public:
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		//
		ITKConnectedThresholdDialog d(callback, parent);
		
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
			
			//
			typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
			typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
			
			typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > CurvatureFlowImageFilterType;
			typename CurvatureFlowImageFilterType::Pointer smoothing = CurvatureFlowImageFilterType::New();
			
			typedef itk::ConnectedThresholdImageFilter<OutputImageType, OutputImageType> ConnectedFilterType;
			typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();
			
			typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]
			
			//set \pars
			LandmarkList list_landmark_sub=callback.getLandmark(win_list[i1]);
			if(list_landmark_sub.size()<1)
			{
				v3d_msg(QObject::tr("You should select one seed from your image."));
				return;
			}
			else
			{
			    //seeds
				for(int i=0;  i<list_landmark_sub.size(); i++)
				{
					//
					seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
					seedPosition[1] = list_landmark_sub[i].y -1;
					seedPosition[2] = list_landmark_sub[i].z -1;
					
					connectedThreshold->AddSeed( seedPosition );
				}
			}
			
			const TOutputPixelType lowerThreshold = 210; //mean
			const TOutputPixelType upperThreshold = 255;
			
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
					
					// connected threshold algorithm
					smoothing->SetInput( castImageFilter->GetOutput() );
					connectedThreshold->SetInput( smoothing->GetOutput() );
					
					smoothing->SetNumberOfIterations( 5 );
					smoothing->SetTimeStep( 0.125 );
					
					connectedThreshold->SetLower(  lowerThreshold  );
					connectedThreshold->SetUpper(  upperThreshold  );
					
					connectedThreshold->SetReplaceValue( 255 );
					
					try
					{
						//smoothing->Update();
						connectedThreshold->Update();
					}
					catch( itk::ExceptionObject & excp)
					{
						std::cerr << "Error run this filter." << std::endl;
						std::cerr << excp << std::endl;
						return;
					}
					
					connectedThreshold->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
					try
					{
						connectedThreshold->Update();
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
				
				// connected threshold algorithm
				smoothing->SetInput( castImageFilter->GetOutput() );
				connectedThreshold->SetInput( smoothing->GetOutput() );
				
				smoothing->SetNumberOfIterations( 5 );
				smoothing->SetTimeStep( 0.125 );
				
				connectedThreshold->SetLower(  lowerThreshold  );
				connectedThreshold->SetUpper(  upperThreshold  );
				
				connectedThreshold->SetReplaceValue( 255 );

				try
				{
					//smoothing->Update();
					connectedThreshold->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}

				// output
				typename OutputImageType::PixelContainer * container;
				
				container =connectedThreshold->GetOutput()->GetPixelContainer();
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
		ITKConnectedThresholdSpecializaed< input_pixel_type, output_pixel_type > runner; \
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


void itkConnectedThresholdPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES;
}


