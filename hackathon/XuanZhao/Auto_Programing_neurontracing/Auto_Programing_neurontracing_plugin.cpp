/* Auto_Programing_neurontracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-23 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "Auto_Programing_neurontracing_plugin.h"
#include "mydialog.h"
#include "cut_image_p.h"
#include "cut_image_f.h"
#include "compare_f.h"
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(Auto_Programing_neurontracing, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("getTif_Eswc_Marker")
        <<tr("compare")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("getTif_Eswc_Marker"))
	{
        mydialog dialog;
        dialog.exec();

        parameter1 p=dialog.getParameter();
        if(!getTif_Eswc_Marker(p,callback))
        {
            v3d_msg("parameters is false!");
            return;
        }
	}
    else if (menu_name == tr("compare"))
	{
        const QString dir=QFileDialog::getExistingDirectory(parent);
        //select_cross(dir);
        select_weaksignal(dir);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-23"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

