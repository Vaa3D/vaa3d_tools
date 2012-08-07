/* ITKGeodesicActiveContour.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKGeodesicActiveContour_called.h"
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
#include "itkGeodesicActiveContourLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKGeodesicActiveContour, ITKGeodesicActiveContourPlugin)

void itkGeodesicActiveContourPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK GeodesicActiveContour";
QStringList ITKGeodesicActiveContourPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK GeodesicActiveContour")
           << QObject::tr("about this plugin");
}
QStringList ITKGeodesicActiveContourPlugin::funclist() const
{
    return QStringList() << QObject::tr("ITK GeodesicActiveContour")
           << QObject::tr("about this plugin");
}


void ITKGeodesicActiveContourPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK GeodesicActiveContour"))
    {
        itkGeodesicActiveContourPlugin(callback, parent);
    }
    else if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK Shape Detection 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
    }
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKGeodesicActiveContourSpecializaed: public V3DITKFilterSingleImage<TInputPixelType, TOutputPixelType >
{
public:
    typedef V3DITKFilterSingleImage<TInputPixelType, TOutputPixelType > Superclass;
    typedef typename Superclass::Input3DImageType 	InputImageType;
    typedef typename Superclass::Output3DImageType 	OutputImageType;


    typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
    typedef itk::CastImageFilter< OutputImageType,InputImageType>  CastOutputType;
    typedef itk::RescaleIntensityImageFilter< InputImageType,OutputImageType> CastInputFilterType;


//	typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType    >    ThresholdingFilterType;

    typedef itk::CurvatureAnisotropicDiffusionImageFilter< OutputImageType, OutputImageType >  SmoothingFilterType;

    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< OutputImageType, OutputImageType >  GradientFilterType;
    typedef itk::SigmoidImageFilter< OutputImageType, OutputImageType >  SigmoidFilterType;
    typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingFilterType;
    typedef  itk::GeodesicActiveContourLevelSetImageFilter< OutputImageType, OutputImageType > GeodesicActiveContourFilterType;


    ITKGeodesicActiveContourSpecializaed(V3DPluginCallback* callback):Superclass(callback)
    {
        castImageFilter = CastImageFilterType::New();
        smoothing = SmoothingFilterType::New();
        fastMarching=FastMarchingFilterType::New();
        geodesicActiveContour = GeodesicActiveContourFilterType::New();
        gradientMagnitude=GradientFilterType::New();
        sigmoid=SigmoidFilterType::New();
        castInputFilter=CastInputFilterType::New();
        castOutputFilter=CastOutputType::New();
        this->RegisterInternalFilter(this->geodesicActiveContour,0.6);
        this->RegisterInternalFilter(this->sigmoid,0.1);
        this->RegisterInternalFilter(this->fastMarching,0.3);
    }

    void Execute(V3DPluginCallback &callback, QWidget *parent)
    {
        //
        ITKGeodesicActiveContourDialog d(callback, parent);

        //
        if (d.exec()!=QDialog::Accepted)
        {
            return;
        }
        else
        {
            //passing \pars
            d.update();

            int i1 = d.i1;

            //ori image
            v3dhandleList win_list = callback.getImageWindowList();

            V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
            Image4DSimple *p4DImage = callback.getImage(win_list[i1]);

            //init
            typedef TInputPixelType  PixelType;

            long nx = p4DImage->getXDim();
            long ny = p4DImage->getYDim();
            long nz = p4DImage->getZDim();
            long nc = p4DImage->getCDim();  // Number of channels

            int channelToFilter = globalSetting.iChannel_for_plugin;

            if( channelToFilter >= nc )
            {
                v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
                return;
            }

            sigmoid->SetOutputMinimum(  0.0  );
            sigmoid->SetOutputMaximum(  1.0  );
            typedef typename FastMarchingFilterType::NodeContainer	NodeContainer;
            typedef typename FastMarchingFilterType::NodeType		NodeType;
            typename NodeContainer::Pointer seeds = NodeContainer::New();

            typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

            NodeType node;

            //set \pars
            const double initialDistance = 15.0; //
            const double seedValue = - initialDistance;

            LandmarkList list_landmark_sub=callback.getLandmark(win_list[i1]);
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

            const double stoppingTime = sqrt(nx*nx + ny*ny + nz*nz);

            const double sigma = 0.5; // GradientMagnitudeRecursiveGaussianImageFilter

            const double alpha =  -1; // SigmoidImageFilter
            const double beta  =  20;

            const double curvatureScaling   = 1.0; // Level Set
            const double propagationScaling = 1.0;

            gradientMagnitude->SetSigma(  sigma  );

            smoothing->SetTimeStep( 0.05 ); // less than 0.625 for 3D less than 0.125 for 2D
            smoothing->SetNumberOfIterations(  5 );
            smoothing->SetConductanceParameter( 3.0 );

            sigmoid->SetOutputMinimum(  0.0  );
            sigmoid->SetOutputMaximum(  1.0  );

            sigmoid->SetAlpha( alpha );
            sigmoid->SetBeta(  beta  );

            fastMarching->SetTrialPoints(  seeds  );
            fastMarching->SetSpeedConstant( 1.0 );

            geodesicActiveContour->SetPropagationScaling( propagationScaling );
            geodesicActiveContour->SetCurvatureScaling( curvatureScaling );
            geodesicActiveContour->SetAdvectionScaling( 1.0 );

            geodesicActiveContour->SetMaximumRMSError( 0.02 );
            geodesicActiveContour->SetNumberOfIterations( stoppingTime ); //
            this->Compute();
        }
    }
    virtual void ComputeOneRegion()
    {
        fastMarching->SetOutputSize(this->GetInput3DImage()->GetBufferedRegion().GetSize() );
        castImageFilter->SetInput( this->GetInput3DImage() );
        smoothing->SetInput( castImageFilter->GetOutput() );
        gradientMagnitude->SetInput( smoothing->GetOutput() );
        sigmoid->SetInput( gradientMagnitude->GetOutput() );

        geodesicActiveContour->SetInput( fastMarching->GetOutput() );
        geodesicActiveContour->SetFeatureImage( sigmoid->GetOutput() );
        geodesicActiveContour->Update();
        this->SetOutputImage(this->geodesicActiveContour->GetOutput());

        std::cout << std::endl;
        std::cout << "Max. no. iterations: " << geodesicActiveContour->GetNumberOfIterations() << std::endl;
        std::cout << "Max. RMS error: " << geodesicActiveContour->GetMaximumRMSError() << std::endl;
        std::cout << std::endl;
        std::cout << "No. elpased iterations: " << geodesicActiveContour->GetElapsedIterations() << std::endl;
        std::cout << "RMS change: " << geodesicActiveContour->GetRMSChange() << std::endl;
    }

   bool ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

        typedef typename FastMarchingFilterType::NodeContainer  NodeContainer;
        typedef typename FastMarchingFilterType::NodeType    NodeType;
        typename NodeContainer::Pointer seeds = NodeContainer::New();

        typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]

        NodeType node;
        //get the parameter
        V3DITKGenericDialog dialog("ITKShapeDetection");

        dialog.AddDialogElement("initialDistance",5.0, 0.0, 255.0);
        dialog.AddDialogElement("propagationScaling",2.0, 0.0, 255.0);

        if( dialog.exec() != QDialog::Accepted )
        {
            return false;
        }

        const double initialDistance = dialog.GetValue("initialDistance"); //
        const double propagationScaling = dialog.GetValue("propagationScaling");
        const double seedValue = - initialDistance;

        seeds->Initialize();

        v3dhandleList windowList = this->m_V3DPluginCallback->getImageWindowList();

        LandmarkList listOfLandmarks = this->m_V3DPluginCallback->getLandmark( windowList[0] ); // FIXME

        unsigned int numberOfSeedPoints = listOfLandmarks.size();

        if ( ! numberOfSeedPoints )
        {
            v3d_msg(QObject::tr("You should select one seed from your image."));
            return false;
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
        const double curvatureScaling   = 1.0; // Level Set

        geodesicActiveContour->SetPropagationScaling( propagationScaling );
        geodesicActiveContour->SetCurvatureScaling( curvatureScaling );
        geodesicActiveContour->SetAdvectionScaling( 1.0 );

        geodesicActiveContour->SetMaximumRMSError( 0.02 );
        geodesicActiveContour->SetNumberOfIterations(800); //
        castInputFilter->SetOutputMinimum(0.0001);
        castInputFilter->SetOutputMaximum(1.0);


        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter( this->geodesicActiveContour, 1.0 );

        void * p=NULL;
        p=(void*)input.at(0).p;
        if(!p)perror("errro");

        this->castInputFilter->SetInput((InputImageType*) p);
        this->castInputFilter->Update();
        this->fastMarching->SetOutputSize(castInputFilter->GetOutput()->GetBufferedRegion().GetSize());
        geodesicActiveContour->SetInput( fastMarching->GetOutput() );
        geodesicActiveContour->SetFeatureImage(this->castInputFilter->GetOutput());
        this->castOutputFilter->SetInput(geodesicActiveContour->GetOutput());


        this->castOutputFilter->Update();
        V3DPluginArgItem arg;
        typename InputImageType::Pointer outputImage = this->castOutputFilter->GetOutput();
        outputImage->Register();
        arg.p = (void*)outputImage;
        arg.type="UINT8Image";
        output.replace(0,arg);
        return true;
    }



private:
    typename CastImageFilterType::Pointer castImageFilter;
    typename SmoothingFilterType::Pointer smoothing ;
    typename FastMarchingFilterType::Pointer fastMarching;
    typename GeodesicActiveContourFilterType::Pointer geodesicActiveContour ;
    typename GradientFilterType::Pointer gradientMagnitude;
    typename SigmoidFilterType::Pointer sigmoid;
    typename CastOutputType::Pointer	castOutputFilter;
    typename CastInputFilterType::Pointer  castInputFilter;
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKGeodesicActiveContourSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
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

bool ITKGeodesicActiveContourPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
        V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    ITKGeodesicActiveContourSpecializaed<unsigned char,float> runner(&v3d);
    bool result = runner.ComputeOneRegion(input, output);
    return result;

}
void itkGeodesicActiveContourPlugin(V3DPluginCallback &callback, QWidget *parent)
{
    Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
    if (!p4DImage)
    {
        v3d_msg(QObject::tr("You don't have any image open in the main window."));
        return;
    }

    EXECUTE_ALL_PIXEL_TYPES;
}


