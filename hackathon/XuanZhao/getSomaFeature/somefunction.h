#ifndef SOMEFUNCTION_H
#define SOMEFUNCTION_H

#include "v3d_interface.h"
#include <vector>

using namespace std;

struct somaFeature{
    QString name;
    int anum,bnum,cnum;
    double admean,bdmean,cdmean;
    double bIntensityMean, bIntensityStd;
    double th;
    double somaIntensityMean;
    somaFeature(){
        anum = bnum = cnum = 0;
        admean = bdmean = cdmean = 0;
        bIntensityMean = bIntensityStd = 0;
        th =0;
        somaIntensityMean = 0;
    }
};

vector<somaFeature> getBrainSomasFeature(QString brainPath, V3DPluginCallback2& callback);

#endif // SOMEFUNCTION_H
