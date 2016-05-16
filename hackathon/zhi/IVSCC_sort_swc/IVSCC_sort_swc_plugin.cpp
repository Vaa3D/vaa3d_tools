/* IVSCC_sort_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-5-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_sort_swc_plugin.h"
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "openSWCDialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(IVSCC_sort_swc, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("sort_swc")
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
    if (menu_name == tr("sort_swc"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        NeuronTree nt = openDlg->nt;
        QList<NeuronSWC> neuron = nt.listNeuron;

        swcSortDialog dialog(callback, parent,neuron);
        if (dialog.exec()!=QDialog::Accepted)
            return;


        V3DLONG rootid = dialog.rootid;
        V3DLONG thres = dialog.thres;

        if(rootid == 0)  rootid = VOID;
        if(thres == -1)  rootid = VOID;

        QString fileOpenName = openDlg->file_name;
        NeuronTree nt_sort = SortSWC_pipeline(neuron,rootid, thres);

        QString fileDefaultName = fileOpenName+QString("_sorted.swc");
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                            fileDefaultName,
                                                            QObject::tr("Supported file (*.swc)"
                                                                        ";;Neuron structure	(*.swc)"
                                                                        ));
        if (fileSaveName.isEmpty())
            return;
        if (!export_list2file(nt_sort.listNeuron,fileSaveName,fileOpenName))
        {
            v3d_msg("fail to write the output swc file.");
            return;
        }

	}
	else
	{
        v3d_msg(tr("This is a swc sorting plugin for IVSCC data at Allen Institute.. "
			"Developed by Zhi Zhou, 2016-5-16"));
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
	else return false;

	return true;
}

