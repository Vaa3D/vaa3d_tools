

/* resample_swc_func.cpp
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 */
#include"wrong_area_search_plugin.h"
#include "find_wrong_area.h"
#include <iostream>
//#include"openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include"sort_swc.h"
#include"resampling.h"
#include"local_aligment.h"


bool sort_with_standard(QList<NeuronSWC>  & neuron1, QList<NeuronSWC> & neuron2,QList<NeuronSWC>  &result)
{
    V3DLONG siz = neuron1.size();
    V3DLONG root_id = 1;
    double dist;
    if (siz==0) return false;
    double min_dist = sqrt((neuron1[0].x-neuron2[0].x)*(neuron1[0].x-neuron2[0].x)
         +(neuron1[0].y-neuron2[0].y)*(neuron1[0].y-neuron2[0].y)
         +(neuron1[0].z-neuron2[0].z)*(neuron1[0].z-neuron2[0].z));
    for(V3DLONG i=0; i<siz; i++)
    {
         dist = sqrt((neuron1[i].x-neuron2[0].x)*(neuron1[i].x-neuron2[0].x)
                +(neuron1[i].y-neuron2[0].y)*(neuron1[i].y-neuron2[0].y)
                +(neuron1[i].z-neuron2[0].z)*(neuron1[i].z-neuron2[0].z));
         if(min_dist > dist) {min_dist = dist; root_id = i+1;}
    }
    cout<<"min_dist = "<< min_dist <<endl;
    cout<<"root_id = " << root_id <<endl;

    //sort_swc process
    double thres = 10000;
    if(!SortSWC(neuron1,result,root_id,thres))
    {\
        cout<<"Error in sorting swc"<<endl;
        return false;
    }
    return true;
}

bool find_wrong_area(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    if(input.size() != 2)
    {
        printf("Please specify both input file and step length parameter.\n");
        return false;
    }
    paralist = (vector<char*>*)(input.at(1).p);
    if(paralist->size()!=1)
    {
        printf("Please specify only one parameter -  the resampling step length");
        return false;
    }
    double step = atof(paralist->at(0));
    QString fileOpenName = QString(inlist->at(0));
    QString fileSaveName;
    if (output.size()==0)
    {
        printf("No outputfile specified.\n");
        fileSaveName = fileOpenName + "_sort.swc";
    }
    else if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        fileSaveName = QString(outlist->at(0));
    }
    else
    {
        printf("You have specified more than 1 output file.\n");
        return false;
    }

    // resample input_1
    NeuronTree nt,temp_neuron;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        nt = readSWC_file(fileOpenName);
    NeuronTree resample_result = resample(nt,step);


    //  find the nearest point as new root node calculated between input_1's all node  and input_2's root node
    cout<<"find_roodID begin.";
    QList<NeuronSWC> neuron1,neuron2;

    neuron1 = resample_result.listNeuron;
    QString fileOpenName2 = QString(inlist->at(1));
    if (fileOpenName2.toUpper().endsWith(".SWC") || fileOpenName2.toUpper().endsWith(".ESWC"))
    {

        temp_neuron = readSWC_file(fileOpenName2);
        neuron2 = readSWC_file(fileOpenName2).listNeuron;
    }

    NeuronTree resample_gold = resample(temp_neuron,step);

    QList<NeuronSWC> result,result_2;
    sort_with_standard(neuron1,neuron2,result);
    sort_with_standard(resample_gold.listNeuron,neuron2,result_2);
    if (!export_list2file(result, fileSaveName, fileOpenName))
    {
        printf("fail to write the output swc file.\n");
        return false;
    }
    vector<MyMarker *> neuron_m;
    vector<MyMarker *> neuron_gold;
    neuron_m = nt2mm(result,fileSaveName);
    neuron_gold = nt2mm(result_2,fileOpenName2);
    string name1 = fileSaveName.toStdString();
    string name2 = fileOpenName2.toStdString();
    cout<<"name1 = "<<name1<<endl;
    cout<<"name2 = "<<name2<<endl;

    cout<<"neuron_m_size = "<<neuron_m.size()<<endl;
    cout<<"neuron_gold_size = "<<neuron_gold.size()<<endl;
    //cout<<"hello"<<endl;

    vector<map<MyMarker*, MyMarker*> > map_result;
    if (!neuron_mapping_dynamic(neuron_m, neuron_gold, map_result))
    {
        printf("error in neuron_mapping\n");
        return false;
    }
   string name_result =  "aligned_result.swc";
    vector<MyMarker*> map_swc;
    convert_matchmap_2swc(map_result, map_swc);
    saveSWC_file(name_result, map_swc);








    for (int i=0;i<neuron_m.size();i++)
        if (neuron_m[i]) {delete(neuron_m[i]); neuron_m[i]=NULL;}
    for (int i=0;i<neuron2.size();i++)
        if (neuron_gold[i]) {delete(neuron_gold[i]); neuron_gold[i]=NULL;}
    for (int i=0;i<map_swc.size();i++)
        if (map_swc[i]) {delete(map_swc[i]); map_swc[i]=NULL;}



    return true;


}


QList<NeuronSWC> choose_long_alignment(QList<NeuronSWC> &neuron,double thres)
{
    QList<NeuronSWC> result;
    V3DLONG siz = neuron.size();
    double dist;
    for(V3DLONG i=0; i<siz-1;i=i+2)
    {
        dist = sqrt((neuron[i].x-neuron[i+1].x)*(neuron[i].x-neuron[i+1].x)
                +(neuron[i].y-neuron[i+1].y)*(neuron[i].y-neuron[i+1].y)
                +(neuron[i].z-neuron[i+1].z)*(neuron[i].z-neuron[i+1].z));
        if(dist < 0 && dist > thres){result.push_back(neuron[i]);}
    }
    return result;
}
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName)
{
    QString tempSaveName = fileSaveName + "temp.swc";
    export_neuronList2file(inswc, tempSaveName);
    vector<MyMarker*> mm_out = readSWC_file(tempSaveName.toStdString());
    //const char * tempRemoveName = tempSaveName.toLatin1().data();
    QFile f;
   // if(remove(tempRemoveName))
    if( !f.remove(tempSaveName))
    {
        cout << "mm_temp file didn't remove."<< endl;
        perror("remove");
    }
    return mm_out;
}
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    // myfile<<"# generated by Vaa3D Plugin overlap_gold"<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}
