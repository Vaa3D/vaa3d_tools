/* resample_swc_func.cpp
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 */
#include"blastneuron_bjut_func.h"
#include "resampling.h"
#include <iostream>
#include"openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include"sort_swc.h"
#include"pre-process.h"

bool sort_with_standard(QList<NeuronSWC>  & neuron1, QList<NeuronSWC> & neuron2,QList<NeuronSWC>  &result)
{
    V3DLONG siz = neuron1.size();
    V3DLONG root_id = 1;
    double dist;
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

bool resampling_main(const V3DPluginArgList & input, V3DPluginArgList & output)
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
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        nt = readSWC_file(fileOpenName);
    NeuronTree resample_result = resample(nt,step);
    
    //  find the nearest point as new root node calculated between input_1's all node  and input_2's root node
    cout<<"find_roodID begin.";
    QList<NeuronSWC> neuron1,neuron2;
    neuron1 = resample_result.listNeuron;
    QString fileOpenName2 = QString(inlist->at(1));
    if (fileOpenName2.toUpper().endsWith(".SWC") || fileOpenName2.toUpper().endsWith(".ESWC"))
        neuron2 = readSWC_file(fileOpenName2).listNeuron;

    QList<NeuronSWC> result;
    sort_with_standard(neuron1,neuron2,result);
    if (!export_list2file(result, fileSaveName, fileOpenName))
    {
        printf("fail to write the output swc file.\n");
        return false;
    }

    return true;
}
