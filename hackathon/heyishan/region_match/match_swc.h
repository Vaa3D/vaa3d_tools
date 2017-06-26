#ifndef MATCH_SWC_H
#define MATCH_SWC_H

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>

//#define VOID 1000000000
//#define PI 3.14159265359
//#define min(a,b) (a)<(b)?(a):(b)
//#define max(a,b) (a)>(b)?(a):(b)
//#define mean(a,b) (a+b)/2.0
//#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
//#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
//#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)

using namespace std;

void match_swc(NeuronTree &nt1, NeuronTree &nt2, NeuronTree &s_mk,  NeuronTree &s_mk_sorted, vector<NeuronTree> &s_forest,vector<vector<V3DLONG> > & p_to_cube);
void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
vector<V3DLONG> getTargetNode(const NeuronTree & nt, V3DLONG num);
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI);
#endif // MATCH_SWC_H
