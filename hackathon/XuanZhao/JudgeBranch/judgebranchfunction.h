#ifndef JUDGEBRANCHFUNCTION_H
#define JUDGEBRANCHFUNCTION_H

#include <v3d_interface.h>

#include "randomforest.h"
#include "branchtree.h"

RandomForest* train(RandomForest* rf, V3DPluginCallback2* callback);

void judgeBranch(RandomForest* rf, V3DPluginCallback2 *callback);

void splitBranch(Branch* b, unsigned char* data1d, V3DLONG* sz, NeuronTree& nt, RandomForest* rf);


#endif // JUDGEBRANCHFUNCTION_H
