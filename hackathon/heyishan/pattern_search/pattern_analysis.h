#ifndef PATTERN_ANALYSIS_H
#define PATTERN_ANALYSIS_H

#include "v3d_message.h"
#include "basic_surf_objs.h"
#include <v3d_interface.h>
#include <vector>

using namespace std;

enum{ALIVE=1, FAR=0};

struct StackElem
{
    V3DLONG id;
    V3DLONG len;
    StackElem()
    {
        id=0;
        len=0;
    }
    StackElem(V3DLONG _id,V3DLONG _len)
    {
        id=_id;
        len=_len;
    }
};

bool pattern_analysis(const NeuronTree &nt,const NeuronTree &boundary,vector<NeuronTree> & pt_list, vector<int> & pt_lens,V3DPluginCallback2 &callback);

#endif // PATTERN_ANALYSIS_H
