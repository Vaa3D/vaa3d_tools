#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Kmeans_called.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkVectorImageKmeansImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Kmeans, KmeansPlugin)


QStringList KmeansPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Kmeans")
           << QObject::tr("about this plugin");
}

QStringList KmeansPlugin::funclist() const
{
    return QStringList() << QObject::tr("ITK Kmeans")
           << QObject::tr("about this plugin");
}

template <typename TPixelType>
class KmeansSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
public:
    typedef V3DITKFilterSingleImage< TPixelType, TPixelType >    Superclass;
    typedef typename Superclass::Input3DImageType			 ImageType;
    typedef itk::VectorImageKmeansImageFilter<ImageType,ImageType>  FilterType;

    KmeansSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
        this->filter=FilterType::New();
        this->RegisterInternalFilter(this->filter,1.0);
    }
    virtual ~KmeansSpecialized() {};


    void Execute(const QString &menu_name, QWidget *parent)
    {
        V3DITKGenericDialog dialog("Kmeans");
        dialog.AddDialogElement("InitialMean1",0.0,0.0,255.0);
        dialog.AddDialogElement("InitialMean2",100.0,0.0,255.0);
        dialog.AddDialogElement("UseNonContiguouslabels",1.0,0.0,1.0);

        if(dialog.exec()==QDialog::Accepted)
        {
            this->filter->AddClassWithInitialMean(dialog.GetValue("InitialMean1"));
            this->filter->AddClassWithInitialMean(dialog.GetValue("InitialMean2"));
            this->filter->SetUseNonContiguousLabels(dialog.GetValue("UseNonContiguouslabels"));
            this->Compute();
        }
    }

    virtual void ComputeOneRegion()
    {
        /*    typedef TPixelType  PixelType;

            typedef typename Superclass::Input3DImageType   ImageType;

            typedef itk::VectorImageKmeansImageFilter< ImageType, ImageType > FilterType;
            typename FilterType::Pointer filter = FilterType::New();
        */
        filter->SetInput( this->GetInput3DImage() );


        // These values should be provided by the GUI Dialog...
        // filter->AddClassWithInitialMean(   0 );
        //   filter->AddClassWithInitialMean( 100 );

//     filter->SetUseNonContiguousLabels( 1 );

        filter->Update();

        this->SetOutputImage( filter->GetOutput() );
    }

    void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

        V3DITKGenericDialog dialog("Kmeans");
        dialog.AddDialogElement("InitialMean1",0.0,0.0,255.0);
        dialog.AddDialogElement("InitialMean2",100.0,0.0,255.0);
        dialog.AddDialogElement("UseNonContiguouslabels",1.0,0.0,1.0);

        if(dialog.exec()==QDialog::Accepted)
        {
            this->filter->AddClassWithInitialMean(dialog.GetValue("InitialMean1"));
            this->filter->AddClassWithInitialMean(dialog.GetValue("InitialMean2"));
            this->filter->SetUseNonContiguousLabels(dialog.GetValue("UseNonContiguouslabels"));

            V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

            this->AddObserver( progressDialog.GetCommand() );
            progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
            progressDialog.show();
            this->RegisterInternalFilter( this->filter, 1.0 );

            void * p=NULL;
            p=(void*)input.at(0).p;
            if(!p)perror("errro");

            this->filter->SetInput((ImageType*) p );

            this->filter->Update();
            V3DPluginArgItem arg;
            typename ImageType::Pointer outputImage = filter->GetOutput();
            outputImage->Register();
            arg.p = (void*)outputImage;
            arg.type="outputImage";
            output.replace(0,arg);
        }
    }




    virtual void SetupParameters()
    {
    }
private:
    typename  FilterType::Pointer	filter;
};


#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    KmeansSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }

#define EXECUTE_ALL_PIXEL_TYPES \
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

bool KmeansPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                          V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
        return false ;
    }
    KmeansSpecialized<unsigned char> runner(&v3d);
    runner.ComputeOneRegion(input, output);

    return true;
}

void KmeansPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info", "ITK Kmeasn 1.0 (2010-June-3): this plugin is developed by Luis Ibanez.");
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

    EXECUTE_ALL_PIXEL_TYPES;
}
