#ifndef CALLPLUGIN_H
#define CALLPLUGIN_H

#include "v3d_interface.h"
#include <vector>
#include "SpLog.h"

using std::vector;

class CallPlugin{
public:
    CallPlugin();
    CallPlugin(V3DPluginCallback2 &callback,QString plugin,QString func);
    ~CallPlugin(){}
    void Warning(QString warninginfo);
    void AssignPara(vector<char*> *para,int no);
    void RunPlugin();
    bool getReady();

    vector<char*> *getOutput();
    int type=0;/* 0 for preprocess, 1 for trace, 2 for postprocess,3 for predict*/
private:
    V3DPluginCallback2 *_callback;
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Input, InputParam, Output;
    vector<char*> pluginInputArgList;
    bool AllowUI=false;
    bool ready=false;
    bool hasinput=false;
    bool hasinputpara=false;
    bool hasoutput=false;
    QString PluginName;
    QString FunctionName;
};

#endif // CALLPLUGIN_H
