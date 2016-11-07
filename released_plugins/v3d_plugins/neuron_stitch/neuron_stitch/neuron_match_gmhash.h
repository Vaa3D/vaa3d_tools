#ifndef NEURO_MATCH_GMHASH_H
#define NEURO_MATCH_GMHASH_H

#include "neuron_stitch_func.h"
#include <v3d_interface.h>

#include <QDialog>
#include <vector>
#include <map>

using namespace std;

//temporary designed for testing geometric hashing's performance
//reference: geometric hashing: an overview
class neuron_match_gmhash
{

private:
    //1 always has larger coordinate than 0 in stacking direction
    //0 and 1 should have already been stacked in z direction and rescaled in z direction to make isotropic

    //neuron
    NeuronTree *nt0;
    NeuronTree *nt1;
    HBNeuronGraph ng0, ng1;

    //candidate point for matching
    QList<int> candID0, candID1; //neuron tree point ids of candidates
    QList<XYZ> candcoord0, candcoord1, candcoord1_adj; //cooridnate of candidates
    QList<int> pmatch0, pmatch1; //matched candidate, the index of neuron tree point
    QList<int> candmatch0, candmatch1; //matched candidate, the index of candidate in List

    //geometric hashing of nt 0
    QHash<QPair<int,int>, QHash<QPair<int,int>, QList<int> > > gmhash0;
    multimap<int, QPair<QPair<int,int>, QPair<int,int> > , std::greater<int> > rankedPairMatch;

public:
    //parameters
    double spanCand;  //searching span from the stack plane for the candidate
    double midplane;    //coordinate of the plane in between
    double pmatchThr;   //match threshold for points
    float minmatchrate; //percentage of point matched to be recorded
    unsigned short hashbin;     //bin size of each hash pixel (default 1)
    int trialNum;   //number of trials to try
    int direction; //0:x/1:y/2:z(default)
    double zscale;

    //transformation
    double shift_x, shift_y, shift_z, rotation_ang, rotation_cx, rotation_cy, rotation_cz;


public:
    neuron_match_gmhash(NeuronTree* botNeuron, NeuronTree* topNeuron);
    void init();
    void globalmatch();
    void output(QString fname);

    //for testing purpose
    //for testing data, the border tips with the same type should be matched ground truth
    void examineMatchingResult(double num[11], NeuronTree* nt_truth); //num[0]: TP; num[1]:FP; num[2]:FN; num[3]:total number of border tips should be matched
                                            //num[4]/num[5]: border tips number of nt0/1; num[6]: number of items in hash; num[7]: number of matched edges
                                            //num[8]: highest number of matched border tips by gmhash; num[9]: average distance to truth

private:
    //orientation should be 1/-1 for smaller/larger stack in direction
    void initNeuronAndCandidate(NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& cand, QList<XYZ>& candcoord);
    void initHash0();
    void affine_nt1();
};

#endif // NEURO_MATCH_GMHASH_H
