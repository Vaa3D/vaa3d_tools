/* soma_sorting_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-1-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_sorting_swc_plugin.h"
#include <iostream>
#include <boost/lexical_cast.hpp>


using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
    return a*a;

}
Q_EXPORT_PLUGIN2(soma_sorting_swc, soma_sorting);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, int soma_ID)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    for (V3DLONG i=0;i<lN.size();i++)
    {
        if(lN.at(i).n == soma_ID)
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
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2016-1-28"));
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
        cout<<"Welcome to soma sorting plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need the gold standard swc file"<<endl;
            return false;
        }

        QString  gsswc_file =  infiles[0];
        QString  inputswc_file = infiles[1];
        if(inputswc_file.isEmpty())
        {
            cerr<<"Need the input swc file"<<endl;
            return false;
        }

        if (inparas.size()!=1)
        {
            printf("Please specify only one parameter - the step size for resampling.\n");
            return false;
        }
        double dis_th = atof(inparas.at(0));
        string S_th = boost::lexical_cast<string>(dis_th);
        char* C_th = new char[S_th.length() + 1];
        strcpy(C_th,S_th.c_str());

        double sort_th = dis_th * 2; //the parameter in sort_neuron_swc plugin, specifying the length threshold to bridge the gap 
        string S_sort_th = boost::lexical_cast<string>(sort_th);
        char* C_sort_th = new char[S_sort_th.length() + 1];
        strcpy(C_sort_th,S_sort_th.c_str());

        QString  outswc_file =  outfiles[0];
        cout<<"gsswc_file = "<<gsswc_file.toStdString().c_str()<<endl;
        cout<<"inswc_file = "<<inputswc_file.toStdString().c_str()<<endl;
        cout<<"distance threshold = "<<dis_th<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        V3DPluginArgItem arg;
        V3DPluginArgList input_resample;
        V3DPluginArgList input_sort;
        V3DPluginArgList output;

        arg.type = "random";std::vector<char*> arg_input_resample;
        std:: string inputfile_Qstring(inputswc_file.toStdString());char* inputfile_string =  new char[inputfile_Qstring.length() + 1]; strcpy(inputfile_string, inputfile_Qstring.c_str());
        arg_input_resample.push_back(inputfile_string);
        arg.p = (void *) & arg_input_resample; input_resample<< arg;
        arg.type = "random";std::vector<char*> arg_resample_para; arg_resample_para.push_back(C_th);arg.p = (void *) & arg_resample_para; input_resample << arg;
        std:: string outputfile_Qstring(outswc_file.toStdString());char* outputfile_string =  new char[outputfile_Qstring.length() + 1]; strcpy(outputfile_string, outputfile_Qstring.c_str());
        arg_input_resample.push_back(outputfile_string);
        arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(outputfile_string); arg.p = (void *) & arg_output; output<< arg;

        QString full_plugin_name_resample = "resample_swc";
        QString func_name_resample = "resample_swc";
        callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);

        arg.type = "random";std::vector<char*> arg_input_sort;
        arg_input_sort.push_back(outputfile_string);
        arg.p = (void *) & arg_input_sort; input_sort<< arg;
        arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back(C_sort_th);arg.p = (void *) & arg_sort_para; input_sort << arg;
        QString full_plugin_name_sort = "sort_neuron_swc";
        QString func_name_sort = "sort_swc";
        callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

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


        NeuronTree nt = readSWC_file(outswc_file);
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
        for (V3DLONG i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                int parent_tip = getParent(i,nt);
                if(parent_tip == 1000000000)
                    flag[i] = -1;
                else if(getParent(parent_tip,nt) == 1000000000 && childs[parent_tip].size() ==1)
                {
                    flag[i] = -1;
                    flag[parent_tip] = -1;
                }
            }
        }

        //Prune small segments
        NeuronTree nt_pruned;
        QList <NeuronSWC> listNeuron;
        QHash <int, int>  hashNeuron;
        listNeuron.clear();
        hashNeuron.clear();
        //set node
        NeuronSWC S;
        for (V3DLONG i=0;i<list.size();i++)
        {
            if(flag[i] == 1)
            {
                NeuronSWC curr = list.at(i);
                S.n 	= curr.n;
                S.type 	= curr.type;
                S.x 	= curr.x;
                S.y 	= curr.y;
                S.z 	= curr.z;
                S.r 	= curr.r;
                S.pn 	= curr.pn;
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
            }
        }
        nt_pruned.n = -1;
        nt_pruned.on = true;
        nt_pruned.listNeuron = listNeuron;
        nt_pruned.hashNeuron = hashNeuron;
        if(flag) {delete[] flag; flag = 0;}

        QVector<QVector<V3DLONG> > childs_prunned;

        V3DLONG neuronNum_prunned = nt_pruned.listNeuron.size();
        childs_prunned = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i=0;i<neuronNum_prunned;i++)
        {
            V3DLONG par = nt_pruned.listNeuron[i].pn;
            if (par<0) continue;
            childs_prunned[nt_pruned.hashNeuron.value(par)].push_back(i);
        }

        double Dist = 10000000000;
        double Dist_inrange = 10000000000;
        V3DLONG soma_ID = -1;
        V3DLONG dist_ID = -1;
        int child_num = 0;
        QList<NeuronSWC> list_prunned = nt_pruned.listNeuron;

        // set the distance threshold to searching for matching soma node

        double search_distance_th = soma_r*5 ;
        if (search_distance_th < sort_th *5)
         {
             search_distance_th = sort_th *5;
         }

        for (V3DLONG i=0;i<list_prunned.size();i++)
        {
            NeuronSWC curr = list_prunned.at(i);
            double nodedist = sqrt(pow2(curr.x - soma_x) + pow2(curr.y - soma_y) + pow2(curr.z - soma_z));
            if(nodedist <= search_distance_th && curr.pn <0)
            {
                soma_ID = curr.n;
                child_num = 1;
                break;
            }

            if(nodedist <= search_distance_th  && childs_prunned[i].size() > child_num)
            {
                soma_ID = curr.n;
                child_num = childs_prunned[i].size();
                Dist_inrange = nodedist;
            }

            if(nodedist <= search_distance_th && childs_prunned[i].size() == child_num && nodedist < Dist_inrange)
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

        export_list2file(nt_pruned.listNeuron,outswc_file,soma_ID);

        string S_soma_ID = boost::lexical_cast<string>(soma_ID);
        char* C_soma_ID = new char[S_soma_ID.length() + 1];
        strcpy(C_soma_ID,S_soma_ID.c_str());
        arg_sort_para.push_back(C_soma_ID);arg.p = (void *) & arg_sort_para; input_sort << arg;
        callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

	}
	else if (func_name == tr("help"))
	{
        cout << "This plugin is used to post-processing auto reconstructions for comparisons.  It will identify the soma for each input reconstruction" <<endl;
        cout << "based on the gold standard SWC file, and resample them according to specified stepsize and sort SWC nodes based on the soma root while bridging" <<endl;
        cout << " all disconneted components when the gap is less then  2*stepsize. The search range for the maching soma  is  max(5* soma_radius, 10* stepsize)." <<endl;
        cout<<"Usage : <vaa3d> -x soma_sorting_swc -f soma_sorting -i <gsswc_file> <inswc_file> -o <outswc_file> -p <step_size>"<<endl;

        cout<<endl;
	}
	else return false;

	return true;
}

