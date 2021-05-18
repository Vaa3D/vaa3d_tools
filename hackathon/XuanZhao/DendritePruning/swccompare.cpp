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

bool compareSwc(NeuronTree swcAuto, NeuronTree swcManual, NeuronTree swcPruned, compareResult &cr, float dTh, QString optimalPrunedTreePath){
    cr.aBranchNumber = cr.fBranchNumber = cr.kBranchNumber = cr.rBranchNumber = cr.negativeBranchNumber = 0;
    cr.alength = cr.fLength = cr.rLength = cr.negativeLength = 0;
    cr.branchAutoSL = cr.branchOptimalSL = 0;
    cr.branchAutoSN = cr.branchOptimalSN = 0;
    cr.otherSoma = false;

    BranchTree btManual = BranchTree();
    btManual.initialize(swcManual);

    BranchTree btPruned = BranchTree();
    btPruned.initialize(swcPruned);
    if(btPruned.branches.size() >= 1000)
        return false;

    BranchTree btAuto = BranchTree();
    btAuto.initialize(swcAuto);

    int size = btAuto.branches.size();
    vector<int> groundTruth = vector<int>(size,0);
    vector<bool> prunedTruth = vector<bool>(size,false);

    vector<int> parentManualBranchIndex = vector<int>(size,-1);

    vector<int> level0Index;
    btAuto.getLevelIndex(level0Index,0);
    queue<int> branchQ;
    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        branchQ.push(branchIndex);
    }

    NeuronTree& nt = btAuto.nt;
    V3DLONG rootIndex = btAuto.somaIndex;
    double rootR = nt.listNeuron[rootIndex].radius;
    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        double d = btAuto.branches[branchIndex].distToNeuronTree(&swcManual);

//        int flag = btAuto.branches[branchIndex].distToNeuronTree2(&swcManual,dTh);
        int pmbi = parentManualBranchIndex[branchIndex];
        int flag = btAuto.branches[branchIndex].distToNeuronTree3(btManual,&swcManual,dTh,pmbi);
//        qDebug()<<d;

        V3DLONG headIndex = btAuto.branches[branchIndex].headPointIndex;
        V3DLONG tailIndex = btAuto.branches[branchIndex].endPointIndex;

        bool isCenterRegin = (zx_dist(nt.listNeuron[rootIndex],nt.listNeuron[headIndex])<rootR*5
                              && btAuto.branches[branchIndex].rLevel>1);
        if(isCenterRegin || flag>=0){
//            if(btAuto.branches[branchIndex].rLevel == 0 && d>3){
//                continue;
//            }
            if(!isCenterRegin && flag>0){
                groundTruth[branchIndex] = flag;
                continue;
            }
            groundTruth[branchIndex] = 1;
            for(int j=0; j<btAuto.branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = btAuto.branches[branchIndex].childrenIndex[j];
                parentManualBranchIndex[cIndex] = pmbi;
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

    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        branchQ.push(branchIndex);
    }


    double somaRTh = 20;
    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        if(btAuto.branches[branchIndex].rLevel == -1){
            continue;
        }
        vector<V3DLONG> pointsIndex;
        btAuto.branches[branchIndex].get_pointsIndex_of_branch(pointsIndex);
        bool meetOtherSoma = false;
        for(int i=0; i<pointsIndex.size(); ++i){
            V3DLONG curIndex = pointsIndex[i];
            if(nt.listNeuron[curIndex].r>somaRTh &&
                    zx_dist(nt.listNeuron[rootIndex],nt.listNeuron[curIndex])>rootR*2){
                meetOtherSoma = true;
                cr.otherSoma = true;
                break;
            }
        }

        if(meetOtherSoma){
            break;
        }else{
            for(int j=0; j<btAuto.branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = btAuto.branches[branchIndex].childrenIndex[j];
                if(btAuto.branches[cIndex].rLevel != -1){
                    branchQ.push(cIndex);
                }
            }
        }
    }


    for(int i=0; i<size; ++i){
        if(groundTruth[i]>1)
            continue;
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

    cr.branchAutoSN  = btAuto.branches.size();
    for(int i=0; i<size; ++i){
        cr.branchAutoSL += btAuto.branches[i].length;
        if(groundTruth[i] == 0){
            btAuto.branches[i].setBranchType(ToDeleteType);
        }else if(groundTruth[i]>1){
            btAuto.nt.listNeuron[groundTruth[i]].type = BreakType;
            btAuto.branches[i].setBranchType(ToDeleteType,true);
        }
    }
    btAuto.update();

    cr.branchOptimalSN = btAuto.branches.size();
    for(int i=0; i<size; ++i){
        cr.branchOptimalSL += btAuto.branches[i].length;
    }

    btAuto.savePrunedNeuronTree(optimalPrunedTreePath);

    return true;
}

bool shiftSwc(NeuronTree &nt, unsigned char *pdata, V3DLONG* sz){
    int step = 3;
    double sumMax = 0;
    int size = nt.listNeuron.size();
    int iMax = 0, jMax = 0, kMax = 0;
    for(int i=-step; i<=step; ++i){
        for(int j=-step; j<=step; ++j){
            for(int k=-step; k<=step; ++k){
                int count = 0;
                int c = 0;
                double sum = 0;
                for(int s=0; s<nt.listNeuron.size(); ++s){
                    V3DLONG x = nt.listNeuron[s].x + i + 0.5;
                    V3DLONG y = nt.listNeuron[s].y + j + 0.5;
                    V3DLONG z = nt.listNeuron[s].z + k + 0.5;
                    if(x>=0 && x<sz[0]
                            && y>=0 && y<sz[1]
                            && z>=0 && z<sz[2]){
                        count++;
                        sum += pdata[z*sz[0]*sz[1]+y*sz[0]+x];
                    }
                }
                sum /= count;
                if(sum>sumMax){
                    sumMax = sum;
                    iMax = i;
                    jMax = j;
                    kMax = k;
                }
            }
        }
    }

    qDebug()<<"iMax,jMax,kMax: "<<iMax<<" "<<jMax<<" "<<kMax;
    for(int i=0; i<nt.listNeuron.size(); ++i){
        nt.listNeuron[i].x += iMax;
        nt.listNeuron[i].y += jMax;
        nt.listNeuron[i].z += kMax;
    }
    return true;
}

bool getBranchNumber(vector<NeuronTree> trees, ofstream& csvFile){
    BranchTree bt = BranchTree();
    for(int i=0; i<trees.size(); ++i){
        bt.initialize(trees[i]);
        csvFile<<','<<bt.branches.size();
    }
    csvFile<<endl;
    return true;
}

bool compareSwc2(NeuronTree swcAuto, NeuronTree swcManual, ofstream& csvFile){
    BranchTree btAuto = BranchTree();
    btAuto.initialize(swcAuto);

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
        csvFile<<d<<endl;
        for(int j=0; j<btAuto.branches[branchIndex].childrenIndex.size(); ++j){
            int cIndex = btAuto.branches[branchIndex].childrenIndex[j];
            branchQ.push(cIndex);
        }
    }

    return true;
}







