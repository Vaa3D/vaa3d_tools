#include "CallPlugin.h"

CallPlugin::CallPlugin()
{

}

CallPlugin::CallPlugin(V3DPluginCallback2 &callback, QString plugin, QString func)
{
    _callback=&callback;
    PluginName=plugin;
    FunctionName=func;
    ready=true;
}

void CallPlugin::Warning(QString warninginfo)
{
    if(AllowUI)
        v3d_msg(warninginfo);
    else
        qDebug()<<warninginfo;
}

void CallPlugin::AssignPara(std::vector<char *> *para, int no)
/*
 * 0-----for input files
 * 1-----for input parameters
 * 2-----for output files
*/
{
    bool issucceed=false;
    switch (no) {
    case 0://inputfile
        Input.type="Input";
        Input.p = (void*)(para);
        issucceed=true;
        hasinput=true;
        break;
    case 1://input para
        InputParam.type="InputParam";
        InputParam.p = (void*)(para);
        issucceed=true;
        hasinputpara=true;
        break;
    case 2://output file
        Output.type="Output";
        Output.p = (void*)(para);
        issucceed=true;
        hasoutput=true;
        break;
    }
    if(!issucceed)
        Warning("The parameter may have some mistake.");
}

void CallPlugin::RunPlugin()
{
    if(ready==true){
        pluginInputList.push_back(Input);
        pluginInputList.push_back(InputParam);
        pluginOutputList.push_back(Output);
        _callback->callPluginFunc(PluginName,FunctionName,pluginInputList,pluginOutputList);
        ready=false;
    }

}

bool CallPlugin::getReady()
{
    return ready;

}

vector<char *> *CallPlugin::getOutput()
{
    if(hasoutput){
        vector<char*> *output=(vector<char*> *)Output.p;
        return output;
    }
    return nullptr;
}
