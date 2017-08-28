#ifndef GET_SUBTREES_H
#define GET_SUBTREES_H

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "pattern_analysis.h"
using namespace std;

bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, double boundary_length, int pt_num, vector<vector<V3DLONG> > &p_to_tree);
Boundary getBoundary(const NeuronTree &nt);

#endif // GET_SUBTREES_H
