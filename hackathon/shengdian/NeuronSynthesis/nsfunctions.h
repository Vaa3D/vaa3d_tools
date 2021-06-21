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
    */
    V3DLONG id; V3DLONG parent_id;
    int type,level;
    double length,pathLength;
    QList <NeuronSWC> listNode;
    QHash <int, int>  hashNode;
    BranchUnit() {
        id=0;parent_id=0;
        type=level=0;
        length=pathLength=0.0;
        listNode.clear();hashNode.clear();
    }
    void get_features();
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
    QList<BranchUnit> listBranch;
    QHash <V3DLONG, int>  hashBranch;
    NeuronTree nt;
    QList<BranchSequence> branchseq;
    BranchTree() {
        listBranch.clear();hashBranch.clear();
    }
    bool init(NeuronTree in_nt);
    bool init_branch_sequence(); //from tip-branch to soma-branch
    vector<int> getBranchType();
};
NeuronTree branchTree_to_neurontree(const BranchTree& bt);
BranchTree readBranchTree_file(const QString& filename);
bool writeBranchTree_file(const QString& filename, const BranchTree& bt);
bool writeBranchSequence_file(const QString& filename, const BranchTree& bt);
NeuronTree tip_branch_pruning(NeuronTree nt, int in_thre=5);
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size=5);
NeuronTree to_topology_tree(NeuronTree nt);
NeuronTree reindexNT(NeuronTree nt);
NeuronTree three_bifurcation_processing(NeuronTree nt);
NeuronTree three_bifurcation_remove(NeuronTree nt,bool not_remove_just_label=false);
bool split_neuron_type(QString inswcpath,QString outpath,int saveESWC=0);
#endif // NSFUNCTIONS_H
