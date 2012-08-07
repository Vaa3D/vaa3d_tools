#ifndef ITKPLUGINMANAGER_H
#define ITKPLUGINMANAGER_H
#include <QDir>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QList>
#include <v3d_interface.h>

class QPluginLoader;
class QObject;

//typedef enum ItkPluginInterfaceName {  itkPluginInterface,
                                       //itkPluginInterface2,
                                       //itkPluginInterface2_1
                                    //} InterfaceType;

class ItkPluginManager
{

typedef enum ItkPluginInterfaceName {  itkPluginInterface,
                                       itkPluginInterface2,
                                       itkPluginInterface2_1
                                    } InterfaceType;
public:
    ItkPluginManager(const QString& intialDir );
    ItkPluginManager();
    ~ItkPluginManager();
    bool searchAllItkPlugins();
    bool runItkPluginMenuFunc ( const QString& itkPluginName );
    void setCallback ( V3DPluginCallback2* callback );
    void setIntialDir (const QString& intialDir);
    QStringList getAllItkPluginNames();
    QHash<QString, int> getItkPluginsHash() const;

private:
    void getItkPluginFiles( QDir& dir );
    ItkPluginManager::InterfaceType getItkPluginInterfaceType ( const QObject* itkPlugin );
    void clear();
private:
    QStringList 			    itkPluginFiles;
    QHash<QString, int>   itkPluginsHash;
    QList < QPluginLoader* > 	itkPlugins;
    QString				        intialDir;
    V3DPluginCallback2*		    v3dHook;
};
#endif
