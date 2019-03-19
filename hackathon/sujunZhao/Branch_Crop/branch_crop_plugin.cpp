/* branch_crop_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-19 : by YourName
 */
 

#include "branch_detect.h"
#include <stdio.h>
#include <iostream>
#include "branch_crop_plugin.h"

using namespace std;
Q_EXPORT_PLUGIN2(Branch_Crop, TestPlugin)

//void get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
//QList<int> get_branch_points(NeuronTree nt, bool include_root);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("get_branch_sample")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("get_branch_point")
        <<tr("branch_point_sample")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_branch_sample"))
	{
         v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-12-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
//	vector<char*> infiles, inparas, outfiles;
//	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get_branch_point"))
	{
        get_branches(input,output,callback);
	}
    else if (func_name == tr("branch_point_sample"))
	{
		v3d_msg("To be implemented.");
	}
    else if (func_name == tr("help")){
        v3d_msg("To be implemented.");
    }

}
