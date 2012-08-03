#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "BinaryThreshold.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkBinaryThresholdImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BinaryThreshold, BinaryThresholdPlugin)


QStringList BinaryThresholdPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK BinaryThreshold")
           << QObject::tr("about this plugin");
}

QStringList BinaryThresholdPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK BinaryThreshold")
           << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
    typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
    typedef typename Superclass::Input3DImageType               ImageType;

    typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > FilterType;

public:

    PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
        this->m_Filter = FilterType::New();
        this->RegisterInternalFilter(this->m_Filter,1.0);
    }

    virtual ~PluginSpecialized() {};


    void Execute(const QString &menu_name, QWidget *parent)
    {
        V3DITKGenericDialog dialog("Binary Threshold");

        dialog.AddDialogElement("Lower",128.0, 0.0, 255.0);
        dialog.AddDialogElement("Upper",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetUpperThreshold( dialog.GetValue("Upper") );
            this->m_Filter->SetLowerThreshold( dialog.GetValue("Lower") );
            this->m_Filter->SetInsideValue(255);
            this->m_Filter->SetOutsideValue(0);

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
    void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

        V3DITKGenericDialog dialog("Binary Threshold");

        dialog.AddDialogElement("Lower",128.0, 0.0, 255.0);
        dialog.AddDialogElement("Upper",255.0, 0.0, 255.0);

        if( dialog.exec() == QDialog::Accepted )
        {
            this->m_Filter->SetUpperThreshold( dialog.GetValue("Upper") );
            this->m_Filter->SetLowerThreshold( dialog.GetValue("Lower") );
            this->m_Filter->SetInsideValue(255);
            this->m_Filter->SetOutsideValue(0);

            V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

            this->AddObserver( progressDialog.GetCommand() );
            progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
            progressDialog.show();
            this->RegisterInternalFilter( this->m_Filter, 1.0 );

            void * p=NULL;
            p=(void*)input.at(0).p;
            if(!p)perror("errro");

            this->m_Filter->SetInput((ImageType*) p );

            this->m_Filter->Update();
            V3DPluginArgItem arg;
            arg.p=m_Filter->GetOutput();
            if( input.at(0).type=="UINT8Image" )
            {
                arg.type="UINT8Image";
            }
            else
            {
                arg.type="floatImage";
            }
            output.replace(0,arg);
        }
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


bool BinaryThresholdPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                                   V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    if(input.at(0).type=="UINT8Image")
    {
        PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
        runner->ComputeOneRegion(input, output);
        return true;
    }
    else
    {
        PluginSpecialized<float> *runner=new PluginSpecialized<float>(&v3d);
        runner->ComputeOneRegion(input, output);
        return true;
    }
}


void BinaryThresholdPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK BinaryThreshold 1.0 (2010-Jun-21): this plugin is developed by Sophie Chen.");
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

