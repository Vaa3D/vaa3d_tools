/* neuron_toolbox_func.cpp
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "neuron_toolbox_func.h"
#include "toolbox_gui.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <iostream>
using namespace std;

const QString title = QObject::tr("Neuron Toolbox");

//dofunc for neuron toolbox is reserved for pop-up menu in main program
bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent)
{
	SelectPluginDlg * selectDlg = new SelectPluginDlg(parent, callback);
	selectDlg->setWindowTitle("Select plugin to run...");
	selectDlg->show();
	return true;

}

bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	QString test_str1 = QString(((vaa3d_neurontoolbox_paras *)(input.at(0).p))->nt.file);
	QString test_str = QFileInfo(test_str1).baseName() + "." + QFileInfo(test_str1).completeSuffix();
	SelectPluginDlg * selectDlg = new SelectPluginDlg(parent, callback, input);
	selectDlg->setWindowTitle(test_str + " - Select plugin to run...");
	selectDlg->show();
//	const QString plugin_name = "/Users/wany/Work/v3d_external/bin/plugins/neuron_utilities/sort_neuron_swc/libsort_neuron_swc_debug.dylib";
//	const QString menu_name = "TOOLBOXsort_swc";
//	callback.callPluginFunc(plugin_name, menu_name, input, output);
	return true;
}

void help(bool b_useWin)
{
    v3d_msg("This is a super plugin that gather all sub-plugins related to neuron structure processing.. Developed by Yinan Wan, 2012-04-06", b_useWin);
}


