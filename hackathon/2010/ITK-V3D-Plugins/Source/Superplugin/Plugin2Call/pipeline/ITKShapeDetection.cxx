#include <QtGui>

#include <math.h>
#include <stdlib.h>


#include "V3DITKFilterSingleImage.h"


#include "ITKShapeDetection.h"


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

void itkShapeDetectionPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK ShapeDetection";
QStringList ITKShapeDetectionPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ShapeDetection")
						 << QObject::tr("about this plugin");
}
QStringList ITKShapeDetectionPlugin::funclist() const
{
	return QStringList() << QObject::tr("ITK ShapeDetection")
						 << QObject::tr("about this plugin");
}

void ITKShapeDetectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK ShapeDetection"))
    {
    	itkShapeDetectionPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Shape Detection 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKShapeDetectionSpecializaed: public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
  typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >      Superclass;
  typedef typename Superclass::Input3DImageType               InputImageType;
  typedef typename Superclass::Output3DImageType              OutputImageType;

  typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
  typedef itk::RescaleIntensityImageFilter< InputImageType,OutputImageType> CastInputFilterType;

  typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType    >    ThresholdingFilterType;
  typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType >  SmoothingFilterType;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
  typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  SigmoidFilterType;
  typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingFilterType;
  typedef  itk::ShapeDetectionLevelSetImageFilter< OutputImageType, OutputImageType > ShapeDetectionFilterType;
  typedef itk::CastImageFilter< OutputImageType, InputImageType>  CastOutputType;


public:

  ITKShapeDetectionSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
    {
	castImageFilter=CastImageFilterType::New();
        smoothing=SmoothingFilterType::New();
        gradientMagnitude=GradientFilterType::New() ;
        sigmoid=SigmoidFilterType::New() ;
        fastMarching=FastMarchingFilterType::New();
        thresholder=ThresholdingFilterType::New() ;
        shapeDetection=ShapeDetectionFilterType::New();
	castOutputFilter=CastOutputType::New();
	castInputFilter=CastInputFilterType::New();
	this->RegisterInternalFilter(this->shapeDetection,0.6);
	this->RegisterInternalFilter(this->fastMarching,0.2);
	this->RegisterInternalFilter(this->gradientMagnitude,0.2);
	
    }

  virtual ~ITKShapeDetectionSpecializaed() {};


  void Execute(V3DPluginCallback &callback, QWidget *parent)
    {
      typedef typename FastMarchingFilterType::NodeContainer  NodeContainer;
      typedef typename FastMarchingFilterType::NodeType    NodeType;
      typename NodeContainer::Pointer seeds = NodeContainer::New();

      typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

      NodeType node;

      //set \pars
	const double initialDistance = 15.0; //
	const double seedValue = - initialDistance;
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

      int nx=this->m_NumberOfPixelsAlongX,ny=this->m_NumberOfPixelsAlongY,nz=this->m_NumberOfPixelsAlongZ;
      const double stoppingTime = sqrt(nx*nx + ny*ny + nz*nz);


    
	
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
					
	fastMarching->SetTrialPoints( seeds);

	fastMarching->SetSpeedConstant( 1.0 );
					
	shapeDetection->SetPropagationScaling(  propagationScaling );
	shapeDetection->SetCurvatureScaling( curvatureScaling ); 
					
	shapeDetection->SetMaximumRMSError( 0.02 );
	shapeDetection->SetNumberOfIterations( stoppingTime );


    this->Compute();
    }

  virtual void ComputeOneRegion()
    {
	fastMarching->SetOutputSize(this->GetInput3DImage()->GetBufferedRegion().GetSize() );

	castImageFilter->SetInput( this->GetInput3DImage() );
	smoothing->SetInput( castImageFilter->GetOutput() );
	gradientMagnitude->SetInput( smoothing->GetOutput() );
	sigmoid->SetInput( gradientMagnitude->GetOutput() );
					
	shapeDetection->SetInput( fastMarching->GetOutput() );
	shapeDetection->SetFeatureImage( sigmoid->GetOutput() ); 
	shapeDetection->Update();   
    

        this->SetOutputImage( this->shapeDetection->GetOutput() );
    }
void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

      	typedef typename FastMarchingFilterType::NodeContainer  NodeContainer;
      	typedef typename FastMarchingFilterType::NodeType    NodeType;
      	typename NodeContainer::Pointer seeds = NodeContainer::New();

      	typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

      	NodeType node;
	//get the parameter		
	V3DITKGenericDialog dialog("ITKShapeDetection");

    	dialog.AddDialogElement("initialDistance",5.0, 0.0, 255.0);

    		if( dialog.exec() != QDialog::Accepted )
      		{
			return;	
		}
  
	const double initialDistance = dialog.GetValue("initialDistance");; //
	const double seedValue = - initialDistance;

    	seeds->Initialize();

    	v3dhandleList windowList = this->m_V3DPluginCallback->getImageWindowList();

   	LandmarkList listOfLandmarks = this->m_V3DPluginCallback->getLandmark( windowList[0] ); // FIXME

    	unsigned int numberOfSeedPoints = listOfLandmarks.size();

    	if ( ! numberOfSeedPoints )
      	{
      	v3d_msg(QObject::tr("You should select one seed from your image."));
      	return;
      	}

    	for(unsigned int i = 0;  i < numberOfSeedPoints; i++ )
      	{
      	seedPosition[0] = listOfLandmarks[i].x - 1; // notice 0-based and 1-based difference
      	seedPosition[1] = listOfLandmarks[i].y - 1;
      	seedPosition[2] = listOfLandmarks[i].z - 1;

      	node.SetValue( seedValue );
      	node.SetIndex( seedPosition );

      	seeds->InsertElement( i, node );
      	}

	fastMarching->SetTrialPoints( seeds);

	fastMarching->SetSpeedConstant( 1.0 );
	const double curvatureScaling   = 0.05; // Level Set 
	const double propagationScaling = 1.0; 
	shapeDetection->SetPropagationScaling(  propagationScaling );
	shapeDetection->SetCurvatureScaling( curvatureScaling ); 
	shapeDetection->SetMaximumRMSError( 0.02 );
	shapeDetection->SetNumberOfIterations( 800 );
	castInputFilter->SetOutputMinimum(0.0001);
	castInputFilter->SetOutputMaximum(1.0);




       	V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

       	this->AddObserver( progressDialog.GetCommand() );
       	progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
       	progressDialog.show();
	this->RegisterInternalFilter( this->shapeDetection, 1.0 );

	void * p=NULL;
	p=(void*)input.at(0).p;
	if(!p)perror("errro");
	
	this->castInputFilter->SetInput((InputImageType*) p);
	this->castInputFilter->Update();
	this->fastMarching->SetOutputSize(castInputFilter->GetOutput()->GetBufferedRegion().GetSize());
	shapeDetection->SetInput( fastMarching->GetOutput() );
	shapeDetection->SetFeatureImage(this->castInputFilter->GetOutput());
	this->castOutputFilter->SetInput(shapeDetection->GetOutput());
	

	this->castOutputFilter->Update();
	V3DPluginArgItem arg;
	arg.p=castOutputFilter->GetOutput();
	arg.type="UINT8Image";
	output.replace(0,arg);

    }

private:

      typename CastImageFilterType::Pointer castImageFilter ;
      typename SmoothingFilterType::Pointer smoothing;
      typename GradientFilterType::Pointer  gradientMagnitude ;
      typename SigmoidFilterType::Pointer sigmoid ;
      typename FastMarchingFilterType::Pointer fastMarching;
      typename ThresholdingFilterType::Pointer thresholder ;
      typename ShapeDetectionFilterType::Pointer shapeDetection;
      typename CastOutputType::Pointer	castOutputFilter;
      typename CastInputFilterType::Pointer  castInputFilter;
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKShapeDetectionSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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

bool ITKShapeDetectionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
  if (func_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
    return false ;
    }
	ITKShapeDetectionSpecializaed<unsigned char,float> *runner=new ITKShapeDetectionSpecializaed<unsigned char,float>(&v3d);
	runner->ComputeOneRegion(input, output); 
	return true;  

}

void itkShapeDetectionPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES;
}




