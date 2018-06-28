/* segment_profiling_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-6-27 : by LXF
 */
 
#include "v3d_message.h"
#include "openSWCDialog.h"
#include <vector>
#include "segment_profiling_plugin.h"
#include "segment_profiling_main.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
using namespace std;
Q_EXPORT_PLUGIN2(segment_profiling, segment_profilingPlugin);
 
QStringList segment_profilingPlugin::menulist() const
{
	return QStringList() 
        <<tr("segment_profiling")
		<<tr("menu2")
		<<tr("about");
}

QStringList segment_profilingPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void segment_profilingPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("segment_profiling"))
	{

        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);

        if (!openDlg->exec())
            return;
        NeuronTree nt,nt_p;
        nt_p = openDlg->nt;

        QString filename = openDlg->file_name;
        cout<<filename.toStdString()<<endl;
        SortSWC(nt_p.listNeuron,nt.listNeuron,nt_p.listNeuron[0].n,100000000);
        qDebug() <<"check2"<<endl;


        segment_profiling_main(callback,nt,filename);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-6-27"));
	}
}

bool segment_profilingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
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

