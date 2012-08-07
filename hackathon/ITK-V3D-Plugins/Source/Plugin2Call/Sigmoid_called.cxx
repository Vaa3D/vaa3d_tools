#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Sigmoid_called.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkSigmoidImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Sigmoid, SigmoidPlugin)


QStringList SigmoidPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Sigmoid")
           << QObject::tr("about this plugin");
}

QStringList SigmoidPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK Sigmoid")
           << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
    typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
    typedef typename Superclass::Input3DImageType               ImageType;

    typedef itk::SigmoidImageFilter< ImageType, ImageType > FilterType;

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

        dialog.AddDialogElement("Alpha",1.0, -5.0, 5.0);
        dialog.AddDialogElement("Beta",10.0, 0.0, 255.0);
        dialog.AddDialogElement("Minimum",0.0, 0.0, 255.0);
        dialog.AddDialogElement("Maximum",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetAlpha( dialog.GetValue("Alpha") );
            this->m_Filter->SetBeta( dialog.GetValue("Beta") );
            this->m_Filter->SetOutputMinimum( dialog.GetValue("Minimum") );
            this->m_Filter->SetOutputMaximum( dialog.GetValue("Maximum") );

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

        dialog.AddDialogElement("Alpha",1.0, -5.0, 5.0);
        dialog.AddDialogElement("Beta",10.0, 0.0, 255.0);
        dialog.AddDialogElement("Minimum",0.0, 0.0, 255.0);
        dialog.AddDialogElement("Maximum",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetAlpha( dialog.GetValue("Alpha") );
            this->m_Filter->SetBeta( dialog.GetValue("Beta") );
            this->m_Filter->SetOutputMinimum( dialog.GetValue("Minimum") );
            this->m_Filter->SetOutputMaximum( dialog.GetValue("Maximum") );


            V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

            this->AddObserver( progressDialog.GetCommand() );
            progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
            progressDialog.show();
            this->RegisterInternalFilter( this->m_Filter, 1.0 );

            void * p=NULL;
            p=(void*)input.at(0).p;
            if(!p)perror("erro");

            this->m_Filter->SetInput((ImageType*) p );

            this->m_Filter->Update();
            V3DPluginArgItem arg;
            typename ImageType::Pointer outputImage = m_Filter->GetOutput();
            outputImage->Register();
            arg.p = (void*)outputImage;
            if( input.at(0).type=="UINT8Image" )
            {
                arg.type="UINT8Image";
            }
            else
            {
                arg.type="floatImage";
            }
            output.replace(0,arg);
            return true;
        }
        return false;
    }



private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool SigmoidPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                           V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    if(input.at(0).type=="UINT8Image")
    {
        PluginSpecialized<unsigned char> runner(&v3d);
        bool result = runner.ComputeOneRegion(input, output);
        return result;
    }
    else
    {
        printf("use float\n");
        PluginSpecialized<float> *runner=new PluginSpecialized<float>(&v3d);
        bool result = runner->ComputeOneRegion(input, output);
        return result;
    }
}


void SigmoidPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK Sigmoid 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

