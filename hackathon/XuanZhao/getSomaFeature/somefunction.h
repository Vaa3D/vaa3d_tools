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

    double bIntensityPartialStd;
    double thPercent;
    int intensityPercent1,intensityPercent5;

    int multiType;
    int isLowContrast;
    int isLayered;

    somaFeature(){
        anum = bnum = cnum = 0;
        admean = bdmean = cdmean = 0;
        bIntensityMean = bIntensityStd = 0;
        thPercent = 0;
        intensityPercent1 = intensityPercent5 = 0;
        bIntensityPartialStd = 0;
        th =0;
        somaIntensityMean = 0;
        multiType = isLowContrast = isLayered = 0;
    }
};

double getZSliceBackMean(unsigned char* pdata, V3DLONG* sz, int z);

vector<somaFeature> getBrainSomasFeature(QString brainPath, V3DPluginCallback2& callback);

vector<somaFeature> getBrainSomasFeature2(QString brainPath, V3DPluginCallback2& callback);

#endif // SOMEFUNCTION_H
