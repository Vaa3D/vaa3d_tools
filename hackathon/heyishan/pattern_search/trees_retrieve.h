#ifndef TREES_RETRIEVE_H
#define TREES_RETRIEVE_H

#include "v3d_message.h"
#include "basic_surf_objs.h"
#include <vector>
using namespace std;
bool trees_retrieve(vector<NeuronTree> & sub_trees, const NeuronTree & pt_consensus,vector<V3DLONG> &selected_trees);

#endif // TREES_RETRIEVE_H
