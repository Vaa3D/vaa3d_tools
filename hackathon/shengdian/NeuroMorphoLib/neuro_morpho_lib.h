#ifndef NEURO_MORPHO_LIB_H
#define NEURO_MORPHO_LIB_H
#include "basic_surf_objs.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <iostream>
#include <cmath>

using namespace std;

#define PI 3.1415926
#define MINVALUE -9999999
#define MAXVALUE 9999999

template<typename T>
double dis(T node1, T node2, bool bouton_fea=false){
    ///for computing of distance between two nodes
    if(bouton_fea){
        node1.x=node1.fea_val.at(0);
        node1.y=node1.fea_val.at(1);
        node1.z=node1.fea_val.at(2);
        node2.x=node2.fea_val.at(0);
        node2.y=node2.fea_val.at(1);
        node2.z=node2.fea_val.at(2);
    }
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    if(a<=0)
        return double(0.0);
    return double(sqrt(a));
}
template<typename T>
double angle_3d(T n1, T n2, T n3, T n4,bool debug_falg=false){
    ///for computing of angle between two 3d-vector
    T v1,v2;
    v1.x=n1.x-n2.x; v1.y=n1.y-n2.y; v1.z=n1.z-n2.z;
    v2.x=n3.x-n4.x; v2.y=n3.y-n4.y; v2.z=n3.z-n4.z;
    double v1_len=sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
    double v2_len=sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
    if(v1_len==0||v2_len==0)
        return double(0.0);

    double angle_3d=(v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)/(v1_len*v2_len);
    if(debug_falg){
        cout<<"length="<<v1_len<<","<<v2_len<<endl;
        cout<<"angle="<<angle_3d<<",return="<<double(180)*acos(angle_3d)/PI<<endl;
    }
    if(angle_3d>=1)
        return double(0.0);
    //angle_3d=(angle_3d>0)?angle_3d:(-1.0)*angle_3d;
    return double(180)*acos(angle_3d)/PI;
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
    double width,height,depth;
    double angle_io1,angle_io1_remote,angle_io2,angle_io2_remote;
    /*in-branch-radius, left-branch-radius and right-branch-rdius*/
    double radius,lcradius,rcradius;
    /*in-branch-length, left-branch-length and right-branch-length*/
    double length,pathLength;
    double edist2soma,pdist2soma;
    double lclength,lcpathLength,rclength,rcpathLength;
    double lslength,lspathLength,rslength,rspathLength;
    /*left-branch-tips and right-branch-tips*/
    uint lstips,rstips;
     /*list of nodes of a branch: index=0 is the head (branching) node*/
    /*bouton features*/
    uint boutons,pboutons,lcboutons,rcboutons;
    double mean_spatial_neighbor_boutons,mean_MINdist2topo_bouton,mean_dist2parent_bouton,uniform_bouton_dist;

    QList <NeuronSWC> listNode;
    QHash <int, int>  hashNode;
    BranchUnit() {
        id=0;parent_id=0;
        width=height=depth=0.0;
        angle=angle_remote=angle_io2=angle_io1=angle_io1_remote=angle_io2_remote=double(0.0);
        type=level=0;
        radius=lcradius=rcradius=double(0.0);
        length=pathLength=double(0.0);
        edist2soma=pdist2soma=0.0;
        lclength=lcpathLength=rclength=rcpathLength=double(0.0);
        lslength=lspathLength=rslength=rspathLength=double(0.0);
        boutons=pboutons=lcboutons=rcboutons=0;
        mean_spatial_neighbor_boutons=mean_MINdist2topo_bouton=mean_dist2parent_bouton=uniform_bouton_dist=0.0;
        lstips=rstips=0;
        listNode.clear();hashNode.clear();
    }
    void get_features(bool bouton_fea=false);
    void bouton_features(int btype=4,int interb_dist_index=5,int spatial_nb_index=6,int topo_nb_dist_index=9);
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
    double seqLength,seqPathLength; //seqLength: total Euclidean length of branches
    int seqSize,seqType;
    BranchSequence() {
        seqSize=seqType=0;
        seqLength=seqPathLength=0.0;
        listbr.clear();
    }
};
struct BranchTree
{
    bool initialized; //a flag for indicating that branchtree is being initialized.
    //global features, add at 2021-06-29
    double total_length, total_path_length; //total_length is branch-line-length; total_path_length is actual length of neuron tree
    double width,height,depth,volume;
    V3DLONG tip_branches,total_branches,soma_branches,max_branch_level;

    QList<BranchUnit> listBranch;
    QHash <V3DLONG, int>  hashBranch;
    NeuronTree nt;
    QList<BranchSequence> branchseq;
    BranchTree() {
        listBranch.clear();hashBranch.clear();
        initialized=false;
        total_length=total_path_length=0.0;
        width=height=depth=volume=0.0;
        tip_branches=soma_branches=total_branches=max_branch_level=0;
    }
    bool init(NeuronTree in_nt,bool bouton_fea=false);
    bool init_branch_sequence(); //from tip-branch to soma-branch
    bool to_soma_br_seq(V3DLONG inbr_index,BranchSequence & brs);
    bool get_enhacedFeatures(bool bouton_fea=false);// child branch features
    vector< vector<V3DLONG> > get_branch_child_index();
    bool get_branch_child_angle();
    bool get_branch_angle_io();
    bool get_volsize();
    void get_globalFeatures(); //nt length,branches,tip_branches
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
NeuronSWC getBranchNearNode(BranchUnit bu, bool near_head=true,double min_dist=5.0);
NeuronTree to_topology_tree(NeuronTree nt);
bool getNodeOrder(NeuronTree nt,vector<int> & norder,V3DLONG somaid=-1);
bool getNodeType(NeuronTree nt,vector<int> & ntype,V3DLONG somaid=-1);
NeuronTree reindexNT(NeuronTree nt);
double getNT_len(NeuronTree nt,float *res);
NeuronTree tip_branch_pruning(NeuronTree nt, float in_thre=2.0);
NeuronTree duplicated_tip_branch_pruning(NeuronTree nt,float dist_thre=20);
bool loop_checking(NeuronTree nt);
bool multi_bifurcations_checking(NeuronTree nt,QList<CellAPO> & out_3bifs,V3DLONG somaid=-1);
bool three_bif_decompose(NeuronTree& in_nt,V3DLONG bif_child_id,V3DLONG somaid=-1);
vector<V3DLONG> child_node_indexes(NeuronTree in_nt, V3DLONG query_node_index=1);
bool multi_bifurcations_processing(NeuronTree& in_nt,V3DLONG somaid=-1);
//bool multi_bifurcation_processing(NeuronTree& in_nt,V3DLONG somaid=-1);
V3DLONG get_soma(NeuronTree& nt,bool connect=false);
QList<NeuronTree> nt_2_trees(NeuronTree nt);
NeuronTree node_interpolation(NeuronTree nt,int Min_Interpolation_Pixels=4,bool sort_index=false);
NeuronTree internode_pruning(NeuronTree nt,float pruning_dist=2.0,bool profiled=false);
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size=5);
double seg_median(std::vector<double> input);
double vector_max(std::vector<double> input);
double vector_mean(std::vector<double> input);
double vector_std(std::vector<double> input);
double vector_min(std::vector<double> input);
#endif // NEURO_MORPHO_LIB_H
