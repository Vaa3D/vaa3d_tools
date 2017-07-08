#ifndef GET_SUBTREES_H
#define GET_SUBTREES_H

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
using namespace std;
bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, int boundary_length, vector<vector<V3DLONG> >p_to_tree);


#endif // GET_SUBTREES_H
