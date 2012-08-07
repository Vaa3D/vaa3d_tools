/* DiscreteGaussianFilter.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DiscreteGaussianFilter_called.h"
#include "V3DITKFilterSingleImage.h"
// ITK Header Files
#include "itkRescaleIntensityImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DiscreteGaussianFilter, ITKDiscreteGaussianFilterPlugin)

QStringList ITKDiscreteGaussianFilterPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK DiscreteGaussian Filter ...");
}
QStringList ITKDiscreteGaussianFilterPlugin::funclist() const
{
    return QStringList() << QObject::tr("ITK DiscreteGaussian Filter ...");
}

template<typename TPixelType>
class ITKDiscreteGaussianFilterSpecializaed:public V3DITKFilterSingleImage<TPixelType,TPixelType>
{
public:
    typedef V3DITKFilterSingleImage<TPixelType,TPixelType>	Superclass;
    typedef typename Superclass::Input3DImageType		ImageType;
    typedef itk::Image< float, 3 >   			ImageType_mid;
    typedef itk::RescaleIntensityImageFilter<ImageType, ImageType_mid > RescaleFilterType_input;
    typedef itk::DiscreteGaussianImageFilter<ImageType_mid,ImageType_mid> FilterType;
    typedef itk::RescaleIntensityImageFilter<ImageType_mid,ImageType> RescaleFilterType_output;


    ITKDiscreteGaussianFilterSpecializaed(V3DPluginCallback *callback):Superclass(callback)
    {
        rescaler_8u_32f = RescaleFilterType_input::New();
        filter = FilterType::New();
        rescaler_32f_8u = RescaleFilterType_output::New();
        this->RegisterInternalFilter(this->filter,0.8);
        this->RegisterInternalFilter(this->rescaler_8u_32f,0.1);
        this->RegisterInternalFilter(this->rescaler_32f_8u,0.1);
    }



    void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
    {


        rescaler_8u_32f->SetOutputMinimum(   0 );
        rescaler_8u_32f->SetOutputMaximum( 255 );

        float gaussianVariance=1.0;
        unsigned int maxKernelWidth=3;
        filter->SetVariance( gaussianVariance );
        filter->SetMaximumKernelWidth( maxKernelWidth );

        rescaler_32f_8u->SetOutputMinimum(   0 );
        rescaler_32f_8u->SetOutputMaximum( 255 );
        this->Compute();


    }
    virtual void ComputeOneRegion()
    {
        rescaler_8u_32f->SetInput(this->GetInput3DImage());
        filter->SetInput(rescaler_8u_32f->GetOutput());
        rescaler_32f_8u->SetInput(filter->GetOutput());
        rescaler_32f_8u->Update();
        this->SetOutputImage(this->rescaler_32f_8u->GetOutput());
    }

    void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter(this->filter,0.8);
        this->RegisterInternalFilter(this->rescaler_8u_32f,0.1);
        this->RegisterInternalFilter(this->rescaler_32f_8u,0.1);

        void * p=NULL;
        p=(void*)input.at(0).p;
        if(!p)perror("errro");

        rescaler_8u_32f->SetInput((ImageType*) p );
        filter->SetInput(rescaler_8u_32f->GetOutput());
        rescaler_32f_8u->SetInput(filter->GetOutput());
        rescaler_32f_8u->Update();

        V3DPluginArgItem arg;
        typename ImageType::Pointer outputImage = rescaler_32f_8u->GetOutput();
        outputImage->Register();
        arg.p = (void*)outputImage;
        arg.type="outputImage";
        output.replace(0,arg);
    }
private:
    typename RescaleFilterType_input::Pointer rescaler_8u_32f ;
    typename FilterType::Pointer filter;
    typename RescaleFilterType_output::Pointer rescaler_32f_8u;


};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKDiscreteGaussianFilterSpecializaed< c_pixel_type > runner(&callback); \
    runner.Execute(  menu_name, callback, parent ); \
    break; \
    }

#define EXECUTE_ALL_PIXEL_TYPES \
	Image4DSimple *p4DImage = callback.getImage(curwin); \
    if (! p4DImage) return; \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE( V3D_UINT8, unsigned char );  \
      EXECUTE( V3D_UINT16, unsigned short int );  \
      EXECUTE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
    {  \
    }  \
      }
bool ITKDiscreteGaussianFilterPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
        V3DPluginCallback2 & v3d,  QWidget * parent)
{
    ITKDiscreteGaussianFilterSpecializaed<unsigned char> runner(&v3d);
    runner.ComputeOneRegion(input, output);

    return true;
}


void ITKDiscreteGaussianFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg(tr("You don't have any image open in the main window."));
        return;
    }

    EXECUTE_ALL_PIXEL_TYPES;
}
