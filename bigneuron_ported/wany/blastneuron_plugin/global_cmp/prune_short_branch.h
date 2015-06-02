#ifndef __PRUNE_SHORT_BRANCH__
#define __PRUNE_SHORT_BRANCH__
#include "basic_surf_objs.h"
#include <QtGlobal>
#include <vector>
using namespace std;
bool prune_branch(NeuronTree nt, NeuronTree & result);
bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches);
#endif
