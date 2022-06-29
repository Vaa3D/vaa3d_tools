#ifndef NSFUNCTIONS_H
#define NSFUNCTIONS_H
#include "basic_surf_objs.h"
#include <vector>
#include <iostream>
#include"Utility_function.h"
using namespace std;
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
    double angle;//angle of two child branches, if tip branch, angle=0
    double length,pathLength;
    double lclength,lcpathLength,rclength,rcpathLength;
    double lslength,lspathLength,rslength,rspathLength;
    uint lstips,rstips;
    QList <NeuronSWC> listNode;
    QHash <int, int>  hashNode;
    BranchUnit() {
        id=0;parent_id=0;
        angle=0;
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
    bool get_branch_child_angle();
    void get_globalFeatures();
    vector<int> getBranchType();
    bool normalize_branchTree();
    QList<V3DLONG> getSubtreeBranches(V3DLONG inbr_index=0);//input is index of listBranch, not branch id
};
void scale_swc(NeuronTree& nt,float scale_xy=1.0,float scale_z=0.3);
NeuronTree branchTree_to_neurontree(const BranchTree& bt);
BranchTree readBranchTree_file(const QString& filename);
bool writeBranchMotif_file(const QString& filename, const BranchTree& bt);
bool writeBranchTree_file(const QString& filename, const BranchTree& bt,bool enhanced=false);
bool writeBranchSequence_file(const QString& filename, const BranchTree& bt,bool enhanced=false);
NeuronTree tip_branch_pruning(NeuronTree nt, int in_thre=5);
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size=5);
NeuronTree to_topology_tree(NeuronTree nt);
NeuronTree reindexNT(NeuronTree nt);
NeuronTree three_bifurcation_processing(NeuronTree nt);
NeuronTree redundancy_bifurcation_pruning(NeuronTree nt,bool not_remove_just_label=false);
bool split_neuron_type(QString inswcpath,QString outpath,int saveESWC=0);
#endif // NSFUNCTIONS_H
