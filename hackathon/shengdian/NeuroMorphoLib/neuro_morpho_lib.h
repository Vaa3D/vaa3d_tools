#ifndef NEURO_MORPHO_LIB_H
#define NEURO_MORPHO_LIB_H
#include "basic_surf_objs.h"
#include <vector>
#include <iostream>
using namespace std;

template<typename T>
double dis(T node1, T node2){
    ///for computing of distance between two nodes
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    return sqrt(a);
}
template<typename T>
double angle_3d(T n1, T n2, T n3, T n4){
    ///for computing of angle between two 3d-vector
    T v1,v2;
    v1.x=n1.x-n2.x; v1.y=n1.y-n2.y; v1.z=n1.z-n2.z;
    v2.x=n3.x-n4.x; v2.y=n3.y-n4.y; v2.z=n3.z-n4.z;
    double v1_len=sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
    double v2_len=sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
    double angle_3d=(v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)/(v1_len*v2_len);
    //angle_3d=(angle_3d>0)?angle_3d:(-1.0)*angle_3d;
    return (angle_3d);
}

struct BranchUnit
{
    /*tail node: tip or branch nodes
     * head node: branch or soma nodes
            *  parent_id=-1
       * ###(enhanced_features of left and rigth child-branch)
                    * lclength,lcpathLength,rclength,rcpathLength
       * ###(enhanced_features of left and right subtree)
                    * lslength,lspathLength,rslength,rspathLength
       * ###(enhanced_features of left and rigth subtree tips)
                    *lstips,rstips
        *PS: left has a big tip number value than right.
    */
    V3DLONG id; V3DLONG parent_id;
    int type,level;
    //For a branching point, local or remote angle of two child branches, if tip branch, angle=0
    double angle,angle_remote;
     /*For a branching point, angle of input-branch and output-branch
      * io1: input-branch with left-branch
      * io2: input-branch with right-branch
    */
    double angle_io1,angle_io1_remote,angle_io2,angle_io2_remote;
    /*in-branch-radius, left-branch-radius and right-branch-rdius*/
    double radius,lcradius,rcradius;
    /*in-branch-length, left-branch-length and right-branch-length*/
    double length,pathLength;
    double lclength,lcpathLength,rclength,rcpathLength;
    double lslength,lspathLength,rslength,rspathLength;
    /*left-branch-tips and right-branch-tips*/
    uint lstips,rstips;
     /*list of nodes of a branch: index=0 is the head (branching) node*/
    QList <NeuronSWC> listNode;
    QHash <int, int>  hashNode;
    BranchUnit() {
        id=0;parent_id=0;
        angle=angle_remote=angle_io2=angle_io1=angle_io1_remote=angle_io2_remote=0.0;
        type=level=0;
        radius=lcradius=rcradius=0.0;
        length=pathLength=0.0;
        lclength=lcpathLength=rclength=rcpathLength=0.0;
        lslength=lspathLength=rslength=rspathLength=0.0;
        lstips=rstips=0;
        listNode.clear();hashNode.clear();
    }
    void get_features();
    void get_radius();
    void radius_smooth(int half_win=2);
    void normalize_tip(V3DLONG scale_num){this->lstips/=scale_num;this->rstips/=scale_num;}
    void normalize_len(double scale_len){
        if(scale_len)
        {
            this->length/=scale_len;
            this->lclength/=scale_len;
            this->lslength/=scale_len;
            this->rclength/=scale_len;
            this->rslength/=scale_len;
        }
    }
    void normalize_pathlen(double scale_len){
        if(scale_len)
        {
            this->pathLength/=scale_len;
            this->lcpathLength/=scale_len;
            this->rcpathLength/=scale_len;
            this->lspathLength/=scale_len;
            this->rspathLength/=scale_len;
        }
    }
};
struct BranchSequence
{
    // branch sequence contains a list of branches from soma-stem to tip-branch
    QList<V3DLONG> listbr; //index of listBranch, not BranchUnit id
    int seqSize,seqLength,seqPathLength,seqType;
    BranchSequence() {
        seqSize=seqLength=seqPathLength=seqType=0;
        listbr.clear();
    }
};
struct BranchTree
{
    bool initialized; //a flag for indicating that branchtree is being initialized.
    //global features, add at 2021-06-29
    double total_length, total_path_length; //total_length is branch-line-length; total_path_length is actual length of neuron tree
    V3DLONG tip_branches,total_branches,soma_branches,max_branch_level;

    QList<BranchUnit> listBranch;
    QHash <V3DLONG, int>  hashBranch;
    NeuronTree nt;
    QList<BranchSequence> branchseq;
    BranchTree() {
        listBranch.clear();hashBranch.clear();
        initialized=false;
        total_length=total_path_length=0.0;
        tip_branches=soma_branches=total_branches=max_branch_level=0;
    }
    bool init(NeuronTree in_nt);
    bool init_branch_sequence(); //from tip-branch to soma-branch
    bool to_soma_br_seq(V3DLONG inbr_index,BranchSequence & brs);
    bool get_enhacedFeatures();
    vector< vector<V3DLONG> > get_branch_child_index();
    bool get_branch_child_angle();
    bool get_branch_angle_io();
    void get_globalFeatures();
    vector<int> getBranchType();
    bool normalize_branchTree();
    QList<V3DLONG> getSubtreeBranches(V3DLONG inbr_index=0);//input is index of listBranch, not branch id
};
void scale_nt_radius(NeuronTree& nt,float rs=1.0);
void scale_nt_coor(NeuronTree& nt,float scale_xy=0.3,float scale_z=1.0);
//NeuronTree branchTree_to_neurontree(const BranchTree& bt);
BranchTree readBranchTree_file(const QString& filename);
bool writeBranchTree_file(const QString& filename, const BranchTree& bt,bool enhanced=false);
bool writeBranchSequence_file(const QString& filename, const BranchTree& bt,bool enhanced=false);

NeuronTree to_topology_tree(NeuronTree nt);
bool getNodeOrder(NeuronTree nt,vector<int> & norder);
std::vector<int> getNodeType(NeuronTree nt);
NeuronTree reindexNT(NeuronTree nt);
double getNT_len(NeuronTree nt,float *res);
NeuronTree tip_branch_pruning(NeuronTree nt, float in_thre=2.0);
NeuronTree duplicated_tip_branch_pruning(NeuronTree nt,float dist_thre=20);
bool loop_checking(NeuronTree nt);
bool three_bifurcation_processing(NeuronTree& in_nt);
V3DLONG get_soma(NeuronTree& nt,bool connect=false);
NeuronTree node_interpolation(NeuronTree nt,int Min_Interpolation_Pixels=4,bool sort_index=false);
NeuronTree internode_pruning(NeuronTree nt,float pruning_dist=2.0,bool profiled=false);
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size=5);


double seg_median(std::vector<double> input);
#endif // NEURO_MORPHO_LIB_H
