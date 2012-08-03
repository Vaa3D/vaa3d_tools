#ifndef _SUPERPLUGIN_H_
#define _SUPERPLUGIN_H_

#include <QtGui>
#include <iostream>
#include <v3d_interface.h>

#include "V3DITKFilterSingleImage.h"
#include "V3DITKFilterDualImage.h"


class Superplugin:public QObject,public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)
public:
    Superplugin() {}
    QStringList menulist()const;
    QStringList funclist()const;
    void domenu(const QString &menu_name,V3DPluginCallback &callback,QWidget* parent);
    virtual void dofunc(const QString & func_name,const V3DPluginArgList &input,V3DPluginArgList & output,QWidget *parent);
};


template <typename TPixelType>
class PluginSpecialized:public V3DITKFilterSingleImage< TPixelType,	TPixelType>
{
    typedef V3DITKFilterSingleImage<TPixelType,TPixelType>	Superclass;
    typedef typename Superclass::Input3DImageType			ImageType;
    typedef typename Superclass::Output3DImageType			OutputImageType;
    typedef typename itk::Image<float,3>					FImageType;

    const QString rootDir = QDir::fromNativeSeparators(QString("%1/plugins/ITK/Superplugin/Plugin2Call/").arg(QDir::currentPath()));

public:
    PluginSpecialized(V3DPluginCallback* callback):Superclass(callback)
    {
        plugin_name="ITK/Superplugin/NOT/NOT.so";
        function_name="doFunction";
        usePipeline=false;
    }

    void SetPluginName(const QString &plugin_name)
    {
        this->plugin_name = rootDir + plugin_name;
    }
    void AddPluginName(const QString &plugin_name)
    {
        this->plugin_name_list << rootDir + plugin_name;
    }
    void SetUsePipeline(bool b)
    {
        this->usePipeline=b;
    }
    void Execute(const QString &menu_name,QWidget *parent)
    {
        this->Compute();
    }
    void Compute()
    {
        this->Initialize();

        const V3DLONG x1 = 0;
        const V3DLONG y1 = 0;
        const V3DLONG z1 = 0;

        const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
        const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
        const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

        QList< V3D_Image3DBasic > inputImageList =
            getChannelDataForProcessingFromGlobalSetting( this-> m_4DImage, *(this->m_V3DPluginCallback) );

        QList< V3D_Image3DBasic > outputImageList;
        const unsigned int numberOfChannelsToProcess = inputImageList.size();
        if (numberOfChannelsToProcess<=0)
        {
            return;
        }

        for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
        {
            const V3D_Image3DBasic inputImage = inputImageList.at(channel);

            this->TransferInput( inputImage, x1, x2, y1, y2, z1, z2 );
            if(usePipeline)this->ComputePipeline();
            else this->ComputeOneRegion();

            this->AddOutputImageChannel( channel );
        }

        this->ComposeOutputImage();
    }


    virtual void ComputeOneRegion()
    {
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg.p=(void*)this->GetInput3DImage();
        arg.type="UINT8Image";
        input<<arg;
        output<<arg;
        this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
        printf("Get out\n");
        fprintf(stdout,"size %d\n",output.size());
        //input.replace(0,output.at(0));
        //this->m_V3DPluginCallback->callPluginFunc(QString("ITK/Superplugin/NOT/NOT.so"),function_name,input,output);

        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.so";
            input.replace(0,output.at(0));
            this->m_V3DPluginCallback->callPluginFunc(pluginname,function_name,input,output);
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);

        }

        else
        {
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);
        }
    }
    void ComputePipeline()
    {
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg.p=(void*)this->GetInput3DImage();
        arg.type="UINT8Image";
        input<<arg;
        output<<arg;
        for(int i=0; i<plugin_name_list.size(); i++)
        {

            this->m_V3DPluginCallback->callPluginFunc(plugin_name_list.at(i),function_name,input,output);
            input.replace(0,output.at(0));
        }
        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.so";
            input.replace(0,output.at(0));
            this->m_V3DPluginCallback->callPluginFunc(pluginname,function_name,input,output);
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);

        }

        else
        {
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);
        }
    }

private:
    QString plugin_name;
    QString function_name;
    QList< QString> plugin_name_list;
    bool usePipeline;
};

template <typename TPixelType>
class PluginSpecializedForDual:public V3DITKFilterDualImage< TPixelType,TPixelType>
{
    typedef V3DITKFilterDualImage< TPixelType,TPixelType>	Superclass;
    typedef typename Superclass::Input3DImageType			ImageType;
    typedef typename Superclass::Output3DImageType			OutputImageType;
    
    const QString rootDir = QDir::fromNativeSeparators(QString("%1/plugins/ITK/Superplugin/Plugin2Call/").arg(QDir::currentPath()));
public:
    PluginSpecializedForDual(V3DPluginCallback* callback):Superclass(callback)
    {
        plugin_name="ITK/Superplugin/NOT/NOT.so";
        function_name="doFunction";
    }

    void SetPluginName(const QString &plugin_name)
    {
        this->plugin_name= rootDir + plugin_name;
    }
    void Execute(const QString &menu_name,QWidget *parent)
    {
        this->SetImageSelectionDialogTitle("Input Images");
        this->AddImageSelectionLabel("Image 1");
        this->AddImageSelectionLabel("Image 2");
        this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
        this->Compute();
    }
    void Compute()
    {

        this->Initialize();
        const V3DLONG x1 = 0;
        const V3DLONG y1 = 0;
        const V3DLONG z1 = 0;

        const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
        const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
        const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

        //===================================================
        //get image pointers
        v3dhandleList wndlist =this->m_V3DPluginCallback->getImageWindowList();
        if(wndlist.size()<2)
        {
            v3d_msg(QObject::tr("This plugin needs at least two images!"));
            return;
        }

        if( this->m_ImageSelectionDialog.exec() != QDialog::Accepted )
        {
            return;
        }

        Image4DSimple* p4DImage_1 = this->GetInputImageFromIndex( this->m_ImageSelectionDialog.Imageselected0);
        Image4DSimple* p4DImage_2 = this->GetInputImageFromIndex( this->m_ImageSelectionDialog.Imageselected1);
        //===================================================


        QList< V3D_Image3DBasic > inputImageList1 =
            getChannelDataForProcessingFromGlobalSetting( p4DImage_1, *(this->m_V3DPluginCallback) );
        QList< V3D_Image3DBasic > inputImageList2 =
            getChannelDataForProcessingFromGlobalSetting( p4DImage_2, *(this->m_V3DPluginCallback) );

        const unsigned int numberOfChannelsToProcess = inputImageList1.size();
        if (numberOfChannelsToProcess<=0)
        {
            return;
        }

        for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
        {
            //   const V3D_Image3DBasic inputImage1 = inputImageList.at(channel);
            const V3D_Image3DBasic inputImage1 = inputImageList1.at(channel);
            const V3D_Image3DBasic inputImage2 = inputImageList2.at(channel);

            this->TransferInputImages( inputImage1,inputImage2,x1,x2,y1,y2,z1,z2);

            this->ComputeOneRegion();

            this->AddOutputImageChannel( channel );
        }

        this->ComposeOutputImage();
    }


    virtual void ComputeOneRegion()
    {
        V3DPluginArgItem arg1,arg2;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg1.p=(void*)this->GetInput3DImage1();
        arg2.p=(void*)this->GetInput3DImage2();
        input<<arg1<<arg2;
        output<<arg1;
        this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
        fprintf(stdout,"size %d\n",output.size());
        //if the output image type is Float change them to unsigned char
        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.so";
            input.replace(0,output.at(0));
            this->m_V3DPluginCallback->callPluginFunc(pluginname,function_name,input,output);
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);

        }
        else
        {
            ImageType* out=(ImageType*)(output.at(0).p);
            if(!out)std::cerr<<"NULL Image"<<std::endl;
            this->SetOutputImage(out);
        }
    }

private:
    QString plugin_name;
    QString function_name;

};
#endif

