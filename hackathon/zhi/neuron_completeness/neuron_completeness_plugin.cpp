/* neuron_completeness_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_completeness_plugin.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include <fstream>

using namespace std;
Q_EXPORT_PLUGIN2(neuron_completeness, TestPlugin);
double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("eval")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("eval"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        NeuronTree nt = openDlg->nt;
        QList<NeuronSWC> sorted_neuron;
        SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);
        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        int first = 0;
        for(V3DLONG i=0; i<sorted_neuron.size();i++)
        {
            if(sorted_neuron.at(i).pn == -1)
            {
                if(i!=0)
                {
                    multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
                }
                first = i;

            }
            if(!map_type.count(sorted_neuron.at(i).type))
            {
                map_type[sorted_neuron.at(i).type] = 1;
            }
        }
        multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
        QList<double> dist;
        dist.push_back(0);
        QMultiMap<int, QList<NeuronSWC> >::iterator it = multi_neurons.end()-1;
        QList<NeuronSWC> largest_neuron = it.value();

        for (it = multi_neurons.end()-2; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double minD = minDist(largest_neuron,cur);
            dist.push_back(minD);
        }

        QString info_tree;
        int index = 1;
        for (it = multi_neurons.end()-1; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double cur_dist = dist.front(); dist.erase(dist.begin());
            double cur_per = 100*(double)cur.size()/(double)sorted_neuron.size();
            info_tree += QString("neuron-tree %1 : percentage is %2%,  distance is %3<br>").arg(index++).arg(QString::number(cur_per,'f',2)).arg(QString::number(cur_dist,'f',2));
        }
        QMessageBox infoBox;
        infoBox.setText("Neuron completeness:");
        QString info_type;
        for (QHash<int,int>::iterator it = map_type.begin(); it != map_type.end(); ++it)
        {
            info_type += info_type.size() ==0? QString("%1").arg(it.key()): QString(",%1").arg(it.key());
        }
        infoBox.setInformativeText(QString("<pre><font size='4'>"
                    "number of neuron-trees : %1<br>"
                    "%2<br>"
                    "number of types        : %3<br>"
                    "types:                 : %4</font></pre>")
                    .arg(multi_neurons.size())
                    .arg(info_tree.toStdString().c_str())
                    .arg(map_type.size())
                    .arg(info_type.toStdString().c_str()));

        infoBox.exec();
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2019-2-25"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

    if (func_name == tr("eval"))
	{
        NeuronTree nt = readSWC_file(QString(infiles[0]));
        QList<NeuronSWC> sorted_neuron;
        SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);
        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        int first = 0;
        for(V3DLONG i=0; i<sorted_neuron.size();i++)
        {
            if(sorted_neuron.at(i).pn == -1)
            {
                if(i!=0)
                {
                    multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
                }
                first = i;

            }
            if(!map_type.count(sorted_neuron.at(i).type))
            {
                map_type[sorted_neuron.at(i).type] = 1;
            }
        }
        multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
        QList<double> dist;
        dist.push_back(0);
        QMultiMap<int, QList<NeuronSWC> >::iterator it = multi_neurons.end()-1;
        QList<NeuronSWC> largest_neuron = it.value();

        for (it = multi_neurons.end()-2; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double minD = minDist(largest_neuron,cur);
            dist.push_back(minD);
        }

        QString info_tree;
        int index = 1;
        for (it = multi_neurons.end()-1; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double cur_dist = dist.front(); dist.erase(dist.begin());
            double cur_per = 100*(double)cur.size()/(double)sorted_neuron.size();
            info_tree += QString("neuron-tree %1 : percentage is %2%,  distance is %3<br>").arg(index++).arg(QString::number(cur_per,'f',2)).arg(QString::number(cur_dist,'f',2));
        }
        QString info_type;
        for (QHash<int,int>::iterator it = map_type.begin(); it != map_type.end(); ++it)
        {
            info_type += info_type.size() ==0? QString("%1").arg(it.key()): QString(",%1").arg(it.key());
        }
        v3d_msg(QString("\n\nnumber of neuron-trees : %1\n"
                    "%2\n"
                    "number of types        : %3\n"
                    "types:                 : %4\n")
                    .arg(multi_neurons.size())
                    .arg(info_tree.toStdString().c_str())
                    .arg(map_type.size())
                    .arg(info_type.toStdString().c_str()),0);

        if (output.size() == 1)
        {
            char *outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
            QString imagename = QFileInfo(QString::fromStdString(infiles[0])).baseName();
            ofstream myfile;
            myfile.open (outimg_file,ios::out | ios::app );
            myfile << imagename.toStdString().c_str()<<"\t"<<multi_neurons.size() << "\t" << map_type.size()<<endl;
            myfile.close();
        }
        return true;
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

double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2)
{
    double dis=MAX_DOUBLE;
    for(int i=0;i<neuron1.size();i++)
    {
        for(int j=0;j<neuron2.size();j++)
        {
            if(NTDIS(neuron1[i],neuron2[j])<dis)
                dis = NTDIS(neuron1[i],neuron2[j]);
        }
    }
    return dis;
}
