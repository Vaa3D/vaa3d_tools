#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "RescaleIntensity_called.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkRescaleIntensityImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(RescaleIntensity, RescaleIntensityPlugin)


QStringList RescaleIntensityPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK RescaleIntensity")
           << QObject::tr("about this plugin");
}

QStringList RescaleIntensityPlugin::funclist() const
{
    return QStringList() << QObject::tr("ITK RescaleIntensity")
           << QObject::tr("about this plugin");
}


template <typename TInputPixelType, typename TOutputPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
    typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >   Superclass;
    //typedef itk::Image< TInputPixelType, 3 >           InputImageType;
    //typedef itk::Image< TOutputPixelType,3 >          OutputImageType;
    typedef typename Superclass::Input3DImageType               InputImageType;
    typedef typename Superclass::Output3DImageType              OutputImageType;
    typedef itk::RescaleIntensityImageFilter< InputImageType, OutputImageType > FilterType;

public:

    PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
        this->m_Filter = FilterType::New();
        this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

    virtual ~PluginSpecialized() {};


    void Execute(const QString &menu_name, QWidget *parent)
    {
        V3DITKGenericDialog dialog("Sigmoid");

        dialog.AddDialogElement("Output Minimum",1.0, 0.0, 255.0);
        dialog.AddDialogElement("Output Maximum",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetOutputMinimum( dialog.GetValue("Output Minimum") );
            this->m_Filter->SetOutputMaximum( dialog.GetValue("Output Maximum") );

            this->Compute();
        }
    }

    virtual void ComputeOneRegion()
    {

        this->m_Filter->SetInput( this->GetInput3DImage() );

        if( !this->ShouldGenerateNewWindow() )
        {
            this->m_Filter->InPlaceOn();
        }

        this->m_Filter->Update();

        this->SetOutputImage( this->m_Filter->GetOutput() );
    }
    bool ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
        V3DITKGenericDialog dialog("Sigmoid");

        dialog.AddDialogElement("Output Minimum",1.0, 0.0, 255.0);
        dialog.AddDialogElement("Output Maximum",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetOutputMinimum( dialog.GetValue("Output Minimum") );
            this->m_Filter->SetOutputMaximum( dialog.GetValue("Output Maximum") );

            V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

            this->AddObserver( progressDialog.GetCommand() );
            progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
            progressDialog.show();
            this->RegisterInternalFilter( this->m_Filter, 1.0 );

            void * p=NULL;
            p=(void*)input.at(0).p;
            if(!p)perror("errro");

            this->m_Filter->SetInput((InputImageType*) p );

            this->m_Filter->Update();
            V3DPluginArgItem arg;
            typename InputImageType::Pointer outputImage = m_Filter->GetOutput();
            outputImage->Register();
            arg.p = (void*)outputImage;
            arg.type="UIN8Image";
            output.replace(0,arg);
            return true;
        }
        return false;
    }

private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( v3d_input_pixel_type, v3d_output_pixel_type, c_input_pixel_type, c_output_pixel_type ) \
  case v3d_output_pixel_type: \
    { \
    PluginSpecialized< c_input_pixel_type, c_output_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool RescaleIntensityPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                                    V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    PluginSpecialized <unsigned char,unsigned char> runner(&v3d);
    bool result = runner.ComputeOneRegion(input, output);

    return result;
}


void RescaleIntensityPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK RescaleIntensity 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

    EXECUTE_PLUGIN_FOR_ALL_INPUT_AND_OUTPUT_PIXEL_TYPES;
}

