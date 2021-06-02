#ifndef NSFUNCTIONS_H
#define NSFUNCTIONS_H
#include "basic_surf_objs.h"
#include <vector>
#include <iostream>
using namespace std;
NeuronTree tip_branch_pruning(NeuronTree nt, int in_thre=5);
NeuronTree to_topology_tree(NeuronTree nt);
NeuronTree reindexNT(NeuronTree nt);
NeuronTree three_bifurcation_processing(NeuronTree nt);
bool split_neuron_type(QString inswcpath,QString outpath,int saveESWC=0);
#endif // NSFUNCTIONS_H
