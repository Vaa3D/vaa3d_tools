/* ITKConfidenceConnected.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKConfidenceConnected.h"
#include "V3DITKFilterSingleImage.h"
// ITK Header Files

#include "itkConfidenceConnectedImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKConfidenceConnected, ITKConfidenceConnectedPlugin)

void itkConfidenceConnectedPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK ConfidenceConnected";
QStringList ITKConfidenceConnectedPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ConfidenceConnected")
	<< QObject::tr("about this plugin");
}

void ITKConfidenceConnectedPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK ConfidenceConnected"))
    {
    	itkConfidenceConnectedPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Confidence Connected 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKConfidenceConnectedSpecializaed:public V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>
{
public:
	typedef V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>  Superclass;
	typedef typename Superclass ::Input3DImageType InputImageType;
	typedef typename Superclass ::Output3DImageType OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;

	typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > CurvatureFlowImageFilterType;

	typedef itk::ConfidenceConnectedImageFilter<OutputImageType, OutputImageType> ConnectedFilterType;

ITKConfidenceConnectedSpecializaed(V3DPluginCallback*callback):Superclass(callback)
{
	castImageFilter = CastImageFilterType::New();
	smoothing = CurvatureFlowImageFilterType::New();
	confidenceConnected = ConnectedFilterType::New();
	this->RegisterInternalFilter(this->confidenceConnected,0.7);
	this->RegisterInternalFilter(this->smoothing,0.2);
	this->RegisterInternalFilter(this->castImageFilter,0.1);

}
	 	
	
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		
			v3dhandleList win_list = callback.getImageWindowList();
			
			LandmarkList list_landmark_sub=callback.getLandmark(win_list[0]);
			typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]
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
					
					confidenceConnected->AddSeed( seedPosition );
				}

				smoothing->SetNumberOfIterations( 2 );
			        smoothing->SetTimeStep( 0.05 );
					
			  	confidenceConnected->SetMultiplier( 2.5 );
				confidenceConnected->SetNumberOfIterations( 5 );
				confidenceConnected->SetInitialNeighborhoodRadius( 2 );
				confidenceConnected->SetReplaceValue( 255 );
				this->Compute();
			}
			
	}
	virtual void ComputeOneRegion()
	{
				
		castImageFilter->SetInput(this->GetInput3DImage());
		smoothing->SetInput( castImageFilter->GetOutput() );
		confidenceConnected->SetInput( smoothing->GetOutput() );
		try
			{
						
			 confidenceConnected->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
		this->SetOutputImage(this->confidenceConnected->GetOutput());
	}	
	
private:
	typename CastImageFilterType::Pointer castImageFilter;
	typename CurvatureFlowImageFilterType::Pointer smoothing;
	typename ConnectedFilterType::Pointer confidenceConnected;
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKConfidenceConnectedSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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


void itkConfidenceConnectedPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES;
}


