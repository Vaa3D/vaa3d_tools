/* ITKThresholdSegmentation.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKThresholdSegmentation.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkZeroCrossingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkThresholdSegmentationLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKThresholdSegmentation, ITKThresholdSegmentationPlugin)

QStringList ITKThresholdSegmentationPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ThresholdSegmentation")
			     << QObject::tr("about this plugin");
}

QStringList ITKThresholdSegmentationPlugin::funclist() const
{
    return QStringList();
}

void ITKThresholdSegmentationPlugin::dofunc(const QString & func_name,
 	const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKThresholdSegmentationSpecializaed  : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >                    	Superclass;
	typedef itk::Image< TInputPixelType, 3 > 							InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 							OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> 					CastImageFilterType;
	typedef itk::ThresholdSegmentationLevelSetImageFilter< OutputImageType, OutputImageType >  	ThresholdSegmentationLevelSetImageFilterType;
	typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType >			FastMarchingFilterType;
	typedef typename FastMarchingFilterType::NodeContainer						NodeContainer;
	typedef typename FastMarchingFilterType::NodeType						NodeType;
		
public:	
	ITKThresholdSegmentationSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->castImageFilter = CastImageFilterType::New();
		this->thresholdSegmentation = ThresholdSegmentationLevelSetImageFilterType::New();
		this->fastMarching = FastMarchingFilterType::New();
		this->seeds = NodeContainer::New();

		this->RegisterInternalFilter( this->castImageFilter, 0.1 );
		this->RegisterInternalFilter( this->thresholdSegmentation, 0.5 );
		this->RegisterInternalFilter( this->fastMarching, 0.4 );
	}

	virtual ~ITKThresholdSegmentationSpecializaed() {};

	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		const double initialDistance = 5.0; 
		const double seedValue = - initialDistance;
		
		v3dhandle curwin = callback.currentImageWindow();
		LandmarkList list_landmark_sub=callback.getLandmark(curwin);
		if(list_landmark_sub.size()<1)
		{
			v3d_msg(QObject::tr("You should select one seed from your image."));
			return;
		}
		else
		{
			
			seeds->Initialize();			
			for(int i=0;  i<list_landmark_sub.size(); i++)
			{
				seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
				seedPosition[1] = list_landmark_sub[i].y -1;
				seedPosition[2] = list_landmark_sub[i].z -1;
				
				node.SetValue( seedValue );
				node.SetIndex( seedPosition );
				
				seeds->InsertElement( i, node );
			}
		}
		
		const double curvatureScaling   = 1.0; // Level Set 
		const double propagationScaling = 1.0; 
		
		fastMarching->SetTrialPoints(  seeds  );
		fastMarching->SetOutputSize( this->GetInput3DImage()->GetBufferedRegion().GetSize() );
		fastMarching->SetSpeedConstant( 1.0 );

		thresholdSegmentation->SetPropagationScaling( propagationScaling );
		thresholdSegmentation->SetCurvatureScaling( curvatureScaling );

		thresholdSegmentation->SetMaximumRMSError( 0.02 );
		thresholdSegmentation->SetNumberOfIterations( 100 ); //stoppingTime

		thresholdSegmentation->SetUpperThreshold( 255 );
		thresholdSegmentation->SetLowerThreshold( 100 );
		thresholdSegmentation->SetIsoSurfaceValue(0.0);

		this->Compute();
	}

	virtual void ComputeOneRegion()
	{
		castImageFilter->SetInput( this->GetInput3DImage() );
		thresholdSegmentation->SetInput( fastMarching->GetOutput() );
		thresholdSegmentation->SetFeatureImage( castImageFilter->GetOutput() );
		
		thresholdSegmentation->Update();
		this->SetOutputImage( this->thresholdSegmentation->GetOutput() );

	}

private:
	typename CastImageFilterType::Pointer castImageFilter;
	typename ThresholdSegmentationLevelSetImageFilterType::Pointer thresholdSegmentation;
	typename FastMarchingFilterType::Pointer fastMarching;
	typename NodeContainer::Pointer seeds;
	typename OutputImageType::IndexType  seedPosition; 
	NodeType node;	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKThresholdSegmentationSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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



void ITKThresholdSegmentationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK ThresholdSegmentation"))
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
	QMessageBox::information(parent, "Version info", "ITK Threshold Segmentation 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
    }
}



