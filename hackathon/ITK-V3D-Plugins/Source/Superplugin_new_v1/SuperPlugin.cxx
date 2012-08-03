#include "SuperPlugin.h"
#include "Dialog.h"
#include <iostream>

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


bool Superplugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
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
    Q_INIT_RESOURCE(superPluginIcons);

    QString initialDir = QDir::fromNativeSeparators(QString("%1/plugins/ITK").arg(QDir::currentPath()));
    Dialog* mydialog = new Dialog((QWidget*)0);
    mydialog->setCallback(callback);
    mydialog->setInitialDir(initialDir);
    mydialog->intialPluginManager();
    mydialog->show();
}



