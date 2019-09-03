/* MorphoHub_DBMS_plugin.cpp
 * This is data management Plugin for neuron reconstruction management.
 * 2019-9-2 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "MorphoHub_DBMS_plugin.h"
#include "MainWindow/morphohub_mainwindow.h"
using namespace std;
Q_EXPORT_PLUGIN2(MorphoHub_DBMS, MorphoHub_DBMS);
 
QStringList MorphoHub_DBMS::menulist() const
{
	return QStringList() 
        <<tr("MorphoHub_DBMS")
		<<tr("about");
}

QStringList MorphoHub_DBMS::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void MorphoHub_DBMS::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("MorphoHub_DBMS"))
	{
        MorphoHub_MainWindow *mainwindow=new MorphoHub_MainWindow(parent);
        mainwindow->setWindowState(Qt::WindowNoState);
        mainwindow->raise();
        mainwindow->activateWindow();
        mainwindow->setGeometry(100,400,1280,1080);
        mainwindow->show();
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is data management Plugin for neuron reconstruction management.. "
			"Developed by Shengdian, 2019-9-2"));
	}
}

bool MorphoHub_DBMS::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

