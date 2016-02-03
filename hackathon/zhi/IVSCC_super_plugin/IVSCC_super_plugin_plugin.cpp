/* IVSCC_super_plugin_plugin.cpp
 * 
 * 2016-2-3 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_super_plugin_plugin.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "sort_swc_fiji.h"

using namespace std;
Q_EXPORT_PLUGIN2(IVSCC_super_plugin, IVSCC_super_plugin);
 
QStringList IVSCC_super_plugin::menulist() const
{
	return QStringList() 
		<<tr("part1")
		<<tr("part2")
		<<tr("about");
}

QStringList IVSCC_super_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void IVSCC_super_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("part1"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;

        NeuronTree nt = readSWC_file(fileOpenName);
        NeuronTree nt_sort = SortSWC(nt.listNeuron,VOID, 0);
        NeuronTree nt_sort_rs = resample(nt_sort, 10);
        NeuronTree nt_sort_rs_sort = SortSWC(nt_sort_rs.listNeuron,VOID,0);
        NeuronTree nt_sort_rs_sort_prun = prunswc(nt_sort_rs_sort,5);
        NeuronTree nt_sort_rs_sort_prun_sort = SortSWC(nt_sort_rs_sort_prun.listNeuron,VOID,0);


        QString fileDefaultName = fileOpenName+QString("_IVSCC_part1.swc");
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        if (fileSaveName.isEmpty())
            return;
        if (!export_list2file(nt_sort_rs_sort_prun_sort.listNeuron,fileSaveName,fileOpenName))
        {
            v3d_msg("fail to write the output swc file.");
            return;
        }




	}
	else if (menu_name == tr("part2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
        v3d_msg(tr("Developed by Zhi Zhou, 2016-2-3"));
	}
}

bool IVSCC_super_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

