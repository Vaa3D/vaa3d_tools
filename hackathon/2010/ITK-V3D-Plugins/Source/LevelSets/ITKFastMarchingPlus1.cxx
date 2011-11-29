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
Q_EXPORT_PLUGIN2(ITKFastMarchingPlus1, ITKFastMarchingPlusPlugin)

void itkFastMarchingPlusPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK FastMarchingPlus";
QStringList ITKFastMarchingPlusPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK FastMarchingPlus")
  << QObject::tr("about this plugin");
}

void ITKFastMarchingPlusPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK FastMarchingPlus"))
    {
      itkFastMarchingPlusPlugin(callback, parent);
    }
  else if (menu_name == QObject::tr("about this plugin"))
  {
    QMessageBox::information(parent, "Version info", "ITK Fast Marching 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
  }
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKFastMarchingPlusSpecializaed: public V3DITKFilterSingleImage<TInputPixelType, TOutputPixelType >
{
public:
	typedef V3DITKFilterSingleImage<TInputPixelType, TOutputPixelType > Superclass;
	typedef typename Superclass::Input3DImageType 	InputImageType;
	typedef typename Superclass::Output3DImageType 	OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
        //typedef itk::RescaleIntensityImageFilter< FloatImageType, InputImageType >   InvCastFilterType;

        typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType>  SmoothingFilterType;
      
        typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType>  GradientFilterType;
        typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  SigmoidFilterType;
	typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingPlusFilterType;
        typedef itk::ThresholdImageFilter<OutputImageType > ThresholdFilterType;
 


  ITKFastMarchingPlusSpecializaed(V3DPluginCallback* callback):Superclass(callback)
{
	castImageFilter = CastImageFilterType::New();
	smoothing = SmoothingFilterType::New();
	gradientMagnitude = GradientFilterType::New();
	sigmoid = SigmoidFilterType::New();
	fastMarching = FastMarchingPlusFilterType::New();
	thresholder = ThresholdFilterType::New();
	//rescaler=InvCastFilterType::New();
	this->RegisterInternalFilter(this->fastMarching,0.8);
	this->RegisterInternalFilter(this->sigmoid,0.1);
	this->RegisterInternalFilter(this->thresholder,0.1);
	
}	
  void Execute(V3DPluginCallback &callback, QWidget *parent)
 {
    V3DITKGenericDialog dialog("FastMarching");

    dialog.AddDialogElement("SpeedScale",255.0, 0.0, 256.0);
    dialog.AddDialogElement("StoppingTime",50.0, 0.0, 10000.0);

    const double stoppingTime = dialog.GetValue("StoppingTime");
    const double speedScale=dialog.GetValue("SpeedScale");

    const double seedValue = 0.0;

    if( dialog.exec() != QDialog::Accepted )
      {
      return;
      }

      sigmoid->SetOutputMinimum(  0.0  );
      sigmoid->SetOutputMaximum(  1.0  );
      typedef typename FastMarchingPlusFilterType::NodeContainer  NodeContainer;
      typedef typename FastMarchingPlusFilterType::NodeType    NodeType;
      typename NodeContainer::Pointer seeds = NodeContainer::New();

      typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

      NodeType node;

      //set \pars
    v3dhandleList windowList = this->m_V3DPluginCallback->getImageWindowList();

    LandmarkList list_landmark_sub = this->m_V3DPluginCallback->getLandmark( windowList[0] ); // FIXME

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

      thresholder->ThresholdAbove( stoppingTime );
      thresholder->SetOutsideValue( stoppingTime );
//	thresholder->SetInsideValue(0);
      fastMarching->SetStoppingValue(  stoppingTime  );
	fastMarching->SetNormalizationFactor( speedScale );
      const double sigma = 0.5; // GradientMagnitudeRecursiveGaussianImageFilter

      const double alpha =  -1; // SigmoidImageFilter
      const double beta  =  20;
      gradientMagnitude->SetSigma(  sigma  );

      smoothing->SetTimeStep( 0.125 );
      smoothing->SetNumberOfIterations(  5 );
      smoothing->SetConductanceParameter( 9.0 );

      sigmoid->SetAlpha( alpha );
      sigmoid->SetBeta(  beta  );

      fastMarching->SetTrialPoints(  seeds  );
	//rescaler->SetOutput
	this->Compute();

}
virtual void ComputeOneRegion()
{
	fastMarching->SetOutputSize(this->GetInput3DImage()->GetBufferedRegion().GetSize() );
	this->castImageFilter->SetInput(this->GetInput3DImage());
	this->smoothing->SetInput( this->castImageFilter->GetOutput() );
        this->gradientMagnitude->SetInput( this->smoothing->GetOutput() );
        this->sigmoid->SetInput( this->gradientMagnitude->GetOutput() );
        this->fastMarching->SetInput( sigmoid->GetOutput() );
        this->thresholder->SetInput( fastMarching->GetOutput() );
	thresholder->Update();
        this->SetOutputImage(this->thresholder->GetOutput());
}

private:

      typename CastImageFilterType::Pointer castImageFilter ;
      typename SmoothingFilterType::Pointer smoothing;
      typename GradientFilterType::Pointer  gradientMagnitude ;
      typename SigmoidFilterType::Pointer sigmoid ;
      typename FastMarchingPlusFilterType::Pointer fastMarching;
      typename ThresholdFilterType::Pointer thresholder ;
	//typename InvCastFilterType::Pointer rescaler;


};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
  case v3d_pixel_type: \
  { \
    ITKFastMarchingPlusSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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


void itkFastMarchingPlusPlugin(V3DPluginCallback &callback, QWidget *parent)
{
  Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
  if (!p4DImage)
    {
    v3d_msg(QObject::tr("You don't have any image open in the main window."));
    return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}


