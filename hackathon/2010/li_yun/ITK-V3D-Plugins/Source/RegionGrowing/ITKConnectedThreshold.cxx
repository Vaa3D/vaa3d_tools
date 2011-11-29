/* ITKConnectedThreshold.cxx
 * 2010-06-03: create this program by Yang Yu
 */


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKConnectedThreshold.h"
#include "V3DITKFilterSingleImage.h"

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

QStringList ITKConnectedThresholdPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ConnectedThreshold")
	<< QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKConnectedThresholdSpecializaed : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >             	Superclass;
	typedef itk::Image< TInputPixelType, 3 > 						InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 						OutputImageType;

	typedef itk::CastImageFilter< InputImageType, OutputImageType> 				CastImageFilterType;
	typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > 		CurvatureFlowImageFilterType;
	typedef itk::ConnectedThresholdImageFilter<OutputImageType, OutputImageType>		ConnectedFilterType;


public:
ITKConnectedThresholdSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
{
	this->castImageFilter	  = CastImageFilterType::New(); 	
	this->smoothing 	  = CurvatureFlowImageFilterType::New();
	this->connectedThreshold  = ConnectedFilterType::New();
	
	this->RegisterInternalFilter( this->castImageFilter, 0.2 );
	this->RegisterInternalFilter( this->connectedThreshold, 0.8 );
};

virtual ~ITKConnectedThresholdSpecializaed() {};

void Execute(V3DPluginCallback &callback, QWidget *parent)
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
	    //seeds
		for(int i=0;  i<list_landmark_sub.size(); i++)
		{
			//
			seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
			seedPosition[1] = list_landmark_sub[i].y -1;
			seedPosition[2] = list_landmark_sub[i].z -1;
			
			
		}
	}
	SetupParameters();
	this->Compute();
}
virtual void ComputeOneRegion()
{
	connectedThreshold->AddSeed( seedPosition );
	castImageFilter->SetInput( this->GetInput3DImage() );
				
	castImageFilter->Update();
	smoothing->SetInput( castImageFilter->GetOutput() );
	connectedThreshold->SetInput( smoothing->GetOutput() );
				
	smoothing->SetNumberOfIterations( 5 );
	smoothing->SetTimeStep( 0.125 );
				
	connectedThreshold->Update();
	this->SetOutputImage( this->connectedThreshold->GetOutput() );
}
virtual void SetupParameters()
{
	V3DITKGenericDialog dialog("ITK ConnectedThreshold");

	dialog.AddDialogElement("LowerThreshold", 10.0, 0.0, 255.0);
	dialog.AddDialogElement("UpperThreshold", 150.0, 0.0, 255.0); 
	dialog.AddDialogElement("ReplaceValue", 255.0, 0.0, 255.0); 

	if( dialog.exec() == QDialog::Accepted )
	{
	connectedThreshold->SetLower( dialog.GetValue("LowerThreshold") );
	connectedThreshold->SetUpper( dialog.GetValue("UpperThreshold") );
	connectedThreshold->SetReplaceValue( dialog.GetValue("ReplaceValue") );
	}
}			

private:
	typename CastImageFilterType::Pointer 			castImageFilter;	
	typename CurvatureFlowImageFilterType::Pointer 		smoothing;
	typename ConnectedFilterType::Pointer 			connectedThreshold;

	typename OutputImageType::IndexType  			seedPosition; 
	
};


#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKConnectedThresholdSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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

void ITKConnectedThresholdPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITK Connected Threshold 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
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




