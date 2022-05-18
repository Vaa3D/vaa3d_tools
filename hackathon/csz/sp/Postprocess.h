#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "v3d_interface.h"
#include <QString>
#include <vector>
using std::vector;

class Postprocess{
public:
    Postprocess(V3DPluginCallback2 *callback){
        mcallback=callback;
    }
    void resample_swc(DataFlow *Postdatamem,vector<char *> inputarg,int i,char * funcname);
    void sort_neuron_swc(DataFlow *Postdatamem,vector<char *> inputarg,int i,char * funcname);
private:
    V3DPluginCallback2 *mcallback;

};

#endif // POSTPROCESS_H
