#ifndef NEURONSPLIT_H
#define NEURONSPLIT_H

#include "branchtree.h"


bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result, V3DLONG somaN);
bool sortSWC(NeuronTree& nt, V3DLONG somaN);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

vector<NeuronTree> splitNeuronTree(NeuronTree nt, const vector<V3DLONG>& somaIndexes, const LandmarkList &markers);

void splitNeuronTreeManu(V3DPluginCallback2 &callback);






#endif // NEURONSPLIT_H
