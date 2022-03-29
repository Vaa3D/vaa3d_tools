#ifndef UTILITY_FUNCTION_H
#define UTILITY_FUNCTION_H
#include "basic_surf_objs.h"
#include <vector>
#include <iostream>
using namespace std;
template<typename T>
double dis(T node1, T node2){
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    return sqrt(a);
}
struct Branch{
    NeuronSWC head_point,end_point;
    Branch* parent;
    int level;
    Branch():level(0),head_point(),end_point()
    {
        parent=0;
    }
    ~Branch(){}
    double get_branch_len(NeuronTree nt);
    bool get_points_of_branch(vector<int> &points,NeuronTree &nt);
    bool get_r_points_of_branch(vector<int> &r_points,NeuronTree &nt);
};
struct SwcTree{
    vector<Branch> branchs;
    NeuronTree nt;
    SwcTree():nt()
    {
        branchs.clear();
    }
    bool initialize(NeuronTree t);
    bool get_level_index(vector<int> &level_index,int level);
    int get_max_level();
};
V3DLONG get_soma(NeuronTree & nt,bool connect=false);
bool getNodeType(NeuronTree nt,vector<int> & ntype);
bool getNodeTips(NeuronTree nt,int &qtips,V3DLONG qindex=0);
bool getNodeOrder(NeuronTree nt,vector<int> & norder);
bool axon_main_path(NeuronTree nt, NeuronTree& nt_out,double len_thre,int tip_thre);
NeuronTree getSubtree(NeuronTree nt,V3DLONG nodeid);
NeuronTree reindexNT(NeuronTree nt);
bool max_arbor_single_tree(NeuronTree nt,NeuronTree& nt_out);
bool get_axonarbor(NeuronTree nt, NeuronTree& nt_out,int arbor_type=6,float r_thre=0.8);
bool arbor_topo(NeuronTree in_nt,NeuronTree& nt_out);
bool get_arbor(NeuronTree nt, NeuronTree& nt_out,int arbor_type=2);
NeuronTree pruning_subtree(NeuronTree nt, int pruning_thre=100);
bool pruning_subtree(NeuronTree nt, NeuronTree& nt_out,float tip_thre);
double get_nt_len(NeuronTree nt);
vector<double> get_node_subtree_len(NeuronTree nt,int normalized_size=0);
vector<double> get_node_subtree_len_v1(NeuronTree nt,int normalized_size=0,double axonRatio=1.0, double otherR=1.0);
void get_node_subtree_tips_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<int> & n_subtree_tips,V3DLONG iter_id);
void get_node_subtree_len_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<double> & n_subtree_len,V3DLONG iter_id);
#endif // UTILITY_FUNCTION_H
