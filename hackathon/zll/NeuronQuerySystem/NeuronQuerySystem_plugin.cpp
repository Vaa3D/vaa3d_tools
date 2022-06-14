/* NeuronQuerySystem_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2022-5-27 : by zll
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuronQuerySystem_plugin.h"
#include "neuronquerymainwindow.h"
using namespace std;
//Q_EXPORT_PLUGIN2(NeuronQuerySystem, TestPlugin);
 
QStringList NeuronQuerySystemPlugin::menulist() const
{
	return QStringList() 
		<<tr("NeuronQuerySystem")
		<<tr("menu2")
		<<tr("about");
}

QStringList NeuronQuerySystemPlugin::funclist() const
{
	return QStringList()
		<<tr("NeuronQuerySystem")
		<<tr("func2")
		<<tr("help");
}

void NeuronQuerySystemPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("NeuronQuerySystem"))
	{
        NeuronQueryMainWindow *mainwindow=new NeuronQueryMainWindow(callback,parent);
        mainwindow->setGeometry(0,0,1280,1080);
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
        mainwindow->setWindowState(Qt::WindowNoState);
        mainwindow->activateWindow();
        mainwindow->raise();

        mainwindow->show();

	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by zll, 2022-5-27"));
	}
}

bool NeuronQuerySystemPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("NeuronQuerySystem"))
	{
        NeuronQueryMainWindow *mainwindow=new NeuronQueryMainWindow(callback,parent);
        mainwindow->setWindowState(Qt::WindowNoState);
        mainwindow->activateWindow();
        mainwindow->raise();
        mainwindow->show();

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

