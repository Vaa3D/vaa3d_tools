/* region_match_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-21 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>
#include "region_match_plugin.h"

#include "match_swc.h"
#include "make_consensus.h"
#include "neuron_utilities/sort_swc.h"
Q_EXPORT_PLUGIN2(region_match, region_match);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt_search;
    NeuronTree nt_pattern;
};

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);


QStringList region_match::menulist() const
{
	return QStringList() 
        <<tr("tracing_func")
		<<tr("about");
}

QStringList region_match::funclist() const
{
	return QStringList()
        <<tr("tracing_func")
		<<tr("help");
}

void region_match::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("tracing_func"))
	{
        bool bmenu = true;
        input_PARA PARA;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-6-21"));
	}
}

bool region_match::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

//        if(infiles.empty())
//        {
//            fprintf (stderr, "Need input image. \n");
//            return false;
//        }
//        else
//            PARA.inimg_file = infiles[0];
        int k=0;
//        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";k++;
        if(!inneuron_file.isEmpty())
            PARA.nt_search = readSWC_file(inneuron_file);
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        //QList<NeuronSWC> file_inmarkers;
        if(!inmarker_file.isEmpty())
            //file_inmarkers=readSWC_file(inmarker_file).listNeuron;
            PARA.nt_pattern=readSWC_file(inmarker_file);
        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of region_match tracing **** \n");
		printf("vaa3d -x region_match -f matching_func -i <inimg_file> -p <channel> <swc_file> <marker_file> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("swc_file         SWC file path.\n");
        printf("marker_file      Marker file path.\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");
        printf("outmarker_file   Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
//    unsigned char* data1d = 0;
//    V3DLONG N,M,P,sc,c;
//    V3DLONG in_sz[4];
    vector<NeuronTree> s_forest;
    NeuronTree s_mk;
    NeuronTree mk;
    vector<vector<V3DLONG> > p_to_cube;
    vector<V3DLONG> num_sorted;
    for(int i=0;i<2;i++) num_sorted.push_back(i);
    if(bmenu)
    {
        PARA.nt_search = readSWC_file("original_vr_neuron.swc");
        PARA.nt_pattern = readSWC_file("areaofinterest.swc");
        //substructure_retrieve(s_mk,
        //calculate_morph(PARA.nt_search,s_forest,s_mk,);
    }
    else
    {
        cout<<"In to dofunc."<<endl;
    }

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON MACHINE LEARNING CODE
    cout<<"******************This is main function*********************"<<endl;


    make_consensus(PARA.nt_search,PARA.nt_pattern,mk,callback);
    match_swc(PARA.nt_search,mk,s_mk,s_forest,p_to_cube);

    cout<<p_to_cube.size()<<endl;
    vector<V3DLONG> result_points;
    for(int i=0; i<num_sorted.size();i++)
    {
        cout<<"num_sorted_size="<<num_sorted.size()<<endl;
        V3DLONG id_cube=num_sorted[i];
        cout<<"id_cube="<<id_cube<<endl;
        vector<V3DLONG> ps_cube=p_to_cube[id_cube];
        cout<<"ps_cube_size="<<ps_cube.size()<<endl;
        for(int j=0;j<ps_cube.size();j++)
        {
            result_points.push_back(ps_cube[j]);
        }
    }
    cout<<"result_points.size="<<result_points.size()<<endl;
    vector<V3DLONG> result_points_set = result_points; // Could be better

    //QList <NeuronSWC> list_pattern = PARA.nt_pattern.listNeuron;
    QList <NeuronSWC> list_search = PARA.nt_search.listNeuron;

    for(V3DLONG i =0; i<result_points_set.size();i++)
    {
        V3DLONG id=result_points_set[i];
        list_search[id].type =2;
    }
    export_list2file(list_search,"updated_vr_neuron.swc","updated_vr_neuron.swc");


//    if(list_pattern.size()!=0 || list_search.size()!=0)
//    {
//        for(V3DLONG i = 0; i < list_search.size(); i++)
//        {
//            PARA.nt_search.listNeuron[i].type += 1;
//        }
//    }

//    //Output
//    writeSWC_file("updated_vr_neuron.swc",PARA.nt_search);

//    if(!bmenu)
//    {
//        if(data1d) {delete []data1d; data1d = 0;}
//    }

    //if(nt_output.listNeuron.size()>0) v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
    //if(marker_output.size()>0) v3d_msg(QString("Now you can drag and drop the generated marker fle [%1] into Vaa3D.").arg(marker_name.toStdString().c_str()),bmenu);


    return;
}













