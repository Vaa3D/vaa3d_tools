/* MorphoHub_plugin.cpp
 * PB-scale data management system
 * 2020-02-12 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "MorphoHub_plugin.h"
#include "GUI/morphohub_gui.h"
using namespace std;
Q_EXPORT_PLUGIN2(MorphoHub, MorphoHub);
 
QStringList MorphoHub::menulist() const
{
	return QStringList() 
		<<tr("MorphoHub")
		<<tr("about");
}

QStringList MorphoHub::funclist() const
{
	return QStringList()
		<<tr("Dendrites_Generation")
		<<tr("L0_Generation")
		<<tr("Bouton_Generation")
		<<tr("help");
}

void MorphoHub::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("MorphoHub"))
	{
        morphoHub_GUI *mainwindow=new morphoHub_GUI(callback,parent);
        mainwindow->setGeometry(100,400,1280,1080);
        mainwindow->show();
	}
	else
	{
		v3d_msg(tr("PB-scale data management system. "
			"Developed by Shengdian, 2020-02-12"));
	}
}

bool MorphoHub::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Dendrites_Generation"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("L0_Generation"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("Bouton_Generation"))
	{

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

