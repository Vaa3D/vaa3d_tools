#ifndef NEURONBRANCHTREE_H
#define NEURONBRANCHTREE_H
#include "basic_surf_objs.h"
#include <vector>
#include "v_neuronswc.h"
#include <iostream>
using namespace std;
template<typename T>
double dis(T node1, T node2){
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    return sqrt(a);
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
    double length,pathLength;
    double lclength,lcpathLength,rclength,rcpathLength;
    double lslength,lspathLength,rslength,rspathLength;
    uint lstips,rstips;
    QList <NeuronSWC> listNode;
    QHash <int, int>  hashNode;
    BranchUnit() {
        id=0;parent_id=0;
        type=level=0;
        length=pathLength=0.0;
        lclength=lcpathLength=rclength=rcpathLength=0.0;
        lslength=lspathLength=rslength=rspathLength=0.0;
        lstips=rstips=0;
        listNode.clear();hashNode.clear();
    }
    void get_features();
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
    bool get_enhacedFeatures();
    void get_globalFeatures();
    vector<int> getBranchType();
    bool normalize_branchTree();
    QList<V3DLONG> getSubtreeBranches(V3DLONG inbr_index=0);//input is index of listBranch, not branch id
};
BranchTree readBranchTree_file(const QString& filename);
bool writeBranchTree_file(const QString& filename, const BranchTree& bt,bool enhanced=false);
bool writeBranchSequence_file(const QString& filename, const BranchTree& bt,bool enhanced=false);
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

#endif // NEURONBRANCHTREE_H
