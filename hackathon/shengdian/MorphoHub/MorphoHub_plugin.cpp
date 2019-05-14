/* MorphoHub_plugin.cpp
 * designed by shengdian
 * 2019-5-14 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include <QApplication>
#include "MorphoHub_plugin.h"
#include "MainWindow/morphohub_mainwindow.h"

using namespace std;
Q_EXPORT_PLUGIN2(MorphoHub, MorphoHubPlugin);
 
QStringList MorphoHubPlugin::menulist() const
{
	return QStringList() 
        <<tr("MorphoHub")
		<<tr("menu2")
		<<tr("about");
}

QStringList MorphoHubPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void MorphoHubPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("MorphoHub"))
	{
        //v3d_msg("To be implemented.");
        //MorphoHub_MainWindow mainwindow;
        //mainwindow.show();
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("designed by shengdian. "
			"Developed by Shengdian, 2019-5-14"));
	}
}

bool MorphoHubPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

//	if (func_name == tr("func1"))
//	{
//		v3d_msg("To be implemented.");
//	}
//	else if (func_name == tr("func2"))
//	{
//		v3d_msg("To be implemented.");
//	}
//	else if (func_name == tr("help"))
//	{
//		v3d_msg("To be implemented.");
//	}
//	else return false;

	return true;
}

