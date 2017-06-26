#ifndef MATCH_SWC_H
#define MATCH_SWC_H

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>

using namespace std;

void match_swc(NeuronTree &nt1, NeuronTree &nt2, NeuronTree &s_mk, vector<NeuronTree> &s_forest,vector<vector<V3DLONG> > & p_to_cube);
void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
vector<V3DLONG> getTargetNode(const NeuronTree & nt, V3DLONG num);
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI);
#endif // MATCH_SWC_H
