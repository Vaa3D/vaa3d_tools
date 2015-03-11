/* inter_node_pruning_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-09-03 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "inter_node_pruning_plugin.h"

#include "my_surf_objs.h"
using namespace std;
Q_EXPORT_PLUGIN2(inter_node_pruning, inter_node_pruning);
 
QStringList inter_node_pruning::menulist() const
{
	return QStringList() 
        <<tr("inter-node pruning")
		<<tr("about");
}

QStringList inter_node_pruning::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

template <class T> T pow2(T a)
{
    return a*a;

}

void inter_node_pruning::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("inter-node pruning"))
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
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            NeuronTree nt = readSWC_file(fileOpenName);
            if(nt.listNeuron[0].pn >0)
            {
                v3d_msg("Please sort the swc file first before using this plugin.");
                return;
            }
            QVector<QVector<V3DLONG> > childs;
            V3DLONG neuronNum = nt.listNeuron.size();
            childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
            for (V3DLONG i=0;i<neuronNum;i++)
            {
                V3DLONG par = nt.listNeuron[i].pn;
                if (par<0) continue;
                childs[nt.hashNeuron.value(par)].push_back(i);
            }

            vector<MyMarker*> final_out_swc = readSWC_file(fileOpenName.toStdString());
            vector<MyMarker*> final_out_swc_updated;
            final_out_swc_updated.push_back(final_out_swc[0]);


            for(int j = 1; j < final_out_swc.size(); j++)
            {
                int flag_prun = 0;
                int par_x = final_out_swc[j]->parent->x;
                int par_y = final_out_swc[j]->parent->y;
                int par_z = final_out_swc[j]->parent->z;
                int par_r = final_out_swc[j]->parent->radius;

                int dis_prun = sqrt(pow2(final_out_swc[j]->x - par_x) + pow2(final_out_swc[j]->y - par_y) + pow2(final_out_swc[j]->z - par_z));
                if( (final_out_swc[j]->radius + par_r - dis_prun)/dis_prun > 0.3)
                {
                    if(childs[j].size() > 0)
                    {
                        for(int jj = 0; jj < childs[j].size(); jj++)
                        final_out_swc[childs[j].at(jj)]->parent = final_out_swc[j]->parent;
                    }
                    flag_prun = 1;
                }

                if(flag_prun == 0)
                {
                   final_out_swc_updated.push_back(final_out_swc[j]);
                }
            }


            QString fileDefaultName = fileOpenName+QString("_pruned.swc");
            QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                    fileDefaultName,
                    QObject::tr("Supported file (*.swc)"
                        ";;Neuron structure	(*.swc)"
                        ));

            saveSWC_file(fileSaveName.toStdString(), final_out_swc_updated);
        }
	}
	else
	{
        v3d_msg(tr("This is a plugin to prun inter-nodes.. "
			"Developed by Zhi Zhou, 2014-09-03"));
	}
}

bool inter_node_pruning::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

