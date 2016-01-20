//last change: by PHC 20130905. Creating this file actually fix the Windows VC++ build problem of creating dynamic library files. (w/o this, the dll was created but not the .lib file and .exp file), and thus the plugin cannot be instantiated.

#include "s2_plugin.h"
#include <iostream>
#include <vector>
#include "basic_surf_objs.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <stdlib.h>

#include <QApplication>
#include "s2Client.h"
#include "s2UI.h"



//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(s2, S2Plugin);
using namespace std;


QStringList S2Plugin::menulist() const
{
    return QStringList()
            << tr("start smartScope2")
            << tr("about");
}

void S2Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == "start smartScope2")
    {

        S2UI myS2( callback, parent);
        myS2.show();
        myS2.exec();
	}
    else if(menu_name == "about")
    {
        QMessageBox::information(0, "smartScope 2 Plugin", \
                                 QObject::tr("contact: brianl@alleninstitute.org"));
    }
}

QStringList S2Plugin::funclist() const
{
    return QStringList()
            << tr("start smartScope 2")
            << tr("help");
}

bool S2Plugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(func_name == "smartScope2")
    {
    }
    else if(func_name == "help")
    {
        cout<<"help?";    
	}
    return false;
}



