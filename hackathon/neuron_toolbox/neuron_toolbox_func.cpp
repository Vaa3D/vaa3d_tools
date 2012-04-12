/* neuron_toolbox_func.cpp
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "neuron_toolbox_func.h"
#include "toolbox_gui.h"

const QString title = QObject::tr("Neuron Toolbox");

//dofunc for neuron toolbox is reserved for pop-up menu in main program
bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent)
{
	SelectPluginDlg * selectDlg = new SelectPluginDlg(parent, &callback);
	selectDlg->setWindowTitle("Select plugin to run...");
	selectDlg->show();
	return true;

}
/*
bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent)
{ 
	V3DPluginArglist input;
	if (!neuron_toolbox_func(input, callback, parent))
	{
		v3d_msg("Error in running neuron tool box");
		return false;
	}
	return true;
}*/

int help(V3DPluginCallback2 &callback, QWidget *parent)
{

}

bool help(const V3DPluginArgList & input, V3DPluginArgList & output)
{
}

