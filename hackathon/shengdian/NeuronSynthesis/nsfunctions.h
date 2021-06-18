#ifndef NSFUNCTIONS_H
#define NSFUNCTIONS_H
#include "basic_surf_objs.h"
#include <vector>
#include <iostream>
#include"Utility_function.h"
using namespace std;
NeuronTree tip_branch_pruning(NeuronTree nt, int in_thre=5);
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size=5);
NeuronTree to_topology_tree(NeuronTree nt);
NeuronTree reindexNT(NeuronTree nt);
NeuronTree three_bifurcation_processing(NeuronTree nt);
NeuronTree three_bifurcation_remove(NeuronTree nt,bool not_remove_just_label=false);
bool split_neuron_type(QString inswcpath,QString outpath,int saveESWC=0);
#endif // NSFUNCTIONS_H
