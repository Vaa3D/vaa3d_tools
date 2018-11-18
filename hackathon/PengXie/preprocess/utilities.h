#ifndef UTILITIES_H
#define UTILITIES_H

#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
using namespace std;

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neuron_connector_func.h"
//#if !defined(Q_OS_WIN32)
//#include "unistd.h"
//#endif

//#if defined(Q_OS_WIN32)
//#include "getopt_tool.h"
//#include "io.h"
//#endif

bool fexists(QString filename);
CellAPO get_marker(NeuronSWC node, double vol, double color_r, double color_g, double color_b);
bool my_saveANO(QString fileNameHeader, bool swc=true, bool apo=true, QString swc_name=NULL);
NeuronTree rm_nodes(NeuronTree nt, QList<int> list);
int get_soma(NeuronTree nt);
QList<int> get_components(NeuronTree nt);
NeuronTree get_ith_component(NeuronTree nt, QList<int> components, int ith);
NeuronTree single_tree(NeuronTree nt, int soma);
NeuronTree get_subtree_by_id(NeuronTree nt, QList <int> idlist);
NeuronTree get_subtree_by_name(NeuronTree nt, QList <int> nlist);
NeuronTree color_subtree_by_id(NeuronTree nt, QList <int> idlist, int new_type=0);
NeuronTree color_components(NeuronTree nt, QList<int> components);
NeuronTree get_subtree_by_type(NeuronTree nt, int type, bool keep_soma=1);
NeuronTree missing_parent(NeuronTree nt);
bool check_duplicate(NeuronTree nt);
QList<int> get_tips(NeuronTree nt, bool include_soma);
double dist_to_parent(NeuronTree nt, int i, double xscale=1, double yscale=1, double zscale=5);

// Neuron operations
NeuronTree neuron_deepcopy(NeuronTree nt);
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist);
QList<NeuronSWC> neuronlist_cat(QList<NeuronSWC> nl1, QList<NeuronSWC> nl2);

bool whether_identical(NeuronTree nt1, NeuronTree nt2);

#endif // UTILITIES_H
