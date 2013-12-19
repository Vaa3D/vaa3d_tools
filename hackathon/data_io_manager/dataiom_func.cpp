/* neuron_toolbox_func.cpp
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "dataiom_func.h"
#include "dataiom_gui.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <iostream>
using namespace std;

const QString title = QObject::tr("Neuron Toolbox");

//dofunc for neuron toolbox is reserved for pop-up menu in main program
bool dataiom_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent)
{
	SelectPluginDlg * selectDlg = new SelectPluginDlg(parent, callback);
	selectDlg->setWindowTitle("Select plugin to run...");
	selectDlg->show();
	return true;
}

bool dataiom_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	QString test_str1 = QString(((vaa3d_neurontoolbox_paras *)(input.at(0).p))->nt.file);
	QString test_str = QFileInfo(test_str1).baseName() + "." + QFileInfo(test_str1).completeSuffix();
	SelectPluginDlg * selectDlg = new SelectPluginDlg(parent, callback, input);
	selectDlg->setWindowTitle(test_str + " - Select plugin to run...");
	selectDlg->show();
	return true;
}

void help(bool b_useWin)
{
    v3d_msg("This is a super plugin that invokes other data io plugins. Developed by Hanchuan Peng, 2013-Dec", b_useWin);
}


