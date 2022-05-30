#include "Computation.h"

void Computation::vn2(DataFlow *Comdatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputswc;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int i=0;i<inputarg.size();i++){
        pluginInputArgList.push_back(inputarg[i]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Comdatamem->outputimg[i]);

    vector<NeuronTree *> outputswc;              //output imgs
    NeuronTree *otswc=new NeuronTree();
    outputswc.push_back(otswc);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputswc.type="Outputimg";
    Outputswc.p = (void*)(&outputswc);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputswc);

    qDebug()<<"app2 start";
    qDebug()<<mcallback->callPluginFunc("vn21.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"app2 end";

    //qDebug()<<otswc;
    Comdatamem->outputswc[i]=otswc;
    //qDebug()<<"swc count: "<<Comdatamem->getswc_cnt();
}
