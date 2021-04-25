#ifndef SWCCOMPARE_H
#define SWCCOMPARE_H

#include "v3d_interface.h"

#include "branchtree.h"

ImageMarker getCenterSoma(unsigned char* pdata, V3DLONG* sz);

struct compareResult{
    int kBranchNumber, rBranchNumber, fBranchNumber, aBranchNumber, negativeBranchNumber;
    double rLength, fLength, alength, negativeLength;
};

bool compareSwc(NeuronTree swcAuto, NeuronTree swcManual, NeuronTree swcPruned, compareResult& cr);

#endif // SWCCOMPARE_H
