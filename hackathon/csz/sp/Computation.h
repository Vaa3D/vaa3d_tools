#ifndef COMPUTATION_H
#define COMPUTATION_H

#include "v3d_interface.h"
#include <QString>
#include <vector>
using std::vector;

class Computation{
public:
    Computation(V3DPluginCallback2 *callback){
        mcallback=callback;
    }
    void vn2(DataFlow *Comdatamem,vector<char *> inputarg,int i,char * funcname);
private:
    V3DPluginCallback2 *mcallback;

};
#endif // COMPUTATION_H
