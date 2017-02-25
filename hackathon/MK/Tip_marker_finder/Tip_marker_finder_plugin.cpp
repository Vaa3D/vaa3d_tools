/* Tip_marker_finder_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-11-9 : by YourName
 */

#include "../../zhi/IVSCC_sort_swc/openSWCDialog.h"
#include "v3d_message.h"
#include <vector>
#include "Tip_marker_finder_plugin.h"
#include "../../../released_plugins/v3d_plugins/gsdt/common_dialog.h"
#include "tip_marker_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(Tip_marker_finder, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("find")
        <<tr("find_all")
        <<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("find"))
    {
		TipMarkerFinderUI* inputForm = new TipMarkerFinderUI(0, &callback, 1);
		QStringList inputInfo;
		if (inputForm->exec()) inputInfo = inputForm->inputs; 
		qDebug() << inputInfo;

		TipProcessor(inputInfo, 1);
		
    }
	else if(menu_name == tr("find_all"))
    {
		TipMarkerFinderUI* inputForm = new TipMarkerFinderUI(0, &callback, 2);
		QStringList inputInfo;
		if (inputForm->exec()) inputInfo = inputForm->inputs; 
		qDebug() << inputInfo;

		TipProcessor(inputInfo, 2);
        //SpecDialog_all(callback,parent);
    }
	else
	{
        v3d_msg(tr("This plugin finds neuron tips near the truncate slice and label as markers.\n  -- Developed by Mars K., 2016, 11, 10."));
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
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

