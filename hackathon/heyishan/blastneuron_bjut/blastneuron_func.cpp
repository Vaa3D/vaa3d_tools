#include "blastneuron_bjut_func.h"
#include "local_alignment.h"
#include <cstring>
using namespace std;

bool blastneuron_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    printf("\n This is blastneuron_main\n");

    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    string name_neuron1 = (QString(inlist->at(0))).toStdString();
    string name_neuron2 = (QString(inlist->at(1))).toStdString();
    string name_result;
    if (output.size()==0)
    {
        printf("No outputfile specified.\n");
        name_result = name_neuron1 +"_" + name_neuron2 + "_aligned.swc";
    }
    else if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        name_result = (QString(outlist->at(0)).toStdString());
    }

    vector<MyMarker *> neuron1, neuron2;
    neuron1 = readSWC_file(name_neuron1);
    neuron2 = readSWC_file(name_neuron2);
    vector<map<MyMarker*, MyMarker*> > map_result;

    if (!neuron_mapping_dynamic(neuron1, neuron2, map_result))
    {
        printf("error in neuron_mapping\n");
        return false;
    }

    vector<MyMarker*> map_swc;
    convert_matchmap_2swc(map_result, map_swc);
    saveSWC_file(name_result, map_swc);

    for (int i=0;i<neuron1.size();i++)
        if (neuron1[i]) {delete(neuron1[i]); neuron1[i]=NULL;}
    for (int i=0;i<neuron2.size();i++)
        if (neuron2[i]) {delete(neuron2[i]); neuron2[i]=NULL;}
    for (int i=0;i<map_swc.size();i++)
        if (map_swc[i]) {delete(map_swc[i]); map_swc[i]=NULL;}

    return 0;
}
