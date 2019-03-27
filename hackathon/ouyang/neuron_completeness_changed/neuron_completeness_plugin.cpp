/* neuron_completeness_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include "neuron_completeness_plugin.h"
#include "neuron_completeness_funcs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
#include <fstream>
#include "neuron_format_converter.h"
#include <iostream>


using namespace std;
Q_EXPORT_PLUGIN2(neuron_completeness, TestPlugin);
#define VOID 1000000000




QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("evalu")
        <<tr("batch_eval")
        <<tr("eval_terafly")
        <<tr("loop_detection(test)")
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

        LandmarkList markerlist;
        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        QList<double> dist;
        exportComplete(nt,sorted_neuron,markerlist,multi_neurons,map_type,dist);


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

    }else if (menu_name == tr("batch_eval"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc/eswc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);


        QStringList SWCList = importSWCFileList(m_InputfolderName);
        QList<NEURON_METRICS> scores;
        for(int i=0; i<SWCList.size();i++)
        {
            NeuronTree nt = readSWC_file(SWCList[i]);
            calComplete(nt,scores);
        }

        QString outputName =  m_InputfolderName + "/eval.csv";
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save CSV File"),
                outputName,
                QObject::tr("Supported file (*.csv)"));

        for(int i=0; i<SWCList.size();i++)
        {
            ofstream myfile;
            myfile.open (fileSaveName.toStdString().c_str(),ios::out | ios::app );
            myfile << SWCList[i].toStdString().c_str()<<","<<scores[i].numTrees << "," << scores[i].numTypes<<","<<scores[i].numSegs<<endl;
            myfile.close();

        }

    }else if (menu_name == tr("eval_terafly"))
    {
        NeuronTree nt = callback.getSWCTeraFly();
        if(nt.listNeuron.size()==0) return;
        QList<NeuronSWC> sorted_neuron;
        LandmarkList markerlist_gap;
        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        QList<double> dist;

        exportComplete(nt,sorted_neuron,markerlist_gap,multi_neurons,map_type,dist);

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

        QList<NeuronSWC> ori_tree1swc=nt.listNeuron;
        QVector<QVector<V3DLONG> > childs;
        NeuronTree n_t;
        QHash <int, int> hash_nt ;
        for(V3DLONG j=0; j<sorted_neuron.size();j++){
            hash_nt.insert(sorted_neuron[j].n, j);
            }
            n_t.listNeuron=sorted_neuron;
            n_t.hashNeuron=hash_nt;

            V3DLONG neuronNum = n_t.listNeuron.size();
            childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
            for (V3DLONG i=0;i<neuronNum;i++)
                {
                    V3DLONG par = n_t.listNeuron[i].pn;
                    if (par<0) continue;
                    childs[n_t.hashNeuron.value(par)].push_back(i);
                }

         LandmarkList markerlist;int numofwrongtype=0,numofwrongplace=0;
         markerlist_before_sorting(ori_tree1swc,markerlist,numofwrongtype);
         markerlist_after_sorting(sorted_neuron,markerlist,childs,numofwrongplace);

         if(markerlist_gap.size() != 0)
         {
               LandmarkList markerlist_orginal = callback.getLandmarkTeraFly();
               for(int i=0; i<markerlist_gap.size(); i++)
                   markerlist_orginal.push_back(markerlist_gap[i]);
               callback.setLandmarkTeraFly(markerlist_orginal);
         }

         if (markerlist.size()!=0)
         {
             printf("You have [%d] white markers in your file.[%d] nodes are not type 1,2,3 or 4.[%d] nodes are with wrong type(still 1,2,3 or 4),please check!",markerlist.size(),numofwrongtype,numofwrongplace);
             QMessageBox change_type;
             change_type.setWindowTitle("Wrong Type Detection");
             change_type.setText(QString("You have [%1] places need to be checked,do you want to set white markers in these locations?").arg(markerlist.size()));
             change_type.setStandardButtons(QMessageBox::Yes);
             change_type.addButton(QMessageBox::No);
             change_type.setDefaultButton(QMessageBox::No);
             if(change_type.exec() == QMessageBox::Yes){

                LandmarkList markerlist_orginal = callback.getLandmarkTeraFly();
                for(int i=0; i<markerlist.size(); i++) markerlist_orginal.push_back(markerlist[i]);
                callback.setLandmarkTeraFly(markerlist_orginal);
               }
         }
         else v3d_msg(QString("There are not any wrong type you need to check!"));

  }
    else if (menu_name == tr("loop_detection(test)"))
    {
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;
        NeuronTree nt = openDlg->nt;
        V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
		//vector<V_NeuronSWC_list> trees = showConnectedSegs(nt_decomposed);
		//std::cout << trees.size() << " trees in this SWC file." << endl;
		vector<NeuronSWC> errorPoints = loopDetection(nt_decomposed);
		

        v3d_msg(QString("%1").arg(nt_decomposed.seg.size()));


    }else
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

    if (func_name == tr("evalu"))
	{
        vector<QString> file_path;
        file_path=GetFileList(infiles[0]);

        for(int k=0;k<file_path.size();k++)
      {
        NeuronTree nt = readSWC_file(QString(file_path[k]));
        QList<NeuronSWC> sorted_neuron;
        LandmarkList markerlist1;
        QHash<int,int> map_type;
        QMultiMap<int, QList<NeuronSWC> > multi_neurons;
        QList<double> dist;

        exportComplete(nt,sorted_neuron,markerlist1,multi_neurons,map_type,dist);

        QStringList list=QString(file_path[k]).split("/");
        QString flag=list.last(); QStringList list1=flag.split(".");
        QString flag2=list1.first();
        QStringList list2=flag2.split("_");
        QString flag1=list2.first()+"\\_"+list2[1];//use \\ for adding \

        QString info_tree;
        vector<double> dis_all;
        int index = 1,index1=1,count=0;//bigger than 2 um

        QString out_result1 =QString(outfiles.at(0))+"/"+"gap"+QString(".txt");

        QDir tempdir1;
        QFile *tempfile1=new QFile;
        tempdir1.setCurrent(outfiles.at(0));

        for (QMultiMap<int, QList<NeuronSWC> >::iterator it = multi_neurons.end()-1; it != multi_neurons.begin()-1; --it)
        {
            QList<NeuronSWC> cur = it.value();
            double cur_dist = (dist.front() == MAX_DOUBLE)?0:dist.front(); dist.erase(dist.begin());
            double cur_per = 100*(double)cur.size()/(double)sorted_neuron.size();
            if(index<10 && cur_dist>=2) {info_tree += QString("neuron-tree %1 : percentage is %2%,  gap is %3<br>").arg(index++).arg(QString::number(cur_per,'f',2)).arg(QString::number(cur_dist,'f',2));
                    count++;dis_all.push_back(cur_dist);
            QString tree_name="tree"+QString("%1").arg(index1++);
                    if(!tempfile1->exists(out_result1))
                    {
                        tempfile1->setFileName(out_result1);
                        if (!tempfile1->open(QIODevice::WriteOnly|QIODevice::Text)) break;
                        QTextStream myfile(tempfile1);
                        //myfile <<"Name"<<"," <<"Gap distance"<<"," <<"Tree"<<endl;
                        myfile << flag1.toStdString().c_str()<<","<< cur_dist<<","<< tree_name<<endl;
                    }
                    else export_file2record_tree(flag1.toStdString(),cur_dist,out_result1,tree_name);
                   }
        }       
         tempfile1->close();

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

      }
      return true;
    }
    else if (func_name == tr("detection_type"))
    {
        detect_type_func(input,output,callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
