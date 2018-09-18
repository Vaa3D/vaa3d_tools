//connect_swc_redefined.cpp
//
//by Peng Xie
//2018-08-13

#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include "neuron_connector_func.h"
#include "utilities.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
using namespace std;

NeuronTree my_connectall(NeuronTree nt, double xscale, double yscale, double zscale,
                         double angThr, double disThr, int matchType, bool b_minusradius, int rootID){
    NeuronTree new_tree;
    QList<NeuronSWC> listneuron;
    connectall(&nt, listneuron, xscale, yscale, zscale, angThr, disThr, matchType, b_minusradius, rootID);
//    export_list2file(new_tree.listNeuron, "temp.swc");
//    NeuronTree new_tree_reload = readSWC_file("temp.swc");
//    whether_identical(new_tree, new_tree_reload);
    new_tree.deepCopy(neuronlist_2_neurontree(listneuron));
    return new_tree;
}


