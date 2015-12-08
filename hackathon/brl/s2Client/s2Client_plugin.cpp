//last change: by PHC 20130905. Creating this file actually fix the Windows VC++ build problem of creating dynamic library files. (w/o this, the dll was created but not the .lib file and .exp file), and thus the plugin cannot be instantiated.

#include "S2Client_plugin.h"
#include <iostream>
#include <vector>
#include <QApplication>
#include "s2Client.h"
#include "s2Client_plugin.h"
#include "basic_surf_objs.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <stdlib.h>
//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(s2Client, S2ClientPlugin);
using namespace std;

static string basename(string para)
{
    int pos1 = para.find_last_of("/");
    int pos2 = para.find_last_of(".");
    if(pos1 == string::npos) pos1 = -1;
    if(pos2 == string::npos) pos2 = para.size();
    return para.substr(pos1+1, pos2 - pos1 -1);
}

// file_type("test.tif") == ".tif"
string file_type(string para)
{
    int pos = para.find_last_of(".");
    if(pos == string::npos) return string("unknown");
    else return para.substr(pos, para.size() - pos);
}


QStringList S2ClientPlugin::menulist() const
{
    return QStringList()
            << tr("S2Client")
            << tr("about");
}

void S2ClientPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == "S2Client")
    {
		S2Client client;
	    client.show();
	    client.exec();
	}
    else if(menu_name == "about")
    {
        QMessageBox::information(0, "S2Client Plugin", \
                                 QObject::tr("prototype Smartscope2 client"));
    }
}

QStringList S2ClientPlugin::funclist() const
{
    return QStringList()
            << tr("S2Client")
            << tr("help");
}

bool S2ClientPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(func_name == "S2Client")
    {
    }
    else if(func_name == "help")
    {
        cout<<"help?";    
	}
    return false;
}



