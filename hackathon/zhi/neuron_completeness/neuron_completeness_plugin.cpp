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

        LandmarkList markerlist;

        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        int first = 0;
        int cur_type = -1;
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
                if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3)
                {
                    LocationSimple t;
                    t.x = sorted_neuron.at(i).x;
                    t.y = sorted_neuron.at(i).y;
                    t.z = sorted_neuron.at(i).z;
                    t.color.r = 255;t.color.g = 0; t.color.b = 0;
                    markerlist.push_back(t);
                }
            }else
            {
                if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3 && sorted_neuron.at(i).type !=cur_type)
                {
                    LocationSimple t;
                    t.x = sorted_neuron.at(i).x;
                    t.y = sorted_neuron.at(i).y;
                    t.z = sorted_neuron.at(i).z;
                    t.color.r = 255;t.color.g = 0; t.color.b = 0;
                    markerlist.push_back(t);
                }
            }
            cur_type = sorted_neuron.at(i).type;
        }
        multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
        QList<double> dist;
        QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
        int row =0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
        {
            QList<NeuronSWC> neuron1= it1.value();
            double minD = MAX_DOUBLE;
            int col=0;
            for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
            {
                if(it1 != it2)
                {
                    if(matrix_dis[row][col] != MAX_DOUBLE)
                        minD = MIN(minD,matrix_dis[row][col]);
                    else
                    {
                        QList<NeuronSWC> neuron2 = it2.value();
                        double cur_dis = minDist(neuron1,neuron2);
                        matrix_dis[row][col] = cur_dis;
                        matrix_dis[col][row] = cur_dis;
                        minD = MIN(minD,cur_dis);
                    }
                }
                col++;
            }
            row++;
            dist.push_back(minD);
            if(minD>20 && minD!=MAX_DOUBLE)
            {
                LocationSimple t;
                t.x = neuron1.at(0).x;
                t.y = neuron1.at(0).y;
                t.z = neuron1.at(0).z;
                t.color.r = 0;t.color.g = 0; t.color.b = 255;
                markerlist.push_back(t);
            }
        }

        QVector<QPair <double, double> > v_tree;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it = multi_neurons.end()-1; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double cur_dist = (dist.front() == MAX_DOUBLE)?0:dist.front(); dist.erase(dist.begin());
            double cur_per = 100*(double)cur.size()/(double)sorted_neuron.size();
            v_tree.push_back(QPair <double, double>(cur_dist,cur_per));
        }

        qSort(v_tree.begin(),v_tree.end(),qGreater<QPair<double, double> >());
        QString info_tree;
        for (int i=0; i<MIN(v_tree.size(),15);i++)
        {
            info_tree += QString("neuron-tree %1 : percentage is %2%,  gap is %3<br>").arg(i+1).arg(QString::number(v_tree[i].second,'f',2)).arg(QString::number(v_tree[i].first,'f',2));
        }
        if(v_tree.size()>15)
            info_tree += ". . .\n";

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
        if(markerlist.size() != 0)
        {
            QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
            bool b_found=false;
            int i;
            for (i=0; i<cur_list_3dviewer.size(); i++)
            {
                QString curname = callback.getImageName(cur_list_3dviewer[i]).remove("3D View [").remove("]");
                if(curname == openDlg->file_name)
                {
                    b_found = true;
                    break;
                }
            }

            V3dR_MainWindow *cur_win;
            if(b_found)
                cur_win = cur_list_3dviewer[i];
            else
                cur_win = callback.open3DViewerForSingleSurfaceFile(openDlg->file_name);

            if(cur_win)
                callback.setHandleLandmarkList_Any3DViewer(cur_win,markerlist);
        }

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

        LandmarkList markerlist;

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
        QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
        int row =0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
        {
            QList<NeuronSWC> neuron1= it1.value();
            double minD = MAX_DOUBLE;
            int col=0;
            for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
            {
                if(it1 != it2)
                {
                    if(matrix_dis[row][col] != MAX_DOUBLE)
                        minD = MIN(minD,matrix_dis[row][col]);
                    else
                    {
                        QList<NeuronSWC> neuron2 = it2.value();
                        double cur_dis = minDist(neuron1,neuron2);
                        matrix_dis[row][col] = cur_dis;
                        matrix_dis[col][row] = cur_dis;
                        minD = MIN(minD,cur_dis);
                    }
                }
                col++;
            }
            row++;
            dist.push_back(minD);
            if(minD>20 && minD!=MAX_DOUBLE)
            {
                LocationSimple t;
                t.x = neuron1.at(0).x;
                t.y = neuron1.at(0).y;
                t.z = neuron1.at(0).z;
                t.color.r = 0;t.color.g = 0; t.color.b = 255;
                markerlist.push_back(t);
            }
        }

        QString info_tree;
        int index = 1;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it = multi_neurons.end()-1; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double cur_dist = (dist.front() == MAX_DOUBLE)?0:dist.front(); dist.erase(dist.begin());
            double cur_per = 100*(double)cur.size()/(double)sorted_neuron.size();
            if(index<10) info_tree += QString("neuron-tree %1 : percentage is %2%,  gap is %3<br>").arg(index++).arg(QString::number(cur_per,'f',2)).arg(QString::number(cur_dist,'f',2));
        }
        QMessageBox infoBox;
        infoBox.setText("Neuron completeness:");
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
            myfile << imagename.toStdString().c_str()<<"\t"<<multi_neurons.size() << "\t" << map_type.size()<<"\t"<<markerlist.size()<<endl;
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
