/* IVSCC_process_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-10-23 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_process_swc_plugin.h"
#include "basic_surf_objs.h"
#include <boost/lexical_cast.hpp>

#include <iostream>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
    return a*a;

}

Q_EXPORT_PLUGIN2(IVSCC_process_swc, IVSCC_process_swc);
 
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n') <<endl;
        }

    }

    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

QStringList IVSCC_process_swc::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList IVSCC_process_swc::funclist() const
{
	return QStringList()
        <<tr("process")
        <<tr("process_v2")
		<<tr("help");
}

void IVSCC_process_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
	{
        v3d_msg(tr("This is a plugin only worked using command line to post process swc files in IVSCC pipeline. "
            "Developed by Zhi Zhou, 2014-10-23"));
	}
}

bool IVSCC_process_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, paras, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) paras = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("process"))
	{
        cout<<"Welcome to IVSCC swc post processing plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = "";
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }

        QString  outswc_file =  outfiles[0];
        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"inmarker_file = "<<inmarker_file.toStdString().c_str()<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        vector<MyMarker> file_inmarkers;
        file_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));

        double length = 15;
        double sum_x = 0, sum_y = 0;
        for(int i = 0; i < file_inmarkers.size(); i ++)
        {
            sum_x+= file_inmarkers[i].x;
            sum_y+= file_inmarkers[i].y;
        }

        double cent_x = sum_x/file_inmarkers.size();
        double cent_y = sum_y/file_inmarkers.size();

        double sum_r = 0;
        for(int i = 0; i < file_inmarkers.size(); i ++)
        {
            sum_r+= sqrt(pow2(file_inmarkers[i].x - cent_x) + pow2(file_inmarkers[i].y - cent_y));
        }

        double soma_r = sum_r/file_inmarkers.size();

        NeuronTree nt = readSWC_file(inswc_file);

        QVector<QVector<V3DLONG> > childs;


        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        V3DLONG *flag = new V3DLONG[neuronNum];

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 1;

            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;
        for (int i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                int index_tip = 0;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {

                    parent_tip = getParent(parent_tip,nt);
                    index_tip++;
                }
                if(index_tip < length)
                {
                    flag[i] = -1;

                    int parent_tip = getParent(i,nt);
                    while(childs[parent_tip].size()<2)
                    {
                        flag[parent_tip] = -1;
                        parent_tip = getParent(parent_tip,nt);
                    }
                }

            }

        }

       //NeutronTree structure
       NeuronTree nt_prunned;
       QList <NeuronSWC> listNeuron;
       QHash <int, int>  hashNeuron;
       listNeuron.clear();
       hashNeuron.clear();

       //set node
       NeuronSWC S;
       for (int i=0;i<list.size();i++)
       {
           if(flag[i] == 1)
           {
                NeuronSWC curr = list.at(i);
                S.n 	= curr.n;
                if(i ==0)
                {
                     S.type = 1;
                     S.r = soma_r;
                }
                else
                {
                    S.type 	= 3;
                    S.r 	= curr.r;
                }
                S.x 	= curr.x;
                S.y 	= curr.y;
                S.z 	= curr.z;
                S.pn 	= curr.pn;
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
           }

      }
       nt_prunned.n = -1;
       nt_prunned.on = true;
       nt_prunned.listNeuron = listNeuron;
       nt_prunned.hashNeuron = hashNeuron;

       if(flag) {delete[] flag; flag = 0;}

       export_list2file(nt_prunned.listNeuron,outswc_file,inswc_file);

       nt = nt_prunned;
       neuronNum = nt.listNeuron.size();
       childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
       for (V3DLONG i=0;i<neuronNum;i++)
       {
           V3DLONG par = nt.listNeuron[i].pn;
           if (par<0) continue;
           childs[nt.hashNeuron.value(par)].push_back(i);
       }

       vector<MyMarker*> final_out_swc = readSWC_file(outswc_file.toStdString());
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

       saveSWC_file(outswc_file.toStdString(), final_out_swc_updated);

    }
    else if (func_name == tr("process_v2"))
    {
        cout<<"Welcome to IVSCC swc post processing plugin v2"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        QString  inmarkerpath_file = infiles[1];
        if(inmarkerpath_file.isEmpty())
        {
            cerr<<"Need a marker path file"<<endl;
            return false;
        }

        QString inmarker_file = infiles[2];
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }

        QString  outswc_file =  outfiles[0];
        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"inmarkerpath_file = "<<inmarkerpath_file.toStdString().c_str()<<endl;
        cout<<"inmarker_file = "<<inmarker_file.toStdString().c_str()<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        vector<MyMarker> file_inmarkers;
        file_inmarkers = readMarker_file(string(qPrintable(inmarkerpath_file)));

        double sum_x = 0, sum_y = 0;
        for(int i = 0; i < file_inmarkers.size(); i ++)
        {
            sum_x+= file_inmarkers[i].x;
            sum_y+= file_inmarkers[i].y;
        }

        double cent_x = sum_x/file_inmarkers.size();
        double cent_y = sum_y/file_inmarkers.size();

        double sum_r = 0;
        for(int i = 0; i < file_inmarkers.size(); i ++)
        {
            sum_r+= sqrt(pow2(file_inmarkers[i].x - cent_x) + pow2(file_inmarkers[i].y - cent_y));
        }

        double soma_r = sum_r/file_inmarkers.size();


        vector<MyMarker> center_inmarkers;
        center_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));
        double soma_x = center_inmarkers[0].x;
        double soma_y = center_inmarkers[0].y;
        double soma_z = center_inmarkers[0].z;


        NeuronTree nt = readSWC_file(inswc_file);
        double Dist = 10000000000;
        V3DLONG soma_ID = -1;
        for (V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            double point_x = nt.listNeuron.at(i).x;
            double point_y = nt.listNeuron.at(i).y;
            double point_z = nt.listNeuron.at(i).z;
            if(sqrt(pow2(point_x - soma_x) + pow2(point_y - soma_y) + pow2(point_z - soma_z)) < Dist)
            {
                Dist = sqrt(pow2(point_x - soma_x) + pow2(point_y - soma_y) + pow2(point_z - soma_z));
                soma_ID = i;
            }
        }

        V3DPluginArgItem arg;
        V3DPluginArgList input_sort;
        V3DPluginArgList output;

        arg.type = "random";std::vector<char*> arg_input_sort;
        char* fileName_string = infiles[0];
        arg_input_sort.push_back(fileName_string);
        arg.p = (void *) & arg_input_sort; input_sort<< arg;
        arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("0");
        string S_soma_ID = boost::lexical_cast<string>(soma_ID);
        char* C_soma_ID = new char[S_soma_ID.length() + 1];
        strcpy(C_soma_ID,S_soma_ID.c_str());
        arg_sort_para.push_back(C_soma_ID);
        arg.p = (void *) & arg_sort_para; input_sort << arg;
        QString full_plugin_name_sort = "sort_neuron_swc";
        QString func_name_sort = "sort_swc";
        callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

        QString swcfilename_sorted = inswc_file + "_sorted.swc";
        vector<MyMarker*> temp_out_swc = readSWC_file(swcfilename_sorted.toStdString());
        for(V3DLONG i = 0; i < temp_out_swc.size(); i++)
        {
            if(i ==0)
            {
                temp_out_swc[i]->radius = soma_r;
                temp_out_swc[i]->type = 1;
            }
            else
                temp_out_swc[i]->type = 3;
        }
        saveSWC_file(outswc_file.toStdString(), temp_out_swc);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f process -i <inswc_folder> -o <outswc_file> -p <somapath_marker>"<<endl;
        cout<<endl;
    }
    else return false;;

	return true;
}

