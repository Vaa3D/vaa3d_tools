/* check_sorted_swc_plugin.cpp
 * Check if the swc file is sorted or not
 * 2013-09-23 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "check_sorted_swc_plugin.h"
#include "openSWCDialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(check_sorted_swc, check_sorted_swc);
 
void check_menu(V3DPluginCallback2 &callback, QWidget *parent);

QStringList check_sorted_swc::menulist() const
{
	return QStringList() 
		<<tr("check_swc")
		<<tr("about");
}

QStringList check_sorted_swc::funclist() const
{
	return QStringList()
		<<tr("check_swc")
		<<tr("help");
}

void check_sorted_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("check_swc"))
	{
        check_menu(callback,parent);
	}
	else
	{
		v3d_msg(tr("Check if the swc file is sorted or not. "
			"Developed by Zhi Zhou, 2013-09-23"));
	}
}

bool check_sorted_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("check_swc"))
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

void check_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    callback.getImageWindowList();

    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;
    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;

    if(neuron.at(0).pn != -1)
    {
        v3d_msg("the root is not sorted!");
         return;
    }

    for (V3DLONG i=1;i<neuron.size();i++)
    {

        if(neuron.at(i).n -neuron.at(i).pn < 1)
        {
            v3d_msg("the swc file is not sorted!");
            return;
        }
    }

    v3d_msg("the swc file is sorted!");
    return;
}
