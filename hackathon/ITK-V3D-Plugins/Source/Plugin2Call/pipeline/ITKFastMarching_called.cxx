#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKFastMarching_called.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkFastMarchingImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkCastImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(FastMarching, FastMarchingPlugin)


QStringList FastMarchingPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK FastMarching")
           << QObject::tr("about this plugin");
}

QStringList FastMarchingPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK FastMarching")
           << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, float >
{
    typedef V3DITKFilterSingleImage< TPixelType, float >        Superclass;
    typedef typename Superclass::Input3DImageType               InputImageType;
    typedef typename Superclass::Output3DImageType              OutputImageType;
    typedef OutputImageType                                     LevelSetImageType;
    typedef InputImageType                                      SpeedImageType;

    typedef itk::CastImageFilter<InputImageType,OutputImageType>			CastInputType;
    typedef itk::CastImageFilter<OutputImageType,InputImageType>			CastOutputType;
    typedef itk::FastMarchingImageFilter< LevelSetImageType, SpeedImageType > FilterType;
    typedef itk::ThresholdImageFilter< LevelSetImageType > ThresholdFilterType;
    typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FloatFilterType;

public:

    PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
        this->m_Filter = FilterType::New();
        this->m_ThresholdFilter = ThresholdFilterType::New();
        this->m_Filter1=FloatFilterType::New();
        this->castInputFilter=CastInputType::New();
        this->castOutputFilter=CastOutputType::New();
        this->RegisterInternalFilter( this->m_Filter, 0.9 );
        this->RegisterInternalFilter( this->m_ThresholdFilter, 0.1 );
    }

    virtual ~PluginSpecialized() {};


    void Execute(const QString &menu_name, QWidget *parent)
    {
        V3DITKGenericDialog dialog("FastMarching");

        dialog.AddDialogElement("SpeedScale",255.0, 0.0, 256.0);
        dialog.AddDialogElement("StoppingTime",50.0, 0.0, 10000.0);

        const double stoppingTime = dialog.GetValue("StoppingTime");

        const double seedValue = 0.0;

        if( dialog.exec() != QDialog::Accepted )
        {
            return;
        }

        this->m_Filter->SetStoppingValue( stoppingTime );
        this->m_Filter->SetNormalizationFactor( dialog.GetValue("SpeedScale") );

        this->m_ThresholdFilter->ThresholdAbove( stoppingTime );
        this->m_ThresholdFilter->SetOutsideValue( stoppingTime );

        typedef typename FilterType::NodeContainer  NodeContainer;
        typedef typename FilterType::NodeType       NodeType;

        typename NodeContainer::Pointer seedPoints = NodeContainer::New();

        NodeType node;

        seedPoints->Initialize();

        v3dhandleList windowList = this->m_V3DPluginCallback->getImageWindowList();

        LandmarkList listOfLandmarks = this->m_V3DPluginCallback->getLandmark( windowList[0] ); // FIXME

        unsigned int numberOfSeedPoints = listOfLandmarks.size();

        if ( ! numberOfSeedPoints )
        {
            v3d_msg(QObject::tr("You should select one seed from your image."));
            return;
        }

        typename OutputImageType::IndexType  seedPosition;

        for(unsigned int i = 0;  i < numberOfSeedPoints; i++ )
        {
            seedPosition[0] = listOfLandmarks[i].x - 1; // notice 0-based and 1-based difference
            seedPosition[1] = listOfLandmarks[i].y - 1;
            seedPosition[2] = listOfLandmarks[i].z - 1;

            node.SetValue( seedValue );
            node.SetIndex( seedPosition );

            seedPoints->InsertElement( i, node );
        }

        this->m_Filter->SetTrialPoints( seedPoints );

        this->Compute();
    }

    virtual void ComputeOneRegion()
    {

        this->m_Filter->SetInput( this->GetInput3DImage() );
        this->m_ThresholdFilter->SetInput( m_Filter->GetOutput() );

        this->m_ThresholdFilter->Update();

        this->SetOutputImage( this->m_ThresholdFilter->GetOutput() );
    }

    void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

        V3DITKGenericDialog dialog("FastMarching");

        dialog.AddDialogElement("SpeedScale",255.0, 0.0, 256.0);
        dialog.AddDialogElement("StoppingTime",50.0, 0.0, 10000.0);

        const double stoppingTime = dialog.GetValue("StoppingTime");

        const double seedValue = 0.0;

        if( dialog.exec() != QDialog::Accepted )
        {
            return;
        }
        this->m_Filter1->SetStoppingValue( stoppingTime );
        this->m_Filter1->SetNormalizationFactor( dialog.GetValue("SpeedScale") );

        typedef typename FloatFilterType::NodeContainer  NodeContainer;
        typedef typename FloatFilterType::NodeType       NodeType;

        typename NodeContainer::Pointer seedPoints = NodeContainer::New();

        NodeType node;

        seedPoints->Initialize();

        v3dhandleList windowList = this->m_V3DPluginCallback->getImageWindowList();

        LandmarkList listOfLandmarks = this->m_V3DPluginCallback->getLandmark( windowList[0] ); // FIXME

        unsigned int numberOfSeedPoints = listOfLandmarks.size();

        if ( ! numberOfSeedPoints )
        {
            v3d_msg(QObject::tr("You should select one seed from your image."));
            return;
        }

        typename OutputImageType::IndexType  seedPosition;

        for(unsigned int i = 0;  i < numberOfSeedPoints; i++ )
        {
            seedPosition[0] = listOfLandmarks[i].x - 1; // notice 0-based and 1-based difference
            seedPosition[1] = listOfLandmarks[i].y - 1;
            seedPosition[2] = listOfLandmarks[i].z - 1;

            node.SetValue( seedValue );
            node.SetIndex( seedPosition );

            seedPoints->InsertElement( i, node );
        }

        this->m_Filter1->SetTrialPoints( seedPoints );


        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter( this->m_Filter1, 1.0 );

        void * p=NULL;
        p=(void*)input.at(0).p;
        if(!p)perror("errro");

        this->castInputFilter->SetInput((InputImageType*) p);
        this->m_Filter1->SetOutputSize(castInputFilter->GetOutput()->GetBufferedRegion().GetSize());
        this->m_Filter1->SetInput(castInputFilter->GetOutput());
        this->castOutputFilter->SetInput(m_Filter1->GetOutput());

        this->castOutputFilter->Update();
        V3DPluginArgItem arg;
        typename InputImageType::Pointer outputImage = this->castOutputFilter->GetOutput();
        outputImage->Register();
        arg.p = (void*)outputImage;
        arg.type="UINT8Image";
        output.replace(0,arg);
    }



private:
    typename CastInputType::Pointer	     castInputFilter;
    typename CastOutputType::Pointer	     castOutputFilter;
    typename FilterType::Pointer            m_Filter;
    typename ThresholdFilterType::Pointer   m_ThresholdFilter;
    typename FloatFilterType::Pointer	     m_Filter1;
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool FastMarchingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                                V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    PluginSpecialized<unsigned char> runner(&v3d);
    runner.ComputeOneRegion(input, output);
    return true;

}


void FastMarchingPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK FastMarching 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

    EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES;
}

