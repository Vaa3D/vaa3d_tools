/* standardize_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-1-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "standardize_swc_plugin.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include <climits>

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
    return a*a;

}
Q_EXPORT_PLUGIN2(standardize_swc, standardize);

bool export_list2file_retype(QList<NeuronSWC> & lN, QString fileSaveName, int soma_ID, int new_type)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile << "#n,type,x,y,z,radius,parent"<< "\n";
    for (V3DLONG i=0;i<lN.size();i++)
    {
        if(i == soma_ID){
            myfile << lN.at(i).n <<" " << "1" << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
        }
        else{
            //retype types
            int t = new_type;
            if ( new_type == -1) //use the original type
                 t = lN.at(i).type;
            myfile << lN.at(i).n <<" " << t << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";
          }
    }
    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};
 

QStringList standardize::menulist() const
{
	return QStringList() 
        <<tr("standardize")
		<<tr("about");
}

QStringList standardize::funclist() const
{
	return QStringList()
        <<tr("standardize")
		<<tr("help");
}

void standardize::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("standardize"))
	{
        v3d_msg("Only works on dofunc for now.");
	}
	else
	{
        v3d_msg(tr("Developed by Xiaoxiao Liu, 2016-5-28"));
	}
}

bool standardize::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("standardize"))
	{
        cout<<"This is standardize_swc plugin"<<endl;
        if(infiles.size() <2)
        {
            cerr<<"Need  both the reference swc file ( to seed the soma) and the input swc file."<<endl;
            cerr<<"You can use the input swc file as the reference swc file, if no particular soma location is required." <<endl;
            return false;
        }

        QString  ref_swc_file =  infiles[0];
        QString  inputswc_file = infiles[1];
        if(inputswc_file.isEmpty() || ref_swc_file.isEmpty())
        {
            cerr<<"please check both input swc files"<<endl;
            return false;
        }

        if (inparas.size() <1)
        {
            printf("Please specify two parameters - 1)the gap size for bridging during the sorting step; 2) the type to be assgined.\n");
            return false;
        }


        double dis_th = atof(inparas.at(0));


        int type = -1;//use the old type
        if (inparas.size() ==2)
           type =  atof(inparas.at(1));

        double sort_th = dis_th ; //the parameter in sort_neuron_swc plugin, specifying the length threshold to bridge the gap

        QString  outswc_file =  outfiles[0];

        cout<<"ref_swc_file = "<<ref_swc_file.toStdString().c_str()<<endl;
        cout<<"inswc_file = "<<inputswc_file.toStdString().c_str()<<endl;
        cout<<"gap threshold = "<<dis_th<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;
        cout<<"new type code = "<<type<<endl;

        NeuronTree nt_input = readSWC_file(inputswc_file);
//        cout<<" Standardize: 1) resample"<<endl;
//        NeuronTree nt_input_rs = resample(nt_input, dis_th);

        NeuronTree nt_ref = readSWC_file(ref_swc_file);
        double soma_x, soma_y, soma_z, soma_r;
        for (V3DLONG i = 0; i < nt_ref.listNeuron.size(); i++)
        {
            if(nt_ref.listNeuron[i].pn<0)
            {
                soma_x = nt_ref.listNeuron.at(i).x;
                soma_y = nt_ref.listNeuron.at(i).y;
                soma_z = nt_ref.listNeuron.at(i).z;
                soma_r = nt_ref.listNeuron.at(i).r;
                break;
            }
        }

        V3DLONG neuronNum = nt_input.listNeuron.size();
        QVector<QVector<V3DLONG> > children_list = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i = 0; i < neuronNum; i++)
        {
            V3DLONG parent = nt_input.listNeuron[i].pn;
            if (parent < 0)
                continue;
            children_list[nt_input.hashNeuron.value(parent)].push_back(i);
        }

        double Dist = INT_MAX;
        double Dist_inrange = INT_MAX;
        V3DLONG soma_ID = -1;
        V3DLONG dist_ID = -1;
        int child_num = 0;

        cout<<" standardize: 1) matching soma roots"<<endl;
       // set the distance threshold to searching for matching soma node

        QList<NeuronSWC> list_neurons = nt_input.listNeuron;
        double search_distance_th = soma_r * 5 ;// choose the bigger value between soma_r*5 and search_distance_th*5
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


        cout<<" Standardize: 2) sort"<<endl;
        QList<NeuronSWC> result;
        if (!SortSWC(nt_input.listNeuron, result ,soma_ID, sort_th))
        {
            v3d_msg("fail to call swc sorting function.",0);
            return false;
        }

        cout<<" Standardize: 3) only keep the first tree(identified by the soma from the reference."<<endl;

        //skip the first root
        for (V3DLONG i = 1;i<result.size();i++)
        {
            if (result[i].pn == -1)
            {//remove other trees, only keep the first one
                cout<< "remove "<<result.size()-i<<" nodes"<<endl;
                result.erase(result.begin()+i, result.end());

            }
        }


        cout<<" Standardize: 4) reset type for soma and neurites"<<endl;
        //1-soma
        //2-axon
        //3-dendrite
        //4-apical dendrite

        export_list2file_retype(result,outswc_file,0,type);
	}
	else if (func_name == tr("help"))
	{
        cout << "This super-plugin  is used to post-processing auto reconstructions for comparisons. "<<endl;
        cout << "It will identify the soma for each input reconstruction " <<endl;
        cout << "based on the reference SWC file, and sort SWC nodes based on the soma root while bridging" <<endl;
        cout << "all disconneted components when the gap is less then the specified gap_threshold. The search range "<<endl;
        cout << "for the matching soma  is  5*soma_radius." <<endl;
        cout << "Usage : <vaa3d> -x standardize -f standardize -i <reference_swc_file> <inswc_file> -o <outswc_file> -p <gap_threshold>  <new_neurite_type> "<<endl;
	}
    else
        return false;

	return true;
}

