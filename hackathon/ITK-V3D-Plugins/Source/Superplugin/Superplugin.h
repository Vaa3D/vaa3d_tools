#ifndef _SUPERPLUGIN_H_
#define _SUPERPLUGIN_H_

#include <QtGui>
#include <iostream>
#include <v3d_interface.h>

#include "V3DITKFilterSingleImage.h"
#include "V3DITKFilterDualImage.h"


class Superplugin:public QObject,public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2)
public:
    Superplugin() {}
    QStringList menulist()const;
    QStringList funclist()const;
    void domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget* parent);
    bool dofunc(const QString & func_name,const V3DPluginArgList &input,V3DPluginArgList & output, V3DPluginCallback2& callback, QWidget *parent);
};


template <typename TPixelType>
class PluginSpecialized:public V3DITKFilterSingleImage< TPixelType,	TPixelType>
{
    typedef V3DITKFilterSingleImage<TPixelType,TPixelType>	Superclass;
    typedef typename Superclass::Input3DImageType			ImageType;
    typedef typename Superclass::Output3DImageType			OutputImageType;
    typedef typename itk::Image<float,3>					FImageType;


public:
    PluginSpecialized(V3DPluginCallback* callback):Superclass(callback)
    {
        plugin_name="ITK/Superplugin/NOT/NOT.so";
        function_name="doFunction";
        usePipeline=false;
    }

//plugin_name the realName, plugin_file_nam: the folder name
    void SetPluginName(const QString &plugin_path,  const QString& plugin_name_showed)
    {
      
        this->plugin_name = plugin_path;
    }
    void AddPluginName(const QString &plugin_name, const QString& plugin_folder_name)
    {
        QString m_name = plugin_name;
        this->plugin_name_list << m_name;
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

        bool result;
        for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
        {
            const V3D_Image3DBasic inputImage = inputImageList.at(channel);

            this->TransferInput( inputImage, x1, x2, y1, y2, z1, z2 );
            if(usePipeline) result = this->ComputePipeline();
            else result = this->ComputeOneRegionNew();

            if (!result) {
             qDebug() << "error call the pipline";
             return;
            } 

            this->AddOutputImageChannel( channel );
        }

        this->ComposeOutputImage();
    }


  bool ComputeOneRegionNew()
    {
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg.p=(void*)this->GetInput3DImage();
        arg.type="UINT8Image";
        input<<arg;
        output<<arg;
        bool result = this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
        if (!result) {
          qDebug() << "error call the other plugin function ";
          return false;
        }
        qDebug() << "successful called";
        qDebug() << QString("size %1").arg(output.size());

        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.dylib";
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
        return true;
    }
    void ComputeOneRegion() {}
    bool ComputePipeline()
    {
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg.p=(void*)this->GetInput3DImage();
        arg.type="UINT8Image";
        input<<arg;
        output<<arg;
        bool result;
        for(int i=0; i<plugin_name_list.size(); i++)
        {

            result = this->m_V3DPluginCallback->callPluginFunc(plugin_name_list.at(i),function_name,input,output);
            if (!result) {
              qDebug() << "error call the other plugin function";
              return false;
            }
            input.replace(0,output.at(0));
        }
        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.dylib";
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
        return true;
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
    
public:
    PluginSpecializedForDual(V3DPluginCallback* callback):Superclass(callback)
    {
        plugin_name="ITK/Superplugin/NOT/NOT.so";
        function_name="doFunction";
    }
//plugin_name the realName, plugin_file_nam: the folder name
    void SetPluginName(const QString &plugin_path,  const QString& plugin_name_showed)
    {
        this->plugin_name = plugin_path;
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

            bool result = this->ComputeOneRegionNew();
            if (!result) {
              qDebug() << "error call the plugins function";
              return;
            }
            this->AddOutputImageChannel( channel );
        }

        this->ComposeOutputImage();
    }


    bool ComputeOneRegionNew()
    {
        V3DPluginArgItem arg1,arg2;
        V3DPluginArgList input;
        V3DPluginArgList output;
        arg1.p=(void*)this->GetInput3DImage1();
        arg2.p=(void*)this->GetInput3DImage2();
        input<<arg1<<arg2;
        output<<arg1;
        
        bool result = this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
        if (!result) {
          qDebug() << "error call other plugin func";
          return false;
        }
        fprintf(stdout,"size %d\n",output.size());
        //if the output image type is Float change them to unsigned char
        if(output.at(0).type=="floatImage")
        {
            printf("Get float ImageType !\n");
            QString pluginname="ITK/Superplugin/Plugin2Call/Cast2UINT8/Cast2UINT8.dylib";
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
        return true;
    }
    void ComputeOneRegion() {}

private:
    QString plugin_name;
    QString function_name;

};
#endif

