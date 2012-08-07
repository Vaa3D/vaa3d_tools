/* ITKCannySegmentation.cxx
 * 2010-06-04: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKCannySegmentation_called.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

#include "itkZeroCrossingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkCannySegmentationLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKCannySegmentationnew, ITKCannySegmentationPlugin)

//plugin funcs
const QString title = "ITK CannySegmentationPlugin1";
QStringList ITKCannySegmentationPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CannySegmentationPlugin1")
           << QObject::tr("about this plugin");
}

QStringList ITKCannySegmentationPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK CannySegmentationPlugin1")
           << QObject::tr("about this plugin");
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKCannySegmentationSpecializaed : public V3DITKFilterDualImage < TInputPixelType, TInputPixelType >
{
public:
    typedef V3DITKFilterDualImage< TInputPixelType, TInputPixelType >    Superclass;

    typedef typename Superclass::Input3DImageType InputImageType;
    typedef typename Superclass::Output3DImageType OutputImageType;

    typedef itk::Image<float,3>  FImageType;

    typedef itk::CastImageFilter< InputImageType, FImageType> CastImageFilterType;
    typedef itk::BinaryThresholdImageFilter< FImageType, InputImageType >  ThresholdingFilterType;
    typedef itk::CannySegmentationLevelSetImageFilter< FImageType, FImageType >  CannySegmentationLevelSetImageFilterType;
    typedef itk::GradientAnisotropicDiffusionImageFilter< FImageType,FImageType> DiffusionFilterType;



    ITKCannySegmentationSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
    {
        castImageFilter1 = CastImageFilterType::New();
        castImageFilter2 = CastImageFilterType::New();
        thresholder = ThresholdingFilterType::New();
        cannySegmentation = CannySegmentationLevelSetImageFilterType::New();
        diffusion = DiffusionFilterType::New();
        this->RegisterInternalFilter(this->cannySegmentation,0.9);
        this->RegisterInternalFilter(this->diffusion,0.1);

        diffusion->SetNumberOfIterations(5);
        diffusion->SetTimeStep(0.05);
        diffusion->SetConductanceParameter(1.0);

        cannySegmentation->SetAdvectionScaling( 10.0 );
        cannySegmentation->SetCurvatureScaling( 1.0 );
        cannySegmentation->SetPropagationScaling( 0.0 );

        cannySegmentation->SetMaximumRMSError( 0.01 );
        cannySegmentation->SetNumberOfIterations( 20 );

        cannySegmentation->SetThreshold( 7.0 );
        cannySegmentation->SetVariance( 0.1 );

        cannySegmentation->SetIsoSurfaceValue( 127.5 );
        thresholder->SetUpperThreshold( 10.0 );
        thresholder->SetLowerThreshold( 0.0 );

        thresholder->SetOutsideValue(  0  );
        thresholder->SetInsideValue(  255 );
    }
    virtual ~ITKCannySegmentationSpecializaed() {};

    //
    void Execute(V3DPluginCallback &callback, QWidget *parent)
    {

        this->SetImageSelectionDialogTitle("CannySegmentation");
        this->AddImageSelectionLabel("Image to Segmentation");
        this->AddImageSelectionLabel("Featrue Image");
        this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

        this->Compute();
    }
    virtual void ComputeOneRegion()
    {

        this->castImageFilter1->SetInput( this->GetInput3DImage1());
        this->castImageFilter2->SetInput( this->GetInput3DImage2());
        this->diffusion->SetInput(this->castImageFilter1->GetOutput());
        this->cannySegmentation->SetInput(this->castImageFilter2->GetOutput());
        this->cannySegmentation->SetFeatureImage(this->diffusion->GetOutput());
        this->thresholder->SetInput(this->cannySegmentation->GetOutput());

        this->thresholder->Update();
        this->SetOutputImage(this->thresholder->GetOutput());





        std::cout << std::endl;
        std::cout << "Max. no. iterations: " << cannySegmentation->GetNumberOfIterations() << std::endl;
        std::cout << "Max. RMS error: " << cannySegmentation->GetMaximumRMSError() << std::endl;
        std::cout << std::endl;
        std::cout << "No. elpased iterations: " << cannySegmentation->GetElapsedIterations() << std::endl;
        std::cout << "RMS change: " << cannySegmentation->GetRMSChange() << std::endl;
    }
    void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter(this->cannySegmentation,0.9);
        this->RegisterInternalFilter(this->diffusion,0.1);

        void * p1=NULL;
        void * p2=NULL;
        p1=input.at(0).p;
        p2=input.at(1).p;

        this->castImageFilter1->SetInput((InputImageType*) p1 );
        this->castImageFilter2->SetInput((InputImageType*) p2 );
        this->diffusion->SetInput(this->castImageFilter1->GetOutput());
        this->cannySegmentation->SetInput(this->castImageFilter2->GetOutput());
        this->cannySegmentation->SetFeatureImage(this->diffusion->GetOutput());
        this->thresholder->SetInput(this->cannySegmentation->GetOutput());

        this->thresholder->Update();

        V3DPluginArgItem arg;
        typename InputImageType::Pointer outputImage = thresholder->GetOutput();
        outputImage->Register();
        arg.p = (void*)outputImage;
        arg.p=thresholder->GetOutput();
        arg.type="outputImage";
        output.replace(0,arg);
    }



private:
    typename CastImageFilterType::Pointer castImageFilter1 ;
    typename CastImageFilterType::Pointer castImageFilter2;
    typename ThresholdingFilterType::Pointer thresholder;
    typename CannySegmentationLevelSetImageFilterType::Pointer cannySegmentation;
    typename DiffusionFilterType::Pointer diffusion ;


};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKCannySegmentationSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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

bool ITKCannySegmentationPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                                        V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    ITKCannySegmentationSpecializaed <unsigned char,unsigned char> runner(&v3d);
    runner.ComputeOneRegion(input, output);

    return true;
}

void ITKCannySegmentationPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK CannySegmentationPlugin1"))
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
        QMessageBox::information(parent, "Version info", "ITK Canny SegmentationPlugin 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
    }
}



