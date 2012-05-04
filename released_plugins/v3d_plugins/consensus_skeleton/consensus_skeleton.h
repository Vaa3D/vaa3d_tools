#ifndef __CONSENSUS_SKELETON_H_
#define __CONSENSUS_SKELETON_H_

#include <vector>
#include "basic_surf_objs.h"
using namespace std;

bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, V3DLONG n_sampling, int method_code);

#endif

