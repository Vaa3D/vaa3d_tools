#include "ItkPluginManager.h"
#include "ImageDataTransmit.h"
#include <QRegExp>
#include <QObject>
#include <QPluginLoader>
#include <iostream>

static QRegExp rx = QRegExp("SuperPlugin.*");

ItkPluginManager :: ItkPluginManager ( const QString& dir ):intialDir(dir) {
}
ItkPluginManager :: ItkPluginManager () {}

void ItkPluginManager :: setIntialDir ( const QString& initialDir) {
    this->intialDir = initialDir;
}

bool ItkPluginManager :: searchAllItkPlugins ()
{
    this->itkPluginFiles.clear();
    this->itkPluginsHash.clear(); 
    QDir intiDir(this->intialDir);
    try{
        this->getItkPluginFiles (intiDir);
    }catch(...){return false;}
    return true;
}

void ItkPluginManager :: getItkPluginFiles( QDir& dir )
{
    dir.setFilter (QDir::NoDotAndDotDot | QDir::AllDirs);
    QStringList dirList = dir.entryList ();
    if (0 != dirList.size ())
    {
        for (int i = 0; i < dirList.size (); i++)
        {
            QDir tempDir(dir);
            tempDir.cd (dirList.at(i));
            this->getItkPluginFiles (tempDir);
        }
    }
    dir.setFilter (QDir::Files);
    QStringList fileList = dir.entryList ();
    //get the plugin name that show on the superplugin
    QRegExp rxFile = QRegExp("(lib)?([^.]+)\\..*");
    for (int i = 0; i < fileList.size(); i++)
    {
        QString file = fileList.at(i);
        int pos = rx.indexIn(file);
        if (pos >= 0) {
          qDebug() << "plugin name contains SuperPlugin, skip it";
          continue;
        }
        
        QPluginLoader* itkPluginLoader = new QPluginLoader(QDir::fromNativeSeparators(dir.absolutePath ()+ "/" + fileList.at(i)));
        QObject* itkPlugin = itkPluginLoader -> instance();
        if ( !itkPlugin )
        {
            std::cerr << "Can't load the ITK plugin : " << fileList.at(i).toStdString() << " !" << std::endl;
            return;
        }
        else
        {
            if (rxFile.indexIn(fileList.at(i)) >= 0) {
              QString fileName = rxFile.cap(2);
              this->itkPlugins.append( itkPluginLoader );
              this->itkPluginFiles << fileName;
              this->itkPluginsHash.insert(fileName, this->itkPluginFiles.size());
            //free the memory or useing delete itkPlugin
              itkPluginLoader -> unload ();
            }
            else { 
              itkPluginLoader->unload();
              continue;
            }
        }

    }
}
QHash<QString, int > ItkPluginManager::getItkPluginsHash() const
{
  return this->itkPluginsHash;
} 
//bool ItkPluginManager :: runItkPluginMenuFunc ( const QString& itkPluginName )
//{

    //int indexOfItkPlugin = this->itkPluginsHash.value(itkPluginName) - 1;
    ////std::cout << "index is : " << indexOfItkPlugin<<std::endl;
    //QObject* itkPlugin = this->itkPlugins.at(indexOfItkPlugin) -> instance();
    //if ( !itkPlugin ) return false;
    //ItkPluginManager::InterfaceType pluginInterfaceType = this->getItkPluginInterfaceType ( itkPlugin );
    ////std::cout << "tpye is : " << pluginInterfaceType<<std::endl;
    //switch ( pluginInterfaceType)
    //{
      //case ItkPluginManager::itkPluginInterface:
        //{
        //V3DPluginInterface* itkPluginCall = qobject_cast < V3DPluginInterface* > (itkPlugin);
        //QStringList menuList = itkPluginCall -> menulist();
        //std::cout << menuList.at(0).toStdString()<<std::endl;
        //itkPluginCall -> domenu( menuList.at(0), *(this->v3dHook), (QWidget*) 0);
        //break;
        //}
    //case ItkPluginManager::itkPluginInterface2:
        //{
        //V3DPluginInterface2* itkPluginCall = qobject_cast < V3DPluginInterface2* > (itkPlugin);
        //QStringList menuList = itkPluginCall -> menulist();
        //itkPluginCall -> domenu( menuList.at(0), *(this->v3dHook), (QWidget*) 0);
        //break;
        //}
    //case ItkPluginManager::itkPluginInterface2_1:
        //{

        //V3DPluginInterface2_1* itkPluginCall = qobject_cast < V3DPluginInterface2_1* > (itkPlugin);
        //QStringList menuList = itkPluginCall -> menulist();
        //itkPluginCall -> domenu( menuList.at(0), *(this->v3dHook), (QWidget*) 0);
        //break;
        //}
    //}
    ////free the memory
    //this->itkPlugins.at(indexOfItkPlugin)->unload();
    //return true;
//}
#define CALL_FUNC( pixelType ) \
switch( pixelType )\
{\
  case V3D_UINT8:\
    {\
      ImageDataTransmit<unsigned char> dataTransmit(v3dHook);\
      dataTransmit.transferInputImage();\
      for (int i = 0; i < channelNum; i++) {\
        V3DPluginArgList input;\
        V3DPluginArgList output;\
        V3DPluginArgItem arg;\
        arg.p = (void*)dataTransmit.getInput3DImage(i);\
        arg.type = QString("UINT8Image");\
        input << arg;\
        output << arg; \
        bool result = itkPluginCall -> dofunc("", input, output, *(this->v3dHook), (QWidget*) 0);\
        if (!result){\
          qDebug() << "Call_FUNC: Error running the plugin"; \
          result = false; \
          break;  \
        } \
        dataTransmit.setOutput3DImage((ImageDataTransmit<unsigned char>::Output3DImageType *)output.at(0).p, i); \
      } \
        dataTransmit.transferOutputImage(); \
      break; \
    }  \
  case V3D_UINT16: \
    { \
      ImageDataTransmit<unsigned short int> dataTransmit(v3dHook); \
      dataTransmit.transferInputImage(); \
      for (int i = 0; i < channelNum; i++) { \
        V3DPluginArgList input; \
        V3DPluginArgList output; \
        V3DPluginArgItem arg; \
        arg.p = (void*)dataTransmit.getInput3DImage(i); \
        arg.type = QString("UINT16Image"); \
        input << arg; \
        bool result = itkPluginCall -> dofunc("", input, output, *(this->v3dHook), (QWidget*) 0); \
        if (!result){ \
          qDebug() << "error run the plugin"; \
          result = false; \
          break;  \
        } \
        dataTransmit.setOutput3DImage((ImageDataTransmit<unsigned short int>::Output3DImageType *)output.at(0).p, i); \
      } \
        dataTransmit.transferOutputImage(); \
      break; \
    }  \
  case V3D_FLOAT32: \
    { \
      ImageDataTransmit<float> dataTransmit(v3dHook); \
      dataTransmit.transferInputImage(); \
      for (int i = 0; i < channelNum; i++) { \
        V3DPluginArgList input; \
        V3DPluginArgList output; \
        V3DPluginArgItem arg; \
        arg.p = (void*)dataTransmit.getInput3DImage(i); \
        arg.type = QString("FLOATImage"); \
        input << arg; \
        bool result = itkPluginCall -> dofunc("", input, output, *(this->v3dHook), (QWidget*) 0); \
        if (!result){ \
          qDebug() << "error run the plugin"; \
          result = false; \
          break; \
        } \
        dataTransmit.setOutput3DImage((ImageDataTransmit<float>::Output3DImageType *)output.at(0).p, i); \
      } \
        dataTransmit.transferOutputImage(); \
      break; \
    }  \
  case V3D_UNKNOWN: \
    { \
    } \
} \


bool ItkPluginManager :: runItkPluginFunc ( const QString& itkPluginName )
{
    v3dhandle curwin = this->v3dHook->currentImageWindow();
    Image4DSimple* p4DImage = this->v3dHook->getImage(curwin); 
    ImagePixelType pixelType = p4DImage->getDatatype();
    int channelNum = p4DImage->getCDim();
    int indexOfItkPlugin = this->itkPluginsHash.value(itkPluginName) - 1;
    std::cout << "index is : " << indexOfItkPlugin<<std::endl;
    QObject* itkPlugin = this->itkPlugins.at(indexOfItkPlugin) -> instance();
    if ( !itkPlugin ) return false;
    ItkPluginManager::InterfaceType pluginInterfaceType = this->getItkPluginInterfaceType ( itkPlugin );
    qDebug() << "The plugin interface type is : " << pluginInterfaceType;
    
    bool result = true;
    switch ( pluginInterfaceType)
    {
      case ItkPluginManager::itkPluginInterface:
        {
        V3DPluginInterface* itkPluginCall = qobject_cast < V3DPluginInterface* > (itkPlugin);
        QStringList funcList = itkPluginCall -> funclist();
        qDebug() << funcList.at(0);
        qDebug() << "Don't support V3DPluginInterface version 1";
        result = false;
        break;
        }
    case ItkPluginManager::itkPluginInterface2:
        {
        qDebug()<<"ItkPluginManager: Interface 2";
        V3DPluginInterface2* itkPluginCall = qobject_cast < V3DPluginInterface2* > (itkPlugin);
        QStringList funcList = itkPluginCall -> funclist();
        CALL_FUNC(pixelType)
        break;
        }
    case ItkPluginManager::itkPluginInterface2_1:
        {
        V3DPluginInterface2_1* itkPluginCall = qobject_cast < V3DPluginInterface2_1* > (itkPlugin);
        QStringList funcList = itkPluginCall -> funclist();
        CALL_FUNC(pixelType)
        break;
        }
    }
    //free the memory

    this->itkPlugins.at(indexOfItkPlugin)->unload();
    return result;
}

ItkPluginManager::InterfaceType ItkPluginManager :: getItkPluginInterfaceType ( const QObject* itkPlugin )
{
    ItkPluginManager::InterfaceType pluginInterfaceType;
    V3DPluginInterface* pluginType1 = qobject_cast < V3DPluginInterface* > (itkPlugin);
    V3DPluginInterface2* pluginType2 = qobject_cast < V3DPluginInterface2* > (itkPlugin);
    V3DPluginInterface2_1* pluginType3 = qobject_cast < V3DPluginInterface2_1* > (itkPlugin);
    if ( pluginType1 ) pluginInterfaceType =  itkPluginInterface;
    if ( pluginType2 ) pluginInterfaceType =  itkPluginInterface2;
    if ( pluginType3 ) pluginInterfaceType =  itkPluginInterface2_1;
    pluginType1 = NULL;
    pluginType2 = NULL;
    pluginType3 = NULL;

    return pluginInterfaceType;
}

void ItkPluginManager :: setCallback ( V3DPluginCallback2* callback )
{
    this->v3dHook = callback;
    
  //dong some test
  //ImageDataTransmit<unsigned char>* dataTransmit = new ImageDataTransmit<unsigned char>(callback);
  //dataTransmit->transferInputImage();
  //typedef ImageDataTransmit<unsigned char>::Output3DImageType::PixelContainer PixelContainer;
  //if (dataTransmit->getInput3DImage(0)->GetPixelContainer()->GetImportPointer()
    //== dataTransmit->getInput3DImage(1)->GetPixelContainer()->GetImportPointer())
  //{
    //qDebug() << "same pointer";
  //}
  //dataTransmit->setOutput3DImage(dataTransmit->getInput3DImage(0), 0);
  //dataTransmit->setOutput3DImage(dataTransmit->getInput3DImage(1), 1);
  //dataTransmit->setOutput3DImage(dataTransmit->getInput3DImage(2), 2);
  //dataTransmit->transferOutputImage();
  //delete dataTransmit;
}

void ItkPluginManager :: clear ()
{
    this->itkPluginFiles.clear();
    for ( int i = 0; i < itkPlugins.size(); i++)
    {
        if ( itkPlugins.at(i) -> isLoaded() )
        {
            itkPlugins.at(i)->unload();
        }
        delete itkPlugins.at(i);
    }
    itkPlugins.clear();
}
ItkPluginManager::~ItkPluginManager()
{
    this->clear();
}
QStringList ItkPluginManager::getAllItkPluginNames ()
{
    return this->itkPluginFiles;
}
//


