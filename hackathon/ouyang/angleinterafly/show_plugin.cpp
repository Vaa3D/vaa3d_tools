/*
 *  sort_func.cpp
 *  io functions for domenu and do func 
 *
 *  2018-05-24 : by Linus Manubens Gil
 *  adapted from 2012-02-01 : by Yinan Wan
 */
 
#include "v3d_message.h"
#include <vector>
#include "show_plugin.h"
#include "show_func.h"
using namespace std;

Q_EXPORT_PLUGIN2(angle_show, ANGLEPlugin);
 
QStringList ANGLEPlugin::menulist() const
{
	return QStringList()
        <<tr("angle_show")
		<<tr("about");
}

QStringList ANGLEPlugin::funclist() const
{
	return QStringList()
        <<tr("angle_show")
		<<tr("help");
}

void ANGLEPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("angle_show"))
	{
        show_menu(callback,parent);
	}
    else if (menu_name == tr("about"))
	{
        v3d_msg(tr("This is a test plugin for finding branches whose remote angles are more than 90 degree,and these branch nodes will be added a marker in red "
            "Developed by Ou Yang, 2018-8-13"));
	}
}

bool ANGLEPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("help"))
    {
        v3d_msg("To be implemented.");
    }
    else return false;

    return true;
}

