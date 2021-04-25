#include "swccompare.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"

#include <set>

ImageMarker getCenterSoma(unsigned char *pdata, long long *sz){
    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>(pdata,sz);
    V3DLONG possPoint = xyz2pos(sz[0]/2,sz[1]/2,sz[2]/2,sz[0],sz[0]*sz[1]);
    vector<float> massCenter = fun_obj.mean_shift_center_mass(possPoint,25);
    ImageMarker soma = ImageMarker(massCenter[0]+1,massCenter[1]+1,massCenter[2]+1);
    soma.color = XYZW(255,0,0,0);
    return soma;
}

bool compareSwc(NeuronTree swcAuto, NeuronTree swcManual, NeuronTree swcPruned, compareResult &cr){
    BranchTree btAuto = BranchTree();
    btAuto.initialize(swcAuto);

    int size = btAuto.branches.size();
    vector<bool> groundTruth = vector<bool>(size,false);
    vector<bool> prunedTruth = vector<bool>(size,false);

    vector<int> level0Index;
    btAuto.getLevelIndex(level0Index,0);
    queue<int> branchQ;
    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        branchQ.push(branchIndex);
    }

    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        double d = btAuto.branches[branchIndex].distToNeuronTree(&swcManual);
        if(d<2){
            groundTruth[branchIndex] = true;
            for(int j=0; j<btAuto.branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = btAuto.branches[branchIndex].childrenIndex[j];
                branchQ.push(cIndex);
            }
        }
    }

    set<V3DLONG> nSet;
    for(int i=0; i<swcPruned.listNeuron.size(); ++i){
        nSet.insert(swcPruned.listNeuron[i].n);
    }

    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        branchQ.push(branchIndex);
    }

    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        V3DLONG endIndex = btAuto.branches[branchIndex].endPointIndex;
        if(nSet.find(swcAuto.listNeuron[endIndex].n) != nSet.end()){
            prunedTruth[branchIndex] = true;
            for(int j=0; j<btAuto.branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = btAuto.branches[branchIndex].childrenIndex[j];
                branchQ.push(cIndex);
            }
        }
    }

    cr.aBranchNumber = cr.fBranchNumber = cr.kBranchNumber = cr.rBranchNumber = cr.negativeBranchNumber = 0;
    cr.alength = cr.fLength = cr.rLength = cr.negativeLength = 0;
    for(int i=0; i<size; ++i){
        if(!prunedTruth[i]){
            cr.aBranchNumber++;
            cr.alength += btAuto.branches[i].length;
            if(groundTruth[i]){
                cr.fBranchNumber++;
                cr.fLength += btAuto.branches[i].length;
            }else{
                cr.rBranchNumber++;
                cr.rLength += btAuto.branches[i].length;
            }
        }else{
            if(!groundTruth[i]){
                cr.negativeBranchNumber++;
                cr.negativeLength += btAuto.branches[i].length;
            }
        }
    }

    return true;
}

