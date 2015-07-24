#ifndef LEARNING_H
#define LEARNING_H

#include <QtGui>
#include "v3d_interface.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <set>

class learning
{
public:
    learning();

private:
    int getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim);
    //bool getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float>& wave);
};

#endif // LEARNING_H
