#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "basic_surf_objs.h"
#include <QtGlobal>
#include <vector>
#include <v3d_interface.h>

#include <iostream>


#define VOID 1000000000
#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define mean(a,b) (a+b)/2.0
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)

using namespace std;
void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
vector<V3DLONG> getTargetNode(const NeuronTree & nt, V3DLONG num);
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI);

bool prune_branch(NeuronTree nt, NeuronTree & result, double prune_size);
bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches);


QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT);
QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons);
void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined);
NeuronTree  sort(NeuronTree input, V3DLONG newrootid, double thres);
bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);

#endif // PREPROCESS_H

