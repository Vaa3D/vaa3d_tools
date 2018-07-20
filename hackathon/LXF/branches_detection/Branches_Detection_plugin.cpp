/* Branches_Detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-29 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "Branches_Detection_plugin.h"
#include "branches_detection.h"
using namespace std;
Q_EXPORT_PLUGIN2(Branches_Detection, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("branch_detection")
		<<tr("menu2")
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
    if (menu_name == tr("branch_detection"))
	{
        v3d_msg("sucess!");
        BRANCH_LS_PARA P;
        terafly_para dialog(callback, parent);
        QString inimg_file = "/home/penglab/mouseID_321237-17302/RES(54600x34412x9847)";
      //  P.inimg_file = dialog.teraflyfilename;
        P.inimg_file =inimg_file;
        branches_detection(callback,parent,P);
        //v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-5-29"));
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

