/* ITKConfidenceConnected.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKConfidenceConnected.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKConfidenceConnected, ITKConfidenceConnectedPlugin)

//plugin funcs
QStringList ITKConfidenceConnectedPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ConfidenceConnected")
	<< QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKConfidenceConnectedSpecializaed : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >             	Superclass;
	typedef itk::Image< TInputPixelType, 3 > 						InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 						OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType>				CastImageFilterType;
	typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > 		CurvatureFlowImageFilterType;
	typedef itk::ConfidenceConnectedImageFilter<OutputImageType, OutputImageType> 		ConnectedFilterType;

public:
ITKConfidenceConnectedSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
{
	this->castImageFilter = CastImageFilterType::New(); 	
	this->smoothing = CurvatureFlowImageFilterType::New();
	this->confidenceConnected = ConnectedFilterType::New();
	
	this->RegisterInternalFilter( this->castImageFilter, 0.2 );
	this->RegisterInternalFilter( this->confidenceConnected, 0.8 );
};

virtual ~ITKConfidenceConnectedSpecializaed() {};


void Execute( V3DPluginCallback &callback, QWidget *parent)
{
		
	v3dhandleList win_list = callback.getImageWindowList();
	LandmarkList list_landmark_sub=callback.getLandmark(win_list[0]);
	if(list_landmark_sub.size()<1)
	{
		v3d_msg(QObject::tr("You should select one seed from your image."));
		return;
	}
	else
	{
		for(int i=0;  i<list_landmark_sub.size(); i++)
		{
			seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
			seedPosition[1] = list_landmark_sub[i].y -1;
			seedPosition[2] = list_landmark_sub[i].z -1;		
		}
	}

	this->Compute();
}
		

virtual void ComputeOneRegion()
{
	confidenceConnected->AddSeed( seedPosition );
	castImageFilter->SetInput( this->GetInput3DImage() );
	
	castImageFilter->Update();
	smoothing->SetInput( castImageFilter->GetOutput() );
	confidenceConnected->SetInput( smoothing->GetOutput() );
	
	smoothing->SetNumberOfIterations( 2 );
	smoothing->SetTimeStep( 0.05 );
	
	confidenceConnected->SetMultiplier( 2.5 );
	confidenceConnected->SetNumberOfIterations( 5 );
	confidenceConnected->SetInitialNeighborhoodRadius( 2 );
	confidenceConnected->SetReplaceValue( 255 );

	confidenceConnected->Update();

	this->SetOutputImage( this->confidenceConnected->GetOutput() );
}

private:
	typename CastImageFilterType::Pointer 			castImageFilter;	
	typename CurvatureFlowImageFilterType::Pointer 		smoothing;
	typename ConnectedFilterType::Pointer 			confidenceConnected;

	typename OutputImageType::IndexType  			seedPosition; 
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKConfidenceConnectedSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
		runner.Execute(callback, parent ); \
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


void ITKConfidenceConnectedPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Confidence Connected 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
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
	
	EXECUTE_ALL_PIXEL_TYPES; 
}





