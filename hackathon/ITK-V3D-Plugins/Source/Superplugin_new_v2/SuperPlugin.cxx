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
    QString initialDir = QDir::fromNativeSeparators(QString("%1/plugins/Vaa3D_ITK/PureITKPlugin").arg(testPluginsDir.absolutePath()));
    QString vaa3dPluginsDir = QDir::fromNativeSeparators(QString("%1/plugins").arg(testPluginsDir.absolutePath()));
    Dialog* mydialog = new Dialog((QWidget*)parent);
    mydialog->setCallback(callback);
    mydialog->setInitialDir(initialDir);
    mydialog->setVaa3DWorkingPluginsDir(vaa3dPluginsDir);
    mydialog->intialPluginManager();
    mydialog->show();
}



