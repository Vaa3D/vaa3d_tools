/* ITKIsolatedConnected.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKIsolatedConnected.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files

#include "itkIsolatedConnectedImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKIsolatedConnected, ITKIsolatedConnectedPlugin)

void itkIsolatedConnectedPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK IsolatedConnected";
QStringList ITKIsolatedConnectedPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK IsolatedConnected")
						<< QObject::tr("about this plugin");
}

void ITKIsolatedConnectedPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK IsolatedConnected"))
    {
    	itkIsolatedConnectedPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Isolated Connected 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKIsolatedConnectedSpecializaed:public V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>
{
public:
	typedef V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>  Superclass;
	typedef typename Superclass ::Input3DImageType InputImageType;
	typedef typename Superclass ::Output3DImageType OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;

	typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > CurvatureFlowImageFilterType;

	typedef itk::IsolatedConnectedImageFilter<OutputImageType, OutputImageType>   ConnectedFilterType;

ITKIsolatedConnectedSpecializaed(V3DPluginCallback*callback):Superclass(callback)
{
	castImageFilter = CastImageFilterType::New();
	smoothing = CurvatureFlowImageFilterType::New();
	isolatedConnected= ConnectedFilterType::New();
	this->RegisterInternalFilter(this->isolatedConnected,0.7);
	this->RegisterInternalFilter(this->smoothing,0.2);
	this->RegisterInternalFilter(this->castImageFilter,0.1);

}
	 	
	
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		
			v3dhandleList win_list = callback.getImageWindowList();
			
			LandmarkList list_landmark_sub=callback.getLandmark(win_list[0]);
			typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]
			if(list_landmark_sub.size()!=2)
			{
				v3d_msg(QObject::tr("You should select two seeds from your image."));
				return;
			}
			else
			{
			    //seeds
				seedPosition[0] = list_landmark_sub[0].x -1; // notice 0-based and 1-based difference
				seedPosition[1] = list_landmark_sub[0].y -1;
				seedPosition[2] = list_landmark_sub[0].z -1;
				
				isolatedConnected->AddSeed1( seedPosition );
				
				seedPosition[0] = list_landmark_sub[1].x -1; // notice 0-based and 1-based difference
				seedPosition[1] = list_landmark_sub[1].y -1;
				seedPosition[2] = list_landmark_sub[1].z -1;
				
				isolatedConnected->AddSeed2( seedPosition );
				
			}
				V3DITKGenericDialog dialog("ITKIsolatedConnected");

    			 	dialog.AddDialogElement("lowerThreshold",200.0, 0.0, 255.0);

    				if( dialog.exec() == QDialog::Accepted )
      				{

				const TOutputPixelType lowerThreshold = dialog.GetValue("lowerThreshold");
				smoothing->SetNumberOfIterations( 5 );
				smoothing->SetTimeStep( 0.125 );
					
				isolatedConnected->SetLower(  lowerThreshold  );
					
				isolatedConnected->SetReplaceValue( 255 );
				this->Compute();
				}
			
			
	}
	virtual void ComputeOneRegion()
	{
				
		castImageFilter->SetInput(this->GetInput3DImage());
		smoothing->SetInput( castImageFilter->GetOutput() );
		isolatedConnected->SetInput( smoothing->GetOutput() );
		try
			{
						
			 isolatedConnected->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
		this->SetOutputImage(this->isolatedConnected->GetOutput());
	}	
	
private:
	typename CastImageFilterType::Pointer castImageFilter;
	typename CurvatureFlowImageFilterType::Pointer smoothing;
	typename ConnectedFilterType::Pointer isolatedConnected;
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKIsolatedConnectedSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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


void itkIsolatedConnectedPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES;
}


