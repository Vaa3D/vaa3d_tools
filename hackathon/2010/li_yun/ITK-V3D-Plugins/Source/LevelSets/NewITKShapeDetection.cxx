/* ITKShapeDetection.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKShapeDetection.h"
#include "V3DITKFilterSingleImage.h"

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
#include "itkShapeDetectionLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKShapeDetection, ITKShapeDetectionPlugin)

QStringList ITKShapeDetectionPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ShapeDetection")
			     << QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKShapeDetectionSpecializaed : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >        			Superclass;
	typedef itk::Image< TInputPixelType, 3 > 	     			 			InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 	      			 			OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> 					CastImageFilterType;
	typedef itk::RescaleIntensityImageFilter< OutputImageType, InputImageType >   			InvCastFilterType;
	typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType >    			ThresholdingFilterType;
	typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType > 	SmoothingFilterType;
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
	typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  				SigmoidFilterType;
	typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > 			FastMarchingFilterType;
	typedef itk::ShapeDetectionLevelSetImageFilter< OutputImageType, OutputImageType > 		ShapeDetectionFilterType;
	typedef typename FastMarchingFilterType::NodeContainer						NodeContainer;
	typedef typename FastMarchingFilterType::NodeType						NodeType;
public:
	ITKShapeDetectionSpecializaed( V3DPluginCallback * callback ): Superclass(callback) 
	{		
		this->castImageFilter = CastImageFilterType::New();
		this->smoothing = SmoothingFilterType::New();
		this->gradientMagnitude = GradientFilterType::New();
		this->sigmoid = SigmoidFilterType::New();
		this->fastMarching = FastMarchingFilterType::New();
		this->shapeDetection = ShapeDetectionFilterType::New();
		this->seeds = NodeContainer::New();

		this->RegisterInternalFilter( this->castImageFilter, 0.1 );
		this->RegisterInternalFilter( this->smoothing, 0.1);
		this->RegisterInternalFilter( this->gradientMagnitude, 0.1 );
		this->RegisterInternalFilter( this->sigmoid, 0.1 );
		this->RegisterInternalFilter( this->fastMarching, 0.1 );
		this->RegisterInternalFilter( this->shapeDetection, 0.5 );
	}

	virtual ~ITKShapeDetectionSpecializaed() {};

	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{		

		V3DITKGenericDialog dialog("ITKShapeDetection");
		dialog.AddDialogElement("StoppingTime", 100.0, 0.0, 10000.0);
		if( dialog.exec() != QDialog::Accepted )
		{
		return;
		}

		const double stoppingTime = dialog.GetValue("StoppingTime");
		
		sigmoid->SetOutputMinimum(  0.0  );
		sigmoid->SetOutputMaximum(  1.0  );

		const double initialDistance = 15.0; 
		const double seedValue = - initialDistance;

		v3dhandleList win_list = this->m_V3DPluginCallback->getImageWindowList();
		LandmarkList list_landmark_sub=callback.getLandmark(win_list[0]);
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
		const double sigma = 0.5; // GradientMagnitudeRecursiveGaussianImageFilter			
		const double alpha =  -1; // SigmoidImageFilter
		const double beta  =  20;

		const double curvatureScaling   = 0.5; // Level Set 
		const double propagationScaling = 1.0; 
		
		gradientMagnitude->SetSigma(  sigma  );

		smoothing->SetTimeStep( 0.125 );
		smoothing->SetNumberOfIterations(  5 );
		smoothing->SetConductanceParameter( 9.0 );
		
		sigmoid->SetAlpha( alpha );
		sigmoid->SetBeta(  beta  );
		
		fastMarching->SetTrialPoints(  seeds  );
		fastMarching->SetOutputSize( this->GetInput3DImage()->GetBufferedRegion().GetSize() );
		//fastMarching->SetStoppingValue(  stoppingTime  );
		fastMarching->SetSpeedConstant( 1.0 );
		
		shapeDetection->SetPropagationScaling(  propagationScaling );
		shapeDetection->SetCurvatureScaling( curvatureScaling ); 
		
		shapeDetection->SetMaximumRMSError( 0.02 );
		shapeDetection->SetNumberOfIterations( stoppingTime );
		this->Compute();

	}
	virtual void ComputeOneRegion()
	{
		castImageFilter->SetInput( this->GetInput3DImage() );
		smoothing->SetInput( castImageFilter->GetOutput() );
		gradientMagnitude->SetInput( smoothing->GetOutput() );
		sigmoid->SetInput( gradientMagnitude->GetOutput() );
		shapeDetection->SetInput( fastMarching->GetOutput() );
		shapeDetection->SetFeatureImage( sigmoid->GetOutput() );

		shapeDetection->Update();
		this->SetOutputImage( this->shapeDetection->GetOutput() );
	}	
private:
	typename CastImageFilterType::Pointer 			castImageFilter;
	typename SmoothingFilterType::Pointer 			smoothing;
	typename GradientFilterType::Pointer  			gradientMagnitude;
	typename SigmoidFilterType::Pointer 			sigmoid;
	typename FastMarchingFilterType::Pointer 		fastMarching;
	typename ShapeDetectionFilterType::Pointer 		shapeDetection;
	typename NodeContainer::Pointer 			seeds;
	typename OutputImageType::IndexType  			seedPosition;
	NodeType 						node;

};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKShapeDetectionSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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
 

void ITKShapeDetectionPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Shape Detection 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
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


