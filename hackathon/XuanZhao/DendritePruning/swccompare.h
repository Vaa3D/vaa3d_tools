#ifndef SWCCOMPARE_H
#define SWCCOMPARE_H

#include "v3d_interface.h"

#include "branchtree.h"

ImageMarker getCenterSoma(unsigned char* pdata, V3DLONG* sz);

struct compareResult{
    int kBranchNumber, rBranchNumber, fBranchNumber, aBranchNumber, negativeBranchNumber;
    double rLength, fLength, alength, negativeLength;

    int branchAutoSN;
    double branchAutoSL;

    int branchOptimalSN;
    double branchOptimalSL;
    bool otherSoma;
};

bool compareSwc(NeuronTree swcAuto, NeuronTree swcManual, NeuronTree swcPruned, compareResult& cr, float dTh, QString optimalPrunedTreePath);
bool shiftSwc(NeuronTree& nt, unsigned char* pdata, long long *sz);
bool getBranchNumber(vector<NeuronTree> trees, ofstream &csvFile);
bool compareSwc2(NeuronTree swcAuto, NeuronTree swcManual, ofstream& csvFile);

#endif // SWCCOMPARE_H
