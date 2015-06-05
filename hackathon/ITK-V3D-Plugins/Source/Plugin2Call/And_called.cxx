#include "And_called.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkAndImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(And, AndPlugin)


QStringList AndPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK And")
           << QObject::tr("about this plugin");
}

QStringList AndPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK And")
           << QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
    typedef V3DITKFilterDualImage< TPixelType, TPixelType >   Superclass;
    typedef typename Superclass::Input3DImageType               ImageType;

    typedef itk::AndImageFilter< ImageType, ImageType > FilterType;

public:

    PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
        this->m_Filter = FilterType::New();
        this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

    virtual ~PluginSpecialized() {};


    void Execute(const QString &menu_name, QWidget *parent)
    {
        this->SetImageSelectionDialogTitle("And Two Images");
        this->AddImageSelectionLabel("Image 1");
        this->AddImageSelectionLabel("Image 2");
        this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
        this->Compute();
    }

    virtual void ComputeOneRegion()
    {

        this->m_Filter->SetInput1( this->GetInput3DImage1() );
        this->m_Filter->SetInput2( this->GetInput3DImage2() );

        this->m_Filter->Update();

        this->SetOutputImage( this->m_Filter->GetOutput() );
    }
    bool ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
        V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

        this->AddObserver( progressDialog.GetCommand() );
        progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
        progressDialog.show();
        this->RegisterInternalFilter( this->m_Filter, 1.0 );

        void * p1=NULL;
        void * p2=NULL;
        p1=input.at(0).p;
        p2=input.at(1).p;

        this->m_Filter->SetInput1((ImageType*) p1 );
        this->m_Filter->SetInput2((ImageType*) p2 );


        this->m_Filter->Update();
        V3DPluginArgItem arg;
        arg.p=m_Filter->GetOutput();
        arg.type="outputImage";
        output.replace(0,arg);
        return true;
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


bool AndPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                       V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    if (input.size() < 2) {
      qDebug() << "need two image!";
      return false;
    }
    PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
    return (runner->ComputeOneRegion(input, output) );
}


void AndPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK And 1.0 (2010-Jul-31): this plugin is developed by Luis Ibanez.");
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

    EXECUTE_PLUGIN_FOR_INTEGER_PIXEL_TYPES;
}

