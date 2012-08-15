#include <QtGui>
#include <math.h>
#include <stdlib.h>
//#include <unistd.h>

#include "Superplugin.h"
#include "V3DITKFilterSingleImage.h"
#include "dialog.h"

Q_EXPORT_PLUGIN2(MySuper,Superplugin)

QStringList Superplugin::menulist()const
{
    return QStringList()<<QObject::tr("ITK MySuper")
           <<QObject::tr("about this plugin");
}

QStringList Superplugin::funclist()const
{
    return QStringList();
}

#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE(v3d_pixel_type,c_pixel_type) \
	case v3d_pixel_type:\
	{ \
		PluginSpecialized<c_pixel_type> runner(&callback);\
		runner.Execute(menu_name,parent);\
		break; \
	}


bool Superplugin::dofunc(const QString & fuc_name,const V3DPluginArgList & input,V3DPluginArgList & output,V3DPluginCallback2& callback, QWidget* parent)
{
  return false;
}
void Superplugin::domenu(const QString & menu_name,V3DPluginCallback2 & callback,QWidget *parent)
{
    if(menu_name==QObject::tr("about this plugin"))
    {
        QMessageBox::information(parent,"Vertion info","ITK Superplugin:this plugin is developed by Ping Yu");
        return;
    }
    v3dhandle curwin =callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg(tr("You don't have any image open in the main window"));
        return;
    }
    Image4DSimple *p4DImage=callback.getImage(curwin);
    if(!p4DImage)
    {
        v3d_msg(tr("The input image is null."));
        return;
    }
    Q_INIT_RESOURCE(Icons);
    QDir testPluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        testPluginsDir.cdUp();
        testPluginsDir.cdUp();
        testPluginsDir.cdUp();
    }
#endif
    QString initialDir = QDir::fromNativeSeparators(QString("%1/plugins/Vaa3D_ITK/Superplugin/Plugin2Call").arg(testPluginsDir.absolutePath()));
    QString vaa3dPluginsDir = QDir::fromNativeSeparators(QString("%1/plugins").arg(testPluginsDir.absolutePath()));
    qDebug() << "initialDir to search: " << initialDir; 
    Dialog* mydialog = new Dialog(parent);
    mydialog->SetCallback(callback);
    mydialog->setInitialDir(initialDir);
    mydialog->setVaa3DWorkingPluginsDir(vaa3dPluginsDir);
    mydialog->initial();
    mydialog->show();
//EXECUTE_PLUGIN_FOR_INTEGER_PIXEL_TYPES;
}
