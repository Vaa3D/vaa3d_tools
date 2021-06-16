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

vector<int> getNodeType(NeuronTree nt);
vector<int> getNodeTips(NeuronTree nt);
vector<int> getNodeOrder(NeuronTree nt);
NeuronTree getSubtree(NeuronTree nt,V3DLONG nodeid);
NeuronTree reindexNT(NeuronTree nt);
NeuronTree pruning_subtree(NeuronTree nt, int pruning_thre=100);
double get_nt_len(NeuronTree nt);
vector<double> get_node_subtree_len(NeuronTree nt,int normalized_size=0);
vector<double> get_node_subtree_len_v1(NeuronTree nt,int normalized_size=0);
void get_node_subtree_tips_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<int> & n_subtree_tips,V3DLONG iter_id);
void get_node_subtree_len_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<double> & n_subtree_len,V3DLONG iter_id);
#endif // UTILITY_FUNCTION_H
