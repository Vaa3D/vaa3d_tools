#include "Postprocess.h"

void Postprocess::resample_swc(DataFlow *Postdatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputswc, InputParam, Outputswc;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int i=0;i<inputarg.size();i++){
        pluginInputArgList.push_back(inputarg[i]);
    }

    vector<NeuronTree *> inputswc;               //input imgs
    inputswc.push_back(Postdatamem->outputswc[i]);

    vector<NeuronTree *> outputswc;              //output imgs
    NeuronTree *otswc=new NeuronTree();
    outputswc.push_back(otswc);

    Inputswc.type="Inputswc";
    Inputswc.p = (void*)(&inputswc);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputswc.type="Outputswc";
    Outputswc.p = (void*)(&outputswc);

    pluginInputList.push_back(Inputswc);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputswc);

    qDebug()<<"resample_swc start";
    qDebug()<<mcallback->callPluginFunc("resample_swc1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"resample_swc end";

    Postdatamem->outputswc[i]=otswc;
}

void Postprocess::sort_neuron_swc(DataFlow *Postdatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputswc, InputParam, Outputswc;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int i=0;i<inputarg.size();i++){
        pluginInputArgList.push_back(inputarg[i]);
    }

    vector<NeuronTree *> inputswc;               //input imgs
    inputswc.push_back(Postdatamem->outputswc[i]);

    vector<NeuronTree *> outputswc;              //output imgs
    NeuronTree *otswc=new NeuronTree();
    outputswc.push_back(otswc);

    Inputswc.type="Inputswc";
    Inputswc.p = (void*)(&inputswc);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputswc.type="Outputswc";
    Outputswc.p = (void*)(&outputswc);

    pluginInputList.push_back(Inputswc);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputswc);

    qDebug()<<"sort_neuron_swc start";
    qDebug()<<mcallback->callPluginFunc("sort_neuron_swc1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"sort_neuron_swc end";

    Postdatamem->outputswc[i]=otswc;
}
