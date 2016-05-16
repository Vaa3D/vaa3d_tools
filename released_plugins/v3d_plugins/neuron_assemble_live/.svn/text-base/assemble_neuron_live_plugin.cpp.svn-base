/* assemble_neuron_live_plugin.cpp
 * This plugin will connect fragments, break loop, and combine neuron files in live.
 * 2015-7-8 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "assemble_neuron_live_plugin.h"
#include "openSWCDialog.h"
#include "assemble_neuron_live_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(assemble_neuron_live, neuron_assembler_live);
 
QStringList neuron_assembler_live::menulist() const
{
	return QStringList() 
		<<tr("assemble_neuron_live")
		<<tr("about");
}

QStringList neuron_assembler_live::funclist() const
{
	return QStringList()
		<<tr("help");
}

void neuron_assembler_live::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("assemble_neuron_live"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(parent, &callback);
        if(!openDlg->exec())
            return;
        assemble_neuron_live_dialog * assemDlg = NULL;
        assemDlg = new assemble_neuron_live_dialog(&callback, openDlg->ntList, openDlg->p_img4d, parent);
        assemDlg->show();
    }
	else
	{
		v3d_msg(tr("This plugin will connect fragments, break loop, and combine neuron files in live.. "
			"Developed by Hanbo Chen, 2015-7-8"));
	}
}

bool neuron_assembler_live::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

