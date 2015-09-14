#ifndef MEDIAN_SWC_H
#define MEDIAN_SWC_H

#include <vector>
#include "basic_surf_objs.h"

using namespace std;

int median_swc(vector<NeuronTree> nt_list);
NeuronTree average_node_position( NeuronTree median_neuron,vector<NeuronTree> nt_list);


#endif // MEDIAN_SWC_H

