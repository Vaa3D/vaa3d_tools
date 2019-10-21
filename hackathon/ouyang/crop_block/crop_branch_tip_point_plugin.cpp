/* crop_branch_tip_point_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-10-14 : by Ouyang
 */
 
#include <vector>
#include "crop_branch_tip_point_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(crop_branch_tip_point, crop_block);
 
QStringList crop_block::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList crop_block::funclist() const
{
	return QStringList()
        <<tr("get_block")
		<<tr("func2")
		<<tr("help");
}

void crop_block::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Ouyang, 2019-10-14"));
	}
}

bool crop_block::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get_block"))
    {
        crop_bt_block(input,output,callback);
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

