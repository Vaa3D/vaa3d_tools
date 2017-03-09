#include "overlap/overlap_gold.h"
#include "pre_overlap.h"
bool pre_overlap(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    // no parameter
    if(inlist->size()!=2)
    {
        printf("need two input./n");
        return false;
    }
    QString fileOpenName = QString(inlist->at(0));
    QString fileSaveName;
    QString raw_img = inlist->at(1);
    if (output.size()==0)
    {
        printf("No outputfile specified.\n");
        fileSaveName = fileOpenName + "_trim_.swc";
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

    NeuronTree neuron = readSWC_file(fileOpenName);
    QList<NeuronSWC> select_result;
    // select middle_points of alignment
    for(int j = 1; j < neuron.listNeuron.size();  j+=2)
    {
        neuron.listNeuron[j].x = (neuron.listNeuron[j-1].x +neuron.listNeuron[j].x)/2;
        neuron.listNeuron[j].y = (neuron.listNeuron[j-1].y +neuron.listNeuron[j].y)/2;
        neuron.listNeuron[j].z = (neuron.listNeuron[j-1].z +neuron.listNeuron[j].z)/2;
        select_result.push_back(neuron.listNeuron[j]);
    }
    for(int i=0;i< select_result.size();i++)
    {
        select_result[i].pn = -1;
    }
    vector<MyMarker*> select_mm = nt2mm(select_result,fileSaveName);
    if(!evaluate_radius(select_mm,raw_img,callback))
    {
        printf("evaluate_radius failed\n");
    }
    NeuronTree trim_nt = mm2nt(select_mm,fileSaveName);
    vector<MyMarker*> trim_mm = nt2mm(trim_nt.listNeuron,fileSaveName);
    vector<MyMarker*> trim_result;
    if(!inter_node_pruning(trim_nt,trim_mm,trim_result))
    {
        printf("inter_node_pruning failed\n");
        return false;
    }
    saveSWC_file(fileSaveName.toStdString(), trim_result);
    return true;

}
