/* soma_sorting_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-1-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_sorting_swc_plugin.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"


using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
    return a*a;

}
Q_EXPORT_PLUGIN2(soma_sorting_swc, soma_sorting);

bool export_list2file_retypeRoot(QList<NeuronSWC> & lN, QString fileSaveName, int soma_ID)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    for (V3DLONG i=0;i<lN.size();i++)
    {
        if(i == soma_ID)
            myfile << lN.at(i).n <<" " << "1" << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
        else
            myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
    }
    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};
 
QStringList soma_sorting::menulist() const
{
	return QStringList() 
		<<tr("soma_sorting")
		<<tr("about");
}

QStringList soma_sorting::funclist() const
{
	return QStringList()
		<<tr("soma_sorting")
		<<tr("help");
}

void soma_sorting::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("soma_sorting"))
	{
        v3d_msg("Only works on dofunc for now.");
	}
	else
	{
        v3d_msg(tr("Developed by Zhi Zhou, 2016-1-28"));
	}
}

bool soma_sorting::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("soma_sorting"))
	{
        cout<<"This is soma sorting plugin"<<endl;
        if(infiles.size() <2)
        {
            cerr<<"Need  both the gold standard swc file and the input swc file"<<endl;
            return false;
        }

        QString  gsswc_file =  infiles[0];
        QString  inputswc_file = infiles[1];
        if(inputswc_file.isEmpty() || gsswc_file.isEmpty())
        {
            cerr<<"please check both input swc files"<<endl;
            return false;
        }

        if (inparas.size()!=1)
        {
            printf("Please specify only one parameter - the step size for resampling.\n");
            return false;
        }


        double dis_th = atof(inparas.at(0));
        double sort_th = dis_th * 2; //the parameter in sort_neuron_swc plugin, specifying the length threshold to bridge the gap

        QString  outswc_file =  outfiles[0];

        cout<<"gsswc_file = "<<gsswc_file.toStdString().c_str()<<endl;
        cout<<"inswc_file = "<<inputswc_file.toStdString().c_str()<<endl;
        cout<<"distance threshold = "<<dis_th<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;


        NeuronTree nt_input = readSWC_file(inputswc_file);
        cout<<" Soma sorting: 1) resample"<<endl;
        NeuronTree nt_input_rs = resample(nt_input, dis_th);

//        NeuronTree nt_input_rs_sort;
//        if (!SortSWC(nt_input_rs.listNeuron, nt_input_rs_sort.listNeuron ,VOID, sort_th))
//        {
//            v3d_msg("fail to call swc sorting function.",0);
//            return false;
//        }
//        QHash <int, int>  hashNeuron;
//        hashNeuron.clear();
//        //set node
//        NeuronSWC S;
//        QList<NeuronSWC> list_neurons = nt_input_rs_sort.listNeuron;
//        for (V3DLONG i=0;i<list_neurons.size();i++)
//        {
//            NeuronSWC curr = list_neurons.at(i);
//            S.n 	= curr.n;
//            S.type 	= curr.type;
//            S.x 	= curr.x;
//            S.y 	= curr.y;
//            S.z 	= curr.z;
//            S.r 	= curr.r;
//            S.pn 	= curr.pn;
//            hashNeuron.insert(S.n, i);
//        }
//        nt_input_rs_sort.hashNeuron = hashNeuron;

        NeuronTree nt_gs = readSWC_file(gsswc_file);
        double soma_x, soma_y, soma_z, soma_r;
        for (V3DLONG i = 0; i < nt_gs.listNeuron.size(); i++)
        {
            if(nt_gs.listNeuron[i].pn<0)
            {
                soma_x = nt_gs.listNeuron.at(i).x;
                soma_y = nt_gs.listNeuron.at(i).y;
                soma_z = nt_gs.listNeuron.at(i).z;
                soma_r = nt_gs.listNeuron.at(i).r;
                break;
            }
        }

        V3DLONG neuronNum = nt_input_rs.listNeuron.size();
        QVector<QVector<V3DLONG> > children_list = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i = 0; i < neuronNum; i++)
        {
            V3DLONG parent = nt_input_rs.listNeuron[i].pn;
            if (parent < 0)
                continue;
            children_list[nt_input_rs.hashNeuron.value(parent)].push_back(i);
        }


//        QVector<QVector<V3DLONG> > childs;

//        V3DLONG neuronNum = nt.listNeuron.size();
//        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
//        V3DLONG *flag = new V3DLONG[neuronNum];

//        for (V3DLONG i=0;i<neuronNum;i++)
//        {
//            flag[i] = 1;
//            V3DLONG par = nt.listNeuron[i].pn;
//            if (par<0) continue;
//            childs[nt.hashNeuron.value(par)].push_back(i);
//        }



//        QList<NeuronSWC> list = nt.listNeuron;
//        for (V3DLONG i=0;i<list.size();i++)
//        {
//            if (childs[i].size()==0)
//            {
//                int parent_tip = getParent(i,nt);
//                if(parent_tip == 1000000000)
//                    flag[i] = -1;
//                else if(getParent(parent_tip,nt) == 1000000000 && childs[parent_tip].size() ==1)
//                {
//                    flag[i] = -1;
//                    flag[parent_tip] = -1;
//                }
//            }
//        }

//        //Prune small segments
//        cout<<" Soma sorting: 2) prune small segments"<<endl;
//        NeuronTree nt_pruned;
//        QList <NeuronSWC> listNeuron;
//        QHash <int, int>  hashNeuron;
//        listNeuron.clear();
//        hashNeuron.clear();
//        //set node
//        NeuronSWC S;
//        for (V3DLONG i=0;i<list.size();i++)
//        {
//            if(flag[i] == 1)
//            {
//                NeuronSWC curr = list.at(i);
//                S.n 	= curr.n;
//                S.type 	= curr.type;
//                S.x 	= curr.x;
//                S.y 	= curr.y;
//                S.z 	= curr.z;
//                S.r 	= curr.r;
//                S.pn 	= curr.pn;
//                listNeuron.append(S);
//                hashNeuron.insert(S.n, listNeuron.size()-1);
//            }
//        }
//        nt_pruned.n = -1;
//        nt_pruned.on = true;
//        nt_pruned.listNeuron = listNeuron;
//        nt_pruned.hashNeuron = hashNeuron;
//        if(flag) {delete[] flag; flag = 0;}



//        QVector<QVector<V3DLONG> > children_list;

//        V3DLONG neuronNum_prunned = nt_pruned.listNeuron.size();
//        children_list = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

//        for (V3DLONG i=0;i<neuronNum_prunned;i++)
//        {
//            V3DLONG par = nt_pruned.listNeuron[i].pn;
//            if (par<0) continue;
//            children_list[nt_pruned.hashNeuron.value(par)].push_back(i);
//        }

        double Dist = 10000000000;
        double Dist_inrange = 10000000000;
        V3DLONG soma_ID = -1;
        V3DLONG dist_ID = -1;
        int child_num = 0;


//        if (list_prunned.size() == 0){
//                cout << "empty tree after prunning!" <<endl;
//                return false;
//        }

        cout<<" Soma sorting: 2) matching soma roots"<<endl;
       // set the distance threshold to searching for matching soma node

        QList<NeuronSWC> list_neurons = nt_input_rs.listNeuron;
        double search_distance_th = soma_r*5 ;
        if (search_distance_th < sort_th *5)
         {
             search_distance_th = sort_th *5;
         }

        for (V3DLONG i=0;i<list_neurons.size();i++)
        {
            NeuronSWC curr = list_neurons.at(i);
            double nodedist = sqrt(pow2(curr.x - soma_x) + pow2(curr.y - soma_y) + pow2(curr.z - soma_z));
            if(nodedist <= search_distance_th && curr.pn <0)
            {
                soma_ID = curr.n;
                child_num = 1;
                break;
            }

            if(nodedist <= search_distance_th  && children_list[i].size() > child_num)
            {
                soma_ID = curr.n;
                child_num = children_list[i].size();
                Dist_inrange = nodedist;
            }

            if(nodedist <= search_distance_th && children_list[i].size() == child_num && nodedist < Dist_inrange)
            {
                soma_ID = curr.n;
                Dist_inrange = nodedist;
            }

            if(nodedist < Dist)
            {
                dist_ID = curr.n;
                Dist = nodedist;
            }
        }

        if(child_num < 1 || soma_ID == -1) soma_ID = dist_ID;


        cout<<" Soma sorting: 3) sort"<<endl;
        QList<NeuronSWC> result;
        if (!SortSWC(nt_input_rs.listNeuron, result ,soma_ID, sort_th))
        {
            v3d_msg("fail to call swc sorting function.",0);
            return false;
        }

        //assign soma type
        cout<<" Soma sorting: 4) assign some type and save the result"<<endl;

        export_list2file_retypeRoot(result,outswc_file,0);
	}
	else if (func_name == tr("help"))
	{
        cout << "This super-plugin (calls resample, and sort plugin)  is used to post-processing auto reconstructions for comparisons.  It will identify the soma for each input reconstruction" <<endl;
        cout << "based on the gold standard SWC file, and resample them according to specified stepsize and sort SWC nodes based on the soma root while bridging" <<endl;
        cout << " all disconneted components when the gap is less then 2*stepsize. The search range for the maching soma  is  max(5* soma_radius, 10* stepsize)." <<endl;
        cout<<"Usage : <vaa3d> -x soma_sorting_swc -f soma_sorting -i <gsswc_file> <inswc_file> -o <outswc_file> -p <step_size>"<<endl;

        cout<<endl;
	}
	else return false;

	return true;
}

