/* ITKFastMarchingPlus.cxx
 * 2010-06-02: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKFastMarchingPlus.h"
#include "V3DITKFilterSingleImage.h"

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

//plugin funcs
QStringList ITKFastMarchingPlusPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK FastMarchingPlus")
  		       << QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKFastMarchingPlusSpecializaed : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
	typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >          			Superclass;
	typedef itk::Image< TInputPixelType, 3 > 							InputImageType;
	typedef itk::Image< TOutputPixelType, 3 > 							OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType>					CastImageFilterType;
	typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType > 	SmoothingFilterType;
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
	typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  				SigmoidFilterType;
	typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > 			FastMarchingPlusFilterType;
	typedef itk::ThresholdImageFilter< OutputImageType > 						ThresholdFilterType;
	typedef typename FastMarchingPlusFilterType::NodeContainer  					NodeContainer;
	typedef typename FastMarchingPlusFilterType::NodeType   					NodeType;

public:
	ITKFastMarchingPlusSpecializaed( V3DPluginCallback * callback ): Superclass(callback) 
	{
		this->castImageFilter = CastImageFilterType::New();
		this->smoothing = SmoothingFilterType::New();
		this->gradientMagnitude = GradientFilterType::New();
		this->sigmoid = SigmoidFilterType::New();
		this->fastMarching = FastMarchingPlusFilterType::New();
		this->seeds = NodeContainer::New();
		this->thresholder = ThresholdFilterType::New();

	    	this->RegisterInternalFilter( this->castImageFilter, 0.1 );
                this->RegisterInternalFilter( this->smoothing, 0.1);
 		this->RegisterInternalFilter( this->gradientMagnitude, 0.1 );
		this->RegisterInternalFilter( this->fastMarching, 0.1 );
		this->RegisterInternalFilter( this->thresholder, 0.6);
	}
	virtual ~ITKFastMarchingPlusSpecializaed() {};

	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		V3DITKGenericDialog dialog("ITKFastMarchingPlus");
		dialog.AddDialogElement("StoppingTime",50.0, 0.0, 10000.0);
		if( dialog.exec() != QDialog::Accepted )
		{
		return;
		}

		sigmoid->SetOutputMinimum(  0.0  );
		sigmoid->SetOutputMaximum(  1.0  );

		const double stoppingTime = dialog.GetValue("StoppingTime");
		const double seedValue = 0.0;

		thresholder->ThresholdAbove( stoppingTime );
		thresholder->SetOutsideValue( stoppingTime );
		fastMarching->SetStoppingValue(  stoppingTime  );   

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

		gradientMagnitude->SetSigma(  sigma  );
		smoothing->SetTimeStep( 0.05 );
		smoothing->SetNumberOfIterations(  5 );
		smoothing->SetConductanceParameter( 3.0 );

		sigmoid->SetOutputMinimum(  0.0  );
		sigmoid->SetOutputMaximum(  1.0  );

		sigmoid->SetAlpha( alpha );
		sigmoid->SetBeta(  beta  );

		fastMarching->SetTrialPoints(  seeds  );
		fastMarching->SetOutputSize( this->GetInput3DImage()->GetBufferedRegion().GetSize() );

		this->Compute();
	}
 
	virtual void ComputeOneRegion()
	{
		castImageFilter->SetInput( this->GetInput3DImage() );
		smoothing->SetInput( castImageFilter->GetOutput() );
		gradientMagnitude->SetInput( smoothing->GetOutput() );
		sigmoid->SetInput( gradientMagnitude->GetOutput() );
		fastMarching->SetInput( sigmoid->GetOutput() );
		thresholder->SetInput( fastMarching->GetOutput() );	
	
		thresholder->Update();
	        this->SetOutputImage( this->thresholder->GetOutput() );
	}

private:
	typename CastImageFilterType::Pointer 					castImageFilter;
	typename SmoothingFilterType::Pointer 					smoothing;
	typename GradientFilterType::Pointer  					gradientMagnitude;
	typename SigmoidFilterType::Pointer 					sigmoid;
	typename FastMarchingPlusFilterType::Pointer 				fastMarching;
	typename ThresholdFilterType::Pointer 					thresholder;
	typename NodeContainer::Pointer						seeds;
	NodeType 								node;
	typename OutputImageType::IndexType  					seedPosition;
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKFastMarchingPlusSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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

void ITKFastMarchingPlusPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Fast Marching 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
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


