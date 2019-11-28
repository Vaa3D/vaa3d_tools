/* staisticSwcDepth_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-28 : by wpkenan
 */
 
#include "v3d_message.h"
#include <vector>
#include "staisticSwcDepth_plugin.h"
#include "statisticSwcDepth.h"
using namespace std;
Q_EXPORT_PLUGIN2(staisticSwcDepth, staisticSwcDepthPlugin);
 
QStringList staisticSwcDepthPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList staisticSwcDepthPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void staisticSwcDepthPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2019-11-28"));
	}
}

bool staisticSwcDepthPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("statisticSwcDepth"))
	{
		
		//v3d_msg("To be implemented.");
		//NeuronTree nt1 = readSWC_file(infiles[0]);
		
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

