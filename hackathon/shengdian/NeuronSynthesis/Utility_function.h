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
template<typename T>
double angle_3d(T n1, T n2, T n3, T n4){
    T v1,v2;
    v1.x=n1.x-n2.x; v1.y=n1.y-n2.y; v1.z=n1.z-n2.z;
    v2.x=n3.x-n4.x; v2.y=n3.y-n4.y; v2.z=n3.z-n4.z;
    double v1_len=sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
    double v2_len=sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
    double angle_3d=(v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)/(v1_len*v2_len);
    //angle_3d=(angle_3d>0)?angle_3d:(-1.0)*angle_3d;
    return (angle_3d);
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
/*for resample swc*/
#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
    V3DLONG level,seg_id;
    QList<float> fea_val;
};
typedef vector<Point*> Segment;
typedef vector<Point*> Tree;
void resample_path(Segment * seg, double step);
NeuronTree resample(NeuronTree input, double step);
//
vector<int> getNodeType(NeuronTree nt);
vector<int> getNodeTips(NeuronTree nt);
vector<int> getNodeOrder(NeuronTree nt);
NeuronTree getSubtree(NeuronTree nt,V3DLONG nodeid);
//NeuronTree reindexNT(NeuronTree nt);
NeuronTree pruning_subtree(NeuronTree nt, int pruning_thre=100);
double get_nt_len(NeuronTree nt);
vector<double> get_node_subtree_len(NeuronTree nt,int normalized_size=0);
vector<double> get_node_subtree_len_v1(NeuronTree nt,int normalized_size=0);
void get_node_subtree_tips_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<int> & n_subtree_tips,V3DLONG iter_id);
void get_node_subtree_len_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<double> & n_subtree_len,V3DLONG iter_id);
#endif // UTILITY_FUNCTION_H
